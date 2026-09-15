#pragma once

#include "CoreMinimal.h"
#include "BackroomsChunkCoord.h"
#include "BackroomsChunkCell.h"
#include "BackroomsRoomVariant.h"

// ---- Слой Generation Data (отдельно от Actor) ----
// Чистые данные и вычисления плотностной генерации чанка, не зависящие от акторной
// части (меш, пропсы, свет, события). Может существовать и пересчитываться без спавна
// акторов, что позволяет кэшировать/тестировать результаты детерминированно.
//
// Содержимое полностью определяется (WorldSeed-derived) Seed + FChunkCoord + параметрами
// профиля уровня, поэтому при одинаковых входных данных даёт одинаковый результат.

class FChunkGenerationData
{
public:
	// Планировка уровня: как раскладываются стены. Лабиринт уместен не везде —
	// открытый океан (L7) собирается как зал с редкими колоннами, а не как
	// бесконечный одинаковый лабиринт.
	enum class EChunkLayoutPattern : uint8
	{
		GridRooms = 0, // комнаты разного размера (неравномерная сетка)
		Maze      = 1, // плотный лабиринт из мелких комнат/коридоров
		OpenHall  = 2, // большие открытые залы, редкие колонны
		Cave      = 3  // органичные пещерные камеры
	};

	// Описание связной комнаты (пола) — результат ComputeRoomInfos.
	struct FRoomInfo
	{
		int32 CellCount = 0;
		FIntPoint MinCell = FIntPoint::ZeroValue;
		FIntPoint MaxCell = FIntPoint::ZeroValue;
	};

	// Параметры генерации, передаваемые из профиля/актора.
	struct FParams
	{
		int32 CellCount = 8;              // ChunkSizeCells (сторона квадратной сетки)
		float WallThreshold = 0.32f;
		float ScatterThreshold = 0.5f;
		float DoorThreshold = 0.15f;
		float LivingRoomThreshold = 0.10f;
		int32 MinRoomCells = 4;
		int32 MaxRoomCells = 12;
		EChunkLayoutPattern Pattern = EChunkLayoutPattern::GridRooms;
		int32 LevelIndex = 0;             // Локация: выбирает таблицу видов комнат.

		// Частоты плотностного шума. Раньше были захардкожены (0.09 / 0.045 /
		// 0.012) — один «стиль шума» на все уровни. Теперь выводятся из размера
		// комнат профиля (см. ABackroomsChunkActor::InitGenData), поэтому
		// мелкие комнаты дают более плотный шум, крупные — более плавный.
		float BaseNoiseFreq = 0.09f;
		float MacroNoiseFreq = 0.012f;
	};

	FChunkGenerationData() = default;
	FChunkGenerationData(const FChunkCoord& InCoord, int32 InSeed, const FParams& InParams)
		: Coord(InCoord), Seed(InSeed), Params(InParams) {}

	// Входные — идентифицируют поколение.
	FChunkCoord Coord;
	int32 Seed = 0;

	// Глобальный seed мира. Поля стен/плотности/дверей/вида комнаты должны
	// зависеть ТОЛЬКО от мировых координат и этого seed, иначе соседний чанк
	// предскажет границу не так, как построил её сам. Seed чанка остаётся
	// только для локального декора (пропсы/свет).
	int32 WorldSeed = 0;
	bool bUseWorldSeed = false;
	int32 LayoutSeed() const { return bUseWorldSeed ? WorldSeed : Seed; }

	FParams Params;

	// Выходные данные (заполняются Generate).
	TArray<FChunkCell> Cells;
	TArray<FRoomInfo> LastRooms;

	// Полный проход: плотность + маски + комнаты.
	void Generate();

	// Мировая координата клетки (для запросов за пределами чанка).
	int32 WorldX(int32 LX) const { return Coord.X * Params.CellCount + LX; }
	int32 WorldY(int32 LY) const { return Coord.Y * Params.CellCount + LY; }

	int32 MaskIndex(int32 LX, int32 LY) const { return LY * Params.CellCount + LX; }

	// Плотность клетки в мировых координатах — непрерывная функция от шума,
	// согласована между соседними чанками (нет швов).
	float CellDensity(int32 WorldCellX, int32 WorldCellY) const;
	bool CellIsWall(int32 WorldCellX, int32 WorldCellY) const;
	bool CellIsScatter(int32 WorldCellX, int32 WorldCellY) const;
	bool CellIsLivingRoom(int32 WorldCellX, int32 WorldCellY) const;

	int32 RoomIndexAt(int32 LX, int32 LY) const;

	// Вид комнаты (стиль/свет/высота/раскладка) для клетки чанка и мировых
	// координат. Чистая функция от координаты + Seed + LevelIndex, поэтому
	// одинакова на реплеях и бесшовна между чанками.
	ERoomVariant CellVariant(int32 LX, int32 LY) const;
	ERoomVariant VariantAtWorld(int32 WorldCellX, int32 WorldCellY) const;
	const FRoomVariantDef& CellVariantDef(int32 LX, int32 LY) const;

	// Оценка размера комнаты (в клетках) из правил профиля. Используется для
	// size-fit подбора вида комнаты ДО того, как реальные комнаты посчитаны.
	int32 EstimateRoomCells() const;

	// Подбор вида комнаты для конкретной мировой клетки под заданный размер
	// комнаты (для пост-подгонки по реальному FRoomInfo::CellCount).
	ERoomVariant PickVariantForCell(int32 WorldCellX, int32 WorldCellY, int32 RoomCells) const;

	// Локальный прямоугольник комнаты в клетках — [Min, Max+1) в клетках чанка.
	FBox2D RoomBoundsLocal(int32 RoomIdx) const;

	// Доступ к клетке по локальным координатам чанка.
	FChunkCell& CellAt(int32 LX, int32 LY) { return Cells[MaskIndex(LX, LY)]; }
	const FChunkCell& CellAt(int32 LX, int32 LY) const { return Cells[MaskIndex(LX, LY)]; }

	// Определение позиций дверей на границах чанка (для кросс-чанковых сокетов).
	void GetBorderDoors(TArray<FIntPoint>& OutDoorPositions, TArray<int32>& OutDirections) const;

private:
	void GenerateDensityField();
	void GenerateDoorMask();
	void ComputeRoomInfos();
	// Пост-подгонка вида комнаты под РЕАЛЬНЫЙ размер связной комнаты
	// (FRoomInfo::CellCount), а не под фиктивные ZoneSize^2. Вызывается после
	// ComputeRoomInfos.
	void RefineVariants();
	bool IsArrivalCell(int32 WorldCellX, int32 WorldCellY) const;
	bool IsBackboneCell(int32 WorldCellX, int32 WorldCellY) const;

	// Кеш VariantAtWorld на время генерации чанка: чистый, но дорогой Перлин
	// раньше пересчитывался по 3 раза на клетку (стены/плотность/пропсы).
	mutable TMap<uint32, uint8> VariantCache;
};

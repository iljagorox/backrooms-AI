#include "BackroomsGenerationData.h"
#include "BackroomsNoise.h"
#include "BackroomsGenerationVerifier.h"
#include "Logging/LogVerbosity.h"

namespace
{
	// Runs the verifier for one pipeline stage right after it produced data.
	// Mode comes from BR.Gen.Verify (Off/Log/Strict); the checks themselves are
	// pure and cheap (O(cells)), so they can stay enabled by default.
	void RunStageVerification(const FChunkGenerationData& Data, EGenerationStage Stage)
	{
		const EGenerationVerifyMode Mode = FChunkGenerationVerifier::GetMode();
		if (Mode == EGenerationVerifyMode::Off)
		{
			return;
		}

		FStageVerifyResult R;
		FChunkGenerationVerifier::VerifyStage(Stage, Data, R);
		const TCHAR* StageName = FChunkGenerationVerifier::StageName(Stage);

		if (R.bPassed)
		{
			UE_LOG(LogTemp, Verbose, TEXT("BR Gen %s (%d,%d): %s"),
				StageName, Data.Coord.X, Data.Coord.Y, *FChunkGenerationVerifier::FormatStage(R));
			return;
		}

		if (Mode == EGenerationVerifyMode::Strict)
		{
			for (const FString& F : R.Failures)
			{
				UE_LOG(LogTemp, Error, TEXT("BR Gen %s (%d,%d): %s"),
					StageName, Data.Coord.X, Data.Coord.Y, *F);
			}
		}
		else
		{
			for (const FString& F : R.Failures)
			{
				UE_LOG(LogTemp, Warning, TEXT("BR Gen %s (%d,%d): %s"),
					StageName, Data.Coord.X, Data.Coord.Y, *F);
			}
		}
		for (const FString& W : R.Warnings)
		{
			UE_LOG(LogTemp, Verbose, TEXT("BR Gen %s (%d,%d) warn: %s"),
				StageName, Data.Coord.X, Data.Coord.Y, *W);
		}
	}

	FORCEINLINE int32 PosMod(int32 V, int32 M)
	{
		const int32 R = V % M;
		return (R < 0) ? R + M : R;
	}

	// Линия стены с УПРАВЛЯЕМЫМ периодом: локальный минимум гладкого 1D-среза
	// Перлина. Частота задаётся желаемым средним шагом (в клетках), поэтому
	// размер комнаты реально зависит от MinRoomCells/MaxRoomCells профиля, а не
	// от жёсткой константы 3. Функция чистая и бесшовная между чанками.
	FORCEINLINE bool IsWallLinePitched(int32 V, uint32 Seed, float PitchCells)
	{
		const int32 Pitch = FMath::Clamp(FMath::RoundToInt(PitchCells), 3, 8);
    const int32 Phase = (int32)(Seed % (uint32)Pitch);
    return ((V - Phase) % Pitch) == 0;
	}

	// Позиционный хэш клетки -> [0,1). Чистый и бесшовный: одинаков для одной
	// и той же мировой клетки независимо от того, какой чанк её запрашивает.
	FORCEINLINE float CellHash01(int32 X, int32 Y, uint32 Seed)
	{
		uint32 H = (uint32)X * 374761393u ^ (uint32)Y * 668265263u ^ Seed * 2246822519u;
		H = (H ^ (H >> 15)) * 2654435761u;
		H ^= H >> 13;
		return (float)(H & 0xFFFFFFu) / (float)0x1000000u;
	}
}

void FChunkGenerationData::Generate()
{
	// Каждый этап проверяется сразу после выполнения: ошибка привязана к шагу,
	// который её породил, а не растворяется в итоговой геометрии.
	GenerateDensityField();
	RunStageVerification(*this, EGenerationStage::Density);
	RunStageVerification(*this, EGenerationStage::Walls);
	GenerateDoorMask();
	RunStageVerification(*this, EGenerationStage::Doors);
	ComputeRoomInfos();
	RefineVariants();
	RunStageVerification(*this, EGenerationStage::Rooms);
}

float FChunkGenerationData::CellDensity(int32 WorldCellX, int32 WorldCellY) const
{
	// Поля мира считаются от ГЛОБАЛЬНОГО seed (LayoutSeed), а не от seed чанка:
	// только тогда соседний чанк предсказывает границу ровно такой, какой она
	// построена здесь. Seed чанка оставлен локальному декору.
	const int32 LaySeed = LayoutSeed();
	// Этап 1: широкая «зона» на десятки клеток. Она остаётся непрерывной между
	// чанками и даёт районы с разной плотностью, а не одинаковый шум везде.
	const float MacroFreq = FMath::Max(0.002f, Params.MacroNoiseFreq);
	const float Macro = BackroomsNoise::Perlin2D(
		(float)WorldCellX * MacroFreq + 91.7f,
		(float)WorldCellY * MacroFreq + 41.3f, LaySeed ^ 0x4C304D30);

	// Этап 2: локальный шум формирует стены и проходы внутри района. Частота
	// берётся из профиля (BaseNoiseFreq), поэтому у каждого уровня свой «стиль»
	// плотности, а не один общий на все 10 локаций.
	const float BaseFreq = FMath::Max(0.01f, Params.BaseNoiseFreq);
	const float N = BackroomsNoise::Perlin2D(
		(float)WorldCellX * BaseFreq + 0.5f,
		(float)WorldCellY * BaseFreq + 0.5f, LaySeed);
	const float N2 = BackroomsNoise::Perlin2D(
		(float)WorldCellX * BaseFreq * 0.5f + 333.3f,
		(float)WorldCellY * BaseFreq * 0.5f + 777.7f, LaySeed ^ 0x0A1B2C3D);
	const float V = N * 0.7f + N2 * 0.55f + Macro * 0.18f;
	return FMath::Clamp(V * 0.5f + 0.5f, 0.0f, 1.0f);
}

bool FChunkGenerationData::IsArrivalCell(int32 WorldCellX, int32 WorldCellY) const
{
	// Этап 0: безопасная зона приземления точно под люком города. Она не
	// зависит от загрузки соседних чанков и гарантирует пол под игроком.
	return WorldCellX >= 2 && WorldCellX <= 5 && WorldCellY >= 2 && WorldCellY <= 5;
}

bool FChunkGenerationData::IsBackboneCell(int32 WorldCellX, int32 WorldCellY) const
{
	// У каждого чанка есть двухклеточный «скелет» проходов через центр.
	// Крайние клетки скелета совпадают у соседних чанков, поэтому из любой
	// точки можно продолжать движение в четыре стороны бесконечного мира.
	const int32 Count = FMath::Max(4, Params.CellCount);
	const int32 LocalX = ((WorldCellX % Count) + Count) % Count;
	const int32 LocalY = ((WorldCellY % Count) + Count) % Count;
	const int32 MidLo = Count / 2 - 1;
	const int32 MidHi = Count / 2;
	return (LocalX >= MidLo && LocalX <= MidHi) || (LocalY >= MidLo && LocalY <= MidHi);
}

bool FChunkGenerationData::CellIsWall(int32 WorldCellX, int32 WorldCellY) const
{
	if (IsArrivalCell(WorldCellX, WorldCellY))
	{
		return false;
	}
	if (IsBackboneCell(WorldCellX, WorldCellY))
	{
		return false;
	}

	// Планировка выбирается профилем уровня. Общий приём для всех схем: пороги
	// плотности добавляют «органику» (колонны внутри комнат и сквозные разрывы в
	// стенах), чтобы раскладка не выглядела идеально механической.
	//
	// ЧЕСТНЫЙ КОНТРАКТ WallThreshold: выше значение => больше стен ВО ВСЕХ схемах,
	// ниже => больше проходов. Отклонение T от дефолта 0.32 сдвигает пороги
	// монотонно, а при 0.32 формулы дают ровно прежнюю геометрию (можно менять
	// порог в профиле без перетюнинга уровня). Сложность (BackroomsDifficulty)
	// умножает WallThreshold, поэтому обещание «на харде больше стен» работает и
	// для GridRooms/Maze (раньше порог трогал только органику).
	const float D = CellDensity(WorldCellX, WorldCellY);
	bool bWall = false;

	const float W = FMath::Clamp(Params.WallThreshold, 0.02f, 0.9f);
	const float T = W - 0.32f;
	// GridRooms/Maze: «органика» — вот плотность стен/проходов. Выше W: порог
	// добавления столбов падает (стен больше), порог выреза разрывов падает
	// (проёмов меньше).
	const float OrganicAddA = FMath::Clamp(0.78f - T * 0.45f, 0.40f, 0.98f);   // 0.78 при 0.32
	const float OrganicAddB = FMath::Clamp(0.88f - T * 0.30f, 0.55f, 0.98f);   // 0.88 при 0.32
	const float OrganicCut = FMath::Clamp(0.10f - T * 0.45f, 0.0f, 0.12f);     // 0.10 при 0.32
	// OpenHall/Cave: прямой порог плотности — выше W => порог ниже => стен больше.
	const float OpenHallAdd = FMath::Clamp(0.70f - T * 1.1f, 0.20f, 0.95f);     // 0.70 при 0.32
	const float CaveAdd = FMath::Clamp(0.58f - T * 0.9f, 0.15f, 0.95f);         // 0.58 при 0.32

	const int32 AvgRoom = EstimateRoomCells();
	float Pitch = FMath::Clamp(FMath::Sqrt((float)AvgRoom) + 1.0f, 2.0f, 8.0f);
	const int32 LaySeed = LayoutSeed();
	{
		const float MacroFreq = FMath::Max(0.002f, Params.MacroNoiseFreq);
		const float Macro = BackroomsNoise::Perlin2D(
			(float)WorldCellX * MacroFreq * 0.7f + 11.1f,
			(float)WorldCellY * MacroFreq * 0.7f + 73.3f, LaySeed ^ 0x6B4A8F71u);
		Pitch *= FMath::Clamp(1.0f + Macro * 0.28f, 0.82f, 1.35f);
		Pitch = FMath::Clamp(Pitch, 2.0f, 8.0f);
	}

	// Вид комнаты этой клетки управляет не только «характером» поверх схемы,
	// но и самим шагом сетки: тесные комнаты (WallBias > 1) загущают
	// перегородки, широкие залы (WallBias < 1) разрежают. Вид — чистая функция
	// мировых координат, поэтому шаг остаётся бесшовным на границе чанков.
	const FRoomVariantDef& Variant = FRoomVariantSystem::GetDef(VariantAtWorld(WorldCellX, WorldCellY));
	Pitch *= FMath::Clamp(FMath::Sqrt(Variant.WallBias), 0.8f, 1.25f);
	Pitch = FMath::Clamp(Pitch, 2.0f, 8.0f);
	const uint32 Gx = (uint32)LaySeed ^ 0x1F123BB5u;
	const uint32 Gy = (uint32)LaySeed ^ 0x7E5A91C3u;

	switch (Params.Pattern)
	{
	case EChunkLayoutPattern::Maze:
	{
		// Плотный лабиринт: стены по сетке с мелким шагом, проходы прорубает
		// GenerateDoorMask. Шаг привязан к профилю (Min/MaxRoomCells), а не
		// жёстко к 3. Уместен в трубах/энергетике/тьме.
		const int32 MazePitch = FMath::Clamp(FMath::RoundToInt(Pitch) - 1, 2, 4);
		const int32 LX = PosMod(WorldCellX + (int32)(Gx % 31u), MazePitch);
		const int32 LY = PosMod(WorldCellY + (int32)(Gy % 31u), MazePitch);
		bWall = (LX == 0) || (LY == 0);
		if (!bWall && D > OrganicAddB) { bWall = true; }
		else if (bWall && D < OrganicCut) { bWall = false; }
		break;
	}
	case EChunkLayoutPattern::OpenHall:
	{
		// Открытые залы: почти всё — пол, стенами становятся только редкие
		// «колонны» по плотности. Так собирается L7 (океан), где лабиринт лишний.
		bWall = D > OpenHallAdd;
		break;
	}
	case EChunkLayoutPattern::Cave:
	{
		// Органика пещер: только блоб-стены, без прямых линий.
		bWall = D > CaveAdd;
		break;
	}
	case EChunkLayoutPattern::GridRooms:
	default:
	{
		// Классический Бэкрумс: сетка комнат с ПЛАВАЮЩИМ шагом (локальные
		// минимумы гладкого поля с частотой из профиля), поэтому соседние
		// комнаты действительно разного размера, а не одинаковые 2x2.
		const bool bWallX = IsWallLinePitched(WorldCellX + (int32)(Gx % 97u), Gx, Pitch);
		const bool bWallY = IsWallLinePitched(WorldCellY + (int32)(Gy % 97u), Gy, Pitch);
		bWall = bWallX || bWallY;
		// Лёгкая органика поверх сетки: редкие колонны в комнате и разрывы в
		// стенах, чтобы раскладка не выглядела идеально механической.
		if (!bWall && D > OrganicAddA) { bWall = true; }
		else if (bWall && D < OrganicCut) { bWall = false; }
		break;
	}
	}

	// Вид комнаты этой клетки добавляет характер поверх базовой схемы: тесные
	// комнаты (WallBias > 1) заращивают открытые клетки, широкие залы
	// (WallBias < 1) прорубают лишние проходы. Это делает локации разными даже
	// при одной схеме раскладки, оставаясь бесшовным (чистая функция координаты).
	if (Variant.WallBias < 0.999f)
	{
		if (bWall && CellHash01(WorldCellX, WorldCellY, (uint32)LaySeed ^ 0xB1A5C3u) > Variant.WallBias)
		{
			bWall = false;
		}
	}
	else if (Variant.WallBias > 1.001f)
	{
		if (!bWall && CellHash01(WorldCellX, WorldCellY, (uint32)LaySeed ^ 0x7F1E2Du) < (Variant.WallBias - 1.0f))
		{
			bWall = true;
		}
	}

	return bWall;
}

int32 FChunkGenerationData::EstimateRoomCells() const
{
	// Среднее из правил профиля. Это лишь оценка ДО того, как комнаты реально
	// посчитаны; настоящий размер подставляется в RefineVariants().
	return FMath::Clamp((Params.MinRoomCells + Params.MaxRoomCells) / 2, 1, 64);
}

ERoomVariant FChunkGenerationData::PickVariantForCell(int32 WorldCellX, int32 WorldCellY, int32 RoomCells) const
{
	// Perlin-поле «района»: низкочастотное, поэтому соседние клетки почти всегда
	// попадают в один и тот же интервал весов — вид комнаты читается как район,
	// а не как случайный набор квадратов. Поле непрерывно и не зависит от чанка,
	// значит раскладка бесшовна: обе стороны границы видят одно значение.
	const float RegionNoise = BackroomsNoise::Perlin2D(
		(float)WorldCellX * 0.03f + 17.3f,
		(float)WorldCellY * 0.03f + 53.9f,
		LayoutSeed() ^ 0x52A9E1C7u ^ ((uint32)Params.LevelIndex * 0x9E3779B1u));
	const float Region01 = RegionNoise * 0.5f + 0.5f;
	return FRoomVariantSystem::PickVariantByNoise(Params.LevelIndex, Region01, RoomCells);
}

ERoomVariant FChunkGenerationData::VariantAtWorld(int32 WorldCellX, int32 WorldCellY) const
{
	// Кеш: раньше Перлин пересчитывался по 3 раза на клетку (стены/плотность/
	// пропсы). Теперь один раз на клетку за генерацию чанка.
	const uint32 Key = (uint32)WorldCellX * 73856093u
		^ (uint32)WorldCellY * 19349663u
		^ (uint32)Params.LevelIndex * 2246822519u
		^ (uint32)LayoutSeed() * 2654435761u;
	if (const uint8* Found = VariantCache.Find(Key))
	{
		return (ERoomVariant)*Found;
	}

	const ERoomVariant Result = PickVariantForCell(WorldCellX, WorldCellY, EstimateRoomCells());
	VariantCache.Add(Key, (uint8)Result);
	return Result;
}

ERoomVariant FChunkGenerationData::CellVariant(int32 LX, int32 LY) const
{
	if (LX < 0 || LY < 0 || LX >= Params.CellCount || LY >= Params.CellCount
		|| Cells.Num() != Params.CellCount * Params.CellCount)
	{
		return ERoomVariant::Neutral;
	}
	return (ERoomVariant)Cells[MaskIndex(LX, LY)].RoomVariant;
}

const FRoomVariantDef& FChunkGenerationData::CellVariantDef(int32 LX, int32 LY) const
{
	return FRoomVariantSystem::GetDef(CellVariant(LX, LY));
}

bool FChunkGenerationData::CellIsScatter(int32 WorldCellX, int32 WorldCellY) const
{
	// Пропсы кладём только на открытый пол, плотность которого НИЖЕ порога
	// профиля (ScatterThreshold). Раньше порог не читался вообще (любой пол),
	// поэтому густота реквизита в профиле ни на что не влияла. Проходы у дверей
	// дополнительно защищает ReserveDoorCorridor.
	if (CellIsWall(WorldCellX, WorldCellY))
	{
		return false;
	}
	const float Limit = FMath::Clamp(Params.ScatterThreshold, 0.05f, 0.95f);
	return CellDensity(WorldCellX, WorldCellY) <= Limit;
}

bool FChunkGenerationData::CellIsLivingRoom(int32 WorldCellX, int32 WorldCellY) const
{
	const bool bNextToLobbyX = (WorldCellX >= 0 && WorldCellX <= 1) && (WorldCellY >= 10 && WorldCellY <= 11);
	const bool bNextToLobbyY = (WorldCellY >= 0 && WorldCellY <= 1) && (WorldCellX >= 10 && WorldCellX <= 11);
	if (bNextToLobbyX || bNextToLobbyY)
	{
		return true;
	}

	// Порог «живых/аномальных» комнат теперь берётся из профиля
	// (LivingRoomThreshold). Раньше были жёсткие 0.35/0.3/0.75, и настройка
	// профиля не влияла на долю жилых комнат.
	const float Living = FMath::Clamp(Params.LivingRoomThreshold, 0.0f, 1.0f);
	if (Living <= 0.0f)
	{
		return false;
	}

	const int32 BlockStep = 4;
	const int32 BX = WorldCellX >= 0 ? WorldCellX / BlockStep : (WorldCellX - BlockStep + 1) / BlockStep;
	const int32 BY = WorldCellY >= 0 ? WorldCellY / BlockStep : (WorldCellY - BlockStep + 1) / BlockStep;

	// Блок активируется примерно при каждой третьей «живой» клетке внутри него.
	const float BlockActivate = FMath::Clamp(Living * 3.0f, 0.05f, 0.9f);
	{
		FRandomStream BlockRng(LayoutSeed() ^ (BX * 73856093) ^ (BY * 19349663) ^ 0x51AB);
		if (BlockRng.FRand() > BlockActivate)
		{
			return false;
		}
	}

	const int32 LocalX = ((WorldCellX % BlockStep) + BlockStep) % BlockStep;
	const int32 LocalY = ((WorldCellY % BlockStep) + BlockStep) % BlockStep;
	FRandomStream CellRng(LayoutSeed() ^ (WorldCellX * 2654435761u) ^ (WorldCellY * 40503u) ^ 0x9E37);
	const float CenterBias = FMath::Lerp(0.3f, 0.75f,
		(float)(LocalX + LocalY < BlockStep ? 1 : 0));
	const float PerCellChance = FMath::Clamp(Living * (1.0f + CenterBias * 2.0f), 0.0f, 1.0f);
	return CellRng.FRand() < PerCellChance;
}

void FChunkGenerationData::GenerateDensityField()
{
	const int32 Count = Params.CellCount;
	Cells.SetNum(Count * Count);
	for (int32 LY = 0; LY < Count; ++LY)
	{
		for (int32 LX = 0; LX < Count; ++LX)
		{
			const int32 WX = WorldX(LX);
			const int32 WY = WorldY(LY);
			FChunkCell& C = Cells[MaskIndex(LX, LY)];
			C.Density = CellDensity(WX, WY);
			C.bIsWall = CellIsWall(WX, WY);
			C.bIsScatter = CellIsScatter(WX, WY);
			C.bIsLivingRoom = CellIsLivingRoom(WX, WY);
			C.RoomVariant = (uint8)VariantAtWorld(WX, WY);
			C.CellType = C.bIsWall ? ECellType::Wall
				: C.bIsLivingRoom ? ECellType::LivingRoom
				: C.bIsScatter ? ECellType::Corridor
				: ECellType::Floor;
		}
	}

	// Второй проход: свободностоящие колонны внутри комнат. Ставим их на клетки,
	// у которых все 4 ОРТОГОНАЛЬНЫХ соседа — пол (диагонали не важны для
	// связности). Раньше требовались все 8 соседей, поэтому в комнатах 2x2
	// (и любой комнате без строго внутренней клетки) колонны не появлялись
	// вообще — мёртвый код. Теперь условие мягче и колонны возможны в комнатах
	// от 3x3 и в крупных Г-образных залах. Внутренние клетки не лежат на границе
	// чанка, поэтому раскладка остаётся бесшовной.
	for (int32 LY = 1; LY < Count - 1; ++LY)
	{
		for (int32 LX = 1; LX < Count - 1; ++LX)
		{
			const int32 WX = WorldX(LX);
			const int32 WY = WorldY(LY);
			if (IsArrivalCell(WX, WY))
			{
				continue;
			}
			FChunkCell& C = Cells[MaskIndex(LX, LY)];
			if (C.bIsWall)
			{
				continue;
			}
			const FRoomVariantDef& Variant = FRoomVariantSystem::GetDef((ERoomVariant)C.RoomVariant);
			if (Variant.PillarChance <= 0.0f)
			{
				continue;
			}
			bool bAllFloor4 = true;
			for (int32 Dir = 0; Dir < 4; ++Dir)
			{
				int32 DX = (Dir == 0) ? -1 : (Dir == 1) ? 1 : 0;
				int32 DY = (Dir == 2) ? -1 : (Dir == 3) ? 1 : 0;
				if (Cells[MaskIndex(LX + DX, LY + DY)].bIsWall)
				{
					bAllFloor4 = false;
					break;
				}
			}
			if (bAllFloor4 && CellHash01(WX, WY, (uint32)LayoutSeed() ^ 0x9E3779B1u) < Variant.PillarChance)
			{
				C.bIsWall = true;
				C.bIsScatter = false;
				C.bIsLivingRoom = false;
				C.CellType = ECellType::Wall;
			}
		}
	}
}

void FChunkGenerationData::GenerateDoorMask()
{
	const int32 Count = Params.CellCount;
	const int32 LaySeed = LayoutSeed();

	TArray<int32> Region;
	Region.SetNum(Count * Count);
	for (int32 i = 0; i < Region.Num(); ++i) { Region[i] = -1; }

	int32 NextRegion = 0;
	for (int32 LY = 0; LY < Count; ++LY)
	{
		for (int32 LX = 0; LX < Count; ++LX)
		{
			const int32 I = MaskIndex(LX, LY);
			if (Cells[I].bIsWall || Region[I] >= 0)
			{
				continue;
			}
			TArray<FIntPoint> Stack;
			Stack.Add(FIntPoint(LX, LY));
			Region[I] = NextRegion;
			while (Stack.Num() > 0)
			{
				const FIntPoint P = Stack.Pop();
				const int32 PIdx = MaskIndex(P.X, P.Y);
				const int32 DX4[4] = { 1, -1, 0, 0 };
				const int32 DY4[4] = { 0, 0, 1, -1 };
				for (int32 d = 0; d < 4; ++d)
				{
					const int32 NX = P.X + DX4[d];
					const int32 NY = P.Y + DY4[d];
					if (NX < 0 || NX >= Count || NY < 0 || NY >= Count)
					{
						continue;
					}
					const int32 NI = MaskIndex(NX, NY);
					if (Cells[NI].bIsWall || Region[NI] >= 0)
					{
						continue;
					}
					Region[NI] = NextRegion;
					Stack.Add(FIntPoint(NX, NY));
				}
			}
			++NextRegion;
		}
	}

	const int32 DX4[4] = { 1, -1, 0, 0 };
	const int32 DY4[4] = { 0, 0, 1, -1 };
	for (int32 LY = 0; LY < Count; ++LY)
	{
		for (int32 LX = 0; LX < Count; ++LX)
		{
			const int32 I = MaskIndex(LX, LY);
			if (!Cells[I].bIsWall)
			{
				continue;
			}
			if (LX == 0 || LX == Count - 1 || LY == 0 || LY == Count - 1)
			{
				continue;
			}
			int32 SeenA = -1;
			bool bSeparates = false;
			for (int32 d = 0; d < 4; ++d)
			{
				const int32 NX = LX + DX4[d];
				const int32 NY = LY + DY4[d];
				if (NX < 0 || NX >= Count || NY < 0 || NY >= Count)
				{
					continue;
				}
				const int32 NI = MaskIndex(NX, NY);
				if (Cells[NI].bIsWall || Region[NI] < 0)
				{
					continue;
				}
				if (SeenA < 0) { SeenA = Region[NI]; }
				else if (Region[NI] != SeenA) { bSeparates = true; break; }
			}

			const float N = BackroomsNoise::Perlin2D(
				(LX + Coord.X * Count) * 0.51f, (LY + Coord.Y * Count) * 0.47f,
				LaySeed ^ 0x5BF53457);
			const float N01 = N * 0.5f + 0.5f;

			// Порог «случайных» дверей масштабируем под схему раскладки: лабиринт
			// (Maze) держится только на дверях-прорубах и требует частых проходов,
			// открытый зал (OpenHall) почти не требует, GridRooms/Cave — базовый
			// порог профиля. Раньше порог был один на все паттерны, поэтому,
			// например, Maze при маленьком DoorThreshold заужался до изолированных
			// комнат без связи.
			float DoorLimit = FMath::Clamp(Params.DoorThreshold, 0.0f, 0.5f);
			switch (Params.Pattern)
			{
			case EChunkLayoutPattern::Maze:
				DoorLimit = FMath::Clamp(Params.DoorThreshold * 3.0f, 0.0f, 0.5f);
				break;
			case EChunkLayoutPattern::OpenHall:
				DoorLimit = FMath::Clamp(Params.DoorThreshold * 0.35f, 0.0f, 0.5f);
				break;
			case EChunkLayoutPattern::Cave:
				DoorLimit = FMath::Clamp(Params.DoorThreshold * 0.9f, 0.0f, 0.5f);
				break;
			default:
				break;
			}
			const bool bRandomDoor = N01 > (1.0f - DoorLimit);

			// Граница чанка: локальная проверка «separates» ненадёжна (у соседа
			// другая связность и, раньше, другой seed), поэтому дверь на краю
			// решает ЧИСТАЯ функция мировой координаты + LayoutSeed. Обе стороны
			// границы получают одинаковый ответ и не оставляют глухой стены.
			bool bBorderDoor = false;
			if (LX == 0 || LX == Count - 1 || LY == 0 || LY == Count - 1)
			{
				const int32 WX = WorldX(LX);
				const int32 WY = WorldY(LY);
				const float BN = BackroomsNoise::Perlin2D(WX * 0.213f + 5.1f, WY * 0.207f + 3.7f, LaySeed ^ 0x3C9A17D5u);
				bBorderDoor = (BN * 0.5f + 0.5f) > (1.0f - DoorLimit);
			}

			if (bSeparates || bRandomDoor || bBorderDoor)
			{
				Cells[I].bIsDoor = true;
				Cells[I].CellType = ECellType::Door;
			}
		}
	}

	{
		const int32 DX2[4] = { 1, -1, 0, 0 };
		const int32 DY2[4] = { 0, 0, 1, -1 };
		for (int32 LY = 0; LY < Count; ++LY)
		{
			for (int32 LX = 0; LX < Count; ++LX)
			{
				const int32 I = MaskIndex(LX, LY);
				if (!Cells[I].bIsWall || Cells[I].bIsDoor)
				{
					continue;
				}
				const bool bBorder = (LX == 0 || LX == Count - 1 || LY == 0 || LY == Count - 1);
				if (!bBorder)
				{
					continue;
				}
				const int32 WX = WorldX(LX);
				const int32 WY = WorldY(LY);
				for (int32 d = 0; d < 4; ++d)
				{
					const bool bExitX = (LX == 0 && DX2[d] < 0) || (LX == Count - 1 && DX2[d] > 0);
					const bool bExitY = (LY == 0 && DY2[d] < 0) || (LY == Count - 1 && DY2[d] > 0);
					if (!bExitX && !bExitY)
					{
						continue;
					}

					const bool bNeighborWall = CellIsWall(WX + DX2[d], WY + DY2[d]);
					bool bNeedDoor = false;
					if (!bNeighborWall)
					{
						bNeedDoor = true;
					}
					else
					{
						const float GX = WX + 0.5f * (1 + DX2[d]);
						const float GY = WY + 0.5f * (1 + DY2[d]);
						const float GN = BackroomsNoise::Perlin2D(GX * 0.51f, GY * 0.47f, LaySeed ^ 0x5BF53457);
						bNeedDoor = (GN * 0.5f + 0.5f) > 0.5f;
					}
					if (bNeedDoor)
					{
						Cells[I].bIsDoor = true;
						Cells[I].CellType = ECellType::Door;
						break;
					}
				}
			}
		}
	}
}

void FChunkGenerationData::ComputeRoomInfos()
{
	const int32 Count = Params.CellCount;
	LastRooms.Reset();

	TArray<int32> Stack;
	for (int32 LY = 0; LY < Count; ++LY)
	{
		for (int32 LX = 0; LX < Count; ++LX)
		{
			const int32 I = MaskIndex(LX, LY);
			const bool bOpen = !Cells[I].bIsWall || Cells[I].bIsDoor;
			if (!bOpen || Cells[I].RoomIndex != INDEX_NONE)
			{
				continue;
			}

			const int32 RoomIdx = LastRooms.Num();
			FRoomInfo R;
			R.MinCell = FIntPoint(LX, LY);
			R.MaxCell = FIntPoint(LX, LY);
			R.CellCount = 0;
			LastRooms.Add(R);
			Cells[I].RoomIndex = RoomIdx;

			Stack.Reset();
			Stack.Add(I);
			while (Stack.Num() > 0)
			{
				const int32 Cur = Stack.Pop(EAllowShrinking::No);
				const int32 CX = Cur % Count;
				const int32 CY = Cur / Count;
				FRoomInfo& Rm = LastRooms[RoomIdx];
				++Rm.CellCount;
				Rm.MinCell.X = FMath::Min(Rm.MinCell.X, CX);
				Rm.MinCell.Y = FMath::Min(Rm.MinCell.Y, CY);
				Rm.MaxCell.X = FMath::Max(Rm.MaxCell.X, CX);
				Rm.MaxCell.Y = FMath::Max(Rm.MaxCell.Y, CY);

				static const int32 D[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
				for (int32 d = 0; d < 4; ++d)
				{
					const int32 NX = CX + D[d][0];
					const int32 NY = CY + D[d][1];
					if (NX < 0 || NY < 0 || NX >= Count || NY >= Count)
					{
						continue;
					}
					const int32 NI = MaskIndex(NX, NY);
					if (Cells[NI].bIsWall && !Cells[NI].bIsDoor)
					{
						continue;
					}
					if (Cells[NI].RoomIndex == INDEX_NONE)
					{
						Cells[NI].RoomIndex = RoomIdx;
						Stack.Add(NI);
					}
				}
			}
		}
	}
}

void FChunkGenerationData::RefineVariants()
{
	// Повторный проход выбора вида комнаты: теперь вид подгоняется под
	// РЕАЛЬНЫЙ размер связной комнаты (FRoomInfo::CellCount из
	// ComputeRoomInfos), а не под фиктивную константу/грубую оценку
	// EstimateRoomCells. Стены при этом уже решены (вид влиял на них только
	// через WallBias в CellIsWall, тоже по фактической комнате), так что
	// свет, высота потолка и раскладка пропсов соответствуют площади —
	// маленький чулан больше не выпадает в «Turbine Hall».
	if (Cells.Num() != Params.CellCount * Params.CellCount)
	{
		return;
	}
	for (int32 I = 0; I < Cells.Num(); ++I)
	{
		const int32 RoomIdx = Cells[I].RoomIndex;
		if (RoomIdx < 0 || RoomIdx >= LastRooms.Num())
		{
			continue;
		}
		const int32 RoomCells = FMath::Max(1, LastRooms[RoomIdx].CellCount);
		const int32 LX = I % Params.CellCount;
		const int32 LY = I / Params.CellCount;
		Cells[I].RoomVariant = (uint8)PickVariantForCell(WorldX(LX), WorldY(LY), RoomCells);
	}
}

int32 FChunkGenerationData::RoomIndexAt(int32 LX, int32 LY) const
{
	if (LX < 0 || LY < 0 || LX >= Params.CellCount || LY >= Params.CellCount)
	{
		return INDEX_NONE;
	}
	if (Cells.Num() != Params.CellCount * Params.CellCount)
	{
		return INDEX_NONE;
	}
	return Cells[MaskIndex(LX, LY)].RoomIndex;
}

FBox2D FChunkGenerationData::RoomBoundsLocal(int32 RoomIdx) const
{
	if (RoomIdx < 0 || RoomIdx >= LastRooms.Num())
	{
		return FBox2D(FVector2D::ZeroVector, FVector2D::ZeroVector);
	}
	const FRoomInfo& R = LastRooms[RoomIdx];
	return FBox2D(
		FVector2D((float)R.MinCell.X, (float)R.MinCell.Y),
		FVector2D((float)R.MaxCell.X + 1.0f, (float)R.MaxCell.Y + 1.0f));
}

void FChunkGenerationData::GetBorderDoors(TArray<FIntPoint>& OutDoorPositions, TArray<int32>& OutDirections) const
{
	OutDoorPositions.Reset();
	OutDirections.Reset();
	const int32 Count = Params.CellCount;
	for (int32 i = 0; i < Count; ++i)
	{
		if (Cells[MaskIndex(i, 0)].bIsDoor)
		{
			OutDoorPositions.Add(FIntPoint(i, 0));
			OutDirections.Add(0);
		}
		if (Cells[MaskIndex(i, Count - 1)].bIsDoor)
		{
			OutDoorPositions.Add(FIntPoint(i, Count - 1));
			OutDirections.Add(2);
		}
		if (Cells[MaskIndex(0, i)].bIsDoor)
		{
			OutDoorPositions.Add(FIntPoint(0, i));
			OutDirections.Add(3);
		}
		if (Cells[MaskIndex(Count - 1, i)].bIsDoor)
		{
			OutDoorPositions.Add(FIntPoint(Count - 1, i));
			OutDirections.Add(1);
		}
	}
}

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "BackroomsGenerationData.h"
#include "BackroomsLevelTheme.h"
#include "LevelGeneratorProfile.generated.h"

class UBackroomsLevelConfig;

// Тип размещения предмета в комнате (повторяет EPropPlacement для профиля).
UENUM(BlueprintType)
enum class ELevelPropPlacement : uint8
{
	FloorRandom UMETA(DisplayName = "Floor Random"),
	FloorCenter UMETA(DisplayName = "Floor Center"),
	AlongWall   UMETA(DisplayName = "Along Wall"),
	WallHang    UMETA(DisplayName = "Wall Hang"),
	CeilingHang UMETA(DisplayName = "Ceiling Hang")
};

// Чем занимался «человек» в аномальной комнате. Каждая сцена = один связанный
// ансамбль предметов (без дублей, без хаоса) — «сюжетный след».
UENUM(BlueprintType)
enum class EStorySceneType : uint8
{
	AbandonedLeft UMETA(DisplayName = "Брошено в спешке"),
	FledInHaste   UMETA(DisplayName = "Убегал"),
	MadeCamp      UMETA(DisplayName = "Привал"),
	SomeoneSick   UMETA(DisplayName = "Тому, кому стало плохо")
};

// Схема планировки локации: передаётся в плотностный генератор чанка.
// Лабиринт уместен не везде — у каждого уровня своя схема.
UENUM(BlueprintType)
enum class ELevelLayoutPattern : uint8
{
	GridRooms UMETA(DisplayName = "Комнаты разного размера"),
	Maze      UMETA(DisplayName = "Лабиринт"),
	OpenHall  UMETA(DisplayName = "Открытые залы"),
	Cave      UMETA(DisplayName = "Пещеры")
};

// Пустовер переменные пула пропсов одного уровня (из Style/LN_*/Props).
USTRUCT(BlueprintType)
struct FLevelPropEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	ELevelPropPlacement Placement = ELevelPropPlacement::FloorRandom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	float MinScale = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	float MaxScale = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	float MaxPerRoom = 1.0f;
};

// Один предмет «живой» сцены: меш, положение, поворот, вес и масштаб.
// Wes в кг: лёгкое катится, тяжёлое стоит. Масштаб реалистичный (или чуть меньше).
USTRUCT(BlueprintType)
struct FStoryPropEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	TSoftObjectPtr<UStaticMesh> Mesh;

	// Позиция внутри сцены: относительное смещение от центра комнаты (см).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	FVector Offset = FVector::ZeroVector;

	// «Слегка неправильный» поворот (0..360 — базовый; сцена добавляет лёгкий сдвиг).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	float Yaw = 0.0f;

	// Вес в кг (лёгкое катится, тяжёлое стоит).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	float WeightKg = 1.0f;

	// Масштаб (реалистичный или чуть меньше).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	float Scale = 1.0f;
};

// Один «сценарный» набор для конкретной истории (привал, побег, и т.д.).
USTRUCT(BlueprintType)
struct FLevelStoryScene
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Story")
	EStorySceneType Type = EStorySceneType::AbandonedLeft;

	// Набор связанных предметов сцены (не более ~5, без дублей).
	UPROPERTY(EditAnywhere, Category = "Story")
	TArray<FStoryPropEntry> Props;
};

// Профиль генерации конкретной локации (уровня).
// «Каждый уровень = свой генератор»: свой профиль с палитрой, размерами,
// пулом пропсов и плотностью.
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew)
class BACKROOMS_API ULevelGeneratorProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	// Название уровня (для отчёта).
	UPROPERTY(EditAnywhere, Category = "Identity")
	FString LevelName = TEXT("L0");

	// Цветовая палитра (BaseColor) чанка: пол / потолок / стены.
	// Приглушённые значения: под Lumen яркий потолок отражает много света и
	// заливает комнаты — для реализма/атмосферы альбедо держим ниже.
	UPROPERTY(EditAnywhere, Category = "Palette")
	FLinearColor FloorColor = FLinearColor(0.42f, 0.38f, 0.27f);

	UPROPERTY(EditAnywhere, Category = "Palette")
	FLinearColor CeilingColor = FLinearColor(0.62f, 0.60f, 0.55f);

	UPROPERTY(EditAnywhere, Category = "Palette")
	FLinearColor WallColor = FLinearColor(0.55f, 0.53f, 0.42f);

	// ---- Опциональные текстурированные материалы уровня ----
	// Если заданы — переопределяют цвета: чанк строится текстурным материалом
	// вместо раскраски через вертексные цвета (FLinearColor) из палитры выше.
	// Если пусто (nullptr) — используется shared-материал с вертексным цветом.
	// Используются в SetupFromProfile -> CommitMesh.
	UPROPERTY(EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> FloorMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> WallMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> CeilingMaterial;

	// Геометрия комнат.
	UPROPERTY(EditAnywhere, Category = "Geometry")
	float CellSize = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Geometry")
	float WallHeight = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Geometry")
	float WallThickness = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Geometry")
	float DoorWidth = 120.0f;

	UPROPERTY(EditAnywhere, Category = "Geometry")
	float DoorHeight = 250.0f;

	// Схема планировки: комнаты разного размера / лабиринт / открытые залы / пещеры.
	UPROPERTY(EditAnywhere, Category = "Geometry")
	ELevelLayoutPattern LayoutPattern = ELevelLayoutPattern::GridRooms;

	// Плотность пропсов.
	UPROPERTY(EditAnywhere, Category = "Fill")
	float MaxPropsPerRoom = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Fill")
	int32 ChunkSizeCells = 8;

	// ---- Слой проверок правил (как match-3: «подходит — оставить, не подходит —
	// перенести или пересобрать») ----
	// Минимальный размер комнаты (в клетках): меньше — «стеновой тупик», сцены
	// туда не кладутся (резолв: пытаемся перенести в большую комнату, иначе пропуск).
	UPROPERTY(EditAnywhere, Category = "Rules", meta = (ClampMin = "1", ClampMax = "64"))
	int32 MinRoomCells = 4;

	// Максимальный размер комнаты (в клетках): больше — «один зал», который
	// полирует сцены/пропсы в центре нерода (ограничение зоны размещения).
	UPROPERTY(EditAnywhere, Category = "Rules", meta = (ClampMin = "1", ClampMax = "64"))
	int32 MaxRoomCells = 12;

	// Радиус «напряжения» (в чанках, от выхода с люка): внутри stress->1 у выхода,
	// убывает линейно до 0 на радиусе. Кривая применяется к плотности пропсов и
	// выбору сюжетных сцен (привал только в спокойных, вдали от люка).
	UPROPERTY(EditAnywhere, Category = "Rules", meta = (ClampMin = "1", ClampMax = "32"))
	float StressRadiusChunks = 2.0f;

	// ---- Пороги плотностной генерации (плотность 0..1 -> маски) ----
	// Густота стен ВСЕХ схем раскладки: выше -> стен больше (гуще органика/
	// колонны в GridRooms и Maze, ниже прямой порог в OpenHall/Cave), ниже ->
	// проходов больше. При 0.32 геометрия та же, что и раньше; сложность
	// масштабирует это значение (BackroomsDifficulty), тем самым работая и для
	// «клеточных» схем, а не только для органики.
	UPROPERTY(EditAnywhere, Category = "Density")
	float WallThreshold = 0.32f;

	// Порог для случайных дверных проёмов в стенах (связность + лабиринт).
	UPROPERTY(EditAnywhere, Category = "Density")
	float DoorThreshold = 0.15f;

	// Порог маски разбросанных объектов: где на полу появляются пропсы.
	UPROPERTY(EditAnywhere, Category = "Density")
	float ScatterThreshold = 0.5f;

	// ---- Дроп предметов (множится сложностью) ----
	// Шанс полезного расходника на «жилую» клетку. Не опускаем ниже
	// минимально играбельного даже на высокой сложности.
	UPROPERTY(EditAnywhere, Category = "Density", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ItemPickupChance = 0.10f;

	// Шанс декоративного физического объекта на клетку. На высокой сложности
	// полезного меньше, но декора — больше.
	UPROPERTY(EditAnywhere, Category = "Density", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DecorPropChance = 0.08f;

	// Гарантированный минимум полезных предметов на чанк (анти-софтлок).
	UPROPERTY(EditAnywhere, Category = "Density", meta = (ClampMin = "0"))
	int32 MinUtilityPerChunk = 1;

	// Порог «живых/аномальных» комнат (0..1, реально ~0.08–0.12). Используется
	// детерминированным генератором живых комнат (FRandomStream + кластеризация).
	UPROPERTY(EditAnywhere, Category = "Density")
	float LivingRoomThreshold = 0.10f;

	// Пул пропсов этого уровня (вызов папки Style/LN_*/Props).
	UPROPERTY(EditAnywhere, Category = "Props")
	TArray<FLevelPropEntry> Props;

	// Сюжетные сцены «живых» комнат: по одной на комнату, ансамбль одной истории.
	UPROPERTY(EditAnywhere, Category = "Props")
	TArray<FLevelStoryScene> StoryScenes;

	// Базовая папка Fab-контента (редактируемая). Используется как префикс
	// при поиске/резолве путей в фильтре (пусто = использовать хардкод /Game/Fab).
	UPROPERTY(EditAnywhere, Category = "Props")
	FString FabContentRoot = TEXT("/Game/Fab");

	// Спавнить ли стартовую платформу.
	UPROPERTY(EditAnywhere, Category = "Start")
	bool bSpawnPlatform = true;

	// Индекс уровня, для которого профиль построен (0..16). Нужен, чтобы
	// FParams.LevelIndex был реальным уровнем (для L10+ таблица VariantSystem
	// даёт Neutral), а не кастом из EBackroomsLevelStyle.
	UPROPERTY()
	int32 ConfigLevelIndex = 0;

	// Активный YAML-конфиг уровня (если BuildDefaultByLevel нашёл LXX.yaml).
	// Хранит источник правды для геометрии/планировки (см. ApplyLevelConfig).
	UPROPERTY()
	TObjectPtr<UBackroomsLevelConfig> LevelConfig;

	// Идентификатор конфига из YAML (id: LXX), пусто — конфиг не загружен.
	UPROPERTY(EditAnywhere, Category = "Identity")
	FString ConfigId = TEXT("");

	// Применить ULevelConfig: метры -> сантиметры, планировка из algorithm,
	// правила комнат из rooms/lights. Геометрия и чанк берутся из конфига.
	void ApplyLevelConfig(const UBackroomsLevelConfig* Cfg);

	// Собрать FParams плотностного генератора из полей профиля (которые
	// приходят из ULevelConfig). Единственный источник FParams для чанка.
	FChunkGenerationData::FParams ToGenParams(int32 LevelIndex) const;

	// Построить профиль из пула по умолчанию (для прототипа без .uasset).
	static ULevelGeneratorProfile* BuildDefaultByLevel(int32 LevelIndex);

	// Тема HUD/меню/аудио для этого уровня (один HUD-класс, разные темы).
	UPROPERTY(EditAnywhere, Category = "Theme")
	FBackroomsLevelTheme Theme;
};

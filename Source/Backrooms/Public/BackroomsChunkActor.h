#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/RectLightComponent.h"
#include "ProceduralMeshComponent.h"
#include "LevelGeneratorProfile.h"
#include "BackroomsRoomGraph.h"
#include "BackroomsFloorPlan.h"
#include "BackroomsChunkActor.generated.h"

class UPropDatabase;
class ABackroomsPhysProp;
class ABackroomsItemPickup;
class FChunkGenerationData;

// ---- События окружения (слой 1) ----
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPropBroken, AActor*, PropActor);
// Сюжетный триггер: сцена комнаты собрана (все пропсы подняты).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarrativeTrigger, EStorySceneType, ScenarioID, const TArray<AActor*>&, PropList);

UENUM(BlueprintType)
enum class EBackroomsLevelStyle : uint8
{
	L0_Lobby     UMETA(DisplayName = "L0 Lobby"),
	L1_Habitable UMETA(DisplayName = "L1 Habitable"),
	L2_Pipes     UMETA(DisplayName = "L2 Pipes"),
	L3_Power     UMETA(DisplayName = "L3 Power"),
	L4_Offices   UMETA(DisplayName = "L4 Offices"),
	L5_Hotel     UMETA(DisplayName = "L5 Hotel"),
	L6_Dark      UMETA(DisplayName = "L6 Dark"),
	L7_Ocean     UMETA(DisplayName = "L7 Ocean"),
	L8_Caves     UMETA(DisplayName = "L8 Caves"),
	L9_Hospital  UMETA(DisplayName = "L9 Hospital")
};

UCLASS()
class BACKROOMS_API ABackroomsChunkActor : public AActor
{
	GENERATED_BODY()

public:
	ABackroomsChunkActor();

	// ---- События окружения (BlueprintAssignable — как в UBackroomsItemSystem) ----
	UPROPERTY(BlueprintAssignable, Category = "Events|Environment")
	FOnPropBroken OnPropBroken;

	UPROPERTY(BlueprintAssignable, Category = "Events|Narrative")
	FOnNarrativeTrigger OnNarrativeTrigger;

	// Сюжетный триггер: игрок поднял проп «живой» сцены комнаты. Чанк собирает
	// пропсы комнаты и излучает OnNarrativeTrigger, когда сцена собрана.
	void NotifyStoryPropPickedUp(class ABackroomsPhysProp* Prop);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	EBackroomsLevelStyle LevelStyle = EBackroomsLevelStyle::L0_Lobby;

	// FloorPlan source of truth — chunk queries this for architecture, does NOT invent it
	UPROPERTY()
	TObjectPtr<UBackroomsFloorPlan> FloorPlan;

	// Режим генерации, задаваемый генератором из конфига уровня (bUseFloorPlan).
	// true  — чанк СТРОИТСЯ ТОЛЬКО из FloorPlan; если плана нет — ошибка, без фолбэка.
	// false — legacy/город (старая плотностная генерация).
	UPROPERTY()
	bool bUseFloorPlan = false;

	// Привязать источник архитектуры: план региона, в который попадает этот чанк.
	// Мировый генератор следит за кешем планов; чанк их только читает (извлекает).
	UFUNCTION(BlueprintCallable, Category = "Chunk")
	void AssignFloorPlan(UBackroomsFloorPlan* InPlan);

	UPROPERTY(VisibleAnywhere, Category = "Chunk")
	TObjectPtr<UProceduralMeshComponent> GeometryMesh;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TObjectPtr<UMaterialInterface> FloorMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TObjectPtr<UMaterialInterface> CeilingMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TObjectPtr<UMaterialInterface> WallMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TObjectPtr<UMaterialInterface> CityFloorMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TObjectPtr<UMaterialInterface> CityCeilingMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TObjectPtr<UMaterialInterface> CityWallMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TObjectPtr<UMaterialInterface> CityRoadMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TObjectPtr<UMaterialInterface> CitySidewalkMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TObjectPtr<UMaterialInterface> CityMarkingMaterial;

	// ---- Nanite-скины стен (Вариант-Б) ----
	// UProceduralMeshComponent не поддерживает Nanite, поэтому поверх процедурных
	// граней стен кладутся плоские Nanite StaticMesh-панели с микрорельефом
	// (трещины/пузыри обоев). Оффсет против z-fighting. Выкл., пока не назначен меш.
	UPROPERTY(EditAnywhere, Category = "Nanite")
	bool bNaniteSkinsEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Nanite")
	TSoftObjectPtr<UStaticMesh> NaniteSkinMesh;

	// Смещение панели от грани вдоль нормали, см (анти-z-fighting).
	UPROPERTY(EditAnywhere, Category = "Nanite", meta = (ClampMin = "0.0"))
	float NaniteSkinOffset = 0.1f;

	// Опорный размер панели в ассете, см: инстанс растягивается под грань
	// (Nanite допускает non-uniform scale через матрицу инстанса).
	UPROPERTY(EditAnywhere, Category = "Nanite", meta = (ClampMin = "1.0"))
	float NaniteSkinRefSize = 100.0f;

	// Панели со стороной меньше этой не кладём (торцы перегородок, перемычки).
	UPROPERTY(EditAnywhere, Category = "Nanite", meta = (ClampMin = "0.0"))
	float NaniteSkinMinSize = 80.0f;

	// ---- Декали-надписи («карандашный» сторителлинг, Deferred Decal) ----
	// Мелкие цифры/даты/имена/палочки-черточки на стенах. Спавн — шанс на стенной
	// грани чанка. Материалы назначаются в редакторе/YAML (см. Content\Decals).
	UPROPERTY(EditAnywhere, Category = "Decals", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GraffitiDecalChance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Decals")
	TArray<TSoftObjectPtr<UMaterialInterface>> GraffitiDecalMaterials;

	// Базовый размер надписи (ширина/высота), см; на грань ложится с разбросом.
	UPROPERTY(EditAnywhere, Category = "Decals", meta = (ClampMin = "1.0"))
	FVector2D GraffitiDecalSizeCm = FVector2D(90.0f, 45.0f);

	UFUNCTION(BlueprintCallable, Category = "Level")
	void ApplyLevelStyle(EBackroomsLevelStyle NewStyle);

	UFUNCTION(BlueprintPure, Category = "Level")
	static FString GetLevelStylePath(EBackroomsLevelStyle Style);

	// Освещена ли точка комнатным светом (для рассудка/темноты).
	UFUNCTION(BlueprintPure, Category = "Level")
	bool IsPointLit(const FVector& Point, float Radius) const;

	int32 ChunkSizeCells = 8;
	float CellSize = 500.0f;
	float WallHeight = 300.0f;
	float WallThickness = 30.0f;
	float DoorWidth = 120.0f;
	float DoorHeight = 210.0f;
	int32 Seed = 1337;
	// Глобальный seed мира: от него считаются плотность/стены/двери/вид комнаты,
	// чтобы границы соседних чанков совпадали. Seed выше — только локальный декор.
	int32 WorldSeed = 0;
	float MaxPropsPerRoom = 3.0f;
	int32 ChunkX = 0;
	int32 ChunkY = 0;

	UPROPERTY(EditAnywhere, Category = "LevelProfile")
	float WallThreshold = 0.32f;

	UPROPERTY(EditAnywhere, Category = "LevelProfile")
	float DoorThreshold = 0.15f;

	UPROPERTY(EditAnywhere, Category = "LevelProfile")
	float ScatterThreshold = 0.5f;

	// Схема планировки текущего уровня (пробрасывается в плотностный генератор).
	UPROPERTY(EditAnywhere, Category = "LevelProfile")
	ELevelLayoutPattern LayoutPattern = ELevelLayoutPattern::GridRooms;

	// Правила размера комнат (в клетках) — используются генератором для
	// расчёта шага сетки (pitch) и подбора вида комнат.
	UPROPERTY(EditAnywhere, Category = "LevelProfile", meta = (ClampMin = "1", ClampMax = "64"))
	int32 MinRoomCells = 4;

	UPROPERTY(EditAnywhere, Category = "LevelProfile", meta = (ClampMin = "1", ClampMax = "64"))
	int32 MaxRoomCells = 12;

	// Порог «живых/аномальных» комнат (0..1).
	UPROPERTY(EditAnywhere, Category = "LevelProfile", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LivingRoomThreshold = 0.10f;

	// Базовые частоты плотностного шума (фолбэк, если профиль не задал).
	UPROPERTY(EditAnywhere, Category = "LevelProfile", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float BaseNoiseFreq = 0.09f;

	UPROPERTY(EditAnywhere, Category = "LevelProfile", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float MacroNoiseFreq = 0.012f;

	bool bSpawnPlatform = false;
	float SpawnPlatformAlt = 3000.0f;
	// Размер стартового города в чанках по каждой оси. Город строится одной
	// цельной плитой, чтобы Бэкрумс не был виден сквозь промежутки между островками.
	int32 PlatformSizeChunks = 5;
	TObjectPtr<UPropDatabase> Database;

	UPROPERTY(EditAnywhere, Category = "LevelProfile")
	TObjectPtr<ULevelGeneratorProfile> Profile;

	// Класс физического предмета для сюжетных сцен (кидать/разглядывать).
	UPROPERTY(EditAnywhere, Category = "Story")
	TSubclassOf<ABackroomsPhysProp> StoryPropClass;

	// Класс подбираемого расходника (миндальная вода, еда, аптечка и т.п.).
	UPROPERTY(EditAnywhere, Category = "Items")
	TSubclassOf<ABackroomsItemPickup> ItemPickupClass;

	// Шанс, что «жилая» клетка получит подбираемый расходник.
	UPROPERTY(EditAnywhere, Category = "Items", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ItemPickupChance = 0.10f;

	// Шанс, что «жилая» клетка получит декоративный физический объект из Fab
	// (можно толкать, брать в руки, осматривать, кидать).
	UPROPERTY(EditAnywhere, Category = "Decor", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DecorPropChance = 0.08f;

	// Шанс, что открытая «жилая» клетка станет комнатой сюжетной сцены.
	UPROPERTY(EditAnywhere, Category = "Story", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StorySceneChance = 0.05f;

	// Гарантированный минимум полезных расходников на чанк. Считается после
	// раскладки: если за проход выпало меньше — добираем в случайных клетках.
	// Так даже на «Кошмаре» игрок не остаётся совсем без снабжения.
	int32 MinUtilityPerChunk = 1;
	// Сколько полезных предметов уже выпало в текущем чанке (см. BuildChunk*).
	int32 UtilitySpawnedThisChunk = 0;

	UPROPERTY(EditAnywhere, Category = "LevelProfile")
	FLinearColor PaletteFloor = FLinearColor(0.42f, 0.38f, 0.27f);

	UPROPERTY(EditAnywhere, Category = "LevelProfile")
	FLinearColor PaletteCeiling = FLinearColor(0.62f, 0.60f, 0.55f);

	UPROPERTY(EditAnywhere, Category = "LevelProfile")
	FLinearColor PaletteWall = FLinearColor(0.55f, 0.53f, 0.42f);

	virtual void BuildChunk(int32 InChunkX, int32 InChunkY, int32 InSeed, const TObjectPtr<UPropDatabase>& InDatabase);
	virtual void BuildChunkFromProfile(int32 InChunkX, int32 InChunkY, int32 InSeed, const ULevelGeneratorProfile* InProfile);
	void SetupFromProfile(const ULevelGeneratorProfile* InProfile);

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	void SetCityBuildings(const TArray<TSoftObjectPtr<UStaticMesh>>& InCityBuildings);

	// Сбой лампы: несколько источников (Count) в радиусе от точки кратко
	// «проседают» и мигают, как перегорающая лента. Не стробоскоп: 2-3 быстрых
	// затухания за Duration и возврат к норме. Вызывается событием LightFlicker.
	UFUNCTION(BlueprintCallable, Category = "Chunk|Lights")
	void FlickerLightsNear(const FVector& WorldLocation, float Radius, int32 Count, float Duration);

	// Сколько ламп в этом чанке (для отладки/аллокации эффекта).
	UFUNCTION(BlueprintPure, Category = "Chunk|Lights")
	int32 GetRoomLightCount() const { return RoomLights.Num(); }

	FString AsciiMapText() const;

	// Кросс-чанковый граф комнат: BuildFromChunk + рёбра-границы с соседями
	// (Neighbors[4] — в порядке +X, -X, +Y, -Y). Строит WorldGenerator после
	// генерации данных соседних чанков (см. SpawnPending), поэтому API открытый.
	void BuildRoomGraph(const FChunkGenerationData* Neighbors[4]);
	const FChunkGenerationData* GetGenData() const { return GenData ? GenData.Get() : nullptr; }
	const FRoomGraph* GetRoomGraph() const { return &RoomGraph; }

	virtual void BeginDestroy() override;

private:
	enum class EMaterialSlot : uint8
	{
		Floor,
		Ceiling,
		Wall,
		Road,
		Sidewalk,
		Marking
	};

	void ClearMeshData();
	void CommitMesh();
	void AddQuad(FVector A, FVector B, FVector C, FVector D, FVector Normal, const FLinearColor& Color, float UVScale, EMaterialSlot Slot);
	void AddBox(FVector Center, FVector Half, const FLinearColor& Color, float UVScale, EMaterialSlot Slot);
	static float WorldNoise(float X, float Y, int32 Seed, float FreqX = 0.0f, float FreqY = 0.0f);
	bool EdgeHasWall(int32 CellX, int32 CellY, int32 DirX, int32 DirY) const;
	bool EdgeHasDoor(int32 CellX, int32 CellY, int32 DirX, int32 DirY) const;
	void BuildEdgeWall(int32 CellX, int32 CellY, int32 DirX, int32 DirY);
	void PlaceProps(int32 CellX, int32 CellY, FRandomStream& Random);
	void PlacePropsFromProfile(int32 CellX, int32 CellY, FRandomStream& Random);
	// Спавн «живой» сюжетной сцены комнаты: физические предметы, которые можно
	// поднять, осмотреть, толкнуть и бросить (ABackroomsPhysProp).
	void SpawnStoryProps(int32 CellX, int32 CellY, FRandomStream& Random);
	// Подбираемый расходник в «жилой» клетке (петля выживания: ItemId из
	// каталога UBackroomsItemSystem).
	void SpawnItemPickup(int32 CellX, int32 CellY, FRandomStream& Random);
	// Гарантированный спавн полезного предмета (анти-софтлок, см. TopUpUtilityLoot).
	void SpawnOneUtility(int32 CellX, int32 CellY, FRandomStream& Random);
	// Добрать полезный лут до MinUtilityPerChunk, если случайный проход недодал.
	void TopUpUtilityLoot(FRandomStream& Random);
	// Декоративный физический объект из Fab: можно толкать, брать, кидать.
	// OffsetXY двигает позицию от центра клетки (для «островков» у стены).
	void SpawnDecorProp(int32 CellX, int32 CellY, FRandomStream& Random, const FVector* OffsetXY = nullptr);
	// Слой 2 «Инженерка»: розетки/выключатели/вентиляция/плинтус. Расставляются
	// «пучками» вдоль стенных граней клетки (якорь клетка + соседи пучка).
	// Грязь у стыка стена-пол — декалями из LevelConfig.Decor.
	void SpawnWallGear(int32 CellX, int32 CellY, const TArray<uint8>& WallMask, FRandomStream& Random, const struct FBackroomsDecorConfig& Decor);
	// Слой 3 «История» + пятна: островки мусора у стен и пятна/подтёки на полу
	// (декали). Пятна ковра в обычных клетках — мелкий «микро-клаттер».
	void SpawnStainsAndHistory(int32 CellX, int32 CellY, FRandomStream& Random, const struct FBackroomsDecorConfig& Decor);
	bool IsWallEdgeCell(int32 LocX, int32 LocY, const TArray<uint8>& WallMask, FVector& OutNormalIn, FVector& OutNormalOut, int32& OutNeighborLocalIndex) const;
	void ReserveDoorCorridor(int32 CellX, int32 CellY);
	void BuildSpawnPlatform();
	bool OverlapsOccupied(const FBox2D& Box) const;
	UInstancedStaticMeshComponent* GetOrCreateInstancer(UStaticMesh* Mesh, bool bBlocking = false);
	FBox2D CellBounds(int32 CellX, int32 CellY) const;

	// ---- Generation Data layer ----
	void InitGenData();
	void GenerateMasksFromGenData(TArray<uint8>& WallMask, TArray<uint8>& DoorMask) const;
	TSharedPtr<FChunkGenerationData> GenData;

	// ---- FloorPlan extraction path ----
	// Собрать чанк из плана региона: ExtractChunk → материалы → меши → свет →
	// лут. Чанк НЕ решает архитектуру — он лишь показывает кусок плана.
	void BuildFromFloorPlan();
	// Пропс/колонна/светильник по позиции из FChunkGeometryData: FloorPlan
	// решает ЧТО и ГДЕ, а чанк — только материализует это в мире.
	void PlacePropsAtPosition(const FVector& Position);
	void AddColumnAtPosition(const FVector& Position);
	void AddLightAtPosition(const FVector& Position, const FLinearColor& Color);
	bool IsCellBlocked_FloorPlan(int32 WorldCellX, int32 WorldCellY) const;

	FRoomGraph RoomGraph;

	// ---- Сюжетные сцены: собранные пропсы комнаты (для OnNarrativeTrigger) ----
	TMap<int32, TArray<TWeakObjectPtr<AActor>>> StoryPropsByRoom;
	TSet<int32> PickedNarrativeRooms;

	void BuildGeometryFromMasks(const TArray<uint8>& WallMask, const TArray<uint8>& DoorMask);

	// Nanite-скины: спавнит инстансы NaniteSkinMesh поверх граней стен
	// (BuildNaniteWallSkins вызывается до CommitMesh, пока живы Vertices/QuadSlots).
	void BuildNaniteWallSkins();

	// Декали-надписи на стенах (вызывается до CommitMesh, пока живы квады).
	void SpawnGraffitiDecals();

	void SpawnRoomLights(const TArray<uint8>& WallMask);

	// --- Характер ламп: часть плафонов ведёт себя «живо» (см. ApplyLampPersonality). ---
	// Доли на чанк: сломана (не горит), умирающая (розовый/зелёный, тускло),
	// мерцающая (плохой контакт). Сумме между собой не обязаны равняться 1.
	UPROPERTY(EditAnywhere, Category = "Lights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LampBrokenChance = 0.02f;

	UPROPERTY(EditAnywhere, Category = "Lights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LampDyingChance = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Lights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LampFlickerChance = 0.10f;

	void ApplyLampPersonality(URectLightComponent* Light, FRandomStream& Random);

	// Светильники-«панели»: RectLight под каждым плафоном. Прямоугольный
	// источник + Lumen даёт мягкие, физически правильные тени по углам комнат
	// (у точечного света тень от каждой панели «звенит»).
	UPROPERTY(Transient)
	TArray<TObjectPtr<URectLightComponent>> RoomLights;

	// Объёмный вклад ламп в волюметрик-туман (лучи света «God Rays»). 0 — без
	// лучей; ~0.3-0.6 — ощутимые световые колонны под панелями.
	UPROPERTY(EditAnywhere, Category = "Lights", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float LampVolumetricIntensity = 0.35f;

	static int32 MaskIndex(int32 LX, int32 LY, int32 Count)
	{
		return LY * Count + LX;
	}

	TArray<uint8> LastWallMask;
	TArray<uint8> LastDoorMask;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
	TArray<uint8> QuadSlots;

	UStaticMesh* CachedLoadMesh(const TSoftObjectPtr<UStaticMesh>& SoftMesh);
	TMap<FName, UStaticMesh*> LoadedMeshes;
	TSet<FName> MissingMeshes;

	UPROPERTY(EditAnywhere, Category = "City")
	TArray<TSoftObjectPtr<UStaticMesh>> CityBuildings;

	void SpawnCityBuildings(FRandomStream& Random, float HalfExtent);

	TArray<FBox2D> OccupiedBoxes;
	// Комнаты, для которых уже разложены пропсы (PlacePropsFromProfile работает
	// по комнате целиком, а не по клетке — см. фикс «пропсы по комнатам»).
	TSet<int32> PropRoomsPlaced;
	TMap<UStaticMesh*, TObjectPtr<UInstancedStaticMeshComponent>> Instancers;
};

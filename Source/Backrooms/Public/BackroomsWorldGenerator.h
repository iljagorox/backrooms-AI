#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BackroomsLevelTheme.h"
#include "BackroomsWorldGenerator.generated.h"

class ABackroomsChunkActor;
class ABackroomsRiggedMonster;
class ABackroomsExitActor;
class ABackroomsLevelAudioActor;
class UPropDatabase;
class ULevelGeneratorProfile;
class ULoadingBarWidget;
class USphereComponent;
class UStaticMesh;
class UMaterial;
class AExponentialHeightFog;
class UBackroomsFloorPlan;

// Слот сетки чанков: мировая логическая координата + актор. «Классика типа
// три-в-ряд»: мир = плоская 2D-сетка (row,col), индекс = (row+Off)*Stride+(col+Off),
// соседи берутся сдвигом ±1. Акторы всегда стоят в (0,0,0), «где чанк» — только
// Coord (геометрия строится в мировых координатах), поэтому сетка — единственный
// владелец размещения.
USTRUCT()
struct FChunkGridSlot
{
	GENERATED_BODY()

	UPROPERTY()
	FIntPoint Coord = FIntPoint::ZeroValue;

	UPROPERTY()
	TObjectPtr<ABackroomsChunkActor> Chunk;
};

UCLASS()
class BACKROOMS_API ABackroomsWorldGenerator : public AActor
{
	GENERATED_BODY()

public:
	ABackroomsWorldGenerator();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Generation")
	int32 Seed = 1337;

	// Активный уровень (локация) — определяет, какой профиль-«генератор» включить.
	UPROPERTY(EditAnywhere, Category = "Level")
	int32 LevelIndex = 0;

	// Фиксированная карта уровня (Lvl_L*): уровень берётся из имени карты,
	// смена локации — только через OpenLevel с seed в URL (см. ExitActor).
	// При включённом флаге SetLevel игнорируется (guard).
	UPROPERTY(EditAnywhere, Category = "Level")
	bool bFixedLevel = false;

	// Ручной профиль уровня (если задан, используется вместо стандартного по LevelIndex).
	UPROPERTY(EditAnywhere, Category = "Level")
	TObjectPtr<ULevelGeneratorProfile> LevelProfileOverride;

	// Тема уровня (HUD/меню/аудио): DataAsset-конфиг → тема профиля → дефолт.
	UFUNCTION(BlueprintPure, Category = "Level")
	FBackroomsLevelTheme GetActiveTheme() const;

	// Перегенерировать мир с текущей сложностью (вызывается меню после её смены).
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void RebuildForDifficulty();

	// Установить уровень и перегенерировать мир под его профиль.
	UFUNCTION(BlueprintCallable, Category = "Level")
	void SetLevel(int32 InLevelIndex);

	// Прелоад мешей пропсов и города ДО начала движения игрока. Синхронная
	// загрузка (LoadSynchronous) на старте — один раз — вместо «фризов» в
	// середине игры, когда CachedLoadMesh грузит меш с диска по ходу стриминга.
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void PreloadPropMeshes();

	// Текстовый отчёт о том, что настроил генератор (для DevTool/логов).
	UFUNCTION(BlueprintCallable, Category = "Dev")
	FString GenerateReport();

	// Самопроверка конвейера генерации: заново пересчитывает выборку чанков и
	// проверяет детерминизм (совпадение хэшей на реплеях) и инварианты каждого
	// этапа. Работает на чистых данных, поэтому пригодна для любого уровня.
	UFUNCTION(BlueprintCallable, Category = "Dev")
	FString GenerateVerificationReport();

	// Текущая ступень «давления среды» (растёт, пока игрок избегает выход).
	// Event-система использует это для роста вариативности и враждебности.
	UFUNCTION(BlueprintPure, Category = "Exit")
	int32 GetExitPressure() const { return ExitPressure; }

	// Игрок уже под городом, в бесконечных Бэкрумсах (а не на стартовой
	// платформе-городе). HUD по этому флагу прячет выживальческие шкалы в городе.
	UFUNCTION(BlueprintPure, Category = "Generation")
	bool IsPlayerInBackrooms() const { return bPlayerFellToBackrooms; }

	// Есть ли в мире активный выход и где он (для триггеров/звуков).
	UFUNCTION(BlueprintPure, Category = "Exit")
	bool GetExitLocation(FVector& OutLocation) const;

	// Кешированный сглаженный вектор движения игрока (напр., для монстра).
	UFUNCTION(BlueprintPure, Category = "Exit")
	FVector GetTravelDirection() const { return FVector(TravelDir.X, TravelDir.Y, 0.0f); }

	// Следующая локация по списку LevelProgression (для настройки выхода).
	UFUNCTION(BlueprintPure, Category = "Exit")
	int32 GetNextLevelIndex() const;

	// Лиминальность-слой звука из активного конфига уровня (acoustics).
	// AmbientLayer: lamp_hum / drip / ventilation / room_tone_film / wind.
	UFUNCTION(BlueprintPure, Category = "Level")
	FString GetActiveAmbientLayer() const;

	// Реверб-сцена активного уровня (acoustics.reverb, напр. "office"/"cave").
	UFUNCTION(BlueprintPure, Category = "Level")
	FString GetActiveReverbScene() const;

	// Эхо активного уровня (acoustics.echo).
	UFUNCTION(BlueprintPure, Category = "Level")
	bool GetActiveEchoEnabled() const;

	// Радиус действия комнатного света активного уровня (в см) — та же формула,
	// что и AttenuationRadius в SpawnRoomLights (lights.every_m * 100 * 0.9).
	// Нужен игроку для «свет считается при мне» без дублирования хардкода 900.
	UFUNCTION(BlueprintPure, Category = "Level")
	float GetActiveLightRadius() const;

	// Размер клетки генератора активного уровня (см). Игрок использует его для
	// проб смены комнат (OnPlayerEnteredRoom), чтобы RoomID совпадал с картой
	// геометрии; 500 = фолбэк при отсутствии профиля (CellSize при tile 0.5 м).
	UFUNCTION(BlueprintPure, Category = "Level")
	float GetActiveRoomCellSize() const;

	UPROPERTY(EditAnywhere, Category = "Generation")
	int32 ChunkSizeCells = 8;

	UPROPERTY(EditAnywhere, Category = "Generation")
	float CellSize = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Generation")
	float WallHeight = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Generation")
	float WallThickness = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Generation")
	float DoorWidth = 120.0f;

	UPROPERTY(EditAnywhere, Category = "Generation")
	float DoorHeight = 210.0f;

	// Густота стен активного уровня (см. LevelGeneratorProfile.WallThreshold).
	// Копируется в каждый чанк и в PrintLib-параметры генерации.
	UPROPERTY(EditAnywhere, Category = "Generation")
	float WallThreshold = 0.32f;

	UPROPERTY(EditAnywhere, Category = "Generation")
	float DoorThreshold = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Generation")
	float ScatterThreshold = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Generation")
	// Город занимает 5x5 чанков; радиус 2 полностью укладывает скрытый
	// слой Бэкрумса под городом и не оставляет его видимым по краям.
	int32 RenderRadiusChunks = 2;

	UPROPERTY(EditAnywhere, Category = "Generation")
	int32 MaxChunksPerTick = 3;

	// Радиус (в чанках), внутри которого чанки видимы и освещены. Дальние чанки
	// остаются построенными (повторной генерации нет), но скрыты: без геометрии,
	// света и коллизии они не тратят кадр. С hysteresis, чтобы не мерцать на границе.
	UPROPERTY(EditAnywhere, Category = "Generation", meta = (ClampMin = "0.5"))
	float VisibleRadiusChunks = 1.35f;

	UPROPERTY(EditAnywhere, Category = "Generation")
	float MaxPropsPerRoom = 3.0f;

	// Враг появляется только после того, как игрок оказался в Бэкрумсе и
	// стартовая волна чанков полностью достроена. Это не даёт ему заспавниться
	// в воздухе, в стене или на городе.
	UPROPERTY(EditAnywhere, Category = "Monster")
	bool bSpawnMonsterAfterInitialGeneration = true;

	UPROPERTY(EditAnywhere, Category = "Monster")
	TSubclassOf<ABackroomsRiggedMonster> MonsterClass;

	UPROPERTY(EditAnywhere, Category = "Monster", meta = (ClampMin = "800.0"))
	float MonsterSpawnMinDistance = 1800.0f;

	UPROPERTY(EditAnywhere, Category = "Monster", meta = (ClampMin = "1200.0"))
	float MonsterSpawnMaxDistance = 4200.0f;

	UPROPERTY(EditAnywhere, Category = "SpawnPlatform")
	float SpawnPlatformAlt = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "SpawnPlatform")
	float PlatformDescentSpeed = 150.0f;

	UPROPERTY(EditAnywhere, Category = "SpawnPlatform")
	float PlatformBrakeAlt = 600.0f;

	UPROPERTY(EditAnywhere, Category = "SpawnPlatform")
	bool bSpawnPlatform = true;

	// Максимальное время ожидания загрузки чанков перед принудительным спуском (сек).
	// (Автоспуск отключён: спуск теперь только через люк StartupPlatformTrapdoor.)
	UPROPERTY(EditAnywhere, Category = "SpawnPlatform")
	float CityDescentTimeout = 12.0f;

	UPROPERTY(EditAnywhere, Category = "SpawnPlatform")
	TObjectPtr<UPropDatabase> PlatformDatabase;

	// Здания города (Buildings-OBJ) — передаются стартовой платформе.
	// Если пусто, платформа берёт их из правил PropDatabase "Buildings/*",
	// а при отсутствии — строит процедурный фолбэк (пустой город больше не бывает).
	UPROPERTY(EditAnywhere, Category = "SpawnPlatform")
	TArray<TSoftObjectPtr<UStaticMesh>> CityBuildings;

	// ---- Выход из уровня ---- //
	// Выход — не просто дверь, а развилка: игрок может уйти сразу или остаться
	// исследовать. Избегание награждается «давлением среды»: чем дольше выход
	// существует и чем дальше игрок от него, тем сильнее уровень.
	UPROPERTY(EditAnywhere, Category = "Exit")
	TSubclassOf<ABackroomsExitActor> ExitClass;

	// На каком расстоянии (в чанках) от игрока появляется выход.
	UPROPERTY(EditAnywhere, Category = "Exit", meta = (ClampMin = "1.0"))
	float ExitSpawnDistanceChunks = 2.5f;

	// Порядок локаций через выход: линейный проход 0..16 по каталогу
	// Content/Config/Levels (L0 -> L1 -> ... -> L16). Последний элемент ведёт
	// «в никуда» (+1), если список исчерпан; меню/девтул могут прыгать на любой
	// уровень напрямую через SetLevel.
	UPROPERTY(EditAnywhere, Category = "Exit")
	TArray<int32> LevelProgression = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

	// Секунд спокойствия после готовности мира, прежде чем начинается давление.
	UPROPERTY(EditAnywhere, Category = "Exit")
	float ExitGracePeriod = 90.0f;

	// Секунд на одну ступень эскалации, пока игрок рядом с выходом.
	UPROPERTY(EditAnywhere, Category = "Exit")
	float ExitPressureInterval = 45.0f;

	// Максимум ступеней давления (выше — игрок уже должен был уйти).
	UPROPERTY(EditAnywhere, Category = "Exit", meta = (ClampMin = "0"))
	int32 MaxExitPressure = 6;

	// Ближе этого расстояния (см) к выходу давление замирает и медленно спадает.
	UPROPERTY(EditAnywhere, Category = "Exit")
	float ExitSafeRadius = 900.0f;

	UPROPERTY(EditAnywhere, Category = "Generation")
	TObjectPtr<UPropDatabase> Database;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<ULoadingBarWidget> LoadingBarClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	bool bShowLoadingBar = true;

	// Минимальная длительность показа полосы загрузки (сек). Полоска не доходит
	// до 100%, пока не прошло это время И не построены все запрошенные чанки:
	// даёт миру время на поэтапную генерацию под платформой, а игроку — не
	// «провалиться в пустоту», если чанки строятся быстрее, чем ожидалось.
	UPROPERTY(EditAnywhere, Category = "UI")
	float MinLoadingBarTime = 120.0f;

private:
	FIntPoint WorldToChunk(const FVector& WorldPos) const;
	void UpdateStreaming();
	void SpawnPending();
	void UpdateLoadingBar();
	void SpawnStartPlatform();
	// Небо над городом: атмосфера (синева + солнечный диск), объёмные облака,
	// солнце (DirectionalLight) и лёгкий туман. Бэкрумс внизу закрыт потолком,
	// поэтому атмосферный свет в него почти не попадает — там светят только лампы.
	void SetupSkyAndWeather();
	void ApplyLevelAtmosphere();
	void SitPlayerOnPlatform(bool bSnap);
	void PinPlayerToPlatform();
	float PawnStandZ() const;
	void UpdatePlatformDescent(float DeltaSeconds);
	void RebuildAllChunks();
	ULevelGeneratorProfile* GetActiveProfile();
	// Зафиксировать вход игрока в Бэкрумс (из падения или из люка) и сообщить
	// об этом достижениям — единая точка, чтобы факт не задваивался.
	void MarkBackroomsEntered();
	// Подсистема достижений (может быть nullptr).
	class UBackroomsAchievements* GetAchievements() const;
	void TrySpawnMonsterAfterGeneration();
	void TryAutoDump(float DeltaSeconds);
	void StartDescent();
	UFUNCTION()
	void OnTrapdoorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// ---- Выход и «давление среды» ----
	// Мир бесконечен: игрок может идти в любую сторону сколько угодно. Выход
	// поэтому не статичен — он всегда держится на разумном расстоянии впереди
	// игрока, следуя за направлением его движения. Одновременно в мире ровно
	// один выход. Слишком близко — тривиально, слишком далеко — «12 часов назад».
	void UpdateExit(float DeltaSeconds);
	void SpawnExit(const FIntPoint& NearChunk);
	void RelocateExitIfNeeded();
	void ApplyExitPressure(float DeltaSeconds);

	// Скрывать/показывать дальние чанки: экономим геометрию, свет и коллизии,
	// не пересоздавая мир. Гистерезис — чтобы граница не мерцала.
	void UpdateChunkVisibility();
	void InitGrid();
	// Рецентр окна на NewCenter; вернувшиеся из окна координаты — в OutEvicted.
	void RecentreGrid(const FIntPoint& NewCenter, TArray<FIntPoint>& OutEvicted);
	int32 GridIndex(const FIntPoint& C) const;
	bool GridHas(const FIntPoint& C) const;
	ABackroomsChunkActor* GridGet(const FIntPoint& C) const;
	void GridSet(const FIntPoint& C, ABackroomsChunkActor* Chunk);

	UPROPERTY()
	TArray<FChunkGridSlot> ChunkGrid;

	UPROPERTY()
	TObjectPtr<ABackroomsChunkActor> StartPlatform;

	UPROPERTY()
	TObjectPtr<ABackroomsRiggedMonster> SpawnedMonster;

	// Аудио-актёр уровня: применяет тему (музыка/петля амбиента).
	UPROPERTY()
	TObjectPtr<ABackroomsLevelAudioActor> LevelAudioActor;

	// Единственный выход уровня (один в мире, см. UpdateExit).
	UPROPERTY()
	TObjectPtr<ABackroomsExitActor> LevelExit;
	// Клетка, к которой привязан текущий выход (для поиска и переезда).
	FIntPoint ExitChunk = FIntPoint(MAX_int32, MAX_int32);
	// Намерение игрока: сглаженный вектор движения — по нему выход уходит вперёд.
	FVector2D TravelDir = FVector2D::ZeroVector;
	// Ступень «давления среды» и таймеры выхода.
	int32 ExitPressure = 0;
	float ExitPressureTimer = 0.0f;
	float ExitGraceTimer = 0.0f;

	// Триггер «люка» в центре стартовой платформы: зайдя в него, игрок
	// добровольно спускается в Бэкрумс (раньше спуск был автоматическим).
	UPROPERTY(VisibleAnywhere, Category = "SpawnPlatform")
	TObjectPtr<USphereComponent> StartupPlatformTrapdoor;

	TArray<FIntPoint> PendingOrder;
	TSet<FIntPoint> PendingSet;
	TObjectPtr<ULoadingBarWidget> LoadingBar;
	FIntPoint LastPlayerChunk = FIntPoint::ZeroValue;
	// Окно сетки: Stride = 2*RenderRadiusChunks+1, Off = RenderRadiusChunks.
	int32 GridStride = 0;
	int32 GridOff = 0;
	FIntPoint GridCenter = FIntPoint::ZeroValue;
	bool bInitialized = false;
	bool bPlatformDescentStarted = false;
	bool bPlatformDescentDone = false;
	bool bPlayerSeated = false;
	float StreamTimer = 0.0f;
	float LastActivityAt = 0.0f;
	float DescentTimer = 0.0f;
	float FallTimer = 0.0f;
	bool bFallTeleported = false;
	bool bPlayerFellToBackrooms = false;
	int32 TotalRequestedThisWave = 0;
	float PlatformCurrentZ = 0.0f;
	bool bAutoDump = false;
	float DumpTimer = 0.0f;
	// Сняли ли уже превью текущего уровня для меню уровней.
	bool bThumbnailTaken = false;
	// Сколько времени полоса загрузки уже показана (растёт от 0 до
	// MinLoadingBarTime, а после — «прозрачнее» не пускаем мир раньше времени).
	float LoadingBarTimer = 0.0f;

	// Загруженный материал стартовой площадки (M_CityFloor).
	UPROPERTY()
	TObjectPtr<UMaterial> CityFloorMaterial;

// Туман уровня: создаётся в SetupSkyAndWeather, но плотность/цвет обновляются
// из активного конфига уровня (ApplyLevelAtmosphere) при каждом SetLevel.
UPROPERTY()
TObjectPtr<AExponentialHeightFog> LevelFog;

// ---- FloorPlan: архитектурный конвейер нового уровня ----
// Режим FloorPlan определяется DataAsset-конфигом уровня (bUseFloorPlan).
// Чанки в этом режиме строятся ТОЛЬКО из плана региона; без плана — ошибка.
UPROPERTY()
bool bUseFloorPlanLevel = false;

// Кеш FloorPlan по регионам: регион = ChunkExtent (по умолчанию 8x8 чанков).
// План генерируется детерминированно из WorldSeed + координаты региона.
UPROPERTY()
TMap<FIntPoint, TObjectPtr<UBackroomsFloorPlan>> FloorPlanCache;

UBackroomsFloorPlan* GetOrCreateFloorPlan(const FIntPoint& ChunkCoord);

// Кеш построенного поумолчанию профиля: BuildDefaultByLevel создаёт новый
// объект каждый вызов — без кеша это утечка + несохранные правки уровня.
UPROPERTY()
TObjectPtr<ULevelGeneratorProfile> CachedProfile;
int32 CachedLevelIndex = -1;
uint8 CachedDifficulty = 255; // не совпадает ни с одной сложностью (enum-значение)
};

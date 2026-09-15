#include "BackroomsWorldGenerator.h"
#include "BackroomsCityChunkActor.h"
#include "BackroomsChunkActor.h"
#include "BackroomsRiggedMonster.h"
#include "BackroomsExitActor.h"
#include "BackroomsChunkCoord.h"
#include "BackroomsGenerationData.h"
#include "BackroomsGenerationVerifier.h"
#include "BackroomsAchievements.h"
#include "BackroomsLevelBook.h"
#include "BackroomsFloorPlan.h"
#include "BackroomsLevelThemeConfig.h"
#include "BackroomsLevelAudioActor.h"
#include "BackroomsDifficulty.h"
#include "BackroomsLocalization.h"
#include "PropDatabase.h"
#include "LevelGeneratorProfile.h"
#include "BackroomsLevelConfig.h"
#include "LoadingBarWidget.h"
#include "BackroomsSaveSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Components/CapsuleComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SoftObjectPath.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/GameUserSettings.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

// Явный отладочный флаг: 1 = принудительно использовать FloorPlan-конвейер для
// чанков Бэкрумса (bUseFloorPlanLevel=true), НЕЗАВИСИМО от конфига уровня и от
// LevelIndex. Нужен, чтобы включить FloorPlan для L0 без создания контентного
// /Game/Data/L0_Config (ассета нет). Не влияет на город: StartPlatform строится
// через SpawnStartPlatform (bSpawnPlatform=true) и до этой ветки не доходит.
static TAutoConsoleVariable<int32> CVarForceFloorPlan(
	TEXT("BR.FloorPlan.Force"),
	1,
	TEXT("1 = принудительно включить FloorPlan-конвейер для чанков Бэкрумса (debug)"),
	ECVF_Default);

ABackroomsWorldGenerator::ABackroomsWorldGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
	MonsterClass = ABackroomsRiggedMonster::StaticClass();
	ExitClass = ABackroomsExitActor::StaticClass();

	// «Люк»-триггер в центре стартовой платформы: зайдя в него, игрок спускается
	// в Бэкрумс. Включаем сфера с коллизией-оверлэпом только с павном (игроком).
	StartupPlatformTrapdoor = CreateDefaultSubobject<USphereComponent>(TEXT("StartupPlatformTrapdoor"));
	StartupPlatformTrapdoor->InitSphereRadius(120.0f);
	StartupPlatformTrapdoor->SetupAttachment(RootComponent);
	// Legacy hatch is inert. Entry is automatic after the hidden Backrooms wave.
	StartupPlatformTrapdoor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StartupPlatformTrapdoor->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UMaterial> CityMatFinder(TEXT("/Game/Materials/M_CityFloor"));
	if (CityMatFinder.Succeeded())
	{
		CityFloorMaterial = CityMatFinder.Object;
	}

	// Явный городской пул по умолчанию. Если в уровне задан свой список, он
	// сохраняется; иначе город не сваливается в процедурные коробки.
	if (CityBuildings.Num() == 0)
	{
		CityBuildings.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Buildings/Building-Big-Wide-01.Building-Big-Wide-01"))));
		CityBuildings.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Buildings/Building-Big-Wide-02.Building-Big-Wide-02"))));
		CityBuildings.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Buildings/Building-Medium-01.Building-Medium-01"))));
		CityBuildings.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Buildings/Building-Medium-02.Building-Medium-02"))));
		CityBuildings.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Buildings/Building-Small.Building-Small"))));
		CityBuildings.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Buildings/Skyscraper-01.Skyscraper-01"))));
		CityBuildings.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Buildings/Skyscraper-02.Skyscraper-02"))));
	}
}

void ABackroomsWorldGenerator::PreloadPropMeshes()
{
	// Загружаем ВСЕ меши пропсов из правил базы заранее. CachedLoadMesh у чанка
	// потом найдёт их уже загруженными — стриминг новых чанков не «зависает».
	if (!Database)
	{
		return;
	}
	for (const FPropRule& Rule : Database->Props)
	{
		if (!Rule.Mesh.IsNull())
		{
			Rule.Mesh.LoadSynchronous();
		}
	}

	// Городские здания тоже лучше загрузить заранее (стартовая платформа).
	for (const TSoftObjectPtr<UStaticMesh>& B : CityBuildings)
	{
		if (!B.IsNull())
		{
			B.LoadSynchronous();
		}
	}
}

ULevelGeneratorProfile* ABackroomsWorldGenerator::GetActiveProfile()
{
	if (LevelProfileOverride)
	{
		// Даже ручной профиль проходит через множители сложности, чтобы выбор
		// сложности действовал на любом уровне: генератор остаётся один.
		BackroomsDifficulty::ApplyToProfile(LevelProfileOverride, BackroomsDifficulty::GetCurrent());
		return LevelProfileOverride;
	}

	const EBackroomsDifficulty Diff = BackroomsDifficulty::GetCurrent();
	const uint8 DiffByte = (uint8)Diff;
	if (CachedProfile && CachedLevelIndex == LevelIndex && CachedDifficulty == DiffByte)
	{
		return CachedProfile;
	}

	// BuildDefaultByLevel создаёт новый UObject при каждом вызове: кешируем по
	// индексу уровня И сложности, чтобы не было утечки, а смена сложности
	// пересобирала профиль (и генерация реально менялась).
	CachedProfile = ULevelGeneratorProfile::BuildDefaultByLevel(LevelIndex);
	if (CachedProfile)
	{
		BackroomsDifficulty::ApplyToProfile(CachedProfile, Diff);
	}
	CachedLevelIndex = LevelIndex;
	CachedDifficulty = DiffByte;
	return CachedProfile;
}

FBackroomsLevelTheme ABackroomsWorldGenerator::GetActiveTheme() const
{
	// Приоритет: DataAsset конфига уровня (/Game/Data/L{L}_Config) → тема профиля
	// (BuildDefaultByLevel) → пустая тема-дефолт.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UBackroomsLevelBook* Book = GI->GetSubsystem<UBackroomsLevelBook>())
		{
			if (const UBackroomsLevelThemeConfig* Config = Book->GetConfig(LevelIndex))
			{
				return Config->Theme;
			}
		}
	}
	if (ULevelGeneratorProfile* Profile = const_cast<ABackroomsWorldGenerator*>(this)->GetActiveProfile())
	{
		return Profile->Theme;
	}
	return FBackroomsLevelTheme();
}

void ABackroomsWorldGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (Database)
	{
		// Загружаем меши пропсов и города заранее — один синхронный заход на старте
		// вместо фризов во время стриминга чанков (см. CachedLoadMesh у чанка).
		PreloadPropMeshes();
	}

	// Источник seed: URL-переход (OpenLevel "seed=N&skipmenu") > сейв > случайный.
	// На фиксированных картах уровень берём из имени карты: карта и есть уровень
	// (см. UBackroomsLevelBook::GetLevelIndexFromMapName).
	const FURL& Url = GetWorld()->URL;
	bool bSeedFromUrl = UBackroomsLevelBook::GetSeedFromUrl(Url, Seed);
	if (bFixedLevel)
	{
		const int32 MapLevel = UBackroomsLevelBook::GetLevelIndexFromMapName(Url.Map);
		if (MapLevel >= 0)
		{
			LevelIndex = MapLevel;
		}
	}

	UBackroomsSaveSubsystem* SaveSys = GetGameInstance() ? GetGameInstance()->GetSubsystem<UBackroomsSaveSubsystem>() : nullptr;
	if (!bSeedFromUrl && SaveSys && SaveSys->HasSave())
	{
		int32 SavedSeed = 0;
		int32 SavedLevel = 0;
		SaveSys->LoadProgress(SavedSeed, SavedLevel);
		Seed = SavedSeed;
		if (!bFixedLevel)
		{
			LevelIndex = SavedLevel;
		}
	}
	else if (!bSeedFromUrl)
	{
		Seed = FMath::RandRange(1, 9999999) ^ (int32)(FDateTime::Now().GetTicks() & 0x7fffffff);
		if (!bFixedLevel)
		{
			LevelIndex = 0;
		}
	}

	PlatformCurrentZ = SpawnPlatformAlt;
	SpawnStartPlatform();
	if (!StartPlatform)
	{
		UE_LOG(LogTemp, Warning, TEXT("BR: PLATFORM SPAWN FAILED"));
	}

	// Режим генерации уровня: он решается конфигом LevelThemeConfig, а не профилем.
	// bUseFloorPlan=true — чанки строятся ТОЛЬКО из FloorPlan (никакого legacy).
	// Конфига нет (дефолтный L0) → legacy/город как раньше; отладочный CVar
	// BR.FloorPlan.Force включает FloorPlan явно, не завязываясь на LevelIndex.
	bUseFloorPlanLevel = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UBackroomsLevelBook* Book = GI->GetSubsystem<UBackroomsLevelBook>())
		{
			if (const UBackroomsLevelThemeConfig* Config = Book->GetConfig(LevelIndex))
			{
				bUseFloorPlanLevel = Config->bUseFloorPlan;
			}
		}
	}
	if (CVarForceFloorPlan->GetInt() != 0)
	{
		bUseFloorPlanLevel = true;
	}
	UE_LOG(LogTemp, Display, TEXT("BR: BeginPlay floorplan mode = %s (LevelIndex=%d)"),
		bUseFloorPlanLevel ? TEXT("FLOORPLAN") : TEXT("legacy"), LevelIndex);
	SitPlayerOnPlatform(true);

	// Солнце, небо и облака — город стоит под открытым небом.
	SetupSkyAndWeather();

	// Аудио-актёр уровня: применяет тему (музыка/петля амбиента из конфига).
	if (!LevelAudioActor)
	{
		FActorSpawnParameters AudioParams;
		AudioParams.Name = TEXT("BackroomsLevelAudio");
		LevelAudioActor = GetWorld()->SpawnActor<ABackroomsLevelAudioActor>(
			ABackroomsLevelAudioActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, AudioParams);
	}
	if (LevelAudioActor)
	{
		LevelAudioActor->ApplyTheme(GetActiveTheme());
	}

	if (bShowLoadingBar)
	{
		TSubclassOf<ULoadingBarWidget> BarClass = LoadingBarClass;
		if (!BarClass)
		{
			BarClass = ULoadingBarWidget::StaticClass();
		}
		LoadingBar = CreateWidget<ULoadingBarWidget>(GetWorld(), BarClass);
		if (LoadingBar)
		{
			LoadingBar->AddToViewport(100);
			LoadingBar->SetProgress(0.0f);
			LoadingBar->SetLabel(BackroomsLoc::Get(TEXT("Loading.Level")));
		}
	}

// Чанки Бэкрумса генерируются на любой карте, включая городскую: под городом
	// скрытый слой Бэкрумса ждёт игрока (проваливание через люк).
	// Запускаем генерацию чанков Бэкрумса на Z=0 СРАЗУ (не ждём движение игрока).
	// Чанки строятся под платформой, пока игрок гуляет по городу.
	InitGrid();
	const FIntPoint Center(0, 0);
	for (int32 DX = -RenderRadiusChunks; DX <= RenderRadiusChunks; ++DX)
	{
		for (int32 DY = -RenderRadiusChunks; DY <= RenderRadiusChunks; ++DY)
		{
			const FIntPoint Coord(Center.X + DX, Center.Y + DY);
			if (!GridHas(Coord) && !PendingSet.Contains(Coord))
			{
				PendingOrder.Add(Coord);
				PendingSet.Add(Coord);
				++TotalRequestedThisWave;
			}
		}
	}
	LastActivityAt = GetWorld()->GetTimeSeconds();

	// Автодамп отчёта.
	int32 AutoDump = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("BRDump"), AutoDump) && AutoDump > 0)
	{
		bAutoDump = true;
	}
}

void ABackroomsWorldGenerator::SpawnStartPlatform()
{
	if (!bSpawnPlatform)
	{
		return;
	}

	// Геометрию стартовой платформы берём из активного профиля уровня
	// (профиль уже знает свой ULevelConfig — см. ApplyLevelConfig). Здесь нет
	// хардкод-размеров: любые правки размеров живут в YAML.
	if (ULevelGeneratorProfile* ActiveProfile = GetActiveProfile())
	{
		CellSize = ActiveProfile->CellSize;
		ChunkSizeCells = ActiveProfile->ChunkSizeCells;
		WallHeight = ActiveProfile->WallHeight;
		WallThickness = ActiveProfile->WallThickness;
		DoorWidth = ActiveProfile->DoorWidth;
		DoorHeight = ActiveProfile->DoorHeight;
	}

	FVector PlatformLoc(CellSize * ChunkSizeCells * 0.5f, CellSize * ChunkSizeCells * 0.5f, PlatformCurrentZ);
	StartPlatform = GetWorld()->SpawnActor<ABackroomsCityChunkActor>(ABackroomsCityChunkActor::StaticClass(), PlatformLoc, FRotator::ZeroRotator);
	if (!StartPlatform)
	{
		return;
	}

	StartPlatform->CellSize = CellSize;
	StartPlatform->WallHeight = WallHeight;
	StartPlatform->WallThickness = WallThickness;
	StartPlatform->DoorWidth = DoorWidth;
	StartPlatform->DoorHeight = DoorHeight;
	StartPlatform->PlatformSizeChunks = 5;
	StartPlatform->WallThreshold = WallThreshold;
	StartPlatform->DoorThreshold = DoorThreshold;
	StartPlatform->ScatterThreshold = ScatterThreshold;
	StartPlatform->Seed = Seed;
	// Поля мира (стены/двери/плотность/вид) — от глобального seed, иначе границы
	// соседних чанков расходятся и в стенах остаются «швы».
	StartPlatform->WorldSeed = Seed;
	StartPlatform->MaxPropsPerRoom = MaxPropsPerRoom;
	// Город-остров: верхний слой. Флаг должен остаться true после SetupFromProfile
	// (Setup больше не перезаписывает bSpawnPlatform из профиля).
	StartPlatform->bSpawnPlatform = true;
	if (PlatformDatabase)
	{
		StartPlatform->Database = PlatformDatabase;
	}
	else
	{
		StartPlatform->Database = nullptr;
	}
	StartPlatform->SetCityBuildings(CityBuildings);

	if (CityFloorMaterial)
	{
		StartPlatform->FloorMaterial = CityFloorMaterial;
	}

	ULevelGeneratorProfile* Profile = GetActiveProfile();
	if (Profile)
	{
		// SetupFromProfile больше не сбрасывает bSpawnPlatform — город остаётся городом.
		StartPlatform->bSpawnPlatform = true;
		StartPlatform->LevelStyle = (EBackroomsLevelStyle)FMath::Clamp(LevelIndex, 0, 9);
		StartPlatform->BuildChunkFromProfile(0, 0, (int32)BackroomsChunkSeed(Seed, LevelIndex, FChunkCoord(0, 0)), Profile);
	}
	else
	{
		StartPlatform->bSpawnPlatform = true;
		StartPlatform->BuildChunk(0, 0, (int32)BackroomsChunkSeed(Seed, LevelIndex, FChunkCoord(0, 0)), StartPlatform->Database);
	}
	// Страховка: если кто-то снова начнёт копировать флаг из профиля.
	StartPlatform->bSpawnPlatform = true;
}

void ABackroomsWorldGenerator::SetupSkyAndWeather()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Город живёт под открытым небом: атмосфера, облака и солнце нужны только
	// на уровне 0. В Бэкрумсе (уровни 1+) небо закрыто потолками, а DirectionalLight
	// рисует «игровые» тени и ломает атмосферу подвалов — гасим его полностью.
	const bool bIsCity = (LevelIndex == 0);

	// --- Небесная атмосфера: синее небо и солнечный диск (только город). ---
	if (bIsCity)
	{
		if (!TActorIterator<ASkyAtmosphere>(World))
		{
			World->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass(),
				FVector::ZeroVector, FRotator::ZeroRotator);
		}
	}
	else
	{
		for (TActorIterator<ASkyAtmosphere> It(World); It; ++It)
		{
			It->Destroy();
		}
		for (TActorIterator<AVolumetricCloud> It(World); It; ++It)
		{
			It->Destroy();
		}
		for (TActorIterator<ADirectionalLight> It(World); It; ++It)
		{
			It->Destroy();
		}
	}

	// --- Объёмные облака: купол облаков над городом (только город). ---
	if (bIsCity)
	{
		if (!TActorIterator<AVolumetricCloud>(World))
		{
			World->SpawnActor<AVolumetricCloud>(AVolumetricCloud::StaticClass(),
				FVector::ZeroVector, FRotator::ZeroRotator);
		}
	}

	// --- Солнце: настраиваем существующий DirectionalLight или создаём новый. ---
	if (bIsCity)
	{
		ADirectionalLight* Sun = nullptr;
		for (TActorIterator<ADirectionalLight> It(World); It; ++It)
		{
			Sun = *It;
			break;
		}
		if (!Sun)
		{
			Sun = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(),
				FVector(0.0f, 0.0f, 100000.0f), FRotator(-42.0f, 35.0f, 0.0f));
		}
		if (Sun)
		{
			Sun->SetMobility(EComponentMobility::Movable);
			Sun->SetActorRotation(FRotator(-42.0f, 35.0f, 0.0f));
			if (UDirectionalLightComponent* SunComp = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
			{
				SunComp->SetMobility(EComponentMobility::Movable);
				SunComp->SetIntensity(6.0f);
				SunComp->SetLightColor(FLinearColor(1.0f, 0.96f, 0.86f));
				SunComp->bAtmosphereSunLight = true;
				SunComp->SetCastShadows(true);
			}
		}
	}

	// --- Небесный свет: мягкая заливка от неба (в подвалах его гасит потолок). ---
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		ASkyLight* Sky = *It;
		if (USkyLightComponent* SkyComp = Sky->GetLightComponent())
		{
			SkyComp->SetMobility(EComponentMobility::Movable);
			SkyComp->SetRealTimeCapture(true);
			SkyComp->SetIntensity(1.0f);
			SkyComp->SetLowerHemisphereColor(FLinearColor(0.03f, 0.03f, 0.04f));
		}
		break;
	}

	// --- Лёгкий туман: глубина города, облачная дымка у горизонта. ---
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		LevelFog = *It;
		break;
	}
	if (!LevelFog)
	{
		LevelFog = World->SpawnActor<AExponentialHeightFog>(
			AExponentialHeightFog::StaticClass(), FVector(0.0f, 0.0f, -200.0f), FRotator::ZeroRotator);
	}
	ApplyLevelAtmosphere();

	UE_LOG(LogTemp, Display, TEXT("BR Sky: atmosphere/clouds/sun/fog configured"));
}

void ABackroomsWorldGenerator::ApplyLevelAtmosphere()
{
	if (!LevelFog)
	{
		return;
	}
	UExponentialHeightFogComponent* FogComp = LevelFog->GetComponent();
	if (!FogComp)
	{
		return;
	}

	// Quality gate: volume fog only on mid+ presets (same as before).
	const int32 Quality = GEngine && GEngine->GetGameUserSettings()
		? FMath::Clamp(GEngine->GetGameUserSettings()->GetOverallScalabilityLevel(), 0, 3)
		: 2;
	const bool bHeavyFog = Quality >= 2;

	// Default: warm pale fog for backrooms (lobby, offices…).
	// Плотность притушена: в коридорах Exponential Fog читается «дымом» на
	// дистанции в пару метров, а лиминальный жёлтый тон даёт Lumen-отражения
	// (IndirectLightingIntensity в PP). Волюметрик-часть оставляет только
	// световые колонны у ламп (God Rays, см. SpawnRoomLights).
	FLinearColor FogColor(0.62f, 0.60f, 0.58f);
	float Density = 0.0025f;

	const UBackroomsLevelConfig* Cfg = nullptr;
	if (ULevelGeneratorProfile* P = GetActiveProfile())
	{
		Cfg = P->LevelConfig;
	}
	if (Cfg)
	{
		if (Cfg->Water.bEnabled)
		{
			// Водные уровни (L13/L16): цвет тумана из water.color, плотнее.
			FogColor = Cfg->Water.Color;
			Density = 0.010f;
		}
		else if (Cfg->Sky.bEnabled)
		{
			// Открытые уровни (L11/L14/L15): лёгкая дымка у горизонта.
			FogColor = FLinearColor(0.72f, 0.78f, 0.88f);
			Density = 0.004f;
		}
	}

	FogComp->SetFogInscatteringColor(FogColor);
	FogComp->SetFogDensity(bHeavyFog ? Density : Density * 0.5f);
	FogComp->SetFogHeightFalloff(0.2f);
	// Включить волюметрик со стабильным распределением: лучи от ламп за
	// счёт VolumetricScatteringIntensity на RectLight-источниках.
	FogComp->SetVolumetricFog(bHeavyFog);
	if (bHeavyFog)
	{
		// Старт волюметрика за ~4 м от камеры: ближние комнаты остаются
		// «чистыми», дымка появляется в глубине коридора (меньше «дыма» в лицо).
		FogComp->SetVolumetricFogStartDistance(400.0f);
		FogComp->SetVolumetricFogScatteringDistribution(0.30f);
		FogComp->SetVolumetricFogDistance(6000.0f);
	}
}

FString ABackroomsWorldGenerator::GetActiveAmbientLayer() const
{
	const UBackroomsLevelConfig* Cfg = nullptr;
	// GetActiveProfile mutates cache → non-const. Cast away for a read-only getter
	// (the cache write is harmless and idempotent here).
	if (ULevelGeneratorProfile* P = const_cast<ABackroomsWorldGenerator*>(this)->GetActiveProfile())
	{
		Cfg = P->LevelConfig;
	}
	return Cfg ? Cfg->Acoustics.AmbientLayer : TEXT("lamp_hum");
}

FString ABackroomsWorldGenerator::GetActiveReverbScene() const
{
	const UBackroomsLevelConfig* Cfg = nullptr;
	if (ULevelGeneratorProfile* P = const_cast<ABackroomsWorldGenerator*>(this)->GetActiveProfile())
	{
		Cfg = P->LevelConfig;
	}
	return Cfg ? Cfg->Acoustics.Reverb : TEXT("office");
}

bool ABackroomsWorldGenerator::GetActiveEchoEnabled() const
{
	const UBackroomsLevelConfig* Cfg = nullptr;
	if (ULevelGeneratorProfile* P = const_cast<ABackroomsWorldGenerator*>(this)->GetActiveProfile())
	{
		Cfg = P->LevelConfig;
	}
	return Cfg ? Cfg->Acoustics.bEcho : false;
}

float ABackroomsWorldGenerator::GetActiveLightRadius() const
{
	const UBackroomsLevelConfig* Cfg = nullptr;
	float CellSizeCm = 500.0f;
	if (ULevelGeneratorProfile* P = const_cast<ABackroomsWorldGenerator*>(this)->GetActiveProfile())
	{
		Cfg = P->LevelConfig;
		CellSizeCm = P->CellSize;
	}
	// Та же формула, что SpawnRoomLights использует для AttenuationRadius
	// (луч 0.55 шага сетки панелей, шаг от lights.every_m, минимум 2 клетки).
	const float EveryM = (Cfg && Cfg->Lights.EveryM > 0.0f) ? Cfg->Lights.EveryM : 5.0f;
	const int32 GridCells = FMath::Max(2,
		FMath::RoundToInt(EveryM * 100.0f / FMath::Max(1.0f, CellSizeCm)));
	return FMath::Max(200.0f, (float)GridCells * CellSizeCm * 0.55f);
}

float ABackroomsWorldGenerator::GetActiveRoomCellSize() const
{
	if (ULevelGeneratorProfile* P = const_cast<ABackroomsWorldGenerator*>(this)->GetActiveProfile())
	{
		return FMath::Max(1.0f, P->CellSize);
	}
	return 500.0f;
}

void ABackroomsWorldGenerator::SitPlayerOnPlatform(bool bSnap)
{
	if (bPlayerFellToBackrooms)
	{
		return;
	}

	// Если город-платформа не построена (выключена или спавн упал) — сажаем
	// игрока сразу на пол стартовой комнаты Бэкрумса (Z=0), а не на верх
	// потолка чанка (0,0), где прежде оказывался игрок «на потолке».
	const bool bHasPlatform = (StartPlatform && bSpawnPlatform);
	const float FallbackZ = bHasPlatform ? PlatformCurrentZ : 0.0f;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn)
	{
		return;
	}

	// Центр стартовой комнаты (шпаргалка: PlayerSpawner).
	const float CenterX = CellSize * ChunkSizeCells * 0.5f;
	const float CenterY = CellSize * ChunkSizeCells * 0.5f;
	const float StandZ = PawnStandZ();

	// 1) Опускаем на пол: трассировка вниз от точки над спавном находит
	// верх плиты (3000) или пол Бэкрумса (0).
	float FloorZ = FallbackZ;
	{
		FCollisionQueryParams QP(SCENE_QUERY_STAT(SeatLineTrace), false, Pawn);
		const float ScanTop = FallbackZ + 700.0f;
		const float ScanDepth = bHasPlatform ? 900.0f : 1500.0f;
		const FVector Start(CenterX, CenterY, ScanTop);
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, Start - FVector(0.0f, 0.0f, ScanDepth), ECC_WorldStatic, QP))
		{
			FloorZ = Hit.ImpactPoint.Z;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("BR: SitPlayerOnPlatform PawnLoc=(%s) FloorZ=%.1f"), *Pawn->GetActorLocation().ToString(), FloorZ);

	// 2) Проверка «не в стене» + ближайшее свободное место (FindEmptySpot).
	// Убедимся, что FloorZ не ниже 0 (для Backrooms) и не выше PlatformCurrentZ (для города).
	const float MinZ = bHasPlatform ? PlatformCurrentZ * 0.5f : 0.0f; // безопасный минимум
	const float MaxZ = bHasPlatform ? PlatformCurrentZ : 1000.0f; // безопасный максимум
	FVector SeatPos(CenterX, CenterY, FMath::Clamp(FloorZ, MinZ, MaxZ) + StandZ);
	{
		const float Radius = FMath::Max(Pawn->GetSimpleCollisionRadius(), 20.0f);
		const FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
		auto Blocked = [&](const FVector& P) -> bool
		{
			FCollisionQueryParams QP(SCENE_QUERY_STAT(SeatOverlap), false, Pawn);
			return GetWorld()->OverlapAnyTestByChannel(
				P + FVector(0.0f, 0.0f, 10.0f),
				FQuat::Identity,
				ECC_WorldStatic,
				Shape,
				QP);
		};

		if (Blocked(SeatPos))
		{
			bool bFound = false;
			for (int32 Ring = 1; Ring <= 4 && !bFound; ++Ring)
			{
				for (int32 Angle = 0; Angle < 360 && !bFound; Angle += 45)
				{
					const float R = 140.0f * (float)Ring;
					const FVector P(
						CenterX + FMath::Cos(FMath::DegreesToRadians(Angle)) * R,
						CenterY + FMath::Sin(FMath::DegreesToRadians(Angle)) * R,
						FloorZ + StandZ);
					if (!Blocked(P))
					{
						SeatPos = P;
						bFound = true;
					}
				}
			}
		}
	}

	Pawn->SetActorLocation(SeatPos);
	Pawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
	if (ACharacter* Char = Cast<ACharacter>(Pawn))
	{
		if (UCharacterMovementComponent* Movement = Char->GetCharacterMovement())
		{
			Movement->SetMovementMode(MOVE_Walking);
		}
	}
	bPlayerSeated = true;

	if (StartupPlatformTrapdoor)
	{
		// «Люк»-триггер — видимая точка на дороге, в ~16 м от точки спавна
		// (заодно не срабатывает моментально на респавне в центре платформы).
		StartupPlatformTrapdoor->SetWorldLocation(FVector(
			CenterX + CellSize * 2.0f,
			CenterY,
			FallbackZ + 20.0f));
		StartupPlatformTrapdoor->SetWorldRotation(FRotator::ZeroRotator);
	}
}

void ABackroomsWorldGenerator::PinPlayerToPlatform()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC && PC->GetPawn())
	{
		FVector Pos = PC->GetPawn()->GetActorLocation();
		Pos.Z = PlatformCurrentZ + PawnStandZ();
		PC->GetPawn()->SetActorLocation(Pos);
	}
}

float ABackroomsWorldGenerator::PawnStandZ() const
{
	if (const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (const ACharacter* Char = Cast<ACharacter>(PC->GetPawn()))
		{
			const UCapsuleComponent* Capsule = Char->GetCapsuleComponent();
			if (Capsule)
			{
				return Capsule->GetScaledCapsuleHalfHeight() + 5.0f;
			}
		}
	}
	return 100.0f;
}

FIntPoint ABackroomsWorldGenerator::WorldToChunk(const FVector& WorldPos) const
{
	const float ChunkExtent = CellSize * (float)ChunkSizeCells;
	return FIntPoint(
		FMath::FloorToInt(WorldPos.X / ChunkExtent),
		FMath::FloorToInt(WorldPos.Y / ChunkExtent));
}

void ABackroomsWorldGenerator::InitGrid()
{
	GridStride = 2 * RenderRadiusChunks + 1;
	GridOff = RenderRadiusChunks;
	GridCenter = FIntPoint::ZeroValue;
	ChunkGrid.SetNum(GridStride * GridStride);
	ChunkGrid.Init(FChunkGridSlot(), GridStride * GridStride);
}

int32 ABackroomsWorldGenerator::GridIndex(const FIntPoint& C) const
{
	if (GridStride <= 0 || GridOff <= 0)
	{
		return INDEX_NONE;
	}
	const int32 Row = C.Y - GridCenter.Y + GridOff;
	const int32 Col = C.X - GridCenter.X + GridOff;
	if (Row < 0 || Row >= GridStride || Col < 0 || Col >= GridStride)
	{
		return INDEX_NONE;
	}
	return Row * GridStride + Col;
}

bool ABackroomsWorldGenerator::GridHas(const FIntPoint& C) const
{
	const int32 Idx = GridIndex(C);
	return Idx != INDEX_NONE && ChunkGrid[Idx].Chunk != nullptr;
}

ABackroomsChunkActor* ABackroomsWorldGenerator::GridGet(const FIntPoint& C) const
{
	const int32 Idx = GridIndex(C);
	if (Idx == INDEX_NONE)
	{
		return nullptr;
	}
	return ChunkGrid[Idx].Chunk.Get();
}

void ABackroomsWorldGenerator::GridSet(const FIntPoint& C, ABackroomsChunkActor* Chunk)
{
	const int32 Idx = GridIndex(C);
	if (Idx == INDEX_NONE)
	{
		if (Chunk)
		{
			Chunk->Destroy();
		}
		return;
	}
	ChunkGrid[Idx].Coord = C;
	ChunkGrid[Idx].Chunk = Chunk;
}

void ABackroomsWorldGenerator::RecentreGrid(const FIntPoint& NewCenter, TArray<FIntPoint>& OutEvicted)
{
	OutEvicted.Reset();
	if (GridStride <= 0)
	{
		InitGrid();
	}
	if (GridCenter == NewCenter)
	{
		return;
	}

	// Старые слоты живут в сетке с прежним центром: сначала снимок, потом
	// перепривязка по новому центру (своп тайлов местами, как в match-3).
	TArray<FChunkGridSlot> Old = ChunkGrid;
	ChunkGrid.SetNum(GridStride * GridStride);
	ChunkGrid.Init(FChunkGridSlot(), GridStride * GridStride);
	GridCenter = NewCenter;

	for (const FChunkGridSlot& S : Old)
	{
		if (!S.Chunk)
		{
			continue;
		}
		const int32 Idx = GridIndex(S.Coord);
		if (Idx == INDEX_NONE)
		{
			S.Chunk->Destroy();
			OutEvicted.Add(S.Coord);
		}
		else if (!ChunkGrid[Idx].Chunk)
		{
			ChunkGrid[Idx] = S;
		}
		else if (ChunkGrid[Idx].Chunk != S.Chunk)
		{
			S.Chunk->Destroy();
		}
	}
}

void ABackroomsWorldGenerator::UpdateLoadingBar()
{
	if (!LoadingBar)
	{
		return;
	}

	const int32 Pending = PendingOrder.Num();
	const int32 Total = TotalRequestedThisWave;
	float RealPercent = Total > 0 ? float(Total - Pending) / float(Total) : 1.0f;
	RealPercent = FMath::Clamp(RealPercent, 0.0f, 1.0f);
	const bool bReady = Pending <= 0;
	if (!bReady)
	{
		LoadingBar->SetProgress(RealPercent);
		LoadingBar->SetLabel(FString::Printf(TEXT("%s %d%% (%s: %d)"),
			*BackroomsLoc::Get(TEXT("Loading.Backrooms")),
			FMath::RoundToInt(RealPercent * 100.0f),
			*BackroomsLoc::Get(TEXT("Loading.Chunks")), Pending));
		return;
	}

	LoadingBar->SetProgress(1.0f);
	LoadingBar->SetLabel(BackroomsLoc::Get(TEXT("Loading.WorldReady")));

	// ФИКС: раньше здесь автоматически стартовал провал под текстуры (StartDescent)
	// в момент, когда фоновая генерация чанков Бэкрумса под городом заканчивалась —
	// то есть игрока выдёргивало в Бэкрумс посреди прогулки по городу, независимо
	// от того, где он физически стоял. Генератору не обязательно быть мгновенным:
	// чанки достраиваются в фоне, пока игрок свободно бегает по городу, а спуск
	// начинается ТОЛЬКО когда игрок сам наступает на люк — см. OnTrapdoorOverlap.
}

UBackroomsFloorPlan* ABackroomsWorldGenerator::GetOrCreateFloorPlan(const FIntPoint& ChunkCoord)
{
	// Регион — это блок ChunkExtent×ChunkExtent чанков. FloorDivision:
	// floor (а не truncation), чтобы отрицательные координаты считались верно
	// (чunk -1 относится к региону -1, а не 0).
	const int32 ExtX = 8; // FloorPlan::ChunkExtent (по умолчанию 8x8 чанков)
	const int32 ExtY = 8;
	const FIntPoint RegionCoord(
		FMath::FloorToInt((float)ChunkCoord.X / (float)ExtX),
		FMath::FloorToInt((float)ChunkCoord.Y / (float)ExtY));

	if (TObjectPtr<UBackroomsFloorPlan>* Found = FloorPlanCache.Find(RegionCoord))
	{
		return *Found;
	}

	if (ULevelGeneratorProfile* Profile = GetActiveProfile())
	{
		UBackroomsFloorPlan* Plan = NewObject<UBackroomsFloorPlan>(this);
		Plan->CellSizeWorld = CellSize;
		Plan->ChunkCellSize = ChunkSizeCells;
		Plan->RegionSizeCells = ChunkSizeCells * ExtX;
		Plan->Generate(Seed, RegionCoord);
		FloorPlanCache.Add(RegionCoord, Plan);
		return Plan;
	}

	return nullptr;
}

void ABackroomsWorldGenerator::SpawnPending()
{
	int32 Spawned = 0;
	while (PendingOrder.Num() > 0 && Spawned < MaxChunksPerTick)
	{
		const FIntPoint Coord = PendingOrder[0];
		PendingOrder.RemoveAt(0);
		PendingSet.Remove(Coord);

		if (GridHas(Coord))
		{
			continue;
		}

		// Геометрия строится в МИРОВЫХ координатах (WX*CellSize), поэтому актор
		// чанка всегда стоит в начале координат — иначе чанки (через один) дают
		// двойное смещение: (0,0) сходится, а (1,0) съезжает ещё на целый чанк.
		ABackroomsChunkActor* Chunk = GetWorld()->SpawnActor<ABackroomsChunkActor>(ABackroomsChunkActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Chunk)
		{
			// Нижний слой — только Бэкрумс. Город (деревья/здания/дороги) строится
			// один раз на StartPlatform на высоте SpawnPlatformAlt.
			Chunk->bSpawnPlatform = false;
			Chunk->Database = Database;
			Chunk->WorldSeed = Seed;
			Chunk->WallThreshold = WallThreshold;
			Chunk->DoorThreshold = DoorThreshold;
			Chunk->ScatterThreshold = ScatterThreshold;
			ULevelGeneratorProfile* Profile = GetActiveProfile();
			if (bUseFloorPlanLevel)
			{
				// НОВЫЙ КОНВЕЙЕР: чанк берёт архитектуру из FloorPlan региона.
				// План детерминирован: Seed + RegionCoord → один и тот же результат.
				if (UBackroomsFloorPlan* Plan = GetOrCreateFloorPlan(Coord))
				{
					Chunk->bUseFloorPlan = true;
					Chunk->AssignFloorPlan(Plan);
					Chunk->LevelStyle = (EBackroomsLevelStyle)FMath::Clamp(LevelIndex, 0, 9);
					Chunk->BuildChunkFromProfile(Coord.X, Coord.Y,
						(int32)BackroomsChunkSeed(Seed, LevelIndex, FChunkCoord(Coord.X, Coord.Y)), Profile);
					GridSet(Coord, Chunk);
					++Spawned;
					continue;
				}
				// План не создался — пропускаем чанк (нет фолбэка в legacy).
				continue;
			}
			if (Profile)
			{
				// Своя витрина видов комнат у уровня: EBackroomsLevelStyle покрывает
				// L0..L9 (таблицы FRoomVariantSystem::GetLevelTable), старшие — L0.
				Chunk->LevelStyle = (EBackroomsLevelStyle)FMath::Clamp(LevelIndex, 0, 9);
				Chunk->BuildChunkFromProfile(Coord.X, Coord.Y, (int32)BackroomsChunkSeed(Seed, LevelIndex, FChunkCoord(Coord.X, Coord.Y)), Profile);
			}
			else
			{
				Chunk->BuildChunk(Coord.X, Coord.Y, (int32)BackroomsChunkSeed(Seed, LevelIndex, FChunkCoord(Coord.X, Coord.Y)), Database);
			}
			GridSet(Coord, Chunk);

			// Кросс-чанковый граф комнат: legacy (только для уровней без FloorPlan).
			const FIntPoint Dir4[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
			const FChunkGenerationData* Neighbors[4] = { nullptr, nullptr, nullptr, nullptr };
			for (int32 d = 0; d < 4; ++d)
			{
				if (ABackroomsChunkActor* N = GridGet(Coord + Dir4[d]))
				{
					Neighbors[d] = N->GetGenData();
				}
			}
			Chunk->BuildRoomGraph(Neighbors);
		}
		++Spawned;
	}
}

void ABackroomsWorldGenerator::TrySpawnMonsterAfterGeneration()
{
	if (!bSpawnMonsterAfterInitialGeneration || SpawnedMonster || !bPlayerFellToBackrooms ||
		PendingOrder.Num() > 0 || !MonsterClass || !GetWorld())
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	APawn* Player = PC ? PC->GetPawn() : nullptr;
	if (!Player)
	{
		return;
	}

	// Детерминированный выбор направлений от seed: один и тот же мир даёт
	// одинаковую стартовую угрозу, но не ставит её в комнате игрока.
	FRandomStream Random(Seed ^ (LevelIndex * 486187739) ^ 0x4D4F4E53);
	const float CapsuleHalfHeight = 88.0f;
	const float MinDistance = FMath::Max(800.0f, MonsterSpawnMinDistance);
	const float MaxDistance = FMath::Max(MinDistance + 400.0f, MonsterSpawnMaxDistance);

	for (int32 Attempt = 0; Attempt < 16; ++Attempt)
	{
		const float Angle = Random.FRandRange(0.0f, 2.0f * PI);
		const float Distance = Random.FRandRange(MinDistance, MaxDistance);
		const FVector CandidateXY = Player->GetActorLocation() + FVector(
			FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);

		FHitResult FloorHit;
		FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(MonsterSpawnFloor), false, Player);
		// ФИКС ("монстр летает боком в небе"): трасса шла СНИЗУ ВВЕРХ (от -200
		// до потолка). У процедурного пола коллизия односторонняя (бэкфейсы не
		// блокируют), поэтому луч чаще всего проходил насквозь через пол и первым
		// делом попадал в ПОТОЛОК — монстр спавнился НАД потолком комнаты, в
		// пустоте над геометрией чанка. Там нет навмеша, и CharacterMovement в
		// состоянии Falling + горизонтальный AI-инпут давали то самое "плывущее
		// боком в небе" движение. Правильный порядок — трассировать СВЕРХУ ВНИЗ,
		// как в любом стандартном floor-trace, чтобы первым хитом всегда был
		// верх пола (или предмет на нём), а не изнанка потолка.
		const FVector TraceStart(CandidateXY.X, CandidateXY.Y, WallHeight + 200.0f);
		const FVector TraceEnd(CandidateXY.X, CandidateXY.Y, -200.0f);
		if (!GetWorld()->LineTraceSingleByChannel(FloorHit, TraceStart, TraceEnd, ECC_WorldStatic, TraceParams))
		{
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		const FVector SpawnLocation = FloorHit.ImpactPoint + FVector(0.0f, 0.0f, CapsuleHalfHeight + 2.0f);
		SpawnedMonster = GetWorld()->SpawnActor<ABackroomsRiggedMonster>(MonsterClass, SpawnLocation,
			FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f), SpawnParams);
		if (SpawnedMonster)
		{
			if (UBackroomsAchievements* Ach = GetAchievements())
			{
				Ach->NotifyMonsterEncountered();
			}
			return;
		}
	}
}

void ABackroomsWorldGenerator::UpdateChunkVisibility()
{
	// Скрываем дальние чанки, чтобы не тратить кадр на геометрию, свет и
	// коллизии вне воспринимаемой зоны. Чанки НЕ разрушаются — при возврате
	// игрока они мгновенно появляются (генерация не повторяется).
	// Гистерезис: внутрь видимого радиуса — быстро, наружу — с запасом, иначе
	// на границе чанк мерцает каждый шаг.
	const float ShowSq = VisibleRadiusChunks * VisibleRadiusChunks;
	const float KeepSq = (VisibleRadiusChunks + 0.5f) * (VisibleRadiusChunks + 0.5f);

	for (FChunkGridSlot& Slot : ChunkGrid)
	{
		ABackroomsChunkActor* Chunk = Slot.Chunk.Get();
		if (!Chunk)
		{
			continue;
		}
		const FIntPoint Delta = Slot.Coord - GridCenter;
		const float DistSq = float(Delta.X * Delta.X + Delta.Y * Delta.Y);
		const bool bCurrentlyVisible = !Chunk->IsHidden();
		if (bCurrentlyVisible)
		{
			if (DistSq > KeepSq)
			{
				Chunk->SetActorHiddenInGame(true);
				Chunk->SetActorEnableCollision(false);
			}
		}
		else if (DistSq <= ShowSq)
		{
			Chunk->SetActorHiddenInGame(false);
			Chunk->SetActorEnableCollision(true);
		}
	}
}

void ABackroomsWorldGenerator::SpawnExit(const FIntPoint& NearChunk)
{
	if (!GetWorld() || !ExitClass)
	{
		return;
	}

	// Выход ставим в построенном чанке по направлению движения игрока. Никогда
	// не в клетке игрока: «дверь под носом» бессмысленна.
	FRandomStream Random(Seed ^ (LevelIndex * 40503) ^ 0x45584954);

	// Направление: куда игрок идёт; если стоит — случайный сектор.
	FVector2D Dir = TravelDir;
	if (Dir.SizeSquared() < 0.01f)
	{
		const float Angle = Random.FRandRange(0.0f, 2.0f * PI);
		Dir = FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
	}
	Dir.Normalize();

	const int32 Steps = FMath::Max(1, FMath::RoundToInt(ExitSpawnDistanceChunks));
	FIntPoint Target(
		NearChunk.X + FMath::RoundToInt(Dir.X * Steps),
		NearChunk.Y + FMath::RoundToInt(Dir.Y * Steps));

	// Ищем готовый чанк рядом с целью (по спирали): выход нельзя ставить в
	// ещё не сгенерированное место.
	ABackroomsChunkActor* HostChunk = nullptr;
	const int32 SearchRadius = RenderRadiusChunks + 1;
	for (int32 R = 0; R <= SearchRadius && !HostChunk; ++R)
	{
		for (int32 DX = -R; DX <= R && !HostChunk; ++DX)
		{
			for (int32 DY = -R; DY <= R && !HostChunk; ++DY)
			{
				if (FMath::Max(FMath::Abs(DX), FMath::Abs(DY)) != R)
				{
					continue;
				}
				const FIntPoint C(Target.X + DX, Target.Y + DY);
				if (C == NearChunk)
				{
					continue;
				}
				HostChunk = GridGet(C);
				if (HostChunk)
				{
					Target = C;
				}
			}
		}
	}
	if (!HostChunk)
	{
		return;
	}

	// Точка в комнате около центра клетки — на полу уровня. Локальные 3..5 из 8
	// держат выход ближе к середине чанка, а не в стене у края.
	const int32 LocalX = 3 + Random.RandRange(0, 2);
	const int32 LocalY = 3 + Random.RandRange(0, 2);
	const FVector Pos(
		((float)Target.X * ChunkSizeCells + LocalX + 0.5f) * CellSize,
		((float)Target.Y * ChunkSizeCells + LocalY + 0.5f) * CellSize,
		0.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	LevelExit = GetWorld()->SpawnActor<ABackroomsExitActor>(ExitClass, Pos, FRotator(0.0f, 0.0f, 0.0f), Params);
	if (LevelExit)
	{
		LevelExit->ConfigureExit(GetNextLevelIndex());
		ExitChunk = Target;
	}
}

void ABackroomsWorldGenerator::RelocateExitIfNeeded()
{
	if (!LevelExit)
	{
		return;
	}
	if (ExitChunk.X == MAX_int32)
	{
		return;
	}

	// Если игрок ушёл дальше разумного — выход «переезжает» вперёд по движению,
	// его нельзя заставить идти 12 часов назад. Но и не двигаем его, пока игрок
	// рядом с ним: тогда исчезновение выхода прямо перед носом выглядело бы
	// багом (пока игрок не решил уйти сам — выход стоит).
	const FIntPoint PlayerChunk = GridCenter;
	const FIntPoint Delta = ExitChunk - PlayerChunk;
	const float DistChunks = FMath::Sqrt(float(Delta.X * Delta.X + Delta.Y * Delta.Y));

	if (DistChunks > ExitSpawnDistanceChunks * 2.0f + 2.0f)
	{
		LevelExit->Destroy();
		LevelExit = nullptr;
		ExitChunk = FIntPoint(MAX_int32, MAX_int32);
		SpawnExit(PlayerChunk);
	}
}

void ABackroomsWorldGenerator::ApplyExitPressure(float DeltaSeconds)
{
	if (!LevelExit)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	APawn* Player = PC ? PC->GetPawn() : nullptr;
	if (!Player)
	{
		return;
	}

	// У выхода давление замирает и спадает: близость к выходу — «спасение».
	const float Dist = FVector::Dist(Player->GetActorLocation(), LevelExit->GetActorLocation());
	if (Dist < ExitSafeRadius)
	{
		ExitPressureTimer = FMath::Max(0.0f, ExitPressureTimer - DeltaSeconds * 2.0f);
		ExitGraceTimer = FMath::Max(ExitGraceTimer, ExitGracePeriod);
		if (ExitPressure > 0 && ExitPressureTimer <= 0.0f)
		{
			--ExitPressure;
		}
		return;
	}

	// Мир даёт спокойно осмотреться первые ExitGracePeriod секунд, затем
	// начинает давить: избегание выхода = рост угрозы.
	if (ExitGraceTimer > 0.0f)
	{
		ExitGraceTimer -= DeltaSeconds;
		return;
	}

	ExitPressureTimer += DeltaSeconds;
	if (ExitPressureTimer >= ExitPressureInterval && ExitPressure < MaxExitPressure)
	{
		ExitPressureTimer = 0.0f;
		++ExitPressure;
		if (UBackroomsAchievements* Ach = GetAchievements())
		{
			Ach->SetStatMax(UBackroomsAchievements::StatMaxPressure, (float)ExitPressure);
		}
		UE_LOG(LogTemp, Display, TEXT("BR: exit pressure -> %d (level %d)"), ExitPressure, LevelIndex);
	}
}

int32 ABackroomsWorldGenerator::GetNextLevelIndex() const
{
	// Список порядка локаций: находим текущий и берём следующий. Если список
	// пуст или текущего в нём нет — просто +1 (безопасный фолбэк).
	if (LevelProgression.Num() > 0)
	{
		const int32 Pos = LevelProgression.Find(LevelIndex);
		if (Pos != INDEX_NONE && Pos + 1 < LevelProgression.Num())
		{
			return LevelProgression[Pos + 1];
		}
	}
	return LevelIndex + 1;
}

bool ABackroomsWorldGenerator::GetExitLocation(FVector& OutLocation) const
{
	if (!LevelExit)
	{
		return false;
	}
	OutLocation = LevelExit->GetActorLocation();
	return true;
}

void ABackroomsWorldGenerator::UpdateExit(float DeltaSeconds)
{
	// Мир считается «готовым» только когда игрок реально в Бэкрумсе и стартовая
	// волна построена. До этого выход не нужен.
	if (!bPlayerFellToBackrooms || PendingOrder.Num() > 0)
	{
		return;
	}

	if (!LevelExit)
	{
		SpawnExit(GridCenter);
		if (!LevelExit)
		{
			return; // не нашлось готового чанка — попробуем в следующий тик
		}
	}

	RelocateExitIfNeeded();
	ApplyExitPressure(DeltaSeconds);
}

void ABackroomsWorldGenerator::UpdateStreaming()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	AActor* ViewTarget = PC->GetViewTarget();
	if (!ViewTarget)
	{
		return;
	}

	const FVector PlayerPos = PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : ViewTarget->GetActorLocation();

	// Пока игрок на стартовой платформе (в городе), не обновляем LastPlayerChunk от смещений по X/Y города,
	// чтобы стример не сбрасывал очередь загрузки чанков Бэкрумса внизу
	if (bPlayerSeated && !bPlatformDescentDone)
	{
		UpdateLoadingBar();
		return;
	}

	const FIntPoint NewPlayerChunk = WorldToChunk(PlayerPos);

	if (NewPlayerChunk != LastPlayerChunk)
	{
		// Направление движения игрока (сглаженное): по нему уходит выход вперёд.
		const FIntPoint MoveDelta = NewPlayerChunk - LastPlayerChunk;
		if (MoveDelta.X != 0 || MoveDelta.Y != 0)
		{
			const FVector2D Step((float)MoveDelta.X, (float)MoveDelta.Y);
			TravelDir = FMath::Lerp(TravelDir, Step.GetSafeNormal(), 0.35f).GetSafeNormal();
		}

		LastPlayerChunk = NewPlayerChunk;

		PendingOrder.Empty();
		PendingSet.Empty();

		// Рецентр окна-сетки: чанки, вышедшие за радиус, уничтожены в RecentreGrid
		// (акторы там же и освобождены), осталось почистить очереди.
		TArray<FIntPoint> Evicted;
		RecentreGrid(NewPlayerChunk, Evicted);
		for (const FIntPoint& C : Evicted)
		{
			PendingSet.Remove(C);
			PendingOrder.Remove(C);
		}

		TSet<FIntPoint> Desired;
		const int32 Radius = RenderRadiusChunks;
		for (int32 DX = -Radius; DX <= Radius; ++DX)
		{
			for (int32 DY = -Radius; DY <= Radius; ++DY)
			{
				if (DX * DX + DY * DY <= Radius * Radius)
				{
					Desired.Add(FIntPoint(NewPlayerChunk.X + DX, NewPlayerChunk.Y + DY));
				}
			}
		}

		for (const FIntPoint& Coord : Desired)
		{
			if (!GridHas(Coord) && !PendingSet.Contains(Coord))
			{
				PendingOrder.Add(Coord);
				PendingSet.Add(Coord);
			}
		}

		// ФИКС: раньше очередь шла в порядке итерации TSet (хэш координат,
		// никак не связанный с расстоянием) — из-за этого дальний чанк мог
		// построиться раньше ближнего, и игрок реально добегал до границы
		// сгенерированного мира быстрее, чем она успевала появиться (видимая
		// пустота впереди). Сортируем: сначала ближайшие к игроку кольца,
		// а среди равноудалённых — те, что лежат по направлению движения.
		PendingOrder.Sort([this, NewPlayerChunk](const FIntPoint& A, const FIntPoint& B)
		{
			const FVector2D OffA((float)(A.X - NewPlayerChunk.X), (float)(A.Y - NewPlayerChunk.Y));
			const FVector2D OffB((float)(B.X - NewPlayerChunk.X), (float)(B.Y - NewPlayerChunk.Y));
			const float DistA = OffA.SizeSquared();
			const float DistB = OffB.SizeSquared();
			if (!FMath::IsNearlyEqual(DistA, DistB, 0.5f))
			{
				return DistA < DistB;
			}
			const bool bHaveTravelDir = TravelDir.SizeSquared() > KINDA_SMALL_NUMBER;
			const float AlignA = bHaveTravelDir ? FVector2D::DotProduct(OffA.GetSafeNormal(), TravelDir) : 0.0f;
			const float AlignB = bHaveTravelDir ? FVector2D::DotProduct(OffB.GetSafeNormal(), TravelDir) : 0.0f;
			return AlignA > AlignB;
		});

		TotalRequestedThisWave = PendingOrder.Num();
		LastActivityAt = GetWorld()->GetTimeSeconds();
	}

	UpdateLoadingBar();
}

void ABackroomsWorldGenerator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Пока игрок сидит на стартовой платформе, ведём счётчик показа полосы
	// загрузки (нужен, чтобы полоса шла минимум MinLoadingBarTime секунд).
	if (bPlayerSeated && !bPlatformDescentDone)
	{
		LoadingBarTimer += DeltaSeconds;
	}

	StreamTimer += DeltaSeconds;
	if (StreamTimer < PrimaryActorTick.TickInterval)
	{
		return;
	}
	StreamTimer = 0.0f;

	// Сажаем на платформу только один раз (пока павн ещё не был посажен).
	// Раньше here вызывалось каждый тик, из-за чего игрока телепортировало в
	// центр города 10 раз в секунду — по городу нельзя было бегать.
	if (!bPlayerFellToBackrooms && !bPlayerSeated)
	{
		SitPlayerOnPlatform(false);
	}

	// Automatic city -> Backrooms transition. Never fall onto the generated ceiling.
	if (bPlayerSeated && !bPlatformDescentDone && !bPlayerFellToBackrooms && PendingOrder.Num() == 0)
	{
		DescentTimer += DeltaSeconds;
		if (DescentTimer >= 1.5f) StartDescent();
	}

	UpdateStreaming();

	if (PendingOrder.Num() > 0)
	{
		SpawnPending();
	}
	TrySpawnMonsterAfterGeneration();

	// Дальние чанки — в «спящий» режим (скрыты, без коллизий), ближние —
	// активны. Одновременно ведём единственный выход и давление среды.
	UpdateChunkVisibility();
	UpdateExit(DeltaSeconds);

	TryAutoDump(DeltaSeconds);

	// Первое превью уровня: как только мир достроен и игрок внутри, снимаем
	// настоящий скриншот из этого уровня для меню выбора уровней (один раз).
	if (!bThumbnailTaken && bPlayerFellToBackrooms && PendingOrder.Num() == 0)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UBackroomsLevelBook* Book = GI->GetSubsystem<UBackroomsLevelBook>())
			{
				Book->RequestThumbnail(LevelIndex);
				bThumbnailTaken = true;
			}
		}
	}

	if (bPlatformDescentStarted && !bPlatformDescentDone)
	{
		// Auto-start descent after generation completes — when no pending chunks
		// remain and player is seated on the platform. This replaces waiting for
		// the trapdoor overlap and gives ~2+ minute delay for full generation.
		if (!bPlayerFellToBackrooms && PendingOrder.Num() == 0 && bPlayerSeated)
		{
			StartDescent();
		}
	}
}

void ABackroomsWorldGenerator::UpdatePlatformDescent(float DeltaSeconds)
{
	// Автоспуск отключен: игрок остаётся в городе до конца генерации.
	// Спуск запускается автоматически после завершения генерации (см. ниже).
	if (!StartPlatform || !bSpawnPlatform)
	{
		return;
	}

	if (!bPlatformDescentStarted)
	{
		return;
	}

	// Auto-fall and camera fade removed — descent now controlled after generation complete.
	// FallTimer and timers intentionally left empty; bPlatformDescentDone set later.
}

void ABackroomsWorldGenerator::OnTrapdoorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Legacy hatch transition intentionally disabled.
}

void ABackroomsWorldGenerator::RebuildAllChunks()
{
	if (SpawnedMonster)
	{
		SpawnedMonster->Destroy();
		SpawnedMonster = nullptr;
	}
	if (LevelExit)
	{
		LevelExit->Destroy();
		LevelExit = nullptr;
	}
	ExitChunk = FIntPoint(MAX_int32, MAX_int32);
	ExitPressure = 0;
	ExitPressureTimer = 0.0f;
	ExitGraceTimer = ExitGracePeriod;
	TravelDir = FVector2D::ZeroVector;
	bThumbnailTaken = false;
	for (const FChunkGridSlot& S : ChunkGrid)
	{
		if (S.Chunk)
		{
			S.Chunk->Destroy();
		}
	}
	ChunkGrid.Reset();
	InitGrid();
	PendingOrder.Empty();
	PendingSet.Empty();

	if (StartPlatform)
	{
		StartPlatform->Destroy();
		StartPlatform = nullptr;
	}

	bPlatformDescentStarted = false;
	bPlatformDescentDone = false;
	bPlayerFellToBackrooms = false;
	bFallTeleported = false;
	FallTimer = 0.0f;
	bPlayerSeated = false;
	LastPlayerChunk = FIntPoint::ZeroValue;
	TotalRequestedThisWave = 0;
	PlatformCurrentZ = SpawnPlatformAlt;
	LoadingBarTimer = 0.0f;

	SpawnStartPlatform();
	SitPlayerOnPlatform(true);

	// Заново запускаем генерацию чанков на Z=0.
	const FIntPoint Center(0, 0);
	for (int32 DX = -RenderRadiusChunks; DX <= RenderRadiusChunks; ++DX)
	{
		for (int32 DY = -RenderRadiusChunks; DY <= RenderRadiusChunks; ++DY)
		{
			const FIntPoint Coord(Center.X + DX, Center.Y + DY);
			if (!GridHas(Coord) && !PendingSet.Contains(Coord))
			{
				PendingOrder.Add(Coord);
				PendingSet.Add(Coord);
				++TotalRequestedThisWave;
			}
		}
	}
	LastActivityAt = GetWorld()->GetTimeSeconds();
}

void ABackroomsWorldGenerator::RebuildForDifficulty()
{
	// Сбрасываем кеш профиля: смена сложности должна пересобрать генерацию.
	CachedLevelIndex = -1;
	CachedDifficulty = 255;
	CachedProfile = nullptr;
	RebuildAllChunks();
}

void ABackroomsWorldGenerator::SetLevel(int32 InLevelIndex)
{
	// На фиксированной карте смена локации идёт только через OpenLevel (ExitActor
	// передаёт seed в URL). Локальная смена уровня в той же карте невозможна:
	// у каждой карты свой генератор со своим уровнем. Guard вместо «тихого»
	// телепорта игрока между уровнями внутри одной карты.
	if (bFixedLevel)
	{
		UE_LOG(LogTemp, Display, TEXT("BR: SetLevel(%d) ignored — fixed level map (level %d)"), InLevelIndex, LevelIndex);
		return;
	}

	LevelIndex = InLevelIndex;

	// Режим нового конвейера решается конфигом уровня; смена уровня может
	// включить/выключить FloorPlan — кеш регионов держать в прежнем режиме нельзя.
	bUseFloorPlanLevel = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UBackroomsLevelBook* Book = GI->GetSubsystem<UBackroomsLevelBook>())
		{
			if (const UBackroomsLevelThemeConfig* Config = Book->GetConfig(LevelIndex))
			{
				bUseFloorPlanLevel = Config->bUseFloorPlan;
			}
		}
	}
	if (CVarForceFloorPlan->GetInt() != 0)
	{
		bUseFloorPlanLevel = true;
	}
	FloorPlanCache.Empty();

	// Сохраняем прогресс (Seed + уровень) при смене уровня.
	if (UBackroomsSaveSubsystem* SaveSys = GetGameInstance()->GetSubsystem<UBackroomsSaveSubsystem>())
	{
		SaveSys->SaveProgress(Seed, LevelIndex);
	}

	// Достижения и разблокировка локаций: посещённые уровни копятся как
	// множество — это же служит условием открытия пунктов меню уровней.
	if (UBackroomsAchievements* Ach = GetAchievements())
	{
		Ach->NotifyLevelVisited(LevelIndex);
	}

	RebuildAllChunks();

	// Туман и акустика: туман обновляется из нового профиля через RebuildAllChunks,
	// а лиминальность-слой звука доступен через GetActiveAmbientLayer/Reverb/Echo.
	ApplyLevelAtmosphere();

	// Аудио: тема нового уровня применяется заново (музыка/петля амбиента).
	if (LevelAudioActor)
	{
		LevelAudioActor->ApplyTheme(GetActiveTheme());
	}
}

FString ABackroomsWorldGenerator::GenerateVerificationReport()
{
	const ULevelGeneratorProfile* Profile = GetActiveProfile();

	FChunkGenerationData::FParams Params;
	if (Profile)
	{
		// FParams собирает профиль (который уже знает ULevelConfig) — здесь нет
		// ручного дублирования полей.
		Params = Profile->ToGenParams(Profile->ConfigLevelIndex);
	}
	else
	{
		Params.CellCount = ChunkSizeCells;
		Params.WallThreshold = WallThreshold;
		Params.DoorThreshold = DoorThreshold;
		Params.ScatterThreshold = ScatterThreshold;
		Params.Pattern = FChunkGenerationData::EChunkLayoutPattern::GridRooms;
	}

	static const FIntPoint Samples[] = {
		FIntPoint(0, 0), FIntPoint(1, 0), FIntPoint(0, 1), FIntPoint(1, 1),
		FIntPoint(-1, -1), FIntPoint(2, -2), FIntPoint(-3, 1) };

	FString Out;
	int32 Passed = 0;
	int32 Total = 0;
	for (const FIntPoint& C : Samples)
	{
		const FChunkCoord Coord(C.X, C.Y);
		const int32 ChunkSeed = (int32)BackroomsChunkSeed(Seed, LevelIndex, Coord);
		FChunkGenerationData Data(Coord, ChunkSeed, Params);
		Data.WorldSeed = Seed;
		Data.bUseWorldSeed = true;
		Data.Generate();
		const FGenerationVerifyReport VReport = FChunkGenerationVerifier::RunAll(Data, 2);
		++Total;
		if (VReport.bPassed) { ++Passed; }
		Out += FString::Printf(TEXT("--- chunk (%d,%d) seed %d ---\n"), C.X, C.Y, ChunkSeed);
		Out += VReport.ToString();
	}

	return FString::Printf(TEXT("=== GENERATION SELF-CHECK (level %d, seed %d) ===\npassed %d/%d\n"),
		LevelIndex, Seed, Passed, Total) + Out;
}

FString ABackroomsWorldGenerator::GenerateReport()
{
	FString Report;
	// Если активен FloorPlan-режим — не запускаем legacy-верификацию (FChunkGenerationData
	// и AsciiMapText/LastWallMask) и не читаем структуру комнат. Вместо этого выводим
	// базовую информацию о режиме, чтобы было видно, что генерация идет по плану.
	if (bUseFloorPlanLevel)
	{
		Report += TEXT("=== FLOORPLAN MODE ===\n");
		Report += FString::Printf(TEXT("Mode: FloorPlan (bUseFloorPlanLevel=%d)\n"), bUseFloorPlanLevel);
		Report += FString::Printf(TEXT("Regions count: %d\n"), FMath::Max(1, (int32)FloorPlanCache.Num()));
		Report += FString::Printf(TEXT("Spaces: open regions (FloorPlan geometry)\n"));
		Report += FString::Printf(TEXT("Openings: present via Edge states\n"));
		Report += FString::Printf(TEXT("Connections: active through FloorPlan edges\n"));
		return Report;
	}
	Report += TEXT("=== ОТЧЁТ ГЕНЕРАТОРА ЛОКАЦИИ (проект рассказывает, что настроено) ===\n");

	ULevelGeneratorProfile* Profile = GetActiveProfile();
	if (Profile)
	{
		Report += FString::Printf(TEXT("Уровень (локация): %s\n"), *Profile->LevelName);
		Report += FString::Printf(TEXT("  Палитра: пол RGB(%.2f,%.2f,%.2f) | потолок RGB(%.2f,%.2f,%.2f) | стены RGB(%.2f,%.2f,%.2f)\n"),
			Profile->FloorColor.R, Profile->FloorColor.G, Profile->FloorColor.B,
			Profile->CeilingColor.R, Profile->CeilingColor.G, Profile->CeilingColor.B,
			Profile->WallColor.R, Profile->WallColor.G, Profile->WallColor.B);
		Report += FString::Printf(TEXT("  Геометрия: клетка %.0f см, высота стен %.0f см, толщина %.0f см, ")
			TEXT("дверь %.0f x %.0f см, чанк %dx%d\n"),
			Profile->CellSize, Profile->WallHeight, Profile->WallThickness,
			Profile->DoorWidth, Profile->DoorHeight, Profile->ChunkSizeCells, Profile->ChunkSizeCells);
		Report += FString::Printf(TEXT("  Наполнение: до %.1f пропсов/комнату\n"), Profile->MaxPropsPerRoom);
		Report += FString::Printf(TEXT("  Пул пропсов уровня (вызов папки Style/L%d/Props): %d шт:\n"), LevelIndex, Profile->Props.Num());
		for (const FLevelPropEntry& E : Profile->Props)
		{
			Report += FString::Printf(TEXT("    - %s (вес %.1f, масштаб %.1f-%.1f, макс/комната %.1f)\n"),
				*E.Mesh.GetAssetName(), E.Weight, E.MinScale, E.MaxScale, E.MaxPerRoom);
		}
	}
	else
	{
		Report += TEXT("Профиль не найден — используется общий пул (UPropDatabase).\n");
	}

	Report += FString::Printf(TEXT("Активные чанки в мире: %d\n"), GridStride > 0 ? GridStride * GridStride : 0);
	Report += FString::Printf(TEXT("  из них готово: %d\n"),
		[&]() { int32 N = 0; for (const FChunkGridSlot& S : ChunkGrid) { if (S.Chunk) ++N; } return N; }());
	Report += FString::Printf(TEXT("Seed мира: %d | радиус рендера: %d чанков\n"), Seed, RenderRadiusChunks);
	Report += FString::Printf(TEXT("Стартовая платформа: %s\n"), bSpawnPlatform ? TEXT("включена (спуск с высоты)") : TEXT("выключена"));

	// Самопроверка структуры (см. шпаргалку): ASCII-карта чанков вокруг спавна.
	// Клетка спавна в чанке (0,0) помечена 'S' (открыта) или 'X' (в стене!).
	for (int32 DY = -1; DY <= 1; ++DY)
	{
		for (int32 DX = -1; DX <= 1; ++DX)
		{
			const FIntPoint Key(DX, DY);
			ABackroomsChunkActor* Found = GridGet(Key);
			if (Found)
			{
				Report += FString::Printf(TEXT("\nЧанк (%d,%d):\n"), Key.X, Key.Y);
				Report += Found->AsciiMapText();
			}
		}
	}
	if (ABackroomsChunkActor* C0 = GridGet(FIntPoint(0, 0)))
	{
		if (C0->AsciiMapText().Contains(TEXT("X")))
		{
			Report += TEXT("ВНИМАНИЕ: клетка спавна в чанке (0,0) — В СТЕНЕ!\n");
		}
		else
		{
			Report += TEXT("Клетка спавна открыта (S) — старт безопасен.\n");
		}
	}

	Report += TEXT("\n");
	Report += GenerateVerificationReport();

	return Report;
}

void ABackroomsWorldGenerator::TryAutoDump(float DeltaSeconds)
{
	if (!bAutoDump)
	{
		return;
	}

	// Ждём, пока мир построится (нет pending-чанков), затем немного простоя,
	// и пишем отчёт — проекта «рассказывает», что он настроил.
	const bool bIdle = PendingOrder.Num() == 0 && (!bPlatformDescentStarted || bPlatformDescentDone);
	if (!bIdle)
	{
		DumpTimer = 0.0f;
		return;
	}

	DumpTimer += DeltaSeconds;
	if (DumpTimer < 3.0f)
	{
		return;
	}

	bAutoDump = false;
	const FString Report = GenerateReport();
	// Отчёт пишем в папку Saved проекта, а не в локальный путь разработчика:
	// он недоступен на других машинах и меняется с машины на машину.
	const FString Path = FString::Printf(TEXT("%s/gen_report_L%d.txt"),
		*FPaths::ProjectSavedDir(), LevelIndex);
	FFileHelper::SaveStringToFile(Report, *Path, FFileHelper::EEncodingOptions::ForceUTF8);
	UE_LOG(LogTemp, Display, TEXT("\n%s"), *Report);
	UE_LOG(LogTemp, Display, TEXT("BR: auto report written to %s"), *Path);
}

UBackroomsAchievements* ABackroomsWorldGenerator::GetAchievements() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UBackroomsAchievements>();
	}
	return nullptr;
}

void ABackroomsWorldGenerator::StartDescent()
{
	if (bPlatformDescentDone || bPlayerFellToBackrooms) return;
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn) return;
	const FVector BackroomsFloor(CellSize * ChunkSizeCells * 0.5f, CellSize * ChunkSizeCells * 0.5f, PawnStandZ());
	Pawn->SetActorLocation(BackroomsFloor, false, nullptr, ETeleportType::TeleportPhysics);
	Pawn->SetActorRotation(FRotator::ZeroRotator);
	if (ACharacter* Char = Cast<ACharacter>(Pawn))
	{
		if (UCharacterMovementComponent* Movement = Char->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
			Movement->SetMovementMode(MOVE_Walking);
		}
	}
	if (StartPlatform) { StartPlatform->Destroy(); StartPlatform = nullptr; }
	bPlayerSeated = false;
	bPlatformDescentStarted = true;
	bPlatformDescentDone = true;
	DescentTimer = 0.0f;
	FallTimer = 0.0f;
	bFallTeleported = false;
	MarkBackroomsEntered();
	ExitGraceTimer = ExitGracePeriod;
	UE_LOG(LogTemp, Display, TEXT("BR: city -> Backrooms L0 transition complete after generation + 1.5s"));
}

void ABackroomsWorldGenerator::MarkBackroomsEntered()
{
	if (bPlayerFellToBackrooms)
	{
		return; // уже внутри — не задваиваем вход
	}
	bPlayerFellToBackrooms = true;

	if (UBackroomsAchievements* Ach = GetAchievements())
	{
		Ach->NotifyBackroomsEntered();
		// Посещённая локация — тоже часть «коллекции» для реиграбельности.
		Ach->NotifyLevelVisited(LevelIndex);
	}
}

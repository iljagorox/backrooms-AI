#include "LevelGeneratorProfile.h"
#include "BackroomsLevelConfig.h"
#include "BackroomsYamlLoader.h"

// Внутренние данные: подмножество пропсов для каждого уровня.
// Имена соответствуют ассетам в Content/Props/*.uasset.
namespace
{
	struct FPropSpec
	{
		const TCHAR* Name;
		ELevelPropPlacement Placement;
		float Weight;
		float MinScale;
		float MaxScale;
		float MaxPerRoom;
	};

	void AddProps(ULevelGeneratorProfile* P, const TArray<FPropSpec>& Specs)
	{
		for (const FPropSpec& S : Specs)
		{
			FLevelPropEntry E;
			E.Mesh = FSoftObjectPath(FString::Printf(TEXT("/Game/Props/%s.%s"), S.Name, S.Name));
			E.Placement = S.Placement;
			E.Weight = S.Weight;
			E.MinScale = S.MinScale;
			E.MaxScale = S.MaxScale;
			E.MaxPerRoom = S.MaxPerRoom;
			P->Props.Add(E);
		}
	}

	void AddScene(ULevelGeneratorProfile* P, EStorySceneType Type, const TArray<FStoryPropEntry>& Props)
	{
		FLevelStoryScene S;
		S.Type = Type;
		S.Props = Props;
		P->StoryScenes.Add(S);
	}

	// Общие для всех уровней «следы жизни» (4 сценария). Меши, которых нет на
	// диске или с '_'-именами, отбраковываются на этапе FilterBrokenAssets.
	void AddSharedStoryScenes(ULevelGeneratorProfile* P)
	{
		AddScene(P, EStorySceneType::MadeCamp, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Bed.Bed"))), FVector(0,0,0), 0.0f, 60.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Stool.Stool"))), FVector(-220,-180,0), 90.0f, 4.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Fab/Coffee_Mug___Realistic_3D_Asset/A_Mug_High_01.A_Mug_High_01"))), FVector(-120,-140,0), 15.0f, 0.4f, 0.55f }
		});
		AddScene(P, EStorySceneType::AbandonedLeft, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Box_Cardboard.Box_Cardboard"))), FVector(-160,120,0), 45.0f, 3.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Fab/Tin_cans/tincanssketchfab/StaticMeshes/tincanssketchfab.tincanssketchfab"))), FVector(180,-80,0), 200.0f, 0.8f, 0.6f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Newspaper.Newspaper"))), FVector(40,-200,0), 70.0f, 0.2f, 1.0f }
		});
		AddScene(P, EStorySceneType::FledInHaste, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Sofa.Sofa"))), FVector(120,140,0), 12.0f, 40.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Chair_Wooden.Chair_Wooden"))), FVector(-260,40,0), 105.0f, 8.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Bottle_Glass.Bottle_Glass"))), FVector(-40,-260,0), 250.0f, 0.4f, 0.7f }
		});
		AddScene(P, EStorySceneType::SomeoneSick, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Mattress.Mattress"))), FVector(0,0,0), 0.0f, 18.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Chair_Wooden.Chair_Wooden"))), FVector(-260,120,0), 95.0f, 8.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Fab/Coffee_Mug___Realistic_3D_Asset/A_Mug_High_01.A_Mug_High_01"))), FVector(-180,40,0), 30.0f, 0.4f, 0.55f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Kit_FirstAid.Kit_FirstAid"))), FVector(200,-160,0), 200.0f, 1.2f, 1.0f }
		});
	}
}

// Схема планировки из строки algorithm в YAML. Уровни с «стеновыми» алгоритмами
// (графовый скелет, кварталы, дороги) продолжены как GridRooms с сеткой комнат.
ELevelLayoutPattern LayoutFromAlgorithm(const FString& Algorithm)
{
	if (Algorithm.Contains(TEXT("maze")))
	{
		return ELevelLayoutPattern::Maze;
	}
	if (Algorithm.Contains(TEXT("cellular")) || Algorithm.Contains(TEXT("cave")))
	{
		return ELevelLayoutPattern::Cave;
	}
	if (Algorithm.Contains(TEXT("open_hall")) || Algorithm.Contains(TEXT("open_sparse"))
	    || Algorithm.Contains(TEXT("corridor_branch")))
	{
		return ELevelLayoutPattern::OpenHall;
	}
	return ELevelLayoutPattern::GridRooms;
}

void ULevelGeneratorProfile::ApplyLevelConfig(const UBackroomsLevelConfig* Cfg)
{
	if (!Cfg)
	{
		return;
	}
	LevelConfig = const_cast<UBackroomsLevelConfig*>(Cfg);
	ConfigId = Cfg->Id;
	if (!Cfg->Name.IsEmpty())
	{
		LevelName = Cfg->Name;
	}

	// Метры -> сантиметры. 1 тайл = 0.5 м -> CellSize 500 см (константа сетки).
	const float TileM = FMath::Max(0.01f, Cfg->Grid.TileSizeM);
	CellSize = TileM * 100.0f;
	WallHeight = FMath::Max(100.0f, Cfg->Grid.FloorHeightM) * 100.0f;
	WallThickness = Cfg->Grid.WallThicknessMinM * 100.0f;
	const float DoorMidM = 0.5f * (Cfg->Doors.WidthMinM + Cfg->Doors.WidthMaxM);
	DoorWidth = FMath::Max(60.0f, DoorMidM) * 100.0f;
	DoorHeight = Cfg->Doors.HeightM * 100.0f;

	ChunkSizeCells = FMath::Clamp(Cfg->Generation.ChunkCells, 2, 64);
	LayoutPattern = LayoutFromAlgorithm(Cfg->Generation.Algorithm);

	// Комнаты не больше limit-а из конфига; минимум — треть от максимума.
	const float MaxRoomM = FMath::Max(TileM, Cfg->Rooms.MaxSizeM);
	MaxRoomCells = FMath::Clamp(FMath::RoundToInt(MaxRoomM / TileM), 1, 64);
	MinRoomCells = FMath::Clamp(MaxRoomCells / 3, 1, MaxRoomCells);
}

FChunkGenerationData::FParams ULevelGeneratorProfile::ToGenParams(int32 LevelIndex) const
{
	FChunkGenerationData::FParams P;
	P.CellCount = FMath::Clamp(ChunkSizeCells, 2, 64);
	P.WallThreshold = WallThreshold;
	P.ScatterThreshold = ScatterThreshold;
	P.DoorThreshold = DoorThreshold;
	P.LivingRoomThreshold = LivingRoomThreshold;
	P.MinRoomCells = FMath::Clamp(MinRoomCells, 1, 64);
	P.MaxRoomCells = FMath::Clamp(MaxRoomCells, P.MinRoomCells, 64);
	P.Pattern = static_cast<FChunkGenerationData::EChunkLayoutPattern>((uint8)LayoutPattern);
	P.LevelIndex = LevelIndex;
	const int32 AvgRoom = FMath::Max(1, (P.MinRoomCells + P.MaxRoomCells) / 2);
	P.BaseNoiseFreq = FMath::Clamp(1.0f / (float)(AvgRoom * 14), 0.03f, 0.20f);
	P.MacroNoiseFreq = FMath::Clamp(1.0f / (float)(AvgRoom * 100), 0.004f, 0.03f);
	return P;
}

ULevelGeneratorProfile* ULevelGeneratorProfile::BuildDefaultByLevel(int32 LevelIndex)
{
	ULevelGeneratorProfile* P = NewObject<ULevelGeneratorProfile>();
	if (!P)
	{
		return nullptr;
	}

	P->ConfigLevelIndex = LevelIndex;
	P->LevelName = FString::Printf(TEXT("L%d"), LevelIndex);

	// Тема уровня: акценты HUD/меню из палитры уровня. Один HUD-класс на все
	// уровни — различие только в теме.
	auto SetTheme = [&](const FLinearColor& Accent, const FLinearColor& Interior)
	{
		P->Theme.AccentPrimary = Accent;
		P->Theme.AccentSecondary = Interior;
		P->Theme.PanelBorder = Accent * 0.65f;
		P->Theme.PanelBorder.A = 0.45f;
		P->Theme.MeterSanity = Interior;
		P->Theme.MeterFlashlight = FLinearColor(0.95f, 0.78f, 0.30f, 0.95f);
		P->Theme.MeterHunger = FLinearColor(0.85f, 0.62f, 0.30f, 0.95f);
		P->Theme.MeterThirst = FLinearColor(0.40f, 0.68f, 0.88f, 0.95f);
		P->Theme.MeterStamina = FLinearColor(0.28f, 0.80f, 0.88f, 0.95f);
		P->Theme.MeterHealth = FLinearColor(0.78f, 0.16f, 0.14f, 0.95f);
		P->Theme.MenuAccent = Accent;
	};

	switch (LevelIndex)
	{
	case 0: // Lobby: жёлтый уют.
		SetTheme(FLinearColor(0.92f, 0.78f, 0.35f, 1.0f), FLinearColor(0.45f, 0.72f, 0.35f, 0.95f));
		break;
	case 1: // Обитаемая: безопасная, тёплая.
		SetTheme(FLinearColor(0.55f, 0.72f, 0.45f, 1.0f), FLinearColor(0.40f, 0.66f, 0.38f, 0.95f));
		break;
	case 2: // Водопровод: сталь/синий.
		SetTheme(FLinearColor(0.45f, 0.62f, 0.75f, 1.0f), FLinearColor(0.35f, 0.55f, 0.72f, 0.95f));
		break;
	case 3: // Электростанция: янтарь напряжения.
		SetTheme(FLinearColor(0.95f, 0.65f, 0.15f, 1.0f), FLinearColor(0.85f, 0.55f, 0.15f, 0.95f));
		break;
	case 4: // Офисы: стерильный беж.
		SetTheme(FLinearColor(0.62f, 0.55f, 0.48f, 1.0f), FLinearColor(0.55f, 0.60f, 0.62f, 0.95f));
		break;
	case 5: // Отель: красный/золото.
		SetTheme(FLinearColor(0.82f, 0.30f, 0.20f, 1.0f), FLinearColor(0.92f, 0.82f, 0.45f, 0.95f));
		break;
	case 6: // Тёмная сторона: тусклый фиолетовый.
		SetTheme(FLinearColor(0.45f, 0.32f, 0.60f, 1.0f), FLinearColor(0.38f, 0.30f, 0.50f, 0.95f));
		break;
	case 7: // Талассофобия: глубокая синева.
		SetTheme(FLinearColor(0.22f, 0.42f, 0.68f, 1.0f), FLinearColor(0.25f, 0.50f, 0.75f, 0.95f));
		break;
	case 8: // Пещеры: неоновый кварц.
		SetTheme(FLinearColor(0.30f, 0.75f, 0.80f, 1.0f), FLinearColor(0.35f, 0.80f, 0.70f, 0.95f));
		break;
	case 9: // Больница: холодный бледно-голубой.
		SetTheme(FLinearColor(0.55f, 0.75f, 0.88f, 1.0f), FLinearColor(0.60f, 0.78f, 0.90f, 0.95f));
		break;
	default:
		SetTheme(FLinearColor(0.95f, 0.82f, 0.38f, 1.0f), FLinearColor(0.45f, 0.72f, 0.35f, 0.95f));
		break;
	}

	// Аудио-часть темы: эмбиент/реверб в зависимости от локации.
	// MusicTrack/AmbientLoop — мягкие ссылки; ассетов ещё нет, будет тишина.
	switch (LevelIndex)
	{
	case 0:
		P->Theme.AmbientLayer = TEXT("lamp_hum");
		P->Theme.ReverbScene = TEXT("lobby");
		break;
	case 1:
		P->Theme.AmbientLayer = TEXT("room_tone_film");
		P->Theme.ReverbScene = TEXT("habitable");
		break;
	case 2:
		P->Theme.AmbientLayer = TEXT("drip");
		P->Theme.ReverbScene = TEXT("waterworks");
		break;
	case 3:
		P->Theme.AmbientLayer = TEXT("ventilation");
		P->Theme.ReverbScene = TEXT("power");
		break;
	case 4:
		P->Theme.AmbientLayer = TEXT("office");
		P->Theme.ReverbScene = TEXT("office");
		break;
	case 5:
		P->Theme.AmbientLayer = TEXT("room_tone_film");
		P->Theme.ReverbScene = TEXT("hotel");
		break;
	case 6:
		P->Theme.AmbientLayer = TEXT("wind");
		P->Theme.ReverbScene = TEXT("void");
		P->Theme.bEcho = true;
		break;
	case 7:
		P->Theme.AmbientLayer = TEXT("wind");
		P->Theme.ReverbScene = TEXT("ocean");
		P->Theme.bEcho = true;
		break;
	case 8:
		P->Theme.AmbientLayer = TEXT("drip");
		P->Theme.ReverbScene = TEXT("cave");
		break;
	case 9:
		P->Theme.AmbientLayer = TEXT("lamp_hum");
		P->Theme.ReverbScene = TEXT("hospital");
		break;
	default:
		break;
	}

	// Планировка по локации: трубы/энергетика/тьма — лабиринт; океан — открытые
	// залы; пещеры — органика; остальные — комнаты разного размера.
	switch (LevelIndex)
	{
	case 2:
	case 3:
	case 6:
		P->LayoutPattern = ELevelLayoutPattern::Maze;
		break;
	case 7:
		P->LayoutPattern = ELevelLayoutPattern::OpenHall;
		break;
	case 8:
		P->LayoutPattern = ELevelLayoutPattern::Cave;
		break;
	default:
		P->LayoutPattern = ELevelLayoutPattern::GridRooms;
		break;
	}

	// Материалы уровня: /Game/Style/L{N}/Materials/M_{Floor|Wall|Ceiling}_L{N}
	// Если ассета ещё нет на диске — LoadSynchronous() в SetupFromProfile
	// просто вернёт null, и останется дефолтный материал (безопасный фолбэк).
	const FString StyleDir = FString::Printf(TEXT("/Game/Style/L%d/Materials"), LevelIndex);
	P->FloorMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(
		FString::Printf(TEXT("%s/M_Floor_L%d.M_Floor_L%d"), *StyleDir, LevelIndex, LevelIndex)));
	P->WallMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(
		FString::Printf(TEXT("%s/M_Wall_L%d.M_Wall_L%d"), *StyleDir, LevelIndex, LevelIndex)));
	P->CeilingMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(
		FString::Printf(TEXT("%s/M_Ceiling_L%d.M_Ceiling_L%d"), *StyleDir, LevelIndex, LevelIndex)));

	switch (LevelIndex)
	{
	case 0: // LOBBI: жёлтый уют, минимум нарушения, без сущностей.
		P->FloorColor = FLinearColor(0.72f, 0.62f, 0.44f);
		P->CeilingColor = FLinearColor(0.86f, 0.84f, 0.76f);
		P->WallColor = FLinearColor(0.84f, 0.78f, 0.52f);
		P->MaxPropsPerRoom = 2.2f;
		AddProps(P, {
			{ TEXT("Chair_Wooden"), ELevelPropPlacement::FloorRandom, 3.0f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Table_Wooden"), ELevelPropPlacement::FloorRandom, 2.0f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Bookshelf"),    ELevelPropPlacement::AlongWall,   1.4f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Books_Stack"),  ELevelPropPlacement::FloorRandom, 1.2f, 1.0f, 1.2f, 2.0f},
			{ TEXT("Bottle_Glass"), ELevelPropPlacement::FloorRandom, 0.9f, 1.0f, 1.4f, 3.0f},
			{ TEXT("ToyBlock"),     ELevelPropPlacement::FloorRandom, 0.8f, 0.9f, 1.2f, 2.0f},
			{ TEXT("ToyCar"),       ELevelPropPlacement::FloorRandom, 0.7f, 0.9f, 1.2f, 2.0f},
			{ TEXT("ToyDuck"),      ELevelPropPlacement::FloorRandom, 0.6f, 0.9f, 1.2f, 2.0f},
			{ TEXT("Light_CeilingQuad"), ELevelPropPlacement::CeilingHang, 2.5f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Door_Prop"),    ELevelPropPlacement::AlongWall,   0.5f, 0.9f, 1.1f, 1.0f}
		});
		AddSharedStoryScenes(P);
		AddScene(P, EStorySceneType::AbandonedLeft, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/ToyCar.ToyCar"))), FVector(0,0,0), 0.0f, 1.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Chair_Wooden.Chair_Wooden"))), FVector(-200,0,0), 140.0f, 6.0f, 1.0f }
		});
		break;

	case 1: // Обитаемая зона: бетон, туман, тусклый свет, припасы в ящиках.
		P->FloorColor = FLinearColor(0.55f, 0.52f, 0.48f);
		P->CeilingColor = FLinearColor(0.82f, 0.80f, 0.76f);
		P->WallColor = FLinearColor(0.70f, 0.66f, 0.60f);
		P->MaxPropsPerRoom = 3.5f;
		AddProps(P, {
			// Мебель (лагерь выживших).
			{ TEXT("Bed"),           ELevelPropPlacement::AlongWall,   1.4f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Mattress"),      ELevelPropPlacement::AlongWall,   1.6f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Wardrobe"),      ELevelPropPlacement::AlongWall,   1.0f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Table_Wooden"),  ELevelPropPlacement::FloorRandom, 1.6f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Chair_Wooden"),  ELevelPropPlacement::FloorRandom, 1.4f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Sofa"),          ELevelPropPlacement::AlongWall,   1.2f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Stool"),         ELevelPropPlacement::FloorRandom, 1.5f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Bookshelf"),     ELevelPropPlacement::AlongWall,   1.0f, 0.9f, 1.1f, 1.0f},
			// Склад/ящики.
			{ TEXT("Crate_Wood"),    ELevelPropPlacement::FloorRandom, 1.8f, 0.9f, 1.2f, 3.0f},
			{ TEXT("Box_Cardboard"), ELevelPropPlacement::FloorRandom, 1.8f, 0.9f, 1.2f, 3.0f},
			{ TEXT("Pallet"),        ELevelPropPlacement::FloorRandom, 1.2f, 0.9f, 1.2f, 2.0f},
			// Полезные припасы.
			{ TEXT("Kit_FirstAid"),  ELevelPropPlacement::WallHang,    1.0f, 1.0f, 1.2f, 1.0f},
			{ TEXT("VendingMachine"),ELevelPropPlacement::AlongWall,   0.8f, 0.9f, 1.1f, 1.0f},
			{ TEXT("WaterCooler"),   ELevelPropPlacement::AlongWall,   0.7f, 0.9f, 1.1f, 1.0f},
			// Электроника (Wi-Fi база).
			{ TEXT("Monitor"),       ELevelPropPlacement::FloorRandom, 0.8f, 0.9f, 1.2f, 1.0f},
			{ TEXT("Phone_Desk"),    ELevelPropPlacement::FloorRandom, 0.7f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Radio"),         ELevelPropPlacement::FloorRandom, 0.9f, 1.0f, 1.2f, 1.0f},
			// Арматура/промышленность.
			{ TEXT("Pipe_Rusty"),    ELevelPropPlacement::AlongWall,   1.2f, 0.9f, 1.2f, 2.0f},
			{ TEXT("Pipe_Elbow"),    ELevelPropPlacement::AlongWall,   1.0f, 0.9f, 1.2f, 1.0f},
			{ TEXT("VentDuct"),      ELevelPropPlacement::CeilingHang, 1.0f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Panel_Electric"),ELevelPropPlacement::WallHang,    0.6f, 1.0f, 1.2f, 1.0f},
			{ TEXT("FireExtinguisher"),ELevelPropPlacement::WallHang,  0.8f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Bucket"),        ELevelPropPlacement::FloorRandom, 1.0f, 1.0f, 1.2f, 2.0f},
			// Мусор/атмосфера.
			{ TEXT("Can_Rusty"),     ELevelPropPlacement::FloorRandom, 1.2f, 1.0f, 1.4f, 3.0f},
			{ TEXT("Bottle_Glass"),  ELevelPropPlacement::FloorRandom, 0.9f, 1.0f, 1.4f, 3.0f},
			{ TEXT("Books_Stack"),   ELevelPropPlacement::FloorRandom, 1.0f, 1.0f, 1.2f, 2.0f},
			{ TEXT("Newspaper"),     ELevelPropPlacement::FloorRandom, 0.8f, 1.0f, 1.2f, 3.0f},
			{ TEXT("Lamp_Floor"),    ELevelPropPlacement::FloorRandom, 0.7f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Light_CeilingQuad"), ELevelPropPlacement::CeilingHang, 2.0f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Light_CeilingTube"), ELevelPropPlacement::CeilingHang, 1.8f, 0.9f, 1.1f, 1.0f}
		});
		AddSharedStoryScenes(P);
		AddScene(P, EStorySceneType::MadeCamp, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Box_Cardboard.Box_Cardboard"))), FVector(0,0,0), 0.0f, 3.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Bed.Bed"))), FVector(-240,-180,0), 90.0f, 60.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Kit_FirstAid.Kit_FirstAid"))), FVector(180,-60,0), 200.0f, 1.2f, 1.0f }
		});
		break;

	case 2: // Водопровод: серый бетон, синие бочки-контраст, трубы.
		P->FloorColor = FLinearColor(0.34f, 0.34f, 0.36f);
		P->CeilingColor = FLinearColor(0.62f, 0.60f, 0.58f);
		P->WallColor = FLinearColor(0.58f, 0.56f, 0.55f);
		P->MaxPropsPerRoom = 3.5f;
		AddProps(P, {
			{ TEXT("Pipe_Rusty"),   ELevelPropPlacement::AlongWall,   2.0f, 0.9f, 1.2f, 2.0f},
			{ TEXT("Pipe_Elbow"),   ELevelPropPlacement::AlongWall,   1.6f, 0.9f, 1.2f, 2.0f},
			{ TEXT("VentDuct"),     ELevelPropPlacement::CeilingHang, 1.4f, 0.9f, 1.1f, 1.0f},
			{ TEXT("JunctionBox"),  ELevelPropPlacement::WallHang,    0.8f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Barrel_Metal"), ELevelPropPlacement::FloorRandom, 1.4f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Barrel_PlasticBlue"), ELevelPropPlacement::FloorRandom, 1.4f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Jerrycan"),     ELevelPropPlacement::FloorRandom, 1.2f, 1.0f, 1.2f, 2.0f},
			{ TEXT("Bucket"),       ELevelPropPlacement::FloorRandom, 1.2f, 1.0f, 1.2f, 2.0f},
			{ TEXT("Crate_Wood"),   ELevelPropPlacement::FloorRandom, 1.3f, 0.9f, 1.2f, 2.0f},
			{ TEXT("Pallet"),       ELevelPropPlacement::FloorRandom, 1.1f, 0.9f, 1.2f, 1.0f},
			{ TEXT("Can_Rusty"),    ELevelPropPlacement::FloorRandom, 1.0f, 1.0f, 1.4f, 3.0f},
			{ TEXT("FireExtinguisher"), ELevelPropPlacement::WallHang, 1.0f, 1.0f, 1.2f, 1.0f},
			{ TEXT("VendingMachine"), ELevelPropPlacement::AlongWall, 0.6f, 0.9f, 1.1f, 1.0f}
		});
		AddSharedStoryScenes(P);
		AddScene(P, EStorySceneType::AbandonedLeft, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Crate_Wood.Crate_Wood"))), FVector(0,0,0), 0.0f, 5.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Barrel_Metal.Barrel_Metal"))), FVector(-200,-140,0), 120.0f, 12.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Bucket.Bucket"))), FVector(180,140,0), 250.0f, 1.5f, 1.0f }
		});
		break;

	case 3: // Электростанция: тьма и напряжение, сталь, аварийный оранжевый.
		P->FloorColor = FLinearColor(0.16f, 0.16f, 0.18f);
		P->CeilingColor = FLinearColor(0.30f, 0.30f, 0.32f);
		P->WallColor = FLinearColor(0.30f, 0.30f, 0.34f);
		P->MaxPropsPerRoom = 2.8f;
		AddProps(P, {
			{ TEXT("Panel_Electric"), ELevelPropPlacement::WallHang, 1.4f, 1.0f, 1.2f, 1.0f},
			{ TEXT("JunctionBox"),    ELevelPropPlacement::WallHang, 1.0f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Pipe_Rusty"),     ELevelPropPlacement::AlongWall, 1.8f, 0.9f, 1.2f, 2.0f},
			{ TEXT("VentDuct"),       ELevelPropPlacement::CeilingHang, 1.2f, 0.9f, 1.1f, 1.0f},
			{ TEXT("ServerRack"),     ELevelPropPlacement::AlongWall, 1.2f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Desk_Office"),    ELevelPropPlacement::AlongWall, 1.0f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Monitor"),        ELevelPropPlacement::FloorRandom, 0.9f, 0.9f, 1.2f, 1.0f},
			{ TEXT("Keyboard"),       ELevelPropPlacement::FloorRandom, 0.8f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Barrel_Metal"),   ELevelPropPlacement::FloorRandom, 1.3f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Helmet_Hard"),    ELevelPropPlacement::FloorRandom, 0.8f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Hydrant_Fire"),   ELevelPropPlacement::WallHang, 0.9f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Light_CeilingTube"), ELevelPropPlacement::CeilingHang, 2.2f, 0.9f, 1.1f, 1.0f}
		});
		AddSharedStoryScenes(P);
		AddScene(P, EStorySceneType::FledInHaste, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Desk_Office.Desk_Office"))), FVector(0,0,0), 0.0f, 30.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Helmet_Hard.Helmet_Hard"))), FVector(-180,160,0), 200.0f, 0.8f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/JunctionBox.JunctionBox"))), FVector(200,-140,0), 45.0f, 2.0f, 1.0f }
		});
		break;

	case 4: // Офисы: белый/серый, офисная мебель, «призрачный уют».
		P->FloorColor = FLinearColor(0.70f, 0.70f, 0.66f);
		P->CeilingColor = FLinearColor(0.90f, 0.90f, 0.88f);
		P->WallColor = FLinearColor(0.88f, 0.88f, 0.86f);
		P->MaxPropsPerRoom = 3.0f;
		AddProps(P, {
			{ TEXT("Desk_Office"),  ELevelPropPlacement::AlongWall,   1.8f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Chair_Office"), ELevelPropPlacement::FloorRandom, 1.8f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Monitor"),      ELevelPropPlacement::FloorRandom, 1.0f, 0.9f, 1.2f, 1.0f},
			{ TEXT("Keyboard"),     ELevelPropPlacement::FloorRandom, 0.8f, 1.0f, 1.2f, 1.0f},
			{ TEXT("PcTower"),      ELevelPropPlacement::FloorRandom, 0.9f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Phone_Desk"),   ELevelPropPlacement::FloorRandom, 0.7f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Lamp_Desk"),    ELevelPropPlacement::FloorRandom, 0.8f, 0.9f, 1.1f, 1.0f},
			{ TEXT("ServerRack"),   ELevelPropPlacement::AlongWall,   0.9f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Bookshelf"),    ELevelPropPlacement::AlongWall,   1.2f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Sofa"),         ELevelPropPlacement::AlongWall,   1.0f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Table_Coffee"), ELevelPropPlacement::FloorCenter, 0.9f, 0.9f, 1.1f, 1.0f},
			{ TEXT("WaterCooler"),  ELevelPropPlacement::AlongWall,   0.6f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Newspaper"),    ELevelPropPlacement::FloorRandom, 0.8f, 1.0f, 1.2f, 3.0f},
			{ TEXT("Tv_Crt"),       ELevelPropPlacement::FloorRandom, 0.6f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Painting"),     ELevelPropPlacement::WallHang,   0.9f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Picture_Frame"),ELevelPropPlacement::WallHang,   0.8f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Mirror_Frame"), ELevelPropPlacement::WallHang,   0.7f, 1.0f, 1.2f, 1.0f}
		});
		AddSharedStoryScenes(P);
		AddScene(P, EStorySceneType::SomeoneSick, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Desk_Office.Desk_Office"))), FVector(0,0,0), 0.0f, 30.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Keyboard.Keyboard"))), FVector(-160,120,0), 90.0f, 0.5f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Chair_Office.Chair_Office"))), FVector(180,-140,0), 250.0f, 8.0f, 1.0f }
		});
		break;

	case 5: // Отель: красная роскошь + процедурные номера-чанки.
		// Отель использует отдельный каталог стиля (не по общему шаблону L5).
		P->FloorMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(
			TEXT("/Game/Style/L5_Hotel/Materials/M_Floor_L5.M_Floor_L5")));
		P->WallMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(
			TEXT("/Game/Style/L5_Hotel/Materials/M_Wall_L5.M_Wall_L5")));
		P->CeilingMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(
			TEXT("/Game/Style/L5_Hotel/Materials/M_Ceiling_L5.M_Ceiling_L5")));
		P->FloorColor = FLinearColor(0.42f, 0.16f, 0.14f);
		P->CeilingColor = FLinearColor(0.72f, 0.66f, 0.58f);
		P->WallColor = FLinearColor(0.55f, 0.22f, 0.18f);
		P->MaxPropsPerRoom = 2.6f;
		AddProps(P, {
			{ TEXT("Bed"),           ELevelPropPlacement::AlongWall,   1.6f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Nightstand"),    ELevelPropPlacement::AlongWall,   1.3f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Wardrobe"),      ELevelPropPlacement::AlongWall,   0.9f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Chandelier"),    ELevelPropPlacement::CeilingHang, 1.0f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Sofa"),          ELevelPropPlacement::AlongWall,   1.0f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Table_Coffee"),  ELevelPropPlacement::FloorCenter, 0.9f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Table_Wooden"),  ELevelPropPlacement::FloorRandom, 1.0f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Chair_Wooden"),  ELevelPropPlacement::FloorRandom, 1.2f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Lamp_Floor"),    ELevelPropPlacement::FloorRandom, 0.9f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Mirror_Frame"),  ELevelPropPlacement::WallHang,   0.9f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Painting"),      ELevelPropPlacement::WallHang,   1.0f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Clock_Wall"),    ELevelPropPlacement::WallHang,   0.8f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Bottle_Glass"),  ELevelPropPlacement::FloorRandom, 0.9f, 1.0f, 1.4f, 3.0f},
			{ TEXT("Books_Stack"),   ELevelPropPlacement::FloorRandom, 1.0f, 1.0f, 1.2f, 2.0f},
			{ TEXT("Light_CeilingQuad"), ELevelPropPlacement::CeilingHang, 2.2f, 0.9f, 1.1f, 1.0f}
		});
		AddSharedStoryScenes(P);
		AddScene(P, EStorySceneType::AbandonedLeft, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Bed.Bed"))), FVector(0,0,0), 0.0f, 60.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Nightstand.Nightstand"))), FVector(-260,140,0), 90.0f, 10.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Bottle_Glass.Bottle_Glass"))), FVector(200,-60,0), 200.0f, 0.4f, 0.7f }
		});
		break;

	case 6: // Тёмная сторона: почти без света, фонарик не работает.
		P->FloorColor = FLinearColor(0.10f, 0.10f, 0.12f);
		P->CeilingColor = FLinearColor(0.14f, 0.14f, 0.16f);
		P->WallColor = FLinearColor(0.12f, 0.12f, 0.14f);
		P->MaxPropsPerRoom = 1.5f;
		AddProps(P, {
			{ TEXT("Barrel_Metal"),  ELevelPropPlacement::FloorRandom, 1.3f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Jerrycan"),      ELevelPropPlacement::FloorRandom, 1.0f, 1.0f, 1.2f, 2.0f},
			{ TEXT("Bucket"),        ELevelPropPlacement::FloorRandom, 1.0f, 1.0f, 1.2f, 2.0f},
			{ TEXT("Pipe_Rusty"),    ELevelPropPlacement::AlongWall,   1.4f, 0.9f, 1.2f, 2.0f},
			{ TEXT("Crate_Wood"),    ELevelPropPlacement::FloorRandom, 1.1f, 0.9f, 1.2f, 2.0f},
			{ TEXT("Crystal_Quartz"),ELevelPropPlacement::FloorRandom, 0.8f, 0.8f, 1.2f, 2.0f},
			{ TEXT("Stalagmite"),    ELevelPropPlacement::FloorRandom, 1.0f, 0.8f, 1.2f, 1.0f},
			{ TEXT("Newspaper"),     ELevelPropPlacement::FloorRandom, 0.8f, 1.0f, 1.2f, 3.0f},
			{ TEXT("Kit_FirstAid"),  ELevelPropPlacement::WallHang,   0.6f, 1.0f, 1.2f, 1.0f}
		});
		AddSharedStoryScenes(P);
		AddScene(P, EStorySceneType::SomeoneSick, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Mattress.Mattress"))), FVector(0,0,0), 0.0f, 18.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Kit_FirstAid.Kit_FirstAid"))), FVector(-200,120,0), 140.0f, 1.2f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Crate_Wood.Crate_Wood"))), FVector(180,-160,0), 200.0f, 5.0f, 1.0f }
		});
		break;

	case 7: // Талассофобия: океан, плоты, почти чёрная вода.
		P->FloorColor = FLinearColor(0.08f, 0.10f, 0.14f);
		P->CeilingColor = FLinearColor(0.12f, 0.14f, 0.18f);
		P->WallColor = FLinearColor(0.10f, 0.12f, 0.16f);
		P->MaxPropsPerRoom = 1.8f;
		P->WallHeight = 240.0f;
		AddProps(P, {
			{ TEXT("Raft"),          ELevelPropPlacement::FloorCenter, 1.6f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Pallet"),        ELevelPropPlacement::FloorRandom, 1.2f, 0.9f, 1.2f, 1.0f},
			{ TEXT("Table_Wooden"),  ELevelPropPlacement::FloorRandom, 1.0f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Crate_Wood"),    ELevelPropPlacement::FloorRandom, 1.2f, 0.9f, 1.2f, 2.0f},
			{ TEXT("Barrel_Metal"),  ELevelPropPlacement::FloorRandom, 1.0f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Jerrycan"),      ELevelPropPlacement::FloorRandom, 0.9f, 1.0f, 1.2f, 2.0f},
			{ TEXT("Kit_FirstAid"),  ELevelPropPlacement::WallHang,   0.6f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Stalagmite"),    ELevelPropPlacement::FloorRandom, 0.8f, 0.8f, 1.2f, 1.0f},
			{ TEXT("Crystal_Quartz"),ELevelPropPlacement::FloorRandom, 0.7f, 0.8f, 1.2f, 2.0f}
		});
		AddSharedStoryScenes(P);
		AddScene(P, EStorySceneType::MadeCamp, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Raft.Raft"))), FVector(0,0,0), 0.0f, 25.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Crate_Wood.Crate_Wood"))), FVector(-220,140,0), 90.0f, 5.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Jerrycan.Jerrycan"))), FVector(180,-80,0), 250.0f, 2.0f, 1.0f }
		});
		break;

	case 8: // Пещеры: коричневый/тёмно-зелёный, узкие проходы, кварц.
		P->FloorColor = FLinearColor(0.26f, 0.20f, 0.14f);
		P->CeilingColor = FLinearColor(0.20f, 0.18f, 0.14f);
		P->WallColor = FLinearColor(0.30f, 0.24f, 0.16f);
		P->MaxPropsPerRoom = 2.0f;
		P->WallHeight = 260.0f;
		AddProps(P, {
			{ TEXT("Stalagmite"),    ELevelPropPlacement::FloorRandom, 2.0f, 0.8f, 1.2f, 1.0f},
			{ TEXT("Crystal_Quartz"),ELevelPropPlacement::FloorRandom, 1.4f, 0.8f, 1.2f, 2.0f},
			{ TEXT("Crate_Wood"),    ELevelPropPlacement::FloorRandom, 1.0f, 0.9f, 1.2f, 2.0f},
			{ TEXT("Box_Cardboard"), ELevelPropPlacement::FloorRandom, 0.9f, 0.9f, 1.2f, 2.0f},
			{ TEXT("Barrel_Metal"),  ELevelPropPlacement::FloorRandom, 0.8f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Pickaxe"),       ELevelPropPlacement::FloorRandom, 0.7f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Helmet_Hard"),   ELevelPropPlacement::FloorRandom, 0.7f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Kit_FirstAid"),  ELevelPropPlacement::WallHang,   0.6f, 1.0f, 1.2f, 1.0f}
		});
		AddSharedStoryScenes(P);
		AddScene(P, EStorySceneType::FledInHaste, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Crate_Wood.Crate_Wood"))), FVector(0,0,0), 0.0f, 5.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Pickaxe.Pickaxe"))), FVector(-200,160,0), 90.0f, 1.5f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Helmet_Hard.Helmet_Hard"))), FVector(180,-80,0), 200.0f, 0.8f, 1.0f }
		});
		break;

	case 9: // Больница: белый/холодный синий, стерильность.
		P->FloorColor = FLinearColor(0.70f, 0.74f, 0.76f);
		P->CeilingColor = FLinearColor(0.88f, 0.90f, 0.92f);
		P->WallColor = FLinearColor(0.80f, 0.84f, 0.86f);
		P->MaxPropsPerRoom = 2.4f;
		AddProps(P, {
			{ TEXT("Bed"),           ELevelPropPlacement::AlongWall,   1.6f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Nightstand"),    ELevelPropPlacement::AlongWall,   1.2f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Wardrobe"),      ELevelPropPlacement::AlongWall,   0.9f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Desk_Office"),   ELevelPropPlacement::AlongWall,   1.0f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Monitor"),       ELevelPropPlacement::FloorRandom, 0.8f, 0.9f, 1.2f, 1.0f},
			{ TEXT("Phone_Desk"),    ELevelPropPlacement::FloorRandom, 0.7f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Lamp_Desk"),     ELevelPropPlacement::FloorRandom, 0.8f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Tv_Crt"),        ELevelPropPlacement::FloorRandom, 0.6f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Mirror_Frame"),  ELevelPropPlacement::WallHang,   0.8f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Bottle_Glass"),  ELevelPropPlacement::FloorRandom, 0.8f, 1.0f, 1.4f, 3.0f},
			{ TEXT("Clock_Wall"),    ELevelPropPlacement::WallHang,   0.7f, 1.0f, 1.2f, 1.0f},
			{ TEXT("Hydrant_Fire"),  ELevelPropPlacement::WallHang,   0.8f, 1.0f, 1.2f, 1.0f},
			{ TEXT("VendingMachine"),ELevelPropPlacement::AlongWall,  0.6f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Newspaper"),     ELevelPropPlacement::FloorRandom, 0.7f, 1.0f, 1.2f, 2.0f}
		});
		AddSharedStoryScenes(P);
		AddScene(P, EStorySceneType::SomeoneSick, {
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Bed.Bed"))), FVector(0,0,0), 0.0f, 60.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Kit_FirstAid.Kit_FirstAid"))), FVector(-160,200,0), 90.0f, 1.2f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Chair_Wooden.Chair_Wooden"))), FVector(200,-40,0), 250.0f, 8.0f, 1.0f },
			{ TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Props/Table_Wooden.Table_Wooden"))), FVector(0,-200,0), 45.0f, 12.0f, 1.0f }
		});
		break;

	case 99: // STRESS TEST: прогон как в CS2 — душит железо честно
		P->LayoutPattern = ELevelLayoutPattern::Maze;
		P->ChunkSizeCells = 16;
		P->MinRoomCells = 2;
		P->MaxRoomCells = 6;
		P->WallThreshold = 0.45f;
		P->DoorThreshold = 0.18f;
		P->ScatterThreshold = 0.65f;
		P->MaxPropsPerRoom = 8.0f;
		P->WallHeight = 300.0f;
		P->FloorColor = FLinearColor(0.12f,0.12f,0.14f);
		P->CeilingColor = FLinearColor(0.18f,0.18f,0.20f);
		P->WallColor = FLinearColor(0.15f,0.15f,0.18f);
		AddProps(P, {
			{ TEXT("Stool"), ELevelPropPlacement::FloorRandom, 2.0f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Table_Wooden"), ELevelPropPlacement::FloorRandom, 1.5f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Light_CeilingQuad"), ELevelPropPlacement::CeilingHang, 8.0f, 0.9f, 1.1f, 3.0f},
			{ TEXT("Light_CeilingTube"), ELevelPropPlacement::CeilingHang, 6.0f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Pipe_Rusty"), ELevelPropPlacement::AlongWall, 3.0f, 0.9f, 1.1f, 3.0f},
			{ TEXT("Barrel_Metal"), ELevelPropPlacement::FloorRandom, 2.0f, 0.9f, 1.1f, 2.0f}
		});
		break;
	default:
		// Универсальный/запасной.
		P->MaxPropsPerRoom = 2.5f;
		AddProps(P, {
			{ TEXT("Stool"),         ELevelPropPlacement::FloorRandom, 2.0f, 0.9f, 1.1f, 2.0f},
			{ TEXT("Table_Wooden"),  ELevelPropPlacement::FloorRandom, 1.5f, 0.9f, 1.1f, 1.0f},
			{ TEXT("Light_CeilingQuad"), ELevelPropPlacement::CeilingHang, 2.0f, 0.9f, 1.1f, 1.0f}
		});
		AddSharedStoryScenes(P);
		break;
	}

	// YAML — источник правды: если конфиг уровня найден, он переопределяет
	// геометрию/планировку/высоты, собранные выше из хардкода.
	if (UBackroomsLevelConfig* Cfg = BackroomsYaml::LoadLevelConfig(
		FString::Printf(TEXT("L%02d"), LevelIndex)))
	{
		P->ApplyLevelConfig(Cfg);
	}

	return P;
}

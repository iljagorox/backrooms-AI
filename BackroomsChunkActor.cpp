#include "BackroomsChunkActor.h"
#include "PropDatabase.h"
#include "LevelGeneratorProfile.h"
#include "BackroomsLevelConfig.h"
#include "BackroomsGenerationData.h"
#include "BackroomsGenerationVerifier.h"
#include "BackroomsRoomVariant.h"
#include "BackroomsNoise.h"
#include "BackroomsPhysProp.h"
#include "BackroomsItemPickup.h"
#include "BackroomsItemSystem.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/DecalActor.h"
#include "CollisionQueryParams.h"
#include "Math/UnrealMathUtility.h"
#include "Math/RotationMatrix.h"
#include "Logging/LogVerbosity.h"
#include "UObject/ConstructorHelpers.h"

// Кеш мешей пропсов — метод-член ABackroomsChunkActor (не статический!):
// см. CachedLoadMesh в .h, жить не дольше чанка и очищается в BeginDestroy.

namespace
{
	// Метрики городского островка: размеры планировки (дороги, тротуары,
	// площадь, сетка кварталов, забор, разметка) — доли HalfExtent с clamp-ами,
	// чтобы остров выглядел цельным при любом размере платформы. Физические
	// объекты (машины, деревья, уличная мебель) остаются в реальных см.
	struct FBackroomsCityMetrics
	{
		float E = 0.0f;
		float RoadW = 400.0f;     // ширина дороги-креста
		float WalkW = 180.0f;     // ширина тротуара
		float WalkEdge = 290.0f;  // осевая линия тротуара
		float Sidewalk = 520.0f;  // безопасная зона у оси (здания не ставим)
		float PlazaR = 750.0f;    // радиус спавн-площади
		float TrapXMax = 2400.0f; // коридор к люку: X:[0..TrapXMax]
		float TrapHalfY = 350.0f; // коридор к люку: Y:[-TrapHalfY..TrapHalfY]
		float OceMargin = 0.0f;   // край острова (у забора)
		float MaxHeight = 2500.0f;// потолок высоты здания
		float GridStep = 1800.0f; // шаг сетки слотов зданий
		float StreetStep = 1500.0f; // шаг уличных пропсов вдоль тротуара
		float StreetMargin = 900.0f; // запас уличных пропсов от края
		float FenceH = 60.0f;     // высота заборчика
		float FenceT = 24.0f;     // толщина заборчика
		float MarkHalf = 40.0f;   // разметка: половина штришка
		float MarkStep = 260.0f;  // разметка: шаг штришков
		float TreeMargin = 320.0f;// запас деревьев от края
		float PropMargin = 300.0f;// запас случайных пропсов от края

		static FBackroomsCityMetrics Make(float InHalfExtent)
		{
			FBackroomsCityMetrics M;
			M.E = InHalfExtent;
			M.RoadW = FMath::Clamp(InHalfExtent * 0.111f, 240.0f, 720.0f);
			M.WalkW = FMath::Clamp(InHalfExtent * 0.050f, 90.0f, 180.0f);
			M.WalkEdge = M.RoadW * 0.5f + M.WalkW * 0.5f;
			M.Sidewalk = FMath::Clamp(InHalfExtent * 0.144f, 260.0f, 720.0f);
			M.PlazaR = FMath::Clamp(InHalfExtent * 0.208f, 260.0f, 1100.0f);
			M.TrapXMax = FMath::Clamp(InHalfExtent * 0.667f, 600.0f, 2600.0f);
			M.TrapHalfY = FMath::Clamp(InHalfExtent * 0.097f, 120.0f, 420.0f);
			M.OceMargin = InHalfExtent - FMath::Max(InHalfExtent * 0.056f, 120.0f);
			M.MaxHeight = FMath::Clamp(InHalfExtent * 0.694f, 700.0f, 2800.0f);
			M.GridStep = FMath::Clamp(InHalfExtent * 0.35f, 260.0f, 1600.0f);
			M.StreetStep = FMath::Clamp(InHalfExtent * 0.417f, 300.0f, 1800.0f);
			M.StreetMargin = FMath::Clamp(InHalfExtent * 0.25f, 200.0f, 1400.0f);
			M.FenceH = FMath::Clamp(InHalfExtent * 0.017f, 40.0f, 90.0f);
			M.FenceT = FMath::Clamp(InHalfExtent * 0.007f, 16.0f, 34.0f);
			M.MarkHalf = FMath::Clamp(InHalfExtent * 0.011f, 20.0f, 60.0f);
			M.MarkStep = FMath::Clamp(InHalfExtent * 0.072f, 140.0f, 300.0f);
			M.TreeMargin = FMath::Clamp(InHalfExtent * 0.089f, 140.0f, 500.0f);
			M.PropMargin = FMath::Clamp(InHalfExtent * 0.083f, 140.0f, 500.0f);
			return M;
		}
	};
}

ABackroomsChunkActor::ABackroomsChunkActor()
{
	PrimaryActorTick.bCanEverTick = false;

	GeometryMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeometryMesh"));
	RootComponent = GeometryMesh;
	GeometryMesh->bUseComplexAsSimpleCollision = true;

	StoryPropClass = ABackroomsPhysProp::StaticClass();
	ItemPickupClass = ABackroomsItemPickup::StaticClass();

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FloorFinder(TEXT("/Game/Materials/M_Floor_Tex.M_Floor_Tex"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CeilingFinder(TEXT("/Game/Materials/M_Ceiling_Tex.M_Ceiling_Tex"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> WallFinder(TEXT("/Game/Materials/M_Wall_Tex.M_Wall_Tex"));
	if (FloorFinder.Succeeded()) { FloorMaterial = FloorFinder.Object; }
	if (CeilingFinder.Succeeded()) { CeilingMaterial = CeilingFinder.Object; }
	if (WallFinder.Succeeded()) { WallMaterial = WallFinder.Object; }

	// Городские материалы (для стартовой платформы-островка) — свои, не из Бэкрумса.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CityFloorFinder(TEXT("/Game/Materials/M_CityFloor.M_CityFloor"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CityCeilingFinder(TEXT("/Game/Materials/M_CityCeiling.M_CityCeiling"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CityWallFinder(TEXT("/Game/Materials/M_CityWall.M_CityWall"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CityRoadFinder(TEXT("/Game/Materials/M_CityRoad.M_CityRoad"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CitySidewalkFinder(TEXT("/Game/Materials/M_CitySidewalk.M_CitySidewalk"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CityMarkingFinder(TEXT("/Game/Materials/M_CityMarking.M_CityMarking"));
	if (CityFloorFinder.Succeeded()) { CityFloorMaterial = CityFloorFinder.Object; }
	if (CityCeilingFinder.Succeeded()) { CityCeilingMaterial = CityCeilingFinder.Object; }
	if (CityWallFinder.Succeeded()) { CityWallMaterial = CityWallFinder.Object; }
	if (CityRoadFinder.Succeeded()) { CityRoadMaterial = CityRoadFinder.Object; }
	if (CitySidewalkFinder.Succeeded()) { CitySidewalkMaterial = CitySidewalkFinder.Object; }
	if (CityMarkingFinder.Succeeded()) { CityMarkingMaterial = CityMarkingFinder.Object; }

	// Автозаполнение граффити-материалов (генерируются backrooms_detail_pipeline.py).
	if (GraffitiDecalMaterials.IsEmpty())
	{
		static const TCHAR* DefaultGraffitiMaterials[] =
		{
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_01_NUM.M_Decal_Graffiti_01_NUM"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_02_NUM.M_Decal_Graffiti_02_NUM"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_03_NUM.M_Decal_Graffiti_03_NUM"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_04_NUM.M_Decal_Graffiti_04_NUM"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_05_NUM.M_Decal_Graffiti_05_NUM"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_06_NUM.M_Decal_Graffiti_06_NUM"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_07_NUM.M_Decal_Graffiti_07_NUM"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_08_DATE.M_Decal_Graffiti_08_DATE"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_09_DATE.M_Decal_Graffiti_09_DATE"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_10_DATE.M_Decal_Graffiti_10_DATE"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_11_DATE.M_Decal_Graffiti_11_DATE"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_12_WORD.M_Decal_Graffiti_12_WORD"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_13_WORD.M_Decal_Graffiti_13_WORD"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_14_WORD.M_Decal_Graffiti_14_WORD"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_15_WORD.M_Decal_Graffiti_15_WORD"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_16_WORD.M_Decal_Graffiti_16_WORD"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_17_WORD.M_Decal_Graffiti_17_WORD"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_18_WORD.M_Decal_Graffiti_18_WORD"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_19_WORD.M_Decal_Graffiti_19_WORD"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_20_PHRASE.M_Decal_Graffiti_20_PHRASE"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_21_PHRASE.M_Decal_Graffiti_21_PHRASE"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_22_SYM.M_Decal_Graffiti_22_SYM"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_23_SYM.M_Decal_Graffiti_23_SYM"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_24_SYM.M_Decal_Graffiti_24_SYM"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_25_SYM.M_Decal_Graffiti_25_SYM"),
			TEXT("/Game/Decals/Materials/M_Decal_Graffiti_26_TALLY.M_Decal_Graffiti_26_TALLY"),
		};
		for (const TCHAR* MatPath : DefaultGraffitiMaterials)
		{
			GraffitiDecalMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(MatPath)));
		}
	}
}

FString ABackroomsChunkActor::GetLevelStylePath(EBackroomsLevelStyle Style)
{
	switch (Style)
	{
	case EBackroomsLevelStyle::L0_Lobby:     return TEXT("/Game/Style/L0/Materials");
	case EBackroomsLevelStyle::L1_Habitable: return TEXT("/Game/Style/L1/Materials");
	case EBackroomsLevelStyle::L2_Pipes:     return TEXT("/Game/Style/L2/Materials");
	case EBackroomsLevelStyle::L3_Power:     return TEXT("/Game/Style/L3/Materials");
	case EBackroomsLevelStyle::L4_Offices:   return TEXT("/Game/Style/L4/Materials");
	case EBackroomsLevelStyle::L5_Hotel:     return TEXT("/Game/Style/L5_Hotel/Materials");
	case EBackroomsLevelStyle::L6_Dark:      return TEXT("/Game/Style/L6/Materials");
	case EBackroomsLevelStyle::L7_Ocean:     return TEXT("/Game/Style/L7/Materials");
	case EBackroomsLevelStyle::L8_Caves:     return TEXT("/Game/Style/L8/Materials");
	case EBackroomsLevelStyle::L9_Hospital:  return TEXT("/Game/Style/L9/Materials");
	default:                                 return TEXT("/Game/Style/L0/Materials");
	}
}

void ABackroomsChunkActor::ApplyLevelStyle(EBackroomsLevelStyle NewStyle)
{
	LevelStyle = NewStyle;

	// «Вызываем папку» активного уровня: грузим его материалы из
	// /Game/Style/LN_*/Materials (если ассет ещё не создан на диске,
	// LoadObject просто вернёт null и останется базовый материал).
	const FString Prefix = GetLevelStylePath(NewStyle);
	const uint8 Idx = (uint8)NewStyle;

	auto LoadStyleMat = [&Prefix](const TCHAR* Kind, uint8 InIdx) -> UMaterialInterface*
	{
		const FString Name = FString::Printf(TEXT("M_%s_L%u"), Kind, InIdx);
		const FString Path = FString::Printf(TEXT("%s/%s.%s"), *Prefix, *Name, *Name);
		return Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *Path));
	};

	if (UMaterialInterface* NewFloor = LoadStyleMat(TEXT("Floor"), Idx))
	{
		FloorMaterial = NewFloor;
	}
	if (UMaterialInterface* NewWall = LoadStyleMat(TEXT("Wall"), Idx))
	{
		WallMaterial = NewWall;
	}
	if (UMaterialInterface* NewCeiling = LoadStyleMat(TEXT("Ceiling"), Idx))
	{
		CeilingMaterial = NewCeiling;
	}
}

void ABackroomsChunkActor::BeginDestroy()
{
	Super::BeginDestroy();
	ClearMeshData();
	OccupiedBoxes.Reset();
	// Кеш пропсов живёт не дольше чанка (не статический) — очищаем, чтобы
	// не держать указатели на меши после уничтожения чанка.
	LoadedMeshes.Reset();
	MissingMeshes.Reset();
	Instancers.Reset();
}

UStaticMesh* ABackroomsChunkActor::CachedLoadMesh(const TSoftObjectPtr<UStaticMesh>& SoftMesh)
{
	const FName Key = *SoftMesh.GetLongPackageName();
	if (UStaticMesh** Found = LoadedMeshes.Find(Key))
	{
		return *Found;
	}
	if (MissingMeshes.Contains(Key))
	{
		return nullptr;
	}

	// Попытка асинхронной загрузки (не блокирует game thread).
	if (SoftMesh.IsValid())
	{
		if (UStaticMesh* Mesh = SoftMesh.Get())
		{
			LoadedMeshes.Add(Key, Mesh);
			return Mesh;
		}
	}

	// Фолбэк: синхронная загрузка (только если асинхронная не сработала).
	if (UStaticMesh* Mesh = SoftMesh.LoadSynchronous())
	{
		LoadedMeshes.Add(Key, Mesh);
		return Mesh;
	}

	MissingMeshes.Add(Key);
	return nullptr;
}

void ABackroomsChunkActor::ClearMeshData()
{
	Vertices.Reset();
	Triangles.Reset();
	Normals.Reset();
	UV0.Reset();
	VertexColors.Reset();
	Tangents.Reset();
	QuadSlots.Reset();
}

void ABackroomsChunkActor::AddQuad(FVector A, FVector B, FVector C, FVector D, FVector Normal, const FLinearColor& Color, float UVScale, EMaterialSlot Slot)
{
	const int32 Base = Vertices.Num();

	Vertices.Add(A);
	Vertices.Add(B);
	Vertices.Add(C);
	Vertices.Add(D);

	Triangles.Add(Base + 0);
	Triangles.Add(Base + 1);
	Triangles.Add(Base + 2);
	Triangles.Add(Base + 0);
	Triangles.Add(Base + 2);
	Triangles.Add(Base + 3);

	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);

	const FVector2D UV0V(0.0f, 0.0f);
	const FVector2D UV1V(0.0f, UVScale);
	const FVector2D UV2V(UVScale, UVScale);
	const FVector2D UV3V(UVScale, 0.0f);

	UV0.Add(UV0V);
	UV0.Add(UV1V);
	UV0.Add(UV2V);
	UV0.Add(UV3V);

	const FLinearColor C0 = Color;
	const FLinearColor C1 = Color * 0.9f;
	const FLinearColor C2 = Color * 0.85f;
	const FLinearColor C3 = Color;

	VertexColors.Add(C0);
	VertexColors.Add(C1);
	VertexColors.Add(C2);
	VertexColors.Add(C3);

	const FProcMeshTangent Tangent(0.0f, 0.0f, 1.0f);
	Tangents.Add(Tangent);
	Tangents.Add(Tangent);
	Tangents.Add(Tangent);
	Tangents.Add(Tangent);

	QuadSlots.Add((uint8)Slot);
}

void ABackroomsChunkActor::AddBox(FVector Center, FVector Half, const FLinearColor& Color, float UVScale, EMaterialSlot Slot)
{
	const float X0 = Center.X - Half.X;
	const float X1 = Center.X + Half.X;
	const float Y0 = Center.Y - Half.Y;
	const float Y1 = Center.Y + Half.Y;
	const float Z0 = Center.Z - Half.Z;
	const float Z1 = Center.Z + Half.Z;

	const FVector P[8] = {
		FVector(X0, Y0, Z0), FVector(X1, Y0, Z0), FVector(X1, Y1, Z0), FVector(X0, Y1, Z0),
		FVector(X0, Y0, Z1), FVector(X1, Y0, Z1), FVector(X1, Y1, Z1), FVector(X0, Y1, Z1)
	};

	const EMaterialSlot TopSlot = (Slot == EMaterialSlot::Wall) ? EMaterialSlot::Ceiling : Slot;
	const EMaterialSlot BottomSlot = (Slot == EMaterialSlot::Wall) ? EMaterialSlot::Floor : Slot;

	AddQuad(P[4], P[5], P[6], P[7], FVector(0, 0, 1), Color, UVScale, TopSlot);
	AddQuad(P[3], P[2], P[1], P[0], FVector(0, 0, -1), Color, UVScale, BottomSlot);
	AddQuad(P[1], P[2], P[6], P[5], FVector(1, 0, 0), Color, UVScale, Slot);
	AddQuad(P[4], P[7], P[3], P[0], FVector(-1, 0, 0), Color, UVScale, Slot);
	AddQuad(P[2], P[3], P[7], P[6], FVector(0, 1, 0), Color, UVScale, Slot);
	AddQuad(P[0], P[1], P[5], P[4], FVector(0, -1, 0), Color, UVScale, Slot);
}

float ABackroomsChunkActor::WorldNoise(float X, float Y, int32 SeedVal, float FreqX, float FreqY)
{
	const float FX = FreqX != 0.0f ? FreqX : 0.05f;
	const float FY = FreqY != 0.0f ? FreqY : 0.05f;
	return BackroomsNoise::Perlin2D(X * FX + 12.7f, Y * FY + 99.1f, SeedVal);
}

bool ABackroomsChunkActor::EdgeHasWall(int32 CellX, int32 CellY, int32 DirX, int32 DirY) const
{
	const float Mx = CellX + 0.5f * float(DirX + 1);
	const float My = CellY + 0.5f * float(DirY + 1);
	const float Val = BackroomsNoise::Perlin2D(Mx * 0.09f, My * 0.09f, Seed) + 0.09f;
	return Val > 0.0f;
}

bool ABackroomsChunkActor::EdgeHasDoor(int32 CellX, int32 CellY, int32 DirX, int32 DirY) const
{
	const float Mx = CellX + 0.5f * float(DirX + 1);
	const float My = CellY + 0.5f * float(DirY + 1);
	const float Val = BackroomsNoise::Perlin2D(Mx * 0.51f, My * 0.47f, Seed ^ 0x5BF53457);
	return Val > 0.0f;
}

// ---- Слой Generation Data ----
// Плотность/маски/двери/комнаты вычисляет FChunkGenerationData (см.
// BackroomsGenerationData.*). Актор здесь только фасад: инициализирует GenData
// параметрами чанка и извлекает готовые маски для построения геометрии.
void ABackroomsChunkActor::InitGenData()
{
	GenData = MakeShared<FChunkGenerationData>();
	GenData->Coord = FChunkCoord(ChunkX, ChunkY);
	GenData->Seed = Seed;
	// Поля мира — от глобального seed, локальный декор — от Seed чанка.
	GenData->WorldSeed = WorldSeed;
	GenData->bUseWorldSeed = (WorldSeed != 0);
	FChunkGenerationData::FParams& P = GenData->Params;

	// Профиль уровня (с ULevelConfig или без) — единственный источник FParams.
	// Индекс локации выбирает таблицу видов комнат: используем полный номер
	// уровня (0..16) — для L10..L16 таблица VariantSystem даёт fallback Neutral.
	if (Profile)
	{
		P = Profile->ToGenParams(Profile->ConfigLevelIndex);
		GenData->Generate();
		return;
	}

	P.CellCount = ChunkSizeCells;
	P.WallThreshold = WallThreshold;
	P.DoorThreshold = DoorThreshold;
	P.ScatterThreshold = ScatterThreshold;
	P.LivingRoomThreshold = LivingRoomThreshold;
	P.MinRoomCells = MinRoomCells;
	P.MaxRoomCells = MaxRoomCells;
	P.Pattern = static_cast<FChunkGenerationData::EChunkLayoutPattern>((uint8)LayoutPattern);
	// Частоты шума выводим из правил размера комнат профиля, а не берём из
	// захардкоженного поля: средняя комната ~= 1/(avg*1.4) и ~1/(avg*10)
	// дают прежнее 0.09/0.0125 при Min=4,Max=12, но масштабируются под профиль
	// (мелкие комнаты -> плотнее шум, крупные -> грубее). Отдельное поле
	// BaseNoiseFreq/MacroNoiseFreq больше не является источником истины.
	{
		const int32 AvgRoom = FMath::Max(1, (MinRoomCells + MaxRoomCells) / 2);
		P.BaseNoiseFreq = FMath::Clamp(1.0f / (float)(AvgRoom * 14), 0.03f, 0.20f);
		P.MacroNoiseFreq = FMath::Clamp(1.0f / (float)(AvgRoom * 100), 0.004f, 0.03f);
	}
	// Индекс локации выбирает таблицу видов комнат (сюда попадаем только при
	// отсутствии профиля — BuildChunk). Порядок EBackroomsLevelStyle совпадает
	// с нумерацией L0..L9, поэтому каст корректный; L10..L16 -> L0-таблица.
	P.LevelIndex = (int32)LevelStyle;
	GenData->Generate();
}

// Преобразует готовые клетки GenData в маски WallMask/DoorMask, которыми
// строится геометрия (стена/дверной проём). Семантика сохранена:
//   WallMask[c] == 1 -> плотность выше порога стены (твёрдый блок),
//   DoorMask[c] == 1 -> в стене «высверлен» дверной проём.
void ABackroomsChunkActor::BuildRoomGraph(const FChunkGenerationData* Neighbors[4])
{
	if (!GenData)
	{
		return;
	}
	RoomGraph.BuildFromChunk(*GenData);
	for (int32 Dir = 0; Dir < 4; ++Dir)
	{
		if (Neighbors[Dir])
		{
			RoomGraph.AddBorderEdges(*GenData, *Neighbors[Dir], Dir);
		}
	}
}

void ABackroomsChunkActor::GenerateMasksFromGenData(TArray<uint8>& WallMask, TArray<uint8>& DoorMask) const
{
	const int32 Count = ChunkSizeCells;
	WallMask.SetNum(Count * Count);
	DoorMask.SetNumZeroed(Count * Count);
	if (GenData && GenData->Cells.Num() == Count * Count)
	{
		for (int32 LY = 0; LY < Count; ++LY)
		{
			for (int32 LX = 0; LX < Count; ++LX)
			{
				const int32 I = MaskIndex(LX, LY, Count);
				const FChunkCell& C = GenData->Cells[I];
				WallMask[I] = C.bIsWall ? 1 : 0;
				DoorMask[I] = C.bIsDoor ? 1 : 0;
			}
		}
	}

	// Actor-side stage: the masks fed to the mesh builder must be a faithful
	// projection of the generation data, otherwise geometry silently diverges.
	if (GenData && FChunkGenerationVerifier::GetMode() != EGenerationVerifyMode::Off)
	{
		TArray<FString> Failures;
		int32 Checks = 0;
		FChunkGenerationVerifier::VerifyMasks(*GenData, WallMask, DoorMask, Failures, Checks);
		if (Failures.Num() > 0)
		{
			if (FChunkGenerationVerifier::GetMode() == EGenerationVerifyMode::Strict)
			{
				for (const FString& F : Failures)
				{
					UE_LOG(LogTemp, Error, TEXT("BR Gen Masks (%d,%d): %s"), ChunkX, ChunkY, *F);
				}
			}
			else
			{
				for (const FString& F : Failures)
				{
					UE_LOG(LogTemp, Warning, TEXT("BR Gen Masks (%d,%d): %s"), ChunkX, ChunkY, *F);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Verbose, TEXT("BR Gen Masks (%d,%d): PASS (%d checks)"), ChunkX, ChunkY, Checks);
		}
	}
}

void ABackroomsChunkActor::NotifyStoryPropPickedUp(ABackroomsPhysProp* Prop)
{
	if (!Prop)
	{
		return;
	}
	const int32 RoomID = Prop->GetRoomID();
	if (RoomID < 0)
	{
		return;
	}

	TArray<TWeakObjectPtr<AActor>>& RoomProps = StoryPropsByRoom.FindOrAdd(RoomID);
	const TWeakObjectPtr<AActor> WeakProp(Prop);
	if (RoomProps.Contains(WeakProp))
	{
		return;
	}
	RoomProps.Add(WeakProp);

	// Сюжетная сцена комнаты считается собранной, когда все её известные
	// пропсы оказались в руках игрока; триггер излучается один раз за комнату.
	if (!PickedNarrativeRooms.Contains(RoomID))
	{
		PickedNarrativeRooms.Add(RoomID);
		TArray<AActor*> PropList;
		for (const TWeakObjectPtr<AActor>& W : RoomProps)
		{
			if (W.IsValid())
			{
				PropList.Add(W.Get());
			}
		}
		const EStorySceneType ScenarioID = static_cast<EStorySceneType>(Prop->GetScenarioID());
		OnNarrativeTrigger.Broadcast(ScenarioID, PropList);
	}
}

// Строим геометрию чанка по картам стен (блочная раскладка).
//   Не-стена (WallMask=0, включая «дверные» клетки, высверленные в проходы):
//     плита пола + потолок — проходимое помещение.
//   Стена (WallMask=1): тонкая перегородка по краям клетки (WallThickness),
//     а не сплошной блок на всю клетку. Это делает комнаты просторнее.
void ABackroomsChunkActor::BuildGeometryFromMasks(const TArray<uint8>& WallMask, const TArray<uint8>& DoorMask)
{
	(void)DoorMask; // дверные проёмы режутся прямо здесь из DoorMask
	LastWallMask = WallMask; // для самопроверки и ASCII-карты (см. шпаргалку)
	LastDoorMask = DoorMask;
	const int32 Count = ChunkSizeCells;
	const float UVScale = 4.0f;
	const bool bClearRoom = (ChunkX == 0 && ChunkY == 0);

	// «Чистая» стартовая комната (чанк 0,0): без внутренних стен, но с полноценным
	// полом/потолком на всю площадь. Раньше тут ничего не строилось — игрок
	// проваливался в пустоту и «садился» на верх стен соседних чанков.
	if (bClearRoom)
	{
		const float HalfExtent = (float)Count * CellSize * 0.5f;
		const float CX = (float)ChunkX * Count * CellSize + HalfExtent;
		const float CY = (float)ChunkY * Count * CellSize + HalfExtent;
		AddBox(FVector(CX, CY, -40.0f),
			FVector(HalfExtent, HalfExtent, 40.0f),
			PaletteFloor, UVScale, EMaterialSlot::Floor);
		AddBox(FVector(CX, CY, WallHeight + 40.0f),
			FVector(HalfExtent, HalfExtent, 40.0f),
			PaletteCeiling, UVScale, EMaterialSlot::Ceiling);
	}
	else
	{
		// Сначала пол и потолок под всеми клетками (кроме чистой комнаты).
		for (int32 LY = 0; LY < Count; ++LY)
		{
			for (int32 LX = 0; LX < Count; ++LX)
			{
				const int32 WX = ChunkX * Count + LX;
				const int32 WY = ChunkY * Count + LY;
				const float BX = (float)WX * CellSize;
				const float BY = (float)WY * CellSize;
				const int32 I = MaskIndex(LX, LY, Count);

				// Пол и потолок — под любой клеткой (включая стены и двери),
				// но только если это не сплошная стена БЕЗ двери.
				// Дверная клетка получает пол/потолок в проёме.
				if (WallMask[I] == 0 || DoorMask[I] != 0)
				{
					AddBox(FVector(BX + CellSize * 0.5f, BY + CellSize * 0.5f, -40.0f),
						FVector(CellSize * 0.5f, CellSize * 0.5f, 40.0f),
						PaletteFloor, UVScale, EMaterialSlot::Floor);
					AddBox(FVector(BX + CellSize * 0.5f, BY + CellSize * 0.5f, WallHeight + 40.0f),
						FVector(CellSize * 0.5f, CellSize * 0.5f, 40.0f),
						PaletteCeiling, UVScale, EMaterialSlot::Ceiling);
				}
			}
		}

		// Теперь стены: тонкие перегородки на границах между стеной и полом/дверью.
		// Проходим по всем стенам и смотрим на 4 соседние клетки.
		const float HalfThick = WallThickness * 0.5f;
		const float HalfH = WallHeight * 0.5f;
		const float WallZ = HalfH;

		for (int32 LY = 0; LY < Count; ++LY)
		{
			for (int32 LX = 0; LX < Count; ++LX)
			{
				const int32 WX = ChunkX * Count + LX;
				const int32 WY = ChunkY * Count + LY;
				const float BX = (float)WX * CellSize;
				const float BY = (float)WY * CellSize;
				const int32 I = MaskIndex(LX, LY, Count);

				if (WallMask[I] == 0)
				{
					continue; // не стена — стены не строим
				}

				const bool bIsDoor = DoorMask[I] != 0;

				// 4 направления: +X, -X, +Y, -Y
				static const int32 DX[4] = { 1, -1, 0, 0 };
				static const int32 DY[4] = { 0, 0, 1, -1 };

				for (int32 d = 0; d < 4; ++d)
				{
					const int32 NX = LX + DX[d];
					const int32 NY = LY + DY[d];
					const int32 NWX = WX + DX[d];
					const int32 NWY = WY + DY[d];

					bool bNeighborIsWall = false;
					bool bNeighborIsDoor = false;

					if (NX >= 0 && NX < Count && NY >= 0 && NY < Count)
					{
						const int32 NI = MaskIndex(NX, NY, Count);
						bNeighborIsWall = WallMask[NI] != 0;
						bNeighborIsDoor = DoorMask[NI] != 0;
					}
					else
					{
						// Граница чанка: запрашиваем у GenData только СТЕНУ (чистая
						// функция мировой координаты + WorldSeed). Дверь соседа здесь
						// не предсказываем: дверная клетка — это стена (CellIsWall=true),
						// поэтому общую грань однозначно построит сам сосед, а
						// угадывание двери по старому per-chunk seed давало швы.
						if (GenData)
						{
							bNeighborIsWall = GenData->CellIsWall(NWX, NWY);
						}
					}

					// Строим стену на этом крае, если сосед — НЕ стена ИЛИ сосед — дверь
					// (дверь = проём в стене, поэтому стена на крае двери тоже нужна — пилоны)
					const bool bBuildWallHere = !bNeighborIsWall || bNeighborIsDoor;

					if (!bBuildWallHere)
					{
						continue; // сосед тоже стена — общая стена построится от соседа
					}

					// Определяем ориентацию стены
					const bool bVerticalWall = (DX[d] != 0); // стена вертикальная (разделяет по X)

					if (bIsDoor)
					{
						// Дверной проём: строим пилоны по бокам и перемычку
						const float CX = BX + CellSize * 0.5f;
						const float CY = BY + CellSize * 0.5f;
						const float SideIn = (CellSize - DoorWidth) * 0.5f;
						const float SideH = SideIn * 0.5f;
						const float LintelH = (WallHeight - DoorHeight) * 0.5f;

						// Ось проёма: если стена вертикальная (разделяет по X), проём по X
						const bool bPassageX = bVerticalWall;

						if (!bPassageX)
						{
							// Проём по Y (стена горизонтальная) — пилоны сверху и снизу
							AddBox(FVector(CX, CY - DoorWidth * 0.5f - SideH, WallZ),
								FVector(HalfThick, SideH, HalfH), PaletteWall, 1.0f, EMaterialSlot::Wall);
							AddBox(FVector(CX, CY + DoorWidth * 0.5f + SideH, WallZ),
								FVector(HalfThick, SideH, HalfH), PaletteWall, 1.0f, EMaterialSlot::Wall);
							AddBox(FVector(CX, CY, WallHeight - LintelH),
								FVector(HalfThick, DoorWidth * 0.5f, LintelH), PaletteWall, 1.0f, EMaterialSlot::Wall);

							// Пол и потолок в проёме
							AddBox(FVector(CX, CY, -40.0f),
								FVector(DoorWidth * 0.5f, DoorWidth * 0.5f, 40.0f), PaletteFloor, UVScale, EMaterialSlot::Floor);
							AddBox(FVector(CX, CY, WallHeight + 40.0f),
								FVector(DoorWidth * 0.5f, DoorWidth * 0.5f, 40.0f), PaletteCeiling, UVScale, EMaterialSlot::Ceiling);
						}
						else
						{
							// Проём по X (стена вертикальная) — пилоны слева и справа
							AddBox(FVector(CX - DoorWidth * 0.5f - SideH, CY, WallZ),
								FVector(SideH, HalfThick, HalfH), PaletteWall, 1.0f, EMaterialSlot::Wall);
							AddBox(FVector(CX + DoorWidth * 0.5f + SideH, CY, WallZ),
								FVector(SideH, HalfThick, HalfH), PaletteWall, 1.0f, EMaterialSlot::Wall);
							AddBox(FVector(CX, CY, WallHeight - LintelH),
								FVector(DoorWidth * 0.5f, HalfThick, LintelH), PaletteWall, 1.0f, EMaterialSlot::Wall);

							AddBox(FVector(CX, CY, -40.0f),
								FVector(DoorWidth * 0.5f, DoorWidth * 0.5f, 40.0f), PaletteFloor, UVScale, EMaterialSlot::Floor);
							AddBox(FVector(CX, CY, WallHeight + 40.0f),
								FVector(DoorWidth * 0.5f, DoorWidth * 0.5f, 40.0f), PaletteCeiling, UVScale, EMaterialSlot::Ceiling);
						}
					}
					else
					{
						// Обычная тонкая стена (без двери)
						if (bVerticalWall)
						{
							// Стена по Y (разделяет клетки по X) — тонкий бокс вдоль Y
							const float WallX = BX + (DX[d] > 0 ? CellSize : 0.0f);
							const float WallY = BY + CellSize * 0.5f;
							AddBox(FVector(WallX, WallY, WallZ),
								FVector(HalfThick, CellSize * 0.5f, HalfH),
								PaletteWall, 1.0f, EMaterialSlot::Wall);
						}
						else
						{
							// Стена по X (разделяет клетки по Y) — тонкий бокс вдоль X
							const float WallX = BX + CellSize * 0.5f;
							const float WallY = BY + (DY[d] > 0 ? CellSize : 0.0f);
							AddBox(FVector(WallX, WallY, WallZ),
								FVector(CellSize * 0.5f, HalfThick, HalfH),
								PaletteWall, 1.0f, EMaterialSlot::Wall);
						}
					}
				}
			}
		}
	}
}

void ABackroomsChunkActor::BuildNaniteWallSkins()
{
	if (!bNaniteSkinsEnabled || NaniteSkinMesh.IsNull())
	{
		return;
	}
	UStaticMesh* SkinMesh = CachedLoadMesh(NaniteSkinMesh);
	if (!SkinMesh)
	{
		return;
	}

	UInstancedStaticMeshComponent* Inst = GetOrCreateInstancer(SkinMesh, /*bBlocking=*/false);
	if (!Inst)
	{
		return;
	}
	// Скин лежит на стене — он обязан использовать тот же материал стен, что и
	// процедурная геометрия (иначе панель «светится» другим тоном/фолбэком).
	if (UMaterialInterface* WallMat = WallMaterial ? WallMaterial : CityWallMaterial)
	{
		Inst->SetMaterial(0, WallMat);
	}

	const float Ref = FMath::Max(NaniteSkinRefSize, 1.0f);
	const float MinDim = FMath::Max(NaniteSkinMinSize, 1.0f);

	for (int32 Quad = 0; Quad < QuadSlots.Num(); ++Quad)
	{
		if ((uint8)QuadSlots[Quad] != (uint8)EMaterialSlot::Wall)
		{
			continue;
		}

		const int32 V0 = Quad * 4;
		const FVector& A = Vertices[V0 + 0];
		const FVector& B = Vertices[V0 + 1];
		const FVector& C = Vertices[V0 + 2];
		const FVector& D = Vertices[V0 + 3];

		const FVector E0 = B - A;
		const FVector E1 = C - B;
		const float Len0 = E0.Size();
		const float Len1 = E1.Size();
		if (Len0 < MinDim || Len1 < MinDim)
		{
			continue; // торцы перегородок (толщина) и перемычки дверей — мимо
		}

		const FVector Normal = Normals[V0];
		// Только боковые грани стен (топ/бот уже ушли в Ceiling/Floor-слоты,
		// но для внутренних стен AddBox переопределяет и их — отсекаем по нормали).
		if (FMath::Abs(Normal.Z) > 0.01f)
		{
			continue;
		}

		const FVector Center = (A + B + C + D) * 0.25f;
		const FVector XAxis = E0 / Len0;
		const FVector ZAxis = Normal;
		const FVector YAxis = FVector::CrossProduct(ZAxis, XAxis);
		if (YAxis.IsNearlyZero())
		{
			continue;
		}

		FMatrix M;
		M.SetAxis(0, XAxis);
		M.SetAxis(1, YAxis);
		M.SetAxis(2, ZAxis);
		M.SetOrigin(FVector::ZeroVector);
		const FQuat Rot = M.ToQuat();

		const FVector Origin = Center + Normal * NaniteSkinOffset;
		const FVector Scale(Len0 / Ref, Len1 / Ref, 1.0f);
		Inst->AddInstance(FTransform(Rot, Origin, Scale));
	}
}

void ABackroomsChunkActor::SpawnGraffitiDecals()
{
	if (GraffitiDecalChance <= 0.0f || GraffitiDecalMaterials.IsEmpty())
	{
		return;
	}

	FRandomStream Random(Seed ^ (ChunkX * 58573361) ^ (ChunkY * 39760391));
	const float DepthCm = 2.0f; // проекция декаля не должна доставать до соседней стены

	for (int32 Quad = 0; Quad < QuadSlots.Num(); ++Quad)
	{
		if ((uint8)QuadSlots[Quad] != (uint8)EMaterialSlot::Wall)
		{
			continue;
		}
		if (Random.FRand() > GraffitiDecalChance)
		{
			continue;
		}

		const int32 V0 = Quad * 4;
		const FVector& A = Vertices[V0 + 0];
		const FVector& B = Vertices[V0 + 1];
		const FVector& C = Vertices[V0 + 2];
		const FVector& D = Vertices[V0 + 3];
		const FVector Normal = Normals[V0];
		if (FMath::Abs(Normal.Z) > 0.01f)
		{
			continue;
		}

		const FVector E0 = B - A;
		const FVector E1 = C - B;
		const float Len0 = E0.Size();
		const float Len1 = E1.Size();

		// Грань слишком мала под надпись — мимо.
		if (Len0 < GraffitiDecalSizeCm.X || Len1 < GraffitiDecalSizeCm.Y)
		{
			continue;
		}

		const int32 Mi = Random.RandRange(0, GraffitiDecalMaterials.Num() - 1);
		UMaterialInterface* Mat = GraffitiDecalMaterials[Mi].LoadSynchronous();
		if (!Mat)
		{
			continue;
		}

		UDecalComponent* Decal = NewObject<UDecalComponent>(this);
		if (!Decal)
		{
			continue;
		}

		// Раскладка на грани с разбросом: ширина/высота масштабируются случайно,
		// но не вылезают за грань, высота — на уровне взгляда (плечи/зрение).
		const float Width = FMath::Min(GraffitiDecalSizeCm.X * Random.FRandRange(0.8f, 1.3f), Len0 * 0.85f);
		const float Height = FMath::Min(GraffitiDecalSizeCm.Y * Random.FRandRange(0.8f, 1.3f), Len1 * 0.55f);
		const FVector Center = (A + B + C + D) * 0.25f;
		FVector Loc = Center + Normal * 1.0f;
		Loc.Z = 100.0f + Random.FRandRange(0.0f, FMath::Max(Len1 - Height, 20.0f) * 0.5f);

		// Декал проецируется вдоль своей локальной +Y: X-Axis — вдоль грани,
		// Z-Axis — вверх, тогда +Y направлен в стену (-Normal).
		const FVector XAxis = E0 / Len0;
		const FQuat Rot = FRotationMatrix::MakeFromXZ(XAxis, FVector::UpVector).ToQuat();

		Decal->SetMobility(EComponentMobility::Movable);
		Decal->SetDecalMaterial(Mat);
		Decal->SetWorldLocationAndRotation(Loc, Rot);
		Decal->DecalSize = FVector(Width * 0.5f, DepthCm * 0.5f, Height * 0.5f);
		Decal->RegisterComponent();
	}
}

void ABackroomsChunkActor::BuildEdgeWall(int32 CellX, int32 CellY, int32 DirX, int32 DirY)
{
	const float EdgeX = (float)CellX * CellSize;
	const float EdgeY = (float)CellY * CellSize;
	const float NextX = (float)(CellX + DirX) * CellSize;
	const float NextY = (float)(CellY + DirY) * CellSize;

	const bool bWall = EdgeHasWall(CellX, CellY, DirX, DirY);
	const bool bDoor = bWall && EdgeHasDoor(CellX, CellY, DirX, DirY);

	if (!bWall)
	{
		return;
	}

	FVector Center;
	FVector Half;

	if (DirX != 0)
	{
		Center = FVector((EdgeX + NextX) * 0.5f, (EdgeY + NextY) * 0.5f, WallHeight * 0.5f);
		Half = FVector(WallThickness * 0.5f, CellSize * 0.5f, WallHeight * 0.5f);
	}
	else
	{
		Center = FVector((EdgeX + NextX) * 0.5f, (EdgeY + NextY) * 0.5f, WallHeight * 0.5f);
		Half = FVector(CellSize * 0.5f, WallThickness * 0.5f, WallHeight * 0.5f);
	}

	if (!bDoor)
	{
		AddBox(Center, Half, PaletteWall, 1.0f, EMaterialSlot::Wall);
		return;
	}

	const float WallLength = CellSize;

	if (DirX != 0)
	{
		FVector WallCenterA(Center.X, Center.Y - WallLength * 0.5f + DoorWidth * 0.5f, WallHeight * 0.5f);
		FVector WallCenterB(Center.X, Center.Y + WallLength * 0.5f - DoorWidth * 0.5f, WallHeight * 0.5f);
		FVector WallTop(Center.X, Center.Y, WallHeight - DoorHeight * 0.5f);
		FVector HalfA(WallThickness * 0.5f, (WallLength * 0.5f - DoorWidth * 0.5f), WallHeight * 0.5f);
		FVector HalfB(WallThickness * 0.5f, (WallLength * 0.5f - DoorWidth * 0.5f), WallHeight * 0.5f);
		FVector HalfTop(WallThickness * 0.5f, WallLength * 0.5f, DoorHeight * 0.5f);
		AddBox(WallCenterA, HalfA, PaletteWall, 1.0f, EMaterialSlot::Wall);
		AddBox(WallCenterB, HalfB, PaletteWall, 1.0f, EMaterialSlot::Wall);
		AddBox(WallTop, HalfTop, PaletteWall, 1.0f, EMaterialSlot::Wall);
	}
	else
	{
		FVector WallCenterA(Center.X - WallLength * 0.5f + DoorWidth * 0.5f, Center.Y, WallHeight * 0.5f);
		FVector WallCenterB(Center.X + WallLength * 0.5f - DoorWidth * 0.5f, Center.Y, WallHeight * 0.5f);
		FVector WallTop(Center.X, Center.Y, WallHeight - DoorHeight * 0.5f);
		FVector HalfA((WallLength * 0.5f - DoorWidth * 0.5f), WallThickness * 0.5f, WallHeight * 0.5f);
		FVector HalfB((WallLength * 0.5f - DoorWidth * 0.5f), WallThickness * 0.5f, WallHeight * 0.5f);
		FVector HalfTop(WallLength * 0.5f, WallThickness * 0.5f, DoorHeight * 0.5f);
		AddBox(WallCenterA, HalfA, PaletteWall, 1.0f, EMaterialSlot::Wall);
		AddBox(WallCenterB, HalfB, PaletteWall, 1.0f, EMaterialSlot::Wall);
		AddBox(WallTop, HalfTop, PaletteWall, 1.0f, EMaterialSlot::Wall);
	}
}

FBox2D ABackroomsChunkActor::CellBounds(int32 CellX, int32 CellY) const
{
	const float CX = (float)CellX * CellSize;
	const float CY = (float)CellY * CellSize;
	return FBox2D(FVector2D(CX, CY), FVector2D(CX + CellSize, CY + CellSize));
}

UInstancedStaticMeshComponent* ABackroomsChunkActor::GetOrCreateInstancer(UStaticMesh* Mesh, bool bBlocking)
{
	if (TObjectPtr<UInstancedStaticMeshComponent>* Existing = Instancers.Find(Mesh))
	{
		return Existing->Get();
	}

	UInstancedStaticMeshComponent* Inst = NewObject<UInstancedStaticMeshComponent>(this);
	if (Mesh)
	{
		for (const FStaticMaterial& SM : Mesh->GetStaticMaterials())
		{
			if (SM.MaterialInterface && SM.MaterialInterface->GetBlendMode() == BLEND_Translucent)
			{
				Inst->bDisallowNanite = true;
				break;
			}
		}
	}
	Inst->SetStaticMesh(Mesh);
	{
		const int32 NumSlots = Mesh ? Mesh->GetStaticMaterials().Num() : 0;
		for (int32 S = 0; S < FMath::Max(1, NumSlots); ++S)
		{
			if (!Inst->GetMaterial(S))
			{
				UMaterialInterface* Fallback = CityWallMaterial;
				if (!Fallback) { Fallback = WallMaterial; }
				if (!Fallback) { Fallback = FloorMaterial; }
				if (Fallback)
				{
					Inst->SetMaterial(S, Fallback);
				}
			}
		}
	}
	// Пропсы (bBlocking=false) не блокируют игрока; здания города — блокируют.
	// Примечание: ISMC-инстансы используют per-instance простую коллизию меша
	// (complex-as-simple для инстансов движок не поддерживает), поэтому у
	// зданий должна быть простая коллизия в самом меше (генеруается при импорте).
	if (bBlocking)
	{
		Inst->SetCollisionObjectType(ECC_WorldDynamic);
		Inst->SetCollisionResponseToAllChannels(ECR_Block);
		Inst->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		Inst->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	Inst->SetupAttachment(RootComponent);
	Inst->RegisterComponent();
	AddInstanceComponent(Inst);
	Instancers.Add(Mesh, Inst);
	return Inst;
}

bool ABackroomsChunkActor::OverlapsOccupied(const FBox2D& Box) const
{
	for (const FBox2D& Other : OccupiedBoxes)
	{
		if (Box.Intersect(Other))
		{
			return true;
		}
	}
	return false;
}

void ABackroomsChunkActor::ReserveDoorCorridor(int32 CellX, int32 CellY)
{
	const int32 LX = CellX - ChunkX * ChunkSizeCells;
	const int32 LY = CellY - ChunkY * ChunkSizeCells;
	if (LX < 0 || LX >= ChunkSizeCells || LY < 0 || LY >= ChunkSizeCells)
	{
		return;
	}
	if (LastDoorMask.Num() != ChunkSizeCells * ChunkSizeCells)
	{
		return;
	}
	if (LastDoorMask[MaskIndex(LX, LY, ChunkSizeCells)] == 0)
	{
		return;
	}

	const float CX = (float)CellX * CellSize;
	const float CY = (float)CellY * CellSize;
	const float Deep = 160.0f;
	const float HalfW = DoorWidth * 0.5f;

	// Полосы вдоль проёма в соседних открытых клетках — чтобы пропсы не
	// загораживали проходы между комнатами.
	OccupiedBoxes.Add(FBox2D(FVector2D(CX + CellSize, CY - HalfW),
		FVector2D(CX + CellSize + Deep, CY + HalfW)));
	OccupiedBoxes.Add(FBox2D(FVector2D(CX - Deep, CY - HalfW),
		FVector2D(CX, CY + HalfW)));
	OccupiedBoxes.Add(FBox2D(FVector2D(CX - HalfW, CY + CellSize),
		FVector2D(CX + HalfW, CY + CellSize + Deep)));
	OccupiedBoxes.Add(FBox2D(FVector2D(CX - HalfW, CY - Deep),
		FVector2D(CX + HalfW, CY)));
}

void ABackroomsChunkActor::PlaceProps(int32 CellX, int32 CellY, FRandomStream& Random)
{
	if (!Database || Database->Props.Num() == 0)
	{
		return;
	}

	// Маска разбросанных объектов (scatter): пропсы кладём только в зонах
	// низкой плотности (на полу), а не среди стен — берём готовый флаг
	// плотностного слоя GenData (CellIsScatter).
	{
		if (GenData && !GenData->CellIsScatter(CellX, CellY))
		{
			return;
		}
	}

	const FBox2D Cell = CellBounds(CellX, CellY);

	int32 PropCount = FMath::RoundToInt(Random.FRandRange(0.0f, MaxPropsPerRoom * 0.6f));

	// Кандидатов с загруженными мешами собираем ОДИН раз на комнату.
	struct FCandidate { const FPropRule* Rule; UStaticMesh* Mesh; };
	TArray<FCandidate> Candidates;
	float TotalWeight = 0.0f;
	for (const FPropRule& Rule : Database->Props)
	{
		// Городские здания Buildings/* живут только на платформе-острове —
		// в комнаты Бэкрумса их НЕ кладём.
		const FString Pkg = Rule.Mesh.GetLongPackageName();
		if (Pkg.Contains(TEXT("/Buildings/")))
		{
			continue;
		}
		UStaticMesh* Mesh = CachedLoadMesh(Rule.Mesh);
		if (Mesh)
		{
			Candidates.Add({ &Rule, Mesh });
			TotalWeight += Rule.Probability;
		}
	}

	for (int32 i = 0; i < PropCount; ++i)
	{
		if (Candidates.Num() == 0)
		{
			break;
		}

		float Roll = Random.FRandRange(0.0f, TotalWeight);
		const FCandidate* Chosen = &Candidates.Last();
		for (const FCandidate& Cand : Candidates)
		{
			if (Roll <= Cand.Rule->Probability)
			{
				Chosen = &Cand;
				break;
			}
			Roll -= Cand.Rule->Probability;
		}

		UStaticMesh* Mesh = Chosen->Mesh;
		if (!Mesh)
		{
			break;
		}

		const float Scale = Random.FRandRange(Chosen->Rule->ScaleMin.X, Chosen->Rule->ScaleMax.X);
		const FBoxSphereBounds Bounds = Mesh->GetBounds();

		FVector Pos;
		FRotator Rot;

		const float Inset = 90.0f;

		switch (Chosen->Rule->Placement)
		{
		case EPropPlacement::Floor:
			Pos = FVector(
				Random.FRandRange(Cell.Min.X + Inset, Cell.Max.X - Inset),
				Random.FRandRange(Cell.Min.Y + Inset, Cell.Max.Y - Inset),
				0.0f);
			Rot = FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
			break;
		case EPropPlacement::Wall:
			{
				const int32 Side = Random.RandRange(0, 3);
				switch (Side)
				{
				case 0:
					Pos = FVector(Cell.Min.X + Inset, Random.FRandRange(Cell.Min.Y + Inset, Cell.Max.Y - Inset), 0.0f);
					Rot = FRotator(0.0f, 90.0f, 0.0f);
					break;
				case 1:
					Pos = FVector(Cell.Max.X - Inset, Random.FRandRange(Cell.Min.Y + Inset, Cell.Max.Y - Inset), 0.0f);
					Rot = FRotator(0.0f, -90.0f, 0.0f);
					break;
				case 2:
					Pos = FVector(Random.FRandRange(Cell.Min.X + Inset, Cell.Max.X - Inset), Cell.Min.Y + Inset, 0.0f);
					Rot = FRotator(0.0f, 0.0f, 0.0f);
					break;
				default:
					Pos = FVector(Random.FRandRange(Cell.Min.X + Inset, Cell.Max.X - Inset), Cell.Max.Y - Inset, 0.0f);
					Rot = FRotator(0.0f, 180.0f, 0.0f);
					break;
				}
			}
			break;
		case EPropPlacement::Ceiling:
			Pos = FVector(Cell.GetCenter().X, Cell.GetCenter().Y, WallHeight - 30.0f);
			Rot = FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
			break;
		case EPropPlacement::Corner:
			{
				const int32 Side = Random.RandRange(0, 3);
				const float CX = (Side % 2 == 0) ? Cell.Min.X + Inset : Cell.Max.X - Inset;
				const float CY = (Side < 2) ? Cell.Min.Y + Inset : Cell.Max.Y - Inset;
				Pos = FVector(CX, CY, 0.0f);
				Rot = FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
			}
			break;
		default:
			continue;
		}

const float FootX = Bounds.BoxExtent.X * Scale;
			const float FootY = Bounds.BoxExtent.Y * Scale;
			const float YawRad = FMath::DegreesToRadians(Rot.Yaw);
			const float CosYaw = FMath::Cos(YawRad);
			const float SinYaw = FMath::Sin(YawRad);
			const float RotFootX = FMath::Abs(FootX * CosYaw) + FMath::Abs(FootY * SinYaw);
			const float RotFootY = FMath::Abs(FootX * SinYaw) + FMath::Abs(FootY * CosYaw);
			FBox2D Foot(FVector2D(Pos.X - RotFootX, Pos.Y - RotFootY), FVector2D(Pos.X + RotFootX, Pos.Y + RotFootY));

			{
				FCollisionQueryParams PropQP(SCENE_QUERY_STAT(PropOverlap), false, nullptr);
				const float PropHalfZ = Bounds.BoxExtent.Z * Scale + 10.0f;
				if (GetWorld()->OverlapBlockingTestByChannel(
					FVector(Pos.X, Pos.Y, Pos.Z + PropHalfZ),
					FQuat::Identity,
					ECC_WorldStatic,
					FCollisionShape::MakeBox(FVector(RotFootX + 5.0f, RotFootY + 5.0f, PropHalfZ)),
					PropQP))
				{
					continue;
				}
			}

			if (OverlapsOccupied(Foot))
			{
				continue;
			}

			OccupiedBoxes.Add(Foot);

			// Нижняя грань пропса на полу (а не половина высоты): корректно
			// и для пивота в центре, и для пивота в подошве меша.
			const float GroundZ = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale;
			const FVector FinalPos(Pos.X, Pos.Y, -GroundZ);
			UInstancedStaticMeshComponent* Inst = GetOrCreateInstancer(Mesh);
			if (Inst)
			{
				Inst->AddInstance(FTransform(Rot, FinalPos, FVector(Scale)));
			}
		}
}

void ABackroomsChunkActor::PlacePropsFromProfile(int32 CellX, int32 CellY, FRandomStream& Random)
{
	if (!Profile || Profile->Props.Num() == 0)
	{
		// Нет профиля — откат на общий пул.
		PlaceProps(CellX, CellY, Random);
		return;
	}

	// Пропсы раскладываются ПО КОМНАТАМ, а не по клеткам: комната обрабатывается
	// один раз целиком (PropRoomsPlaced), её реальный размер (FRoomInfo::CellCount)
	// влияет на количество пропсов, а вид комнаты берётся по её центру.
	const int32 Count = GenData ? GenData->Params.CellCount : 0;

	// Без GenData (не было плотностной генерации) — старый покадровый откат.
	if (!GenData || Count == 0)
	{
		PlaceProps(CellX, CellY, Random);
		return;
	}

	const int32 RoomIdx = GenData->RoomIndexAt(
		CellX - GenData->Coord.X * Count,
		CellY - GenData->Coord.Y * Count);
	if (RoomIdx == INDEX_NONE)
	{
		// Стены и переходы — не место для пропсов.
		return;
	}
	if (PropRoomsPlaced.Contains(RoomIdx))
	{
		return;
	}
	PropRoomsPlaced.Add(RoomIdx);

	const FChunkGenerationData::FRoomInfo& Room = GenData->LastRooms[RoomIdx];

	// Scatter-право комнаты считаем по её центру в мировых координатах:
	// решение не зависит от порядка обхода клеток и одинаково для соседних чанков.
	const int32 CenterLX = (Room.MinCell.X + Room.MaxCell.X) / 2;
	const int32 CenterLY = (Room.MinCell.Y + Room.MaxCell.Y) / 2;
	const int32 CenterWX = GenData->WorldX(CenterLX);
	const int32 CenterWY = GenData->WorldY(CenterLY);
	if (!GenData->CellIsScatter(CenterWX, CenterWY))
	{
		return;
	}

	// Мировые границы комнаты (в см): локальная клеточная рамка + смещение чанка.
	const FBox2D LocalB = GenData->RoomBoundsLocal(RoomIdx);
	const float WorldOffX = (float)(GenData->Coord.X * Count) * CellSize;
	const float WorldOffY = (float)(GenData->Coord.Y * Count) * CellSize;
	const FBox2D Cell(
		FVector2D(LocalB.Min.X * CellSize + WorldOffX, LocalB.Min.Y * CellSize + WorldOffY),
		FVector2D(LocalB.Max.X * CellSize + WorldOffX, LocalB.Max.Y * CellSize + WorldOffY));

	// Вид комнаты по её центру — стабилен и согласован между чанками.
	// Берём уточнённый вид (RefineVariants уже подставил фактический размер
	// комнаты вместо черновой оценки), чтобы свет/высота/раскладка пропсов
	// совпадали с тем, что видит игрок в этой комнате.
	const ERoomVariant VariantId = GenData->CellVariant(CenterLX, CenterLY);
	const FRoomVariantDef& Variant = FRoomVariantSystem::GetDef(VariantId);

	// Резервируем дверные коридоры ВСЕХ клеток комнаты: пропс из комнаты не должен
	// перекрыть проход соседней клетки той же комнаты (раньше — только своей).
	OccupiedBoxes.Reset();
	for (int32 LY = Room.MinCell.Y; LY <= Room.MaxCell.Y; ++LY)
	{
		for (int32 LX = Room.MinCell.X; LX <= Room.MaxCell.X; ++LX)
		{
			ReserveDoorCorridor(GenData->WorldX(LX), GenData->WorldY(LY));
		}
	}

	// Количество пропсов растёт с площадью комнаты с насыщением (sqrt + кап):
	// огромные hall'ы не тонут в инстансах, крошечные чуланы не забиваются.
	const int32 RoomCellsCap = FMath::Clamp(Room.CellCount, 1, 96);
	const float RoomScale = FMath::Sqrt((float)RoomCellsCap);
	const int32 PropCount = FMath::RoundToInt(
		Random.FRandRange(0.0f, MaxPropsPerRoom * 0.6f * Variant.PropDensity * RoomScale));

	// Кандидатов с загруженными мешами собираем ОДИН раз на комнату (без фриза).
	struct FCandidate { const FLevelPropEntry* Entry; UStaticMesh* Mesh; };
	TArray<FCandidate> Candidates;
	float TotalWeight = 0.0f;
	for (const FLevelPropEntry& Entry : Profile->Props)
	{
		// Городские здания Buildings/* — только на платформе, не в Бэкрумсе.
		const FString Pkg = Entry.Mesh.GetLongPackageName();
		if (Pkg.Contains(TEXT("/Buildings/")))
		{
			continue;
		}
		UStaticMesh* Mesh = CachedLoadMesh(Entry.Mesh);
		if (Mesh)
		{
			Candidates.Add({ &Entry, Mesh });
			TotalWeight += Entry.Weight;
		}
	}

	for (int32 i = 0; i < PropCount; ++i)
	{
		if (Candidates.Num() == 0)
		{
			break;
		}

		float Roll = Random.FRandRange(0.0f, TotalWeight);
		const FCandidate* Chosen = &Candidates.Last();
		for (const FCandidate& Cand : Candidates)
		{
			if (Roll <= Cand.Entry->Weight)
			{
				Chosen = &Cand;
				break;
			}
			Roll -= Cand.Entry->Weight;
		}

		UStaticMesh* Mesh = Chosen->Mesh;
		if (!Mesh)
		{
			break;
		}

		const float Scale = Random.FRandRange(Chosen->Entry->MinScale, Chosen->Entry->MaxScale);
		const FBoxSphereBounds Bounds = Mesh->GetBounds();

		FVector Pos;
		FRotator Rot;
		const float Inset = 90.0f;

		switch (Chosen->Entry->Placement)
		{
		case ELevelPropPlacement::FloorCenter:
			Pos = FVector(Cell.GetCenter().X, Cell.GetCenter().Y, 0.0f);
			Rot = FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
			break;
		case ELevelPropPlacement::FloorRandom:
			{
				// Стиль вида комнаты переопределяет чистый «где-нибудь на полу».
				const float CX = Cell.GetCenter().X;
				const float CY = Cell.GetCenter().Y;
				switch (Variant.Scatter)
				{
				case EPropScatterStyle::AlongWall:
				{
					const int32 Side = Random.RandRange(0, 3);
					switch (Side)
					{
					case 0:  Pos = FVector(Cell.Min.X + Inset, Random.FRandRange(Cell.Min.Y + Inset, Cell.Max.Y - Inset), 0.0f); break;
					case 1:  Pos = FVector(Cell.Max.X - Inset, Random.FRandRange(Cell.Min.Y + Inset, Cell.Max.Y - Inset), 0.0f); break;
					case 2:  Pos = FVector(Random.FRandRange(Cell.Min.X + Inset, Cell.Max.X - Inset), Cell.Min.Y + Inset, 0.0f); break;
					default: Pos = FVector(Random.FRandRange(Cell.Min.X + Inset, Cell.Max.X - Inset), Cell.Max.Y - Inset, 0.0f); break;
					}
					break;
				}
				case EPropScatterStyle::Corners:
				{
					const int32 Corner = Random.RandRange(0, 3);
					const float X = (Corner == 0 || Corner == 2) ? Cell.Min.X + Inset : Cell.Max.X - Inset;
					const float Y = (Corner < 2) ? Cell.Min.Y + Inset : Cell.Max.Y - Inset;
					Pos = FVector(X + Random.FRandRange(-30.0f, 30.0f), Y + Random.FRandRange(-30.0f, 30.0f), 0.0f);
					break;
				}
				case EPropScatterStyle::Clustered:
				{
					// Одна куча у центра: сужаем радиус по мере отдаления от ядра.
					const float Radius = (Cell.Max.X - Cell.Min.X) * 0.28f;
					Pos = FVector(CX + Random.FRandRange(-Radius, Radius), CY + Random.FRandRange(-Radius, Radius), 0.0f);
					break;
				}
				case EPropScatterStyle::Centerpiece:
					Pos = FVector(CX, CY, 0.0f);
					break;
				case EPropScatterStyle::Scattered:
				default:
					Pos = FVector(
						Random.FRandRange(Cell.Min.X + Inset, Cell.Max.X - Inset),
						Random.FRandRange(Cell.Min.Y + Inset, Cell.Max.Y - Inset),
						0.0f);
					break;
				}
				Rot = FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
			}
			break;
		case ELevelPropPlacement::AlongWall:
			{
				const int32 Side = Random.RandRange(0, 3);
				switch (Side)
				{
				case 0:
					Pos = FVector(Cell.Min.X + Inset, Random.FRandRange(Cell.Min.Y + Inset, Cell.Max.Y - Inset), 0.0f);
					Rot = FRotator(0.0f, 90.0f, 0.0f);
					break;
				case 1:
					Pos = FVector(Cell.Max.X - Inset, Random.FRandRange(Cell.Min.Y + Inset, Cell.Max.Y - Inset), 0.0f);
					Rot = FRotator(0.0f, -90.0f, 0.0f);
					break;
				case 2:
					Pos = FVector(Random.FRandRange(Cell.Min.X + Inset, Cell.Max.X - Inset), Cell.Min.Y + Inset, 0.0f);
					Rot = FRotator(0.0f, 0.0f, 0.0f);
					break;
				default:
					Pos = FVector(Random.FRandRange(Cell.Min.X + Inset, Cell.Max.X - Inset), Cell.Max.Y - Inset, 0.0f);
					Rot = FRotator(0.0f, 180.0f, 0.0f);
					break;
				}
			}
			break;
		case ELevelPropPlacement::WallHang:
			{
				const int32 Side = Random.RandRange(0, 3);
				switch (Side)
				{
				case 0:
					Pos = FVector(Cell.Min.X, Random.FRandRange(Cell.Min.Y + 50.0f, Cell.Max.Y - 50.0f), WallHeight * 0.6f);
					Rot = FRotator(0.0f, -90.0f, 0.0f);
					break;
				case 1:
					Pos = FVector(Cell.Max.X, Random.FRandRange(Cell.Min.Y + 50.0f, Cell.Max.Y - 50.0f), WallHeight * 0.6f);
					Rot = FRotator(0.0f, 90.0f, 0.0f);
					break;
				case 2:
					Pos = FVector(Random.FRandRange(Cell.Min.X + 50.0f, Cell.Max.X - 50.0f), Cell.Min.Y, WallHeight * 0.6f);
					Rot = FRotator(0.0f, 0.0f, 0.0f);
					break;
				default:
					Pos = FVector(Random.FRandRange(Cell.Min.X + 50.0f, Cell.Max.X - 50.0f), Cell.Max.Y, WallHeight * 0.6f);
					Rot = FRotator(0.0f, 180.0f, 0.0f);
					break;
				}
			}
			break;
		case ELevelPropPlacement::CeilingHang:
			Pos = FVector(Cell.GetCenter().X, Cell.GetCenter().Y, WallHeight - 30.0f);
			Rot = FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
			break;
		default:
			continue;
		}

const float FootX = Bounds.BoxExtent.X * Scale;
			const float FootY = Bounds.BoxExtent.Y * Scale;
			const float YawRad = FMath::DegreesToRadians(Rot.Yaw);
			const float CosYaw = FMath::Cos(YawRad);
			const float SinYaw = FMath::Sin(YawRad);
			const float RotFootX = FMath::Abs(FootX * CosYaw) + FMath::Abs(FootY * SinYaw);
			const float RotFootY = FMath::Abs(FootX * SinYaw) + FMath::Abs(FootY * CosYaw);
			FBox2D Foot(FVector2D(Pos.X - RotFootX, Pos.Y - RotFootY), FVector2D(Pos.X + RotFootX, Pos.Y + RotFootY));

			{
				FCollisionQueryParams PropQP(SCENE_QUERY_STAT(PropOverlap), false, nullptr);
				const float PropHalfZ = Bounds.BoxExtent.Z * Scale + 10.0f;
				if (GetWorld()->OverlapBlockingTestByChannel(
					FVector(Pos.X, Pos.Y, Pos.Z + PropHalfZ),
					FQuat::Identity,
					ECC_WorldStatic,
					FCollisionShape::MakeBox(FVector(RotFootX + 5.0f, RotFootY + 5.0f, PropHalfZ)),
					PropQP))
				{
					continue;
				}
			}

			if (OverlapsOccupied(Foot))
			{
				continue;
			}

			OccupiedBoxes.Add(Foot);

			// Нижняя грань пропса на полу (а не половина высоты).
			const float GroundZ = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale;
			const FVector FinalPos(Pos.X, Pos.Y, -GroundZ);
			UInstancedStaticMeshComponent* Inst = GetOrCreateInstancer(Mesh);
			if (Inst)
			{
				Inst->AddInstance(FTransform(Rot, FinalPos, FVector(Scale)));
			}
		}
}

void ABackroomsChunkActor::SpawnStoryProps(int32 CellX, int32 CellY, FRandomStream& Random)
{
	if (!Profile || Profile->StoryScenes.Num() == 0 || !StoryPropClass)
	{
		return;
	}

	// Сцены ставим только в «жилых» клетках с низкой плотностью (пол), как и
	// обычные пропсы: в стенах и на переходах им не место.
	if (GenData && !GenData->CellIsScatter(CellX, CellY))
	{
		return;
	}

	if (Random.FRand() > StorySceneChance)
	{
		return;
	}

	const FLevelStoryScene& Scene = Profile->StoryScenes[Random.RandRange(0, Profile->StoryScenes.Num() - 1)];
	if (Scene.Props.Num() == 0)
	{
		return;
	}

	// Детерминированный идентификатор комнаты по мировой клетке: все пропсы
	// сцены получают один RoomID, чтобы чанк мог собрать сцену при подъёме.
	const int32 RoomID = CellX * 100003 + CellY;
	const int32 ScenarioID = static_cast<int32>(Scene.Type);
	const FBox2D Cell = CellBounds(CellX, CellY);
	const FVector2D Center = Cell.GetCenter();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (const FStoryPropEntry& Entry : Scene.Props)
	{
		UStaticMesh* Mesh = CachedLoadMesh(Entry.Mesh);
		if (!Mesh)
		{
			continue;
		}

		const float Scale = Entry.Scale > 0.0f ? Entry.Scale : 1.0f;
		const FBoxSphereBounds Bounds = Mesh->GetBounds();
		const FRotator Rot(0.0f, Entry.Yaw, 0.0f);

		const FVector Pos(Center.X + Entry.Offset.X, Center.Y + Entry.Offset.Y, 0.0f);

		// Не ставим физический предмет вплотную к другому: считаем след по XY.
		const float FootX = Bounds.BoxExtent.X * Scale;
		const float FootY = Bounds.BoxExtent.Y * Scale;
		const float YawRad = FMath::DegreesToRadians(Rot.Yaw);
		const float CosYaw = FMath::Cos(YawRad);
		const float SinYaw = FMath::Sin(YawRad);
		const float RotFootX = FMath::Abs(FootX * CosYaw) + FMath::Abs(FootY * SinYaw);
		const float RotFootY = FMath::Abs(FootX * SinYaw) + FMath::Abs(FootY * CosYaw);
		const FBox2D Foot(FVector2D(Pos.X - RotFootX, Pos.Y - RotFootY),
			FVector2D(Pos.X + RotFootX, Pos.Y + RotFootY));
		if (OverlapsOccupied(Foot))
		{
			continue;
		}
		OccupiedBoxes.Add(Foot);

		// Нижняя грань — на полу (пол Бэкрумса: верх плиты на Z=0).
		const float GroundZ = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale;
		const FVector FinalPos(Pos.X, Pos.Y, -GroundZ);

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ABackroomsPhysProp* Prop = World->SpawnActor<ABackroomsPhysProp>(
			StoryPropClass, FinalPos, Rot, Params);
		if (!Prop)
		{
			continue;
		}
		Prop->SetOwnerChunk(this);
		Prop->SetStoryTag(RoomID, ScenarioID);
		Prop->Initialize(Mesh, Scale, Entry.WeightKg, FinalPos, Rot);
	}
}

void ABackroomsChunkActor::SpawnItemPickup(int32 CellX, int32 CellY, FRandomStream& Random)
{
	if (!ItemPickupClass)
	{
		return;
	}
	// Как и обычные пропсы: только «жилые» клетки с полом.
	if (GenData && !GenData->CellIsScatter(CellX, CellY))
	{
		return;
	}
	if (Random.FRand() > ItemPickupChance)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Подбор набора зависит от уровня: опасные уровни чаще выдают аптечки,
	// «мокрые» — воду. Seed детерминирован (Random уже от Seed чанка).
	static const FName Common[] = { TEXT("AlmondWater"), TEXT("CanFood"), TEXT("Pill"), TEXT("Energy"), TEXT("Battery") };
	const FName Id = Common[Random.RandRange(0, UE_ARRAY_COUNT(Common) - 1)];

	UStaticMesh* Mesh = UBackroomsItemSystem::GetWorldMeshForItem(Id);
	const FBox2D Cell = CellBounds(CellX, CellY);
	const FVector2D Center = Cell.GetCenter();
	const FVector Pos(Center.X, Center.Y, 0.0f);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ABackroomsItemPickup* Pickup = World->SpawnActor<ABackroomsItemPickup>(ItemPickupClass, Pos, FRotator::ZeroRotator, Params);
	if (Pickup)
	{
		Pickup->Initialize(Id, 1, Mesh, 1.0f);
		++UtilitySpawnedThisChunk;
	}
}

void ABackroomsChunkActor::SpawnDecorProp(int32 CellX, int32 CellY, FRandomStream& Random, const FVector* OffsetXY)
{
	if (!StoryPropClass)
	{
		return;
	}
	if (GenData && !GenData->CellIsScatter(CellX, CellY))
	{
		return;
	}
	if (Random.FRand() > DecorPropChance)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Декоративные физические объекты из Fab: мелочь для рук и броска.
	struct FDecorDef { const TCHAR* Path; float Scale; float Weight; };
	static const FDecorDef Decor[] =
	{
		{ TEXT("/Game/Fab/Rubik_s_Cube_LowPoly/rubiks_cube/StaticMeshes/rubiks_cube.rubiks_cube"), 8.0f, 0.2f },
		{ TEXT("/Game/Fab/Beach_Ball_Low-poly_PBR/beach_ball_fbx/StaticMeshes/beach_ball_fbx.beach_ball_fbx"), 18.0f, 0.4f },
		{ TEXT("/Game/Fab/_coffee_/coffee/StaticMeshes/coffee.coffee"), 1.0f, 0.4f },
		{ TEXT("/Game/Fab/Prescription_Pill_Bottle/PillBottle_fbx.PillBottle_fbx"), 1.0f, 0.1f },
		{ TEXT("/Game/Fab/Batteries/batteries/StaticMeshes/batteries.batteries"), 1.5f, 0.3f },
		{ TEXT("/Game/Fab/Tin_cans/tincanssketchfab/StaticMeshes/tincanssketchfab.tincanssketchfab"), 1.0f, 0.4f },
		{ TEXT("/Game/Fab/Paper_Bag_Scan_MEDPOLY/paper_bag_scan_medpoly/StaticMeshes/paper_bag_scan_medpoly.paper_bag_scan_medpoly"), 1.0f, 0.2f },
		{ TEXT("/Game/Fab/Rubber_Ducky_-_Stylized_3D_Model/ducky/StaticMeshes/ducky.ducky"), 2.0f, 0.2f },
		{ TEXT("/Game/Fab/Lowpoly_Watermelon/lowpoly_watermelon/StaticMeshes/lowpoly_watermelon.lowpoly_watermelon"), 2.0f, 1.5f },
		{ TEXT("/Game/Props/ToyCar.ToyCar"), 1.0f, 0.3f },
		{ TEXT("/Game/Props/ToyDuck.ToyDuck"), 1.0f, 0.2f },
		{ TEXT("/Game/Props/ToyBlock.ToyBlock"), 1.0f, 0.3f },
		{ TEXT("/Game/Props/Can_Rusty.Can_Rusty"), 1.0f, 0.4f },
		{ TEXT("/Game/Props/Bottle_Glass.Bottle_Glass"), 1.0f, 0.5f },
		{ TEXT("/Game/Props/Books_Stack.Books_Stack"), 1.0f, 2.0f },
		{ TEXT("/Game/Props/Bucket.Bucket"), 1.0f, 1.2f },
		{ TEXT("/Game/Props/Stool.Stool"), 1.0f, 3.0f },
		{ TEXT("/Game/Props/Box_Cardboard.Box_Cardboard"), 1.0f, 2.5f },
		{ TEXT("/Game/Props/Crystal_Quartz.Crystal_Quartz"), 1.0f, 1.0f },
		{ TEXT("/Game/Props/Pickaxe.Pickaxe"), 1.0f, 2.5f },
		{ TEXT("/Game/Fab/Megascans/3D/Toy_Giraffe_ujpheaova/High/ujpheaova_tier_1/StaticMeshes/ujpheaova_tier_1.ujpheaova_tier_1"), 30.0f, 0.5f },
		{ TEXT("/Game/Fab/Megascans/3D/Wooden_Toy_Horse_uhcjdhpva/High/uhcjdhpva_tier_1/StaticMeshes/uhcjdhpva_tier_1.uhcjdhpva_tier_1"), 30.0f, 1.0f }
	};

	const FDecorDef& Def = Decor[Random.RandRange(0, UE_ARRAY_COUNT(Decor) - 1)];
	UStaticMesh* Mesh = CachedLoadMesh(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(Def.Path)));
	if (!Mesh)
	{
		return;
	}

	const FBox2D Cell = CellBounds(CellX, CellY);
	FVector2D Center = Cell.GetCenter();
	if (OffsetXY)
	{
		Center += FVector2D(OffsetXY->X, OffsetXY->Y);
	}
	const float Yaw = Random.FRandRange(0.0f, 360.0f);

	const FBoxSphereBounds Bounds = Mesh->GetBounds();
	const float GroundZ = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Def.Scale;
	const FVector FinalPos(Center.X, Center.Y, -GroundZ);
	const FRotator Rot(0.0f, Yaw, 0.0f);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ABackroomsPhysProp* Prop = World->SpawnActor<ABackroomsPhysProp>(StoryPropClass, FinalPos, Rot, Params);
	if (Prop)
	{
		Prop->SetOwnerChunk(this);
		Prop->Initialize(Mesh, Def.Scale, Def.Weight, FinalPos, Rot);
	}
}

void ABackroomsChunkActor::SetupFromProfile(const ULevelGeneratorProfile* InProfile)
{
	if (!InProfile)
	{
		return;
	}
	Profile = const_cast<ULevelGeneratorProfile*>(InProfile);
	CellSize = Profile->CellSize;
	WallHeight = Profile->WallHeight;
	WallThickness = Profile->WallThickness;
	DoorWidth = Profile->DoorWidth;
	DoorHeight = Profile->DoorHeight;
	MaxPropsPerRoom = Profile->MaxPropsPerRoom;
	ChunkSizeCells = Profile->ChunkSizeCells;
	bSpawnPlatform = Profile->bSpawnPlatform;
	WallThreshold = Profile->WallThreshold;
	DoorThreshold = Profile->DoorThreshold;
	ScatterThreshold = Profile->ScatterThreshold;
	LayoutPattern = Profile->LayoutPattern;
	PaletteFloor = Profile->FloorColor;
	PaletteCeiling = Profile->CeilingColor;
	PaletteWall = Profile->WallColor;

	// Правила размера комнат и пороги — теперь реально используются генератором.
	MinRoomCells = Profile->MinRoomCells;
	MaxRoomCells = Profile->MaxRoomCells;
	LivingRoomThreshold = Profile->LivingRoomThreshold;

	// Частоты шума — выводятся из размера комнат (см. InitGenData).
	// Здесь храним базовые значения из профиля для фолбэка.
	BaseNoiseFreq = 0.09f;
	MacroNoiseFreq = 0.012f;

	// Дроп, уже помноженный сложностью: полезного расходника и декоративного
	// физического объекта. Так выбор сложности влияет и на состав лута.
	ItemPickupChance = Profile->ItemPickupChance;
	DecorPropChance = Profile->DecorPropChance;
	MinUtilityPerChunk = Profile->MinUtilityPerChunk;

	// Опциональные текстурированные материалы уровня: если заданы в профиле,
	// переопределяют shared-материалы (FloorMaterial/WallMaterial/CeilingMaterial)
	// для CommitMesh, но не трогают city-материалы города-лобби.
	// ВАЖНО: проверяем именно результат загрузки, а не IsNull(). Профиль может
	// ссылаться на /Game/Style/LN/Materials/*, которых нет на диске — тогда
	// LoadSynchronous() вернёт nullptr. Раньше этот nullptr затирал рабочие
	// текстурированные материалы из конструктора (M_Floor_Tex/M_Wall_Tex/
	// M_Ceiling_Tex), и чанк рисовался без текстуры (потолок «кирпичной»).
	if (UMaterialInterface* M = Profile->FloorMaterial.LoadSynchronous())
	{
		FloorMaterial = M;
	}
	if (UMaterialInterface* M = Profile->WallMaterial.LoadSynchronous())
	{
		WallMaterial = M;
	}
	if (UMaterialInterface* M = Profile->CeilingMaterial.LoadSynchronous())
	{
		CeilingMaterial = M;
	}
}

void ABackroomsChunkActor::CommitMesh()
{
	UMaterialInterface* Mats[(int32)EMaterialSlot::Marking + 1] = {
		FloorMaterial, CeilingMaterial, WallMaterial,
		CityRoadMaterial ? CityRoadMaterial : FloorMaterial,
		CitySidewalkMaterial ? CitySidewalkMaterial : FloorMaterial,
		CityMarkingMaterial ? CityMarkingMaterial : FloorMaterial
	};

	for (uint8 Slot = 0; Slot <= (uint8)EMaterialSlot::Marking; ++Slot)
	{
		TArray<FVector> SecVerts;
		TArray<int32> SecTris;
		TArray<FVector> SecNormals;
		TArray<FVector2D> SecUV0;
		TArray<FLinearColor> SecColors;
		TArray<FProcMeshTangent> SecTangents;
		TArray<int32> Remap;
		Remap.SetNumZeroed(Vertices.Num());

		int32 OutBase = 0;
		for (int32 Quad = 0; Quad < QuadSlots.Num(); ++Quad)
		{
			if (QuadSlots[Quad] != Slot)
			{
				continue;
			}

			const int32 V0 = Quad * 4;
			for (int32 V = 0; V < 4; ++V)
			{
				Remap[V0 + V] = OutBase++;
				SecVerts.Add(Vertices[V0 + V]);
				SecNormals.Add(Normals[V0 + V]);
				SecUV0.Add(UV0[V0 + V]);
				SecColors.Add(VertexColors[V0 + V]);
				SecTangents.Add(Tangents[V0 + V]);
			}

			const int32 T0 = Quad * 6;
			for (int32 T = 0; T < 6; ++T)
			{
				SecTris.Add(Remap[Triangles[T0 + T]]);
			}
		}

		if (SecVerts.Num() > 0)
		{
			// Only solid geometry (floor/ceiling/wall) collides. Road, sidewalk
			// and marking are cosmetic overlays: without this the player trips
			// on the curb edges of the city ground.
			const bool bCreateCollision =
				Slot == (uint8)EMaterialSlot::Floor ||
				Slot == (uint8)EMaterialSlot::Ceiling ||
				Slot == (uint8)EMaterialSlot::Wall;
			GeometryMesh->CreateMeshSection_LinearColor(Slot, SecVerts, SecTris, SecNormals, SecUV0, SecColors, SecTangents, bCreateCollision);
			if (Mats[Slot])
			{
				GeometryMesh->SetMaterial(Slot, Mats[Slot]);
			}
		}
	}

	ClearMeshData();
}

void ABackroomsChunkActor::AssignFloorPlan(UBackroomsFloorPlan* InPlan)
{
	FloorPlan = InPlan;
}

void ABackroomsChunkActor::PlacePropsAtPosition(const FVector& Position)
{
	// ФИКС: раньше это был stub (только лог). FloorPlan::ExtractChunk уже
	// честно считает позиции пропсов (PropPositions/PropTypes), но они
	// нигде не материализовывались — комнаты из FloorPlan-конвейера были
	// буквально пустыми (отсюда и нечем было тестировать осмотр/кидание).
	// Переводим мировую позицию в клетку чанка и переиспользуем уже рабочий
	// декор-спавнер легаси-пути.
	const int32 CellX = FMath::FloorToInt(Position.X / CellSize);
	const int32 CellY = FMath::FloorToInt(Position.Y / CellSize);
	FRandomStream Random(Seed ^ (CellX * 73856093) ^ (CellY * 19349663) ^ 0x50524F50);
	SpawnDecorProp(CellX, CellY, Random, nullptr);
}

void ABackroomsChunkActor::AddColumnAtPosition(const FVector& Position)
{
	// ФИКС: раньше stub. Колонна — вертикальный бокс, той же геометрией, что
	// и стены легаси-конвейера (переиспользуем AddBox, вызывается до
	// CommitMesh() в BuildFromFloorPlan(), так что попадает в тот же меш).
	const float ColHalfXY = FMath::Max(WallThickness * 0.75f, 25.0f);
	const float ColHeight = WallHeight * 0.8f;
	AddBox(FVector(Position.X, Position.Y, ColHeight * 0.5f),
		FVector(ColHalfXY, ColHalfXY, ColHeight * 0.5f),
		FLinearColor(0.85f, 0.85f, 0.85f, 1.0f), 2.0f, EMaterialSlot::Wall);
}

void ABackroomsChunkActor::AddLightAtPosition(const FVector& Position, const FLinearColor& Color)
{
	// ФИКС: раньше stub — комнаты из FloorPlan были полностью тёмными (только
	// небо/амбиент), даже там, где план явно решил "здесь должен быть свет".
	// Тип и параметры компонента — те же, что уже проверены в SpawnRoomLights
	// (легаси-путь): RoomLights — это TArray<URectLightComponent*>, не точечный.
	URectLightComponent* Light = NewObject<URectLightComponent>(this);
	if (!Light)
	{
		return;
	}
	Light->SetWorldLocation(Position);
	Light->SetIntensity(1400.0f);
	Light->SetAttenuationRadius(CellSize * 1.5f);
	Light->SetLightColor(Color);
	Light->SetCastShadows(false);
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetSourceWidth(220.0f);
	Light->SetSourceHeight(60.0f);
	Light->RegisterComponent();
	Light->MarkRenderStateDirty();
	RoomLights.Add(Light);
}

void ABackroomsChunkActor::BuildFromFloorPlan()
{
	// Диспетчер (BuildChunk/BuildChunkFromProfile) уже гарантировал: bUseFloorPlan
	// && FloorPlan != null. Здесь — только материализация плана региона.
	FChunkGeometryData ChunkGeo;
	FloorPlan->ExtractChunk(FChunkCoord(ChunkX, ChunkY), ChunkGeo);

	// Геометрия из плана (стены/полы/потолки из Edges; колонны — из Structure).
	for (const auto& Vert : ChunkGeo.Vertices)     { Vertices.Add(Vert); }
	for (const auto& Tri : ChunkGeo.Triangles)     { Triangles.Add(Tri); }
	for (const auto& Norm : ChunkGeo.Normals)      { Normals.Add(Norm); }
	for (const auto& UV : ChunkGeo.UVs)            { UV0.Add(UV); }
	for (const auto& Color : ChunkGeo.VertexColors){ VertexColors.Add(Color); }

	// КРИТИЧНО: копируем Tangents и QuadSlots, иначе CommitMesh создаёт 0 quads
	Tangents.Append(ChunkGeo.Tangents);
	QuadSlots.Append(ChunkGeo.QuadSlots);

	// Пропсы/колонны/свет — только по решениям плана (никаких random-разбросов).
	for (const auto& PropPos : ChunkGeo.PropPositions)      { PlacePropsAtPosition(PropPos); }
	for (const auto& ColPos : ChunkGeo.ColumnPositions)     { AddColumnAtPosition(ColPos); }
	for (int32 i = 0; i < ChunkGeo.LightPositions.Num(); ++i)
	{
		AddLightAtPosition(ChunkGeo.LightPositions[i], ChunkGeo.LightColors[i]);
	}

	CommitMesh();

	if (ChunkX == 0 && ChunkY == 0)
	{
		return;
	}

	// ФИКС: лут был отключён (if (false)) ради отладочного скриншота "чистой"
	// геометрии плана и так и не включён обратно — из-за этого в FloorPlan-
	// уровнях не было предметов вообще (нечего было подобрать/осмотреть/
	// кинуть). Возвращаем обратно.
	{
		FRandomStream LootRng(Seed);
		TopUpUtilityLoot(LootRng);
	}
}

void ABackroomsChunkActor::BuildChunk(int32 InChunkX, int32 InChunkY, int32 InSeed, const TObjectPtr<UPropDatabase>& InDatabase)
{
	ChunkX = InChunkX;
	ChunkY = InChunkY;
	Seed = InSeed;
	Database = InDatabase;
	Instancers.Empty();
	ClearMeshData();

	if (bSpawnPlatform)
	{
		BuildSpawnPlatform();
		return;
	}

	const bool bClearRoom = InChunkX == 0 && InChunkY == 0;

	// --- ЯВНЫЙ РЕЖИМ ГЕНЕРАЦИИ ---
	// bUseFloorPlan задаёт генератор из конфига уровня. В режиме FloorPlan чанк
	// обязан строить архитектуру ТОЛЬКО из плана: если план не назначен — это
	// ошибка уровня, а не повод молча падать в legacy-генерацию.
	if (bUseFloorPlan)
	{
		if (!FloorPlan)
		{
			UE_LOG(LogTemp, Error,
				TEXT("BR: chunk (%d,%d) в режиме FloorPlan без плана региона — генерация ПРЕРВАНА (легаси-фолбэк запрещён)."),
				InChunkX, InChunkY);
			return;
		}
		BuildFromFloorPlan();
		return;
	}

	// --- ЛЕГАСИЙНЫЙ КОНВЕЙЕР (старый код) ---
	// Используется для города и старых уровней; НЕ является фолбэком для FloorPlan.
	InitGenData();
	TArray<uint8> WallMask;
	TArray<uint8> DoorMask;
	GenerateMasksFromGenData(WallMask, DoorMask);
	BuildGeometryFromMasks(WallMask, DoorMask);
	BuildNaniteWallSkins();
	SpawnGraffitiDecals();
	CommitMesh();

	SpawnRoomLights(WallMask);

	if (bClearRoom)
	{
		return;
	}

	FRandomStream Random(Seed ^ (ChunkX * 73856093) ^ (ChunkY * 19349663));
	UtilitySpawnedThisChunk = 0;

	for (int32 CY = 0; CY < ChunkSizeCells; ++CY)
	{
		for (int32 CX = 0; CX < ChunkSizeCells; ++CX)
		{
			const int32 WorldCellX = ChunkX * ChunkSizeCells + CX;
			const int32 WorldCellY = ChunkY * ChunkSizeCells + CY;
			OccupiedBoxes.Reset();
			ReserveDoorCorridor(WorldCellX, WorldCellY);
			PlaceProps(WorldCellX, WorldCellY, Random);
			SpawnStoryProps(WorldCellX, WorldCellY, Random);
			SpawnItemPickup(WorldCellX, WorldCellY, Random);
			SpawnDecorProp(WorldCellX, WorldCellY, Random);
		}
	}

	TopUpUtilityLoot(Random);
}

void ABackroomsChunkActor::BuildChunkFromProfile(int32 InChunkX, int32 InChunkY, int32 InSeed, const ULevelGeneratorProfile* InProfile)
{
	// bSpawnPlatform выбирает вызывающий (SpawnStartPlatform — true, SpawnPending —
	// false). SetupFromProfile() НЕ должен его затирать значением профиля
	// (по умолчанию true), иначе КАЖДЫЙ чанк Бэкрумса строит целый город-платформу
	// с синхронной загрузкой зданий — мир превращается в «пол города» и зависает.
	const bool bBuildPlatform = bSpawnPlatform;
	SetupFromProfile(InProfile);
	bSpawnPlatform = bBuildPlatform;

	ChunkX = InChunkX;
	ChunkY = InChunkY;
	Seed = InSeed;
	Instancers.Empty();
	ClearMeshData();

	if (bSpawnPlatform)
	{
		BuildSpawnPlatform();
		return;
	}

	const bool bClearRoom = InChunkX == 0 && InChunkY == 0;

	// --- ЯВНЫЙ РЕЖИМ: FloorPlan (копия диспетчера из BuildChunk) ---
	if (bUseFloorPlan)
	{
		if (!FloorPlan)
		{
			UE_LOG(LogTemp, Error,
				TEXT("BR [profile]: chunk (%d,%d) в режиме FloorPlan без плана — генерация ПРЕРВАНА."),
				InChunkX, InChunkY);
			return;
		}
		BuildFromFloorPlan();
		return;
	}

	// --- LEGACY: плотностная генерация (как в BuildChunk, но с профилем уровня) ---
	InitGenData();
	TArray<uint8> WallMask;
	TArray<uint8> DoorMask;
	GenerateMasksFromGenData(WallMask, DoorMask);
	BuildGeometryFromMasks(WallMask, DoorMask);
	BuildNaniteWallSkins();
	SpawnGraffitiDecals();
	CommitMesh();

	SpawnRoomLights(WallMask);

	if (bClearRoom)
	{
		return;
	}

	FRandomStream Random(Seed ^ (ChunkX * 73856093) ^ (ChunkY * 19349663));
	UtilitySpawnedThisChunk = 0;
	PropRoomsPlaced.Reset();

	for (int32 CY = 0; CY < ChunkSizeCells; ++CY)
	{
		for (int32 CX = 0; CX < ChunkSizeCells; ++CX)
		{
			const int32 WorldCellX = ChunkX * ChunkSizeCells + CX;
			const int32 WorldCellY = ChunkY * ChunkSizeCells + CY;
			OccupiedBoxes.Reset();
			ReserveDoorCorridor(WorldCellX, WorldCellY);
			PlacePropsFromProfile(WorldCellX, WorldCellY, Random);
			SpawnStoryProps(WorldCellX, WorldCellY, Random);
			SpawnItemPickup(WorldCellX, WorldCellY, Random);
			SpawnDecorProp(WorldCellX, WorldCellY, Random);
		}
	}

	TopUpUtilityLoot(Random);
}

void ABackroomsChunkActor::TopUpUtilityLoot(FRandomStream& Random)
{
	// Анти-софтлок: если за проход выпало меньше гарантированного минимума,
	// докладываем полезные расходники в случайные открытые клетки. Это мягко:
	// на «Кошмаре» минимум = 1 предмет на чанк — игрок не остаётся без снабжения.
	if (UtilitySpawnedThisChunk >= MinUtilityPerChunk || !ItemPickupClass)
	{
		return;
	}

	// Кандидатов собираем детерминированно из уже готовых данных генерации.
	TArray<int32> OpenCells; // пара (LX,LY), упакованная в int
	const int32 Count = ChunkSizeCells;
	for (int32 LY = 0; LY < Count; ++LY)
	{
		for (int32 LX = 0; LX < Count; ++LX)
		{
			if (GenData && GenData->CellIsScatter(ChunkX * Count + LX, ChunkY * Count + LY))
			{
				OpenCells.Add(LX * 1000 + LY);
			}
		}
	}
	if (OpenCells.Num() == 0)
	{
		return;
	}

	while (UtilitySpawnedThisChunk < MinUtilityPerChunk)
	{
		const int32 Packed = OpenCells[Random.RandRange(0, OpenCells.Num() - 1)];
		SpawnOneUtility(ChunkX * Count + Packed / 1000, ChunkY * Count + Packed % 1000, Random);
	}
}

void ABackroomsChunkActor::SpawnOneUtility(int32 CellX, int32 CellY, FRandomStream& Random)
{
	// Прямой гарантированный спавн одного полезного предмета (без броска шанса).
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	static const FName Common[] = { TEXT("AlmondWater"), TEXT("CanFood"), TEXT("Pill"), TEXT("Energy"), TEXT("Battery") };
	const FName Id = Common[Random.RandRange(0, UE_ARRAY_COUNT(Common) - 1)];
	UStaticMesh* Mesh = UBackroomsItemSystem::GetWorldMeshForItem(Id);
	const FVector2D Center = CellBounds(CellX, CellY).GetCenter();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ABackroomsItemPickup* Pickup = World->SpawnActor<ABackroomsItemPickup>(
		ItemPickupClass, FVector(Center.X, Center.Y, 0.0f), FRotator::ZeroRotator, Params);
	if (Pickup)
	{
		Pickup->Initialize(Id, 1, Mesh, 1.0f);
		++UtilitySpawnedThisChunk;
	}
	else
	{
		// Если заспавнить не удалось — не зацикливаемся.
		++UtilitySpawnedThisChunk;
	}
}

void ABackroomsChunkActor::SpawnRoomLights(const TArray<uint8>& WallMask)
{
	RoomLights.Empty();

	const int32 Count = ChunkSizeCells;
	// Комната прибытия под городом должна быть читаемой с первой секунды.
	// В остальных бесконечных чанках комната без лампы остаётся тёмной:
	// свет идёт именно от светильников под потолком, а не «из воздуха».
	const bool bArrivalChunk = ChunkX == 0 && ChunkY == 0;
	const float LightIntensity = bArrivalChunk ? 3000.0f : 1400.0f;
	const float LampZ = WallHeight - 30.0f; // под потолком в каждой комнате

	// Видимые светильники потолка (то, что реально светит).
	static const TSoftObjectPtr<UStaticMesh> QuadLamp(
		FSoftObjectPath(TEXT("/Game/Props/Light_CeilingQuad.Light_CeilingQuad")));
	static const TSoftObjectPtr<UStaticMesh> TubeLamp(
		FSoftObjectPath(TEXT("/Game/Props/Light_CeilingTube.Light_CeilingTube")));

	// Шаг сетки панелей — узловое значение из конфига уровня (lights.every_m):
	// панель ставится в центре каждого блока GridCells x GridCells клеток
	// комнаты. Минимум 2 клетки, чтобы свет не превращался в «точку в каждой
	// клетке», но оставался плотным (иконичная сетка 2x2).
	const UBackroomsLevelConfig* Cfg = Profile ? Profile->LevelConfig : nullptr;
	const float EveryM = (Cfg && Cfg->Lights.EveryM > 0.0f) ? Cfg->Lights.EveryM : 5.0f;
	const int32 GridCells = FMath::Max(2,
		FMath::RoundToInt(EveryM * 100.0f / FMath::Max(1.0f, CellSize)));

	// Тот же рандом-стрим, что и для пропсов, чтобы результат был детерминирован
	// по Seed чанка.
	FRandomStream Random(Seed ^ (ChunkX * 73856093) ^ (ChunkY * 19349663));

	TArray<uint8> Placed;
	Placed.SetNumZeroed(Count * Count);

	const auto SpawnLightAt = [&](int32 LX, int32 LY, const FRoomVariantDef& Variant)
	{
		if (LX < 0 || LX >= Count || LY < 0 || LY >= Count)
		{
			return;
		}
		const int32 I = MaskIndex(LX, LY, Count);
		// WallMask может быть пустым (режим FloorPlan: SpawnRoomLights вызывается
		// с пустым TArray) — тогда все клетки открыты.
		if (Placed[I] != 0
			|| (WallMask.IsValidIndex(I) && WallMask[I] != 0))
		{
			return;
		}
		Placed[I] = 1;

		const float BX = (float)(ChunkX * Count + LX) * CellSize;
		const float BY = (float)(ChunkY * Count + LY) * CellSize;
		const FVector CellCenter(BX + CellSize * 0.5f, BY + CellSize * 0.5f, 0.0f);

		// 1) Видимый светильник под потолком (детерминированный выбор типа).
		const TSoftObjectPtr<UStaticMesh>& LampSoft = (Random.FRand() < 0.5f) ? QuadLamp : TubeLamp;
		if (UStaticMesh* LampMesh = CachedLoadMesh(LampSoft))
		{
			if (UInstancedStaticMeshComponent* Inst = GetOrCreateInstancer(LampMesh))
			{
				const FRotator LampRot(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
				Inst->AddInstance(FTransform(LampRot, FVector(CellCenter.X, CellCenter.Y, LampZ), FVector(1.0f)));
			}
		}

		// 2) Сам источник света — чуть ниже плафона. Прямоугольная «панель»
		// вместо точки: мягкий рассеянный свет по углам (Lumen) и блик по контуру
		// предметов под лампой.
		URectLightComponent* Light = NewObject<URectLightComponent>(this);
		if (!Light)
		{
			return;
		}
		const FLinearColor CellColor = Cfg ? Cfg->Lights.Color : Variant.LightColor;
		Light->SetWorldLocation(FVector(CellCenter.X, CellCenter.Y, LampZ - 20.0f));
		Light->SetIntensity(bArrivalChunk ? LightIntensity
			: FMath::Max(LightIntensity * 0.25f, Variant.LightIntensity));
		// Радиус света — по реальному шагу сетки панелей (GridCells клеток):
		// ~0.55 шага, чтобы свет перекрывал среднюю точку между панелями и
		// комната не «мерцала» на пути между лампами.
		Light->SetAttenuationRadius(FMath::Max(200.0f, (float)GridCells * CellSize * 0.55f));
		// Тени только в стартовой комнате: десятки теневых источников
		// убивают FPS на слабых GPU (VSM + Lumen).
		Light->SetCastShadows(bArrivalChunk);
		Light->SetMobility(EComponentMobility::Movable);
		// Панель 2.2м x 0.5м: длинный флуоресцентный источник под потолком
		// + мягкая полутень, меньше шума Screen Probe Gather.
		Light->SetSourceWidth(220.0f);
		Light->SetSourceHeight(60.0f);
		// Контактная тень (подошвы на полу): короткий острый стык «предмет/пол»
		// рядом с источником (нужен r.ContactShadows 1, см. ApplyRendererDefaults).
		Light->ContactShadowLength = 2.0f;
		Light->ContactShadowLengthInWS = 1;
		Light->ContactShadowNonCastingIntensity = 1.0f;
		// Объёмный вклад: световые колонны в волюметрик-тумане (God Rays).
		Light->SetVolumetricScatteringIntensity(LampVolumetricIntensity);
		Light->SetCastVolumetricShadow(true);
		Light->RegisterComponent();
		Light->MarkRenderStateDirty();

		RoomLights.Add(Light);
	};

	const auto SpawnVariantLight = [&](int32 LX, int32 LY)
	{
		const FRoomVariantDef& V = GenData
			? GenData->CellVariantDef(LX, LY)
			: FRoomVariantSystem::GetDef(ERoomVariant::Neutral);
		// В стартовом чанке шанс всегда 1.0 (игрок не должен входить в темноту),
		// но цвет/яркость всё равно берём из вида комнаты.
		if (bArrivalChunk || Random.FRand() <= V.LightChance)
		{
			SpawnLightAt(LX, LY, V);
		}
	};

	// Локальные комнаты для раскладки: сетка привязана к бондам (границам)
	// комнаты из плотностного слоя. Без GenData весь чанк считается комнатой.
	TArray<FChunkGenerationData::FRoomInfo> Rooms;
	if (GenData && GenData->LastRooms.Num() > 0)
	{
		Rooms = GenData->LastRooms;
	}
	else
	{
		FChunkGenerationData::FRoomInfo WholeChunk;
		WholeChunk.CellCount = Count * Count;
		WholeChunk.MinCell = FIntPoint::ZeroValue;
		WholeChunk.MaxCell = FIntPoint(Count - 1, Count - 1);
		Rooms.Add(WholeChunk);
	}

	// 1) Регулярная сетка панелей внутри каждой комнаты: ряды с шагом GridCells
	// начинаются от угла комнаты (MinCell) и обрываются на её стенах. В отличие
	// от старых «крестов» на весь чанк это даёт иконичный потолок Бэкрумса —
	// сетка флуоресцентных панелей, выровненная по бондам комнаты.
	for (const FChunkGenerationData::FRoomInfo& Room : Rooms)
	{
		if (Room.CellCount <= 0)
		{
			continue;
		}
		for (int32 LY = Room.MinCell.Y; LY <= Room.MaxCell.Y; LY += GridCells)
		{
			for (int32 LX = Room.MinCell.X; LX <= Room.MaxCell.X; LX += GridCells)
			{
				SpawnVariantLight(LX, LY);
			}
		}
	}

	// 2) Страховка: комната без единой панели (слишком мелкая или все позиции
	// сетки пришлись на стены/двери) получает источник по центру.
	for (const FChunkGenerationData::FRoomInfo& Room : Rooms)
	{
		if (Room.CellCount <= 0)
		{
			continue;
		}
		bool bHasLight = false;
		for (int32 LY = Room.MinCell.Y; LY <= Room.MaxCell.Y && !bHasLight; ++LY)
		{
			for (int32 LX = Room.MinCell.X; LX <= Room.MaxCell.X; ++LX)
			{
				const int32 I = MaskIndex(LX, LY, Count);
				if (Placed[I] != 0)
				{
					bHasLight = true;
					break;
				}
			}
		}
		if (bHasLight)
		{
			continue;
		}
		int32 SLX = -1;
		int32 SLY = -1;
		for (int32 LY = Room.MinCell.Y; LY <= Room.MaxCell.Y && SLX < 0; ++LY)
		{
			for (int32 LX = Room.MinCell.X; LX <= Room.MaxCell.X; ++LX)
			{
				const int32 I = MaskIndex(LX, LY, Count);
				// Пустой WallMask (FloorPlan) — клеток-стен нет, можно ставить свет.
				if (!WallMask.IsValidIndex(I) || WallMask[I] == 0)
				{
					SLX = LX;
					SLY = LY;
					break;
				}
			}
		}
		if (SLX >= 0)
		{
			SpawnVariantLight(SLX, SLY);
		}
	}
}

void ABackroomsChunkActor::SetCityBuildings(const TArray<TSoftObjectPtr<UStaticMesh>>& InCityBuildings)
{
	CityBuildings = InCityBuildings;
}

void ABackroomsChunkActor::FlickerLightsNear(const FVector& WorldLocation, float Radius, int32 Count, float Duration)
{
	// Сбой группы ламп: выбираем ближайшие к точке источники и каждому задаём
	// короткую серию «провалов» яркости. Это похоже на перегорающую ленту:
	// несколько люминесцентных/точечных ламп в одном блоке мигают несинхронно.
	// Не стробоскоп: 2-4 затухания за Duration, с восстановлением к норме.
	if (RoomLights.Num() == 0 || Count <= 0 || Duration <= 0.0f)
	{
		return;
	}

	// Индексы ближайших ламп к событию.
	TArray<TPair<float, int32>> ByDist;
	ByDist.Reserve(RoomLights.Num());
	const float RadiusSq = Radius * Radius;
	for (int32 i = 0; i < RoomLights.Num(); ++i)
	{
		const URectLightComponent* L = RoomLights[i];
		if (!L)
		{
			continue;
		}
		const float D2 = FVector::DistSquared(L->GetComponentLocation(), WorldLocation);
		if (D2 <= RadiusSq)
		{
			ByDist.Add(TPair<float, int32>(D2, i));
		}
	}
	if (ByDist.Num() == 0)
	{
		// Ничего рядом — берём просто несколько случайных, чтобы эффект был.
		for (int32 i = 0; i < RoomLights.Num(); ++i)
		{
			ByDist.Add(TPair<float, int32>(FMath::FRand(), i));
		}
	}
	ByDist.Sort([](const TPair<float, int32>& A, const TPair<float, int32>& B) { return A.Key < B.Key; });

	const int32 Num = FMath::Min(Count, ByDist.Num());
	FRandomStream Rng((int32)(WorldLocation.X * 13.0f) ^ (int32)(WorldLocation.Y * 17.0f) ^ 0x4C414D50);
	for (int32 k = 0; k < Num; ++k)
	{
		URectLightComponent* Light = RoomLights[ByDist[k].Value];
		if (!Light)
		{
			continue;
		}
		// Запоминаем норму яркости (в первый раз; повторные сбои не копят ошибку).
		float Base = Light->Intensity;
		if (Light->ComponentTags.Num() > 0 && Light->ComponentTags[0].ToString().StartsWith(TEXT("BaseI=")))
		{
			Base = FCString::Atof(*Light->ComponentTags[0].ToString().RightChop(6));
		}
		else
		{
			Light->ComponentTags.Add(FName(*FString::Printf(TEXT("BaseI=%.2f"), Base)));
		}

		// Серия провалов: 2-4 вспышки-затухания со сдвигом по фазе на лампу.
		const int32 Blinks = Rng.RandRange(2, 4);
		const float Phase = Rng.FRandRange(0.0f, 0.25f);
		for (int32 b = 0; b < Blinks; ++b)
		{
			const float T = Phase + Duration * (float)b / (float)Blinks;
			const float Dip = Rng.FRandRange(0.05f, 0.35f); // до 95% провала
			const float Rise = Rng.FRandRange(0.02f, 0.06f);
			const float Fall = Rng.FRandRange(0.03f, 0.08f);

			FTimerHandle H1;
			GetWorldTimerManager().SetTimer(H1, [Light, Base, Dip]()
			{
				if (Light) { Light->SetIntensity(Base * Dip); }
			}, T, false);

			FTimerHandle H2;
			GetWorldTimerManager().SetTimer(H2, [Light, Base]()
			{
				if (Light) { Light->SetIntensity(Base); }
			}, T + Rise + Fall, false);
		}
	}
}

bool ABackroomsChunkActor::IsPointLit(const FVector& Point, float Radius) const
{
	const float RadiusSq = Radius * Radius;
	for (const TObjectPtr<URectLightComponent>& Light : RoomLights)
	{
		if (Light && Light->IsVisible() &&
			FVector::DistSquared(Light->GetComponentLocation(), Point) <= RadiusSq)
		{
			return true;
		}
	}
	return false;
}

FString ABackroomsChunkActor::AsciiMapText() const
{
	FString Out;
	const int32 Count = ChunkSizeCells;
	if (LastWallMask.Num() != Count * Count)
	{
		return Out;
	}
	for (int32 LY = Count - 1; LY >= 0; --LY)
	{
		for (int32 LX = 0; LX < Count; ++LX)
		{
			const int32 I = MaskIndex(LX, LY, Count);
			const bool bSpawnMark = (ChunkX == 0 && ChunkY == 0 && LX == Count / 2 && LY == Count / 2);
			if (bSpawnMark)
			{
				Out += (LastWallMask[I] == 0) ? TEXT("S") : TEXT("X");
			}
			else
			{
				Out += (LastWallMask[I] == 0) ? TEXT(".") : TEXT("#");
			}
		}
		Out += TEXT("\n");
	}
	return Out;
}

void ABackroomsChunkActor::SpawnCityBuildings(FRandomStream& Random, float HalfExtent)
{
	// Метрики планировки — доли HalfExtent (см. FBackroomsCityMetrics): при
	// уменьшенном острове сетка слотов и защитные зоны масштабируются вместе
	// с ним (см. справку Buildings-OBJ/00_Справка_расстановка_города.txt).
	const FBackroomsCityMetrics M = FBackroomsCityMetrics::Make(HalfExtent);
	const float Sidewalk = M.Sidewalk;          // безопасная зона у дороги
	const float PlazaR = M.PlazaR;              // площадь спавна
	const float TrapXMax = M.TrapXMax;          // коридор к люку: X:[0..TrapXMax]
	const float TrapHalfY = M.TrapHalfY;        // коридор к люку: Y:[-TrapHalfY..TrapHalfY]
	const float OceMargin = M.OceMargin;        // край острова (у забора)
	const float MaxHeight = M.MaxHeight;        // потолок высоты здания, см
	const float MinScale = 0.15f;

	// Пул мешей: явный список CityBuildings; если пуст — правила Buildings/* из базы.
	TArray<TSoftObjectPtr<UStaticMesh>> Pool = CityBuildings;
	if (Pool.Num() == 0 && Database)
	{
		for (const FPropRule& Rule : Database->Props)
		{
			const FString Pkg = Rule.Mesh.GetLongPackageName();
			if (Pkg.Contains(TEXT("/Buildings/")))
			{
				Pool.Add(Rule.Mesh);
			}
		}
	}

	// Слоты: позиция + поворот. Вращение ТОЛЬКО по осям: 90 = длинной осью по X
	// (вдоль края острова). Скриптованные 360-повороты запрещены.
	struct FSlot { float CX, CY; float Yaw; };
	TArray<FSlot> Slots;
	// Старый layout был рассчитан на один чанк (7200 см). При увеличенном
	// городе те же координаты оставляли здания в центре, поэтому площадка
	// выглядела пустой и Бэкрумс легко было увидеть по краям.
	// Сетка слотов по всему острову (не 8 штук у края): кварталы заполняются
	// зданиями. Ось дорог (X=0 / Y=0) пропускаем. Точная посадка/масштаб —
	// ниже в цикле размещения (защитные зоны, overlap, вписывание).
	const float GridStep = M.GridStep;
	for (int32 GX = -3; GX <= 3; ++GX)
	{
		if (GX == 0) { continue; } // дорога вдоль Y
		for (int32 GY = -3; GY <= 3; ++GY)
		{
			if (GY == 0) { continue; } // дорога вдоль X
			const float Yaw = (Random.RandRange(0, 1) == 0) ? 0.0f : 90.0f;
			Slots.Add(FSlot{ GX * GridStep, GY * GridStep, Yaw });
		}
	}

	// Хелперы пересечения с защитными зонами.
	auto BoxIntersects = [](float CX, float CY, float HX, float HY,
		float X0, float X1, float Y0, float Y1) -> bool
	{
		return CX + HX > X0 && CX - HX < X1 && CY + HY > Y0 && CY - HY < Y1;
	};
	auto BoxCircleDistSq = [](float CX, float CY, float HX, float HY) -> float
	{
		const float DX = FMath::Max(FMath::Abs(CX) - HX, 0.0f);
		const float DY = FMath::Max(FMath::Abs(CY) - HY, 0.0f);
		return DX * DX + DY * DY;
	};

	int32 Placed = 0;
	OccupiedBoxes.Reset();

	if (Pool.Num() > 0)
	{
		// Перемешиваем пул, чтобы соседние запуски отличались.
		for (int32 i = Pool.Num() - 1; i > 0; --i)
		{
			const int32 j = Random.RandRange(0, i);
			Pool.Swap(i, j);
		}

		for (int32 i = 0; i < Slots.Num(); ++i)
		{
			const FSlot& Slot = Slots[i];
			UStaticMesh* Mesh = CachedLoadMesh(Pool[i % Pool.Num()]);
			if (!Mesh)
			{
				continue;
			}

			const FBoxSphereBounds Bounds = Mesh->GetBounds();
			const float HalfX = Bounds.BoxExtent.X;
			const float HalfY = Bounds.BoxExtent.Y;
			const float HalfZ = Bounds.BoxExtent.Z;
			if (HalfX <= 0.0f || HalfY <= 0.0f || HalfZ <= 0.0f)
			{
				continue;
			}

			// Доступное место слота до защитных зон и края острова.
			const float AvailX = FMath::Min(FMath::Abs(Slot.CX) - Sidewalk, OceMargin - FMath::Abs(Slot.CX));
			const float AvailY = FMath::Min(FMath::Abs(Slot.CY) - Sidewalk, OceMargin - FMath::Abs(Slot.CY));

			// Масштаб: вписать отпечаток в слот и не превысить потолок высоты.
			float Scale = FMath::Min3(AvailX / HalfX, AvailY / HalfY, MaxHeight / HalfZ);
			if (Scale < MinScale)
			{
				continue;
			}

			for (int32 Try = 0; Try < 4; ++Try)
			{
				const bool bLong = (Slot.Yaw == 90.0f || Slot.Yaw == 270.0f);
				const float FootX = (bLong ? HalfY : HalfX) * Scale;
				const float FootY = (bLong ? HalfX : HalfY) * Scale;

				if (Scale < MinScale)
				{
					break;
				}
				if (OverlapsOccupied(FBox2D(FVector2D(Slot.CX - FootX, Slot.CY - FootY),
					FVector2D(Slot.CX + FootX, Slot.CY + FootY))) ||
					BoxIntersects(Slot.CX, Slot.CY, FootX, FootY, -Sidewalk, Sidewalk, -OceMargin, OceMargin) ||
					BoxIntersects(Slot.CX, Slot.CY, FootX, FootY, -OceMargin, OceMargin, -Sidewalk, Sidewalk) ||
					BoxCircleDistSq(Slot.CX, Slot.CY, FootX, FootY) < PlazaR * PlazaR ||
					BoxIntersects(Slot.CX, Slot.CY, FootX, FootY, 0.0f, TrapXMax, -TrapHalfY, TrapHalfY))
				{
					Scale *= 0.8f;
					continue;
				}

				// Здание ставим так, чтобы его ГЕОМЕТРИЧЕСКИЙ ЦЕНТР (Bounds.Origin)
				// лёг в слот: у зданий Buildings-OBJ пивот на ближнем торце
				// длинной стороны, поэтому без компенсации здание уезжает всей
				// длиной на дорогу. Слоты — центры (см. 00_Справка... п.5).
				const FRotator Rot(0.0f, Slot.Yaw + 180.0f * (Random.RandRange(0, 1)), 0.0f);
				const float GroundZ = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale;
				const FVector CenterOffset = Rot.RotateVector(Bounds.Origin * Scale);
				const FVector Pos(Slot.CX - CenterOffset.X, Slot.CY - CenterOffset.Y, -GroundZ);

				if (UInstancedStaticMeshComponent* Inst = GetOrCreateInstancer(Mesh, true))
				{
					Inst->AddInstance(FTransform(Rot, Pos, FVector(Scale)));
				}
				OccupiedBoxes.Add(FBox2D(FVector2D(Slot.CX - FootX, Slot.CY - FootY),
					FVector2D(Slot.CX + FootX, Slot.CY + FootY)));
				++Placed;
				break;
			}
		}
	}

	OccupiedBoxes.Reset();

	// Фолбэк: если ни одного OBJ-здания разместить не удалось — процедурные
	// блоки-здания (чтобы город не был пустым даже до импорта Buildings-OBJ).
	if (Placed > 0)
	{
		return;
	}

	const FLinearColor CityColors[] = {
		FLinearColor(0.55f, 0.50f, 0.45f), FLinearColor(0.45f, 0.48f, 0.55f),
		FLinearColor(0.60f, 0.42f, 0.38f), FLinearColor(0.42f, 0.55f, 0.50f),
		FLinearColor(0.50f, 0.50f, 0.54f), FLinearColor(0.35f, 0.38f, 0.42f),
	};
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		const FSlot& Slot = Slots[i];
		const float AvailX = FMath::Min(FMath::Abs(Slot.CX) - Sidewalk, OceMargin - FMath::Abs(Slot.CX));
		const float AvailY = FMath::Min(FMath::Abs(Slot.CY) - Sidewalk, OceMargin - FMath::Abs(Slot.CY));
		const float MaxHalf = FMath::Clamp(HalfExtent * 0.28f, 200.0f, 1400.0f);
		const float HalfW = FMath::Min(AvailX, MaxHalf);
		const float HalfD = FMath::Min(AvailY, MaxHalf);
		const float H = Random.FRandRange(FMath::Clamp(HalfExtent * 0.083f, 180.0f, 600.0f),
			FMath::Min(MaxHeight, FMath::Clamp(HalfExtent * 0.44f, 400.0f, 1800.0f)));
		const bool bLong = (Slot.Yaw == 90.0f || Slot.Yaw == 270.0f);
		const FVector Half(bLong ? HalfD : HalfW, bLong ? HalfW : HalfD, H * 0.5f);
		AddBox(FVector(Slot.CX, Slot.CY, H * 0.5f), Half,
			CityColors[i % UE_ARRAY_COUNT(CityColors)], 4.0f, EMaterialSlot::Wall);
	}
}

void ABackroomsChunkActor::BuildSpawnPlatform()
{
	// ===== ГОРОД-ОСТРОВОК (лобби): площадка, где можно ходить,
	// пока под ней внизу строится Бэкрумс. =====
	// Город использует СВОИ материалы (не ковёр/штукатурку Бэкрумса), иначе
	// здания белые и «город стоит на бэкрумсе». Переопределяем слоты на
	// городские ДО генерации геометрии — CommitMesh применит их ко всему.
	if (CityFloorMaterial) { FloorMaterial = CityFloorMaterial; }
	if (CityCeilingMaterial) { CeilingMaterial = CityCeilingMaterial; }
	if (CityWallMaterial) { WallMaterial = CityWallMaterial; }

	const float UVScale = 2.0f;
	const int32 CityChunks = FMath::Max(5, PlatformSizeChunks);
	const float PlatformSize = (float)CityChunks * (float)ChunkSizeCells * CellSize;
	const float HalfExtent = PlatformSize * 0.5f;
	const FBackroomsCityMetrics M = FBackroomsCityMetrics::Make(HalfExtent);

	// Основание-плита (верх на Z=0, пол островка).
	AddBox(FVector(0.0f, 0.0f, -40.0f),
		FVector(HalfExtent, HalfExtent, 40.0f),
		FLinearColor(0.42f, 0.42f, 0.46f), 6.0f, EMaterialSlot::Floor);

	// Ограждение-бортик по периметру.
	const float FenceH = M.FenceH;
	const float FenceT = M.FenceT;
	AddBox(FVector(-HalfExtent + FenceT * 0.5f, 0.0f, FenceH * 0.5f),
		FVector(FenceT * 0.5f, HalfExtent, FenceH * 0.5f), FLinearColor(0.14f, 0.14f, 0.16f), 2.0f, EMaterialSlot::Wall);
	AddBox(FVector(HalfExtent - FenceT * 0.5f, 0.0f, FenceH * 0.5f),
		FVector(FenceT * 0.5f, HalfExtent, FenceH * 0.5f), FLinearColor(0.14f, 0.14f, 0.16f), 2.0f, EMaterialSlot::Wall);
	AddBox(FVector(0.0f, -HalfExtent + FenceT * 0.5f, FenceH * 0.5f),
		FVector(HalfExtent, FenceT * 0.5f, FenceH * 0.5f), FLinearColor(0.14f, 0.14f, 0.16f), 2.0f, EMaterialSlot::Wall);
	AddBox(FVector(0.0f, HalfExtent - FenceT * 0.5f, FenceH * 0.5f),
		FVector(HalfExtent, FenceT * 0.5f, FenceH * 0.5f), FLinearColor(0.14f, 0.14f, 0.16f), 2.0f, EMaterialSlot::Wall);

	// Road: a flat cross (asphalt, sidewalks, markings). All layers are nearly
	// flush with the floor plate (top at Z=0) and get NO collision (see
	// CommitMesh), otherwise the raised curbs read as walls and the character
	// trips/slides on them. Small per-layer Z offsets kill z-fighting where
	// the strips overlap.
	const float RoadW = M.RoadW;
	const float WalkW = M.WalkW;
	const float Slab = 8.0f;

	// Asphalt: horizontal strip (long X), then vertical strip (long Y).
	AddBox(FVector(0.0f, 0.0f, 0.4f - Slab), FVector(HalfExtent, RoadW * 0.5f, Slab),
		FLinearColor::White, 8.0f, EMaterialSlot::Road);
	AddBox(FVector(0.0f, 0.0f, 0.5f - Slab), FVector(RoadW * 0.5f, HalfExtent, Slab),
		FLinearColor::White, 8.0f, EMaterialSlot::Road);

	// Sidewalks sit OUTSIDE the asphalt on both sides. The old formula put one
	// curb inside the road (Edge = RoadW/2 + Side*WalkW/2), which produced an
	// extra stripe near the axis - the "3 wall bands" seen from above.
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		const float Edge = Side * (RoadW * 0.5f + WalkW * 0.5f);
		AddBox(FVector(0.0f, Edge, 0.7f - Slab), FVector(HalfExtent, WalkW * 0.5f, Slab),
			FLinearColor::White, 6.0f, EMaterialSlot::Sidewalk);
		AddBox(FVector(Edge, 0.0f, 0.8f - Slab), FVector(WalkW * 0.5f, HalfExtent, Slab),
			FLinearColor::White, 6.0f, EMaterialSlot::Sidewalk);
	}

	// Centerline markings: dashed, flat on top of the asphalt.
	const float MarkHalf = M.MarkHalf;
	const float MarkStep = M.MarkStep;
	for (float MX = -HalfExtent; MX <= HalfExtent; MX += MarkStep)
	{
		AddBox(FVector(MX, 0.0f, 1.0f - 0.5f), FVector(MarkHalf * 0.5f - 6.0f, MarkHalf * 0.5f, 0.5f),
			FLinearColor::White, 2.0f, EMaterialSlot::Marking);
		AddBox(FVector(0.0f, MX, 1.1f - 0.5f), FVector(MarkHalf * 0.5f, MarkHalf * 0.5f - 6.0f, 0.5f),
			FLinearColor::White, 2.0f, EMaterialSlot::Marking);
	}

	FRandomStream Random(Seed ^ 0x51AB3D);

	// Зоны, где ничего не ставим (спавн-площадь, коридор к люку, дорога).
	auto InClearZone = [M](float X, float Y) -> bool
	{
		const float Sidewalk = M.Sidewalk;
		const float PlazaR = M.PlazaR;
		const float TrapXMax = M.TrapXMax;
		const float TrapHalfY = M.TrapHalfY;
		if (FMath::Abs(X) < Sidewalk || FMath::Abs(Y) < Sidewalk)
		{
			return true;
		}
		if (X * X + Y * Y < PlazaR * PlazaR)
		{
			return true;
		}
		if (X > -200.0f && X < TrapXMax && FMath::Abs(Y) < TrapHalfY)
		{
			return true;
		}
		return false;
	};

	// Пробуем разместить реальные OBJ-здания.
	SpawnCityBuildings(Random, HalfExtent);

	// Деревья: ствол + крона (примитивы) — в кварталах, не на дороге/площади.
	for (int32 T = 0; T < 7; ++T)
	{
		const float TX = Random.FRandRange(-HalfExtent + M.TreeMargin, HalfExtent - M.TreeMargin);
		const float TY = Random.FRandRange(-HalfExtent + M.TreeMargin, HalfExtent - M.TreeMargin);
		if (InClearZone(TX, TY)) { --T; continue; }
		const float TrunkH = 90.0f;
		AddBox(FVector(TX, TY, TrunkH * 0.5f), FVector(14.0f, 14.0f, TrunkH * 0.5f),
			FLinearColor(0.30f, 0.20f, 0.12f), 3.0f, EMaterialSlot::Wall);
		AddBox(FVector(TX, TY, TrunkH + 55.0f), FVector(52.0f, 52.0f, 70.0f),
			FLinearColor(0.08f, 0.34f, 0.14f), 3.0f, EMaterialSlot::Wall);
	}

	// Машины: парковка у обочины, вне площади и пути к люку. Позиции — доли
	// HalfExtent (у края кварталов), размер — физический (машина реальная).
	const FLinearColor CarColors[] = {
		FLinearColor(0.60f, 0.15f, 0.15f), FLinearColor(0.15f, 0.30f, 0.62f),
		FLinearColor(0.85f, 0.85f, 0.85f), FLinearColor(0.20f, 0.55f, 0.30f),
	};
	const float CarSlotInset = FMath::Clamp(HalfExtent * 0.65f, 380.0f, 1300.0f);
	const float CarSlotMid = FMath::Clamp(HalfExtent * 0.42f, 240.0f, 900.0f);
	const float CarSlots[][2] = {
		{ CarSlotMid,  CarSlotInset }, { CarSlotInset, -CarSlotMid },
		{ -CarSlotMid, -CarSlotInset }, { -CarSlotInset,  CarSlotMid },
	};
	for (int32 C = 0; C < 4; ++C)
	{
		const float CXc = CarSlots[C][0];
		const float CYc = CarSlots[C][1];
		AddBox(FVector(CXc, CYc, 36.0f), FVector(95.0f, 170.0f, 34.0f), CarColors[C], 2.0f, EMaterialSlot::Wall);
		AddBox(FVector(CXc, CYc, 16.0f), FVector(78.0f, 30.0f, 16.0f), FLinearColor(0.05f, 0.05f, 0.05f), 2.0f, EMaterialSlot::Wall);
	}

	// Street furniture from Content/Fab: benches and bins along the sidewalks
	// (outside the asphalt, still inside the building safety margin). This is
	// what makes the island feel inhabited instead of an empty slab.
	const TCHAR* StreetProps[] = {
		TEXT("/Game/Fab/Modern_Outdoor_Wooden_Bench_Pack/fbx/StaticMeshes/fbx.fbx"),
		TEXT("/Game/Fab/Minneapolis_Trash_bin_LOWPOLY/minneapolis_trash_bin_lowpoly/StaticMeshes/minneapolis_trash_bin_lowpoly.minneapolis_trash_bin_lowpoly"),
	};
	const float WalkEdge = M.WalkEdge;
	const float StreetStep = M.StreetStep;
	int32 StreetIdx = 0;
	auto PlaceStreetProp = [&](const TCHAR* Path, float PX, float PY, float Yaw)
	{
		UStaticMesh* Mesh = CachedLoadMesh(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(Path)));
		if (!Mesh)
		{
			return;
		}
		const FBoxSphereBounds B = Mesh->GetBounds();
		if (B.BoxExtent.X <= 0.0f || B.BoxExtent.Y <= 0.0f || B.BoxExtent.Z <= 0.0f)
		{
			return;
		}
		const float MeshHeight = B.BoxExtent.Z * 2.0f;
		const float Scale = (MeshHeight > 300.0f) ? (150.0f / MeshHeight) : 1.0f;
		const float GroundZ = (B.Origin.Z - B.BoxExtent.Z) * Scale;
		const float Rad = FMath::DegreesToRadians(Yaw);
		const float FootX = (FMath::Abs(B.BoxExtent.X * FMath::Cos(Rad)) + FMath::Abs(B.BoxExtent.Y * FMath::Sin(Rad))) * Scale;
		const float FootY = (FMath::Abs(B.BoxExtent.X * FMath::Sin(Rad)) + FMath::Abs(B.BoxExtent.Y * FMath::Cos(Rad))) * Scale;
		const FBox2D Foot(FVector2D(PX - FootX, PY - FootY), FVector2D(PX + FootX, PY + FootY));
		if (OverlapsOccupied(Foot))
		{
			return;
		}
		if (UInstancedStaticMeshComponent* Inst = GetOrCreateInstancer(Mesh, false))
		{
			Inst->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), FVector(PX, PY, -GroundZ), FVector(Scale)));
		}
		OccupiedBoxes.Add(Foot);
	};
	for (float P = -HalfExtent + M.StreetMargin; P <= HalfExtent - M.StreetMargin; P += StreetStep)
	{
		const TCHAR* MeshPath = StreetProps[StreetIdx % UE_ARRAY_COUNT(StreetProps)];
		++StreetIdx;
		for (int32 Side = -1; Side <= 1; Side += 2)
		{
			const float E = Side * WalkEdge;
			PlaceStreetProp(MeshPath, P, E, (Side > 0) ? 0.0f : 180.0f);
			PlaceStreetProp(MeshPath, E, P, (Side > 0) ? 90.0f : 270.0f);
		}
	}

	// Дополнительные пропсы из базы.
	if (Database && Database->Props.Num() > 0)
	{
		struct FCandidate { const FPropRule* Rule; UStaticMesh* Mesh; };
		TArray<FCandidate> Candidates;
		float TotalWeight = 0.0f;
		for (const FPropRule& Rule : Database->Props)
		{
			// Здания из пула Buildings/* в случайные пропсы города НЕ кидаем.
			const FString Pkg = Rule.Mesh.GetLongPackageName();
			if (Pkg.Contains(TEXT("/Buildings/")))
			{
				continue;
			}
			const bool bFloorProp = Rule.Placement == EPropPlacement::Floor;
			UStaticMesh* Mesh = bFloorProp ? CachedLoadMesh(Rule.Mesh) : nullptr;
			if (Mesh)
			{
				Candidates.Add({ &Rule, Mesh });
				TotalWeight += Rule.Probability;
			}
		}

		for (int32 i = 0; i < 10; ++i)
		{
			if (Candidates.Num() == 0)
			{
				break;
			}

			float Roll = Random.FRandRange(0.0f, TotalWeight);
			const FCandidate* Chosen = &Candidates.Last();
			for (const FCandidate& Cand : Candidates)
			{
				if (Roll <= Cand.Rule->Probability)
				{
					Chosen = &Cand;
					break;
				}
				Roll -= Cand.Rule->Probability;
			}

			UStaticMesh* Mesh = Chosen->Mesh;
			if (!Mesh)
			{
				continue;
			}

			const float Scale = Random.FRandRange(Chosen->Rule->ScaleMin.X, Chosen->Rule->ScaleMax.X);
			const float PX = Random.FRandRange(-HalfExtent + M.PropMargin, HalfExtent - M.PropMargin);
			const float PY = Random.FRandRange(-HalfExtent + M.PropMargin, HalfExtent - M.PropMargin);
			// Только в «тихих» зонах: вне спавн-площади, коридора люка и дороги.
			if (InClearZone(PX, PY))
			{
				--i;
				continue;
			}
			const FRotator Rot(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
			// Пропсы OBJ: сажаем нижней гранью на пол (не пол-высоты).
			// Корректно и для пивота в центре, и для пивота в подошве.
			const float GroundZ = (Mesh->GetBounds().Origin.Z - Mesh->GetBounds().BoxExtent.Z) * Scale;
			const FVector Pos(PX, PY, -GroundZ);

			const float FootX = Mesh->GetBounds().BoxExtent.X * Scale;
			const float FootY = Mesh->GetBounds().BoxExtent.Y * Scale;
			const float BYawRad = FMath::DegreesToRadians(Rot.Yaw);
			const float BCosYaw = FMath::Cos(BYawRad);
			const float BSinYaw = FMath::Sin(BYawRad);
			const float BRotFootX = FMath::Abs(FootX * BCosYaw) + FMath::Abs(FootY * BSinYaw);
			const float BRotFootY = FMath::Abs(FootX * BSinYaw) + FMath::Abs(FootY * BCosYaw);
			FBox2D BFoot(FVector2D(Pos.X - BRotFootX, Pos.Y - BRotFootY), FVector2D(Pos.X + BRotFootX, Pos.Y + BRotFootY));

			{
				FCollisionQueryParams PropQP(SCENE_QUERY_STAT(PropOverlap), false, nullptr);
				const float BPropHalfZ = Mesh->GetBounds().BoxExtent.Z * Scale + 10.0f;
				if (GetWorld()->OverlapBlockingTestByChannel(
					FVector(Pos.X, Pos.Y, Pos.Z + BPropHalfZ),
					FQuat::Identity,
					ECC_WorldStatic,
					FCollisionShape::MakeBox(FVector(BRotFootX + 5.0f, BRotFootY + 5.0f, BPropHalfZ)),
					PropQP))
				{
					continue;
				}
			}

			if (OverlapsOccupied(BFoot))
			{
				continue;
			}

			UInstancedStaticMeshComponent* Inst = GetOrCreateInstancer(Mesh);
			if (Inst)
			{
				Inst->AddInstance(FTransform(Rot, Pos, FVector(Scale)));
			}
			OccupiedBoxes.Add(BFoot);
		}
	}

	CommitMesh();

	int32 TotalCityProps = OccupiedBoxes.Num();
	int32 TotalCityBuildings = 0;
	for (const auto& KVP : Instancers)
	{
		if (KVP.Value)
		{
			TotalCityBuildings += KVP.Value->GetInstanceCount();
		}
	}
	UE_LOG(LogTemp, Display, TEXT("BR City: props=%d buildings=%d"), TotalCityProps, TotalCityBuildings);

	// После CommitMesh() восстанавливаем городские материалы для стартовой платформы:
	// SetupFromProfile() затирает FloorMaterial/CeilingMaterial/WallMaterial на L0-материалы,
	// а BuildSpawnPlatform() переопределяет их на городские. CommitMesh() уже применил
	// городские материалы к меш-секциям — но следующий BuildChunk/BuildChunkFromProfile
	// должен начинать с правильных default'ов. Здесь не нужно дополнительных действий:
	// городские материалы уже применены к mCurrentMesh. Восстанавливаем default'ы для
	// будущих чанков.
	if (bSpawnPlatform)
	{
		if (CityFloorMaterial) { FloorMaterial = CityFloorMaterial; }
		if (CityCeilingMaterial) { CeilingMaterial = CityCeilingMaterial; }
		if (CityWallMaterial) { WallMaterial = CityWallMaterial; }
	}
}

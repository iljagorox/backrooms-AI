#include "BackroomsRoomVariant.h"

namespace
{
	// One static definition per variant. Keeping the table in one place makes it
	// obvious how each location reads and behaves, and lets any level reuse any
	// variant (e.g. the hotel can borrow CaveChamber for a flooded sub-basement).
	const FRoomVariantDef GVariants[] =
	{
		//    Variant                Name                  Min  Max   Wall  Pillar Prop  Scatter                         LightChance Intensity Color                              Ceil
		{ ERoomVariant::Neutral,       TEXT("Room"),          1,  64,  1.00f, 0.00f, 1.00f, EPropScatterStyle::Scattered,     0.50f, 1400.0f, FLinearColor(1.00f, 0.85f, 0.55f), 1.00f },

		{ ERoomVariant::LobbyHalls,    TEXT("Lobby Hall"),    8,  64,  0.55f, 0.10f, 0.80f, EPropScatterStyle::Scattered,     0.85f, 2600.0f, FLinearColor(1.00f, 0.92f, 0.62f), 1.00f },
		{ ERoomVariant::CarpetWarren,  TEXT("Carpet Warren"), 3,  10,  1.35f, 0.00f, 1.10f, EPropScatterStyle::AlongWall,     0.45f, 1200.0f, FLinearColor(1.00f, 0.82f, 0.48f), 0.95f },
		{ ERoomVariant::MoldAlcove,    TEXT("Mold Alcove"),   1,   4,  1.60f, 0.00f, 0.60f, EPropScatterStyle::Corners,       0.30f,  800.0f, FLinearColor(0.85f, 0.90f, 0.70f), 0.90f },

		{ ERoomVariant::Barracks,      TEXT("Barracks"),      5,  24,  1.05f, 0.00f, 1.30f, EPropScatterStyle::AlongWall,     0.60f, 1500.0f, FLinearColor(1.00f, 0.88f, 0.60f), 1.00f },
		{ ERoomVariant::SupplyStore,   TEXT("Store Room"),    3,  14,  1.20f, 0.00f, 1.80f, EPropScatterStyle::Clustered,     0.55f, 1500.0f, FLinearColor(1.00f, 0.86f, 0.58f), 1.00f },
		{ ERoomVariant::UtilityCloset, TEXT("Utility Closet"),1,   3,  1.70f, 0.00f, 0.70f, EPropScatterStyle::Corners,       0.35f,  900.0f, FLinearColor(0.90f, 0.92f, 0.80f), 0.92f },

		{ ERoomVariant::PipeGallery,   TEXT("Pipe Gallery"),  4,  20,  1.25f, 0.00f, 1.10f, EPropScatterStyle::AlongWall,     0.45f, 1300.0f, FLinearColor(0.75f, 0.85f, 1.00f), 0.95f },
		{ ERoomVariant::PumpChamber,   TEXT("Pump Chamber"),  6,  30,  0.90f, 0.14f, 1.30f, EPropScatterStyle::Scattered,     0.55f, 1500.0f, FLinearColor(0.80f, 0.88f, 1.00f), 1.05f },
		{ ERoomVariant::FloodedBasin,  TEXT("Flooded Basin"), 4,  16,  0.85f, 0.06f, 0.70f, EPropScatterStyle::Scattered,     0.35f, 1000.0f, FLinearColor(0.55f, 0.75f, 1.00f), 0.82f },

		{ ERoomVariant::TurbineHall,   TEXT("Turbine Hall"),  10, 64,  0.70f, 0.16f, 0.90f, EPropScatterStyle::Scattered,     0.50f, 1800.0f, FLinearColor(1.00f, 0.70f, 0.45f), 1.25f },
		{ ERoomVariant::SwitchRoom,    TEXT("Switch Room"),   3,  12,  1.40f, 0.00f, 1.40f, EPropScatterStyle::AlongWall,     0.45f, 1200.0f, FLinearColor(1.00f, 0.72f, 0.40f), 0.95f },
		{ ERoomVariant::CableTrench,   TEXT("Cable Trench"),  2,   8,  1.30f, 0.00f, 0.70f, EPropScatterStyle::AlongWall,     0.20f,  700.0f, FLinearColor(0.70f, 0.75f, 0.85f), 0.85f },

		{ ERoomVariant::OpenOffice,    TEXT("Open Office"),   8,  48,  0.80f, 0.10f, 1.20f, EPropScatterStyle::Scattered,     0.80f, 2200.0f, FLinearColor(0.95f, 0.98f, 1.00f), 1.05f },
		{ ERoomVariant::CubicleFarm,   TEXT("Cubicle Farm"),  5,  24,  1.30f, 0.00f, 1.50f, EPropScatterStyle::Clustered,     0.65f, 1800.0f, FLinearColor(0.92f, 0.96f, 1.00f), 0.98f },
		{ ERoomVariant::BreakRoom,     TEXT("Break Room"),    3,  12,  1.00f, 0.00f, 1.10f, EPropScatterStyle::Centerpiece,   0.60f, 1600.0f, FLinearColor(1.00f, 0.95f, 0.85f), 1.00f },

		{ ERoomVariant::HotelSuite,    TEXT("Hotel Suite"),   5,  20,  1.00f, 0.00f, 1.10f, EPropScatterStyle::AlongWall,     0.70f, 1900.0f, FLinearColor(1.00f, 0.75f, 0.55f), 1.00f },
		{ ERoomVariant::HotelCorridor, TEXT("Hotel Corridor"),1,   6,  1.60f, 0.00f, 0.50f, EPropScatterStyle::AlongWall,     0.55f, 1500.0f, FLinearColor(1.00f, 0.70f, 0.52f), 0.95f },
		{ ERoomVariant::Ballroom,      TEXT("Ballroom"),      12, 64,  0.65f, 0.12f, 0.80f, EPropScatterStyle::Scattered,     0.60f, 2000.0f, FLinearColor(1.00f, 0.78f, 0.50f), 1.30f },

		{ ERoomVariant::DarkCrawl,     TEXT("Dark Crawl"),    1,   6,  1.50f, 0.00f, 0.40f, EPropScatterStyle::Corners,       0.08f,  400.0f, FLinearColor(0.55f, 0.60f, 0.70f), 0.80f },
		{ ERoomVariant::BlindAlley,    TEXT("Blind Alley"),   2,   8,  1.45f, 0.00f, 0.60f, EPropScatterStyle::AlongWall,     0.15f,  500.0f, FLinearColor(0.60f, 0.65f, 0.75f), 0.85f },
		{ ERoomVariant::FungalGrove,   TEXT("Fungal Grove"),  6,  28,  0.90f, 0.10f, 1.20f, EPropScatterStyle::Scattered,     0.25f,  600.0f, FLinearColor(0.50f, 0.85f, 0.60f), 1.05f },

		{ ERoomVariant::RaftIsland,    TEXT("Raft Island"),   2,   6,  0.90f, 0.00f, 0.80f, EPropScatterStyle::Centerpiece,   0.30f,  700.0f, FLinearColor(0.60f, 0.75f, 0.95f), 0.75f },
		{ ERoomVariant::PillarSea,     TEXT("Pillar Sea"),    8,  64,  0.55f, 0.18f, 0.55f, EPropScatterStyle::Scattered,     0.30f,  800.0f, FLinearColor(0.45f, 0.70f, 0.95f), 0.80f },
		{ ERoomVariant::DebrisField,   TEXT("Debris Field"),  4,  20,  0.80f, 0.04f, 1.40f, EPropScatterStyle::Scattered,     0.25f,  700.0f, FLinearColor(0.55f, 0.72f, 0.90f), 0.78f },

		{ ERoomVariant::CaveChamber,   TEXT("Cave Chamber"),  5,  26,  0.95f, 0.08f, 1.00f, EPropScatterStyle::Scattered,     0.35f,  900.0f, FLinearColor(0.95f, 0.80f, 0.55f), 1.00f },
		{ ERoomVariant::CrystalGrotto, TEXT("Crystal Grotto"),4,  18,  0.85f, 0.06f, 1.10f, EPropScatterStyle::Corners,       0.30f,  800.0f, FLinearColor(0.70f, 0.85f, 1.00f), 1.05f },
		{ ERoomVariant::CollapsedTunnel,TEXT("Collapsed Pass"),2,  10,  1.40f, 0.00f, 1.20f, EPropScatterStyle::Clustered,     0.25f,  700.0f, FLinearColor(0.85f, 0.72f, 0.50f), 0.85f },

		{ ERoomVariant::WardRoom,      TEXT("Ward"),          6,  28,  0.95f, 0.00f, 1.20f, EPropScatterStyle::AlongWall,     0.85f, 2400.0f, FLinearColor(0.90f, 0.97f, 1.00f), 1.00f },
		{ ERoomVariant::OperatingTheatre, TEXT("Operating Theatre"), 4, 16, 1.10f, 0.00f, 1.00f, EPropScatterStyle::Centerpiece, 0.90f, 2800.0f, FLinearColor(0.95f, 1.00f, 1.00f), 1.15f },
		{ ERoomVariant::Reception,     TEXT("Reception"),     4,  20,  0.90f, 0.05f, 1.00f, EPropScatterStyle::Scattered,     0.75f, 2100.0f, FLinearColor(0.92f, 0.98f, 1.00f), 1.05f },
	};

	constexpr int32 GNumVariants = UE_ARRAY_COUNT(GVariants);
}

const FRoomVariantDef& FRoomVariantSystem::GetDef(ERoomVariant Variant)
{
	const int32 Index = (int32)Variant;
	if (Index < 0 || Index >= GNumVariants || GVariants[Index].Variant != Variant)
	{
		return GVariants[0];
	}
	return GVariants[Index];
}

const TCHAR* FRoomVariantSystem::VariantName(ERoomVariant Variant)
{
	return GetDef(Variant).DisplayName;
}

void FRoomVariantSystem::GetLevelTable(int32 LevelIndex, TArray<FRoomVariantWeight>& Out)
{
	Out.Reset();
	switch (LevelIndex)
	{
	case 0:
		Out = { { ERoomVariant::LobbyHalls, 3.0f }, { ERoomVariant::CarpetWarren, 3.0f }, { ERoomVariant::MoldAlcove, 1.0f } };
		break;
	case 1:
		Out = { { ERoomVariant::Barracks, 3.0f }, { ERoomVariant::SupplyStore, 2.5f }, { ERoomVariant::UtilityCloset, 1.5f } };
		break;
	case 2:
		Out = { { ERoomVariant::PipeGallery, 3.0f }, { ERoomVariant::PumpChamber, 2.0f }, { ERoomVariant::FloodedBasin, 1.5f } };
		break;
	case 3:
		Out = { { ERoomVariant::TurbineHall, 2.0f }, { ERoomVariant::SwitchRoom, 3.0f }, { ERoomVariant::CableTrench, 2.0f } };
		break;
	case 4:
		Out = { { ERoomVariant::OpenOffice, 3.0f }, { ERoomVariant::CubicleFarm, 2.5f }, { ERoomVariant::BreakRoom, 1.5f } };
		break;
	case 5:
		Out = { { ERoomVariant::HotelSuite, 2.5f }, { ERoomVariant::HotelCorridor, 3.0f }, { ERoomVariant::Ballroom, 1.0f } };
		break;
	case 6:
		Out = { { ERoomVariant::DarkCrawl, 3.0f }, { ERoomVariant::BlindAlley, 2.5f }, { ERoomVariant::FungalGrove, 1.5f } };
		break;
	case 7:
		Out = { { ERoomVariant::RaftIsland, 2.0f }, { ERoomVariant::PillarSea, 3.0f }, { ERoomVariant::DebrisField, 2.0f } };
		break;
	case 8:
		Out = { { ERoomVariant::CaveChamber, 3.0f }, { ERoomVariant::CrystalGrotto, 2.0f }, { ERoomVariant::CollapsedTunnel, 2.0f } };
		break;
	case 9:
		Out = { { ERoomVariant::WardRoom, 3.0f }, { ERoomVariant::OperatingTheatre, 1.5f }, { ERoomVariant::Reception, 2.0f } };
		break;
	default:
		Out = { { ERoomVariant::Neutral, 1.0f } };
		break;
	}
}

ERoomVariant FRoomVariantSystem::PickVariant(int32 LevelIndex, uint32 Seed, int32 RoomCells)
{
	TArray<FRoomVariantWeight> Table;
	GetLevelTable(LevelIndex, Table);
	if (Table.Num() == 0)
	{
		return ERoomVariant::Neutral;
	}

	// Bias away from variants whose preferred footprint does not fit the room.
	// (A tiny closet should not roll "Turbine Hall"; a wide hall should not roll
	// "Utility Closet".) Size is only a bias, never a hard filter, so selection
	// still works when the room stats are unknown (RoomCells <= 0).
	float Total = 0.0f;
	TArray<float> Adjusted;
	Adjusted.SetNumZeroed(Table.Num());
	for (int32 i = 0; i < Table.Num(); ++i)
	{
		const FRoomVariantDef& Def = GetDef(Table[i].Variant);
		float Fit = 1.0f;
		if (RoomCells > 0)
		{
			if (RoomCells < Def.MinCells) { Fit = FMath::Max(0.05f, (float)RoomCells / (float)Def.MinCells); }
			else if (RoomCells > Def.MaxCells) { Fit = FMath::Max(0.05f, (float)Def.MaxCells / (float)RoomCells); }
		}
		Adjusted[i] = Table[i].Weight * Fit;
		Total += Adjusted[i];
	}
	if (Total <= 0.0f)
	{
		return Table[0].Variant;
	}

	FRandomStream Rng((int32)Seed);
	float Roll = Rng.FRandRange(0.0f, Total);
	for (int32 i = 0; i < Table.Num(); ++i)
	{
		Roll -= Adjusted[i];
		if (Roll <= 0.0f)
		{
			return Table[i].Variant;
		}
	}
	return Table.Last().Variant;
}

ERoomVariant FRoomVariantSystem::PickVariantByNoise(int32 LevelIndex, float Region01, int32 RoomCells)
{
	TArray<FRoomVariantWeight> Table;
	GetLevelTable(LevelIndex, Table);
	if (Table.Num() == 0)
	{
		return ERoomVariant::Neutral;
	}

	// Same size-fit bias as the seeded pick, but the roll is the incoming
	// coherent field value instead of an RNG. Adjacent cells share a smooth
	// Region01, so they fall into the same weight band -> same variant -> the
	// room kind reads as one district rather than per-cell noise.
	float Total = 0.0f;
	TArray<float> Adjusted;
	Adjusted.SetNumZeroed(Table.Num());
	for (int32 i = 0; i < Table.Num(); ++i)
	{
		const FRoomVariantDef& Def = GetDef(Table[i].Variant);
		float Fit = 1.0f;
		if (RoomCells > 0)
		{
			if (RoomCells < Def.MinCells) { Fit = FMath::Max(0.05f, (float)RoomCells / (float)Def.MinCells); }
			else if (RoomCells > Def.MaxCells) { Fit = FMath::Max(0.05f, (float)Def.MaxCells / (float)RoomCells); }
		}
		Adjusted[i] = Table[i].Weight * Fit;
		Total += Adjusted[i];
	}
	if (Total <= 0.0f)
	{
		return Table[0].Variant;
	}

	float Roll = FMath::Clamp(Region01, 0.0f, 0.9999f) * Total;
	for (int32 i = 0; i < Table.Num(); ++i)
	{
		Roll -= Adjusted[i];
		if (Roll <= 0.0f)
		{
			return Table[i].Variant;
		}
	}
	return Table.Last().Variant;
}

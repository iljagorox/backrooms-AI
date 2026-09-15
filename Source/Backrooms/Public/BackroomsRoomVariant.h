#pragma once

#include "CoreMinimal.h"

// ---- Layer: room variants (room kinds per location) ----
// A room variant is a *kind* of room: how dense its walls are, whether it has
// free-standing columns, how tall the ceiling is, what the lights look like and
// how props are arranged inside it. Variants are pure data with no actor
// dependencies, and selecting one is a stateless function of world position +
// level + seed, so:
//   * the same world seed always produces the same rooms (deterministic);
//   * the result is seamless across chunk borders (no per-chunk RNG state);
//   * the same variant table can be reused by any level/profile.
//
// Levels differ only by *which* variants they enable and with what weights
// (see BackroomsRoomVariant.cpp). Adding a location = adding a table row, not
// new generator code.
enum class ERoomVariant : uint8
{
	Neutral = 0,

	// L0 - Lobby / yellow rooms
	LobbyHalls,       // wide open carpet halls, sparse pillars
	CarpetWarren,     // tight rooms, water-stained walls
	MoldAlcove,       // small damp nook

	// L1 - Habitable
	Barracks,         // bunks along the walls
	SupplyStore,      // crates and shelves, dense fill
	UtilityCloset,    // tiny service room

	// L2 - Pipes / waterworks
	PipeGallery,      // long corridors lined with pipes
	PumpChamber,      // machinery hall with columns
	FloodedBasin,     // low ceiling, standing water

	// L3 - Power
	TurbineHall,      // huge hall, tall ceiling
	SwitchRoom,       // cramped panels, dense walls
	CableTrench,      // narrow trench, very dark

	// L4 - Offices
	OpenOffice,       // big room, cubicles
	CubicleFarm,      // dense partitions
	BreakRoom,        // small social room

	// L5 - Hotel
	HotelSuite,       // large luxurious room
	HotelCorridor,    // narrow repeating corridor
	Ballroom,         // grand hall, high ceiling

	// L6 - Dark
	DarkCrawl,        // claustrophobic, almost no light
	BlindAlley,       // dead-end feel
	FungalGrove,      // organic growth chamber

	// L7 - Ocean
	RaftIsland,       // small platform in the dark
	PillarSea,        // open water, distant columns
	DebrisField,      // scattered wreckage

	// L8 - Caves
	CaveChamber,      // rounded rock room
	CrystalGrotto,    // crystal deposits
	CollapsedTunnel,  // rubble-choked passage

	// L9 - Hospital
	WardRoom,         // beds in rows
	OperatingTheatre, // sterile, high ceiling
	Reception,        // open waiting area

	Count
};

// How props are arranged inside a room. Drives PlacePropsFromProfile.
enum class EPropScatterStyle : uint8
{
	Scattered = 0, // anywhere on the floor (default)
	AlongWall,     // pushed to the walls
	Corners,       // tucked into the corners
	Clustered,     // a single pile near the centre
	Centerpiece    // one large object in the middle
};

struct FRoomVariantDef
{
	ERoomVariant Variant = ERoomVariant::Neutral;
	const TCHAR* DisplayName = TEXT("Room");

	// Preferred footprint (in cells). Used to bias selection toward variants
	// that fit the actual connected room.
	int32 MinCells = 1;
	int32 MaxCells = 64;

	// Wall density multiplier applied to the density field:
	//   > 1 -> more walls (cramped), < 1 -> fewer walls (open).
	float WallBias = 1.0f;
	// Chance (0..1) that an interior floor cell becomes a free-standing column.
	// Columns never break connectivity (placed only on fully interior cells).
	float PillarChance = 0.0f;

	// Prop fill.
	float PropDensity = 1.0f;
	EPropScatterStyle Scatter = EPropScatterStyle::Scattered;

	// Lighting.
	float LightChance = 0.5f;
	float LightIntensity = 1400.0f;
	FLinearColor LightColor = FLinearColor(1.0f, 0.85f, 0.55f);

	// Ceiling height as a fraction of the profile's WallHeight.
	float CeilingScale = 1.0f;
};

struct FRoomVariantWeight
{
	ERoomVariant Variant = ERoomVariant::Neutral;
	float Weight = 1.0f;
};

class FRoomVariantSystem
{
public:
	static const FRoomVariantDef& GetDef(ERoomVariant Variant);
	static const TCHAR* VariantName(ERoomVariant Variant);

	// Fills Out with the variants enabled for a location (with weights).
	// Unknown levels fall back to the Neutral entry.
	static void GetLevelTable(int32 LevelIndex, TArray<FRoomVariantWeight>& Out);

	// Deterministic weighted pick. Seed must already encode world position so
	// two callers at the same place derive the same variant.
	static ERoomVariant PickVariant(int32 LevelIndex, uint32 Seed, int32 RoomCells);

	// Coherent pick: Region01 comes from a low-frequency noise field, so nearby
	// positions land in the same weight interval and therefore the same variant.
	// This makes room kinds form connected districts instead of random squares,
	// while staying deterministic and seamless. Size bias still applies.
	static ERoomVariant PickVariantByNoise(int32 LevelIndex, float Region01, int32 RoomCells);
};

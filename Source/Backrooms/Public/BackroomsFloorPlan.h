#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "BackroomsChunkCoord.h"
#include "BackroomsTopology.h"
#include "BackroomsHandles.h"
#include "BackroomsBoundaryContract.h"
#include "BackroomsMacroFields.h"
#include "BackroomsLocationArchetype.h"
#include "BackroomsFloorPlan.generated.h"

// -------------------------------------------------------
// Step D: UBackroomsFloorPlan — Source of truth for architectural layout.
// 
// Integer topology, integer footprints, stable handles.
// Boundary contracts are embedded and immutable (canonical, per-region-pair+axis).
// Region Attempt may change INTERNAL layout only — boundary contracts never change.
// -------------------------------------------------------

// -------------------------------------------------------
// EOpeningType — types of traversable openings in boundaries
// -------------------------------------------------------
UENUM(BlueprintType)
enum class EOpeningType : uint8
{
	Door       = 0,
	Archway    = 1,
	LargeOpening = 2,
	Emergency  = 3,

	Count
};

// -------------------------------------------------------
// EConnectionType — types of semantic connections between spaces
// -------------------------------------------------------
UENUM(BlueprintType)
enum class EConnectionType : uint8
{
	Door          = 0,
	Archway       = 1,
	LargeOpening  = 2,
	Passage       = 3,

	Count
};

// -------------------------------------------------------
// FChunkGeometryData — data passed from FloorPlan to ChunkActor
// for local geometry building. Does NOT contain architectural
// topology — only geometry rendering primitives.
// Layout mirrors CommitMesh expectations:
//   4 vertices + 6 indices per quad, one QuadSlot entry per quad.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FChunkGeometryData
{
	GENERATED_BODY()

	// Wall geometry vertices/tris (local to chunk origin)
	TArray<FVector> Vertices;

	TArray<int32> Triangles;

	TArray<FVector> Normals;

	TArray<FVector2D> UVs;

	TArray<FLinearColor> VertexColors;

	TArray<FProcMeshTangent> Tangents;

	// Material slot per quad (0 = Floor, 1 = Ceiling, 2 = Wall).
	// Must stay in 1:1 order with the quad stream.
	TArray<uint8> QuadSlots;

	// Door/opening sockets for cross-chunk AI/traversal
	TArray<FVector> DoorSocketPositions;

	TArray<FVector> DoorSocketNormals;

	// Prop placement data (positions, types) — from FloorPlan semantics
	TArray<FVector> PropPositions;

	TArray<int32> PropTypes;  // indices into floor plan prop database

	// Lighting data (derived from FloorPlan semantic data)
	TArray<FVector> LightPositions;

	TArray<FLinearColor> LightColors;

	// Structural column positions (from FloorPlan structural data)
	TArray<FVector> ColumnPositions;

	FChunkGeometryData() = default;

	// Clear all data
	void Clear();
};

// -------------------------------------------------------
// FBackroomsSpaceData — semantic space data with integer topology.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FBackroomsSpaceData
{
	GENERATED_BODY()

	// Stable handle — RegionCoord + LocalId, survives reallocations/serialization
	UPROPERTY()
	FBackroomsSpaceHandle Handle;

	// Role — OpenSpace, PartitionedSpace, TransitionSpace, ServiceSpace, StructuralVoid
	UPROPERTY()
	EBackroomsSpaceRole Role = EBackroomsSpaceRole::OpenSpace;

	// Broad-phase bounds (integer grid box)
	UPROPERTY()
	FGridBox Bounds;

	// Actual footprint — integer cell coordinates defining the shape.
	// Preferable to FBox2D for topology: exact, deterministic, no float equality issues.
	// May be convex or L/T-shaped represented as cell spans.
	UPROPERTY()
	TArray<FGridCell> Footprint;  // all cells belonging to this space

	// Area in cells (derived from footprint area)
	UPROPERTY()
	int32 Area = 0;

	// Openness — derived from MacroField at space center, not RNG
	UPROPERTY()
	float Openness = 0.5f;  // 0.0 = cramped, 1.0 = open

	// Ceiling height — derived from MacroField, not hardcoded
	UPROPERTY()
	float CeilingHeight = 3.0f;  // relative to profile WallHeight

	// Structural data — column positions for spans > 6 cells (integer grid)
	UPROPERTY()
	TArray<FGridCell> ColumnPositions;

	// Environmental tags (NO entity tags): Quiet, Dark, Bright, Open, Confined, Wet, Dry, etc.
	UPROPERTY()
	TArray<FName> EnvironmentalTags;

	// Traversal properties
	UPROPERTY()
	float FlowPriority = 0.5f;  // 0.0 = dead end, 1.0 = main thoroughfare

	UPROPERTY()
	bool bIsLoop = false;

	UPROPERTY()
	bool bIsDeadEnd = false;

	// -------------------------------------------------------
	// Validation / metadata
	// -------------------------------------------------------
	UPROPERTY()
	bool bIsValid = true;

	UPROPERTY()
	float ValidationScore = 1.0f;  // overall quality score

	FBackroomsSpaceData() = default;

	// Area from footprint (sum of grid cells, or bounding box area)
	void ComputeArea();

	// Check if grid cell belongs to this space
	bool ContainsCell(const FGridCell& Cell) const;
};

// -------------------------------------------------------
// FBackroomsOpeningData — traversable section in a space boundary.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FBackroomsOpeningData
{
	GENERATED_BODY()

	// Owner space
	UPROPERTY()
	FBackroomsSpaceHandle OwnerSpace;

	// Boundary edge on owner space's footprint where opening is cut
	UPROPERTY()
	EGridDir BoundaryEdge = EGridDir::East;

	// Position on the boundary (grid cell coordinate, local to space footprint)
	UPROPERTY()
	FGridPoint Position;

	// Width in cells (typically 2 for doors, can be larger for archways)
	UPROPERTY()
	float Width = 2.0f;

	// Opening type: Door, Archway, LargeOpening, Emergency
	UPROPERTY()
	EOpeningType Type = EOpeningType::Door;

	// Connection this opening creates (if any)
	UPROPERTY()
	FBackroomsConnectionHandle Connection;

	// -------------------------------------------------------
	// Metadata
	// -------------------------------------------------------
	UPROPERTY()
	bool bIsValid = true;

	FBackroomsOpeningData() = default;
};

// -------------------------------------------------------
// FBackroomsConnectionData — semantic graph edge created by an Opening.
// References spaces by handles. NOT a duplicate of doorway data.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FBackroomsConnectionData
{
	GENERATED_BODY()

	// The opening that created this connection
	UPROPERTY()
	FBackroomsOpeningData Opening;

	// Spaces connected (by stable handles — survives reallocations)
	UPROPERTY()
	FBackroomsSpaceHandle SpaceA;
	UPROPERTY()
	FBackroomsSpaceHandle SpaceB;

	// Connection type derived from opening type
	UPROPERTY()
	EConnectionType Type = EConnectionType::Door;

	// AI traversal data (plain members — mirrors structural layer, not reflection)
	struct FAITraversal
	{
		bool bTwoWay = true;
		float MaxSpeed = 250.0f;
		bool bBlocksLOS = false;
		bool bBlocksSound = false;
	} AI;

	// Streaming/data tags
	struct FStreaming
	{
		bool bVisibleFromDistance = true;
		float SearchRangeBonus = 0.0f;
	} Streaming;

	// -------------------------------------------------------
	// Metadata
	// -------------------------------------------------------
	UPROPERTY()
	bool bIsValid = true;

	FBackroomsConnectionData() = default;
};

// -------------------------------------------------------
// FBackroomsValidationResult — validation output
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FBackroomsValidationResult
{
	GENERATED_BODY()

	// Hard pass: ALL hard constraints satisfied?
	UPROPERTY()
	bool bHardPass = false;

	// Soft metric scores (0.0 = worst, 1.0 = best)
	UPROPERTY()
	float MazeScore = 1.0f;
	UPROPERTY()
	float RepetitionScore = 1.0f;
	UPROPERTY()
	float OpennessScore = 0.5f;
	UPROPERTY()
	float DeadEndRatio = 0.0f;
	UPROPERTY()
	float FlowContinuityScore = 1.0f;

	// Configurable thresholds (from archetype) — for reference only
	UPROPERTY()
	float MazeThreshold = 0.3f;
	UPROPERTY()
	float RepetitionThreshold = 0.5f;
	UPROPERTY()
	float OpennessThreshold = 0.4f;
	UPROPERTY()
	float DeadEndThreshold = 0.15f;
	UPROPERTY()
	float ContinuityThreshold = 0.7f;

	// Failed hard constraints
	UPROPERTY()
	TArray<FName> FailedHardConstraints;

	// Soft metrics above threshold (would reject if thresholds were stricter)
	UPROPERTY()
	TArray<FName> SoftMetricsAboveThreshold;

	FBackroomsValidationResult() = default;
};

// -------------------------------------------------------
// FLOOR CELLS REPRESENT SPACE. EDGES REPRESENT WALLS.
// A wall/partition lives on the EDGE between two adjacent cells and NEVER
// consumes a cell. Openings are traversable sections of a partition edge
// sequence. Region seams are also edges governed by BoundaryContracts.
// -------------------------------------------------------
enum class EFloorEdgeState : uint8
{
	Open = 0,      // free passage
	Wall = 1,      // full-height solid boundary
	Partition = 2, // partial-height boundary (visual subdivision)
	Opening = 3,   // traversable gap cut into a Wall/Partition sequence
};

UCLASS()
class BACKROOMS_API UBackroomsFloorPlan : public UObject
{
	GENERATED_BODY()

public:
	// -------------------------------------------------------
	// Construction/Destruction
	// -------------------------------------------------------
	UBackroomsFloorPlan();
	virtual ~UBackroomsFloorPlan() override = default;

	// Generate the floor plan from deterministic inputs.
	// WorldSeed + RegionCoordinate → identical result regardless of chunk loading order.
	// Attempt changes only internal layout; boundary contracts immutable.
	void Generate(int32 WorldSeed, const FIntPoint& RegionCoordinate, int32 Attempt = 0, UBackroomsLocationArchetype* Archetype = nullptr);

	// Validate internal layout only. Boundary contracts remain unchanged.
	// Hard failure => reject. Soft metrics => scored per archetype thresholds.
	FBackroomsValidationResult Validate() const;

	// Extract chunk data from the floor plan for a given chunk coordinate.
	// Chunk builds local render geometry from this data — does NOT invent topology.
	void ExtractChunk(const FChunkCoord& ChunkCoord, FChunkGeometryData& OutGeometry) const;

	// Get space by stable handle (plain C++: struct pointer cannot be reflected)
	const FBackroomsSpaceData* GetSpaceByHandle(const FBackroomsSpaceHandle& Handle) const;

	// Get boundary contract for a specific region edge (plain C++: struct pointer)
	const FBoundaryContract* GetBoundaryContract(const FIntPoint& RegionCoord, EEdgeID Edge) const;

	// -------------------------------------------------------
	// Public data — all architectural authority resides here.
	// -------------------------------------------------------

	// Region identification
	UPROPERTY()
	int32 WorldSeed = 0;

	UPROPERTY()
	FIntPoint RegionCoordinate;

	UPROPERTY()
	int32 LevelIndex = 0;

	// Grid extent of this floor plan (in chunks, for streaming calculations)
	UPROPERTY()
	FChunkCoord ChunkExtent = FChunkCoord(8, 8);

	// Cells per chunk side (from the active level profile; default 8).
	UPROPERTY()
	int32 ChunkCellSize = 8;

	// Region side in cells (= ChunkExtent * ChunkCellSize).
	UPROPERTY()
	int32 RegionSizeCells = 64;

	// World scale: one cell = CellSizeWorld centimeters.
	UPROPERTY()
	float CellSizeWorld = 500.0f;

	// Wall geometry parameters (mirror ChunkActor values from the active profile).
	UPROPERTY()
	float WallHeightWorld = 300.0f;

	UPROPERTY()
	float WallThicknessWorld = 30.0f;

	// Spaces — architectural areas with integer footprints and stable handles
	UPROPERTY()
	TArray<FBackroomsSpaceData> Spaces;

	// Openings — traversable sections cut into space boundaries
	// Each opening references a boundary edge and connects to another space's opening
	UPROPERTY()
	TArray<FBackroomsOpeningData> Openings;

	// Connections — semantic graph edges derived from openings
	// Do NOT duplicate doorway data; Connection references the source Opening
	UPROPERTY()
	TArray<FBackroomsConnectionData> Connections;

	// Boundary contracts — canonical per-region-pair+axis, immutable
	// Generated once, consumed by region generation, never retried/changed
	UPROPERTY()
	TMap<FEdgeKey, FBoundaryContract> BoundaryContracts;

	// Macro fields — slowly varying coherent fields from world coordinates
	// Ensure "this is still the same place, but gradually changing"
	UPROPERTY()
	FBackroomsMacroFields MacroFields;

	// Generation context metadata
	UPROPERTY()
	int32 GenerationAttempt = 0;

	// Whether this floor plan successfully passed validation
	UPROPERTY()
	bool bValid = false;

	// -------------------------------------------------------
	// Debug/visualization support
	// -------------------------------------------------------
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "FloorPlan|Debug")
	FString GetDebugInfo() const;
#endif

	// Region-local cell index helper (public: used by extraction + debug tools).
	int32 CellIndex(int32 RX, int32 RY) const { return RY * RegionSizeCells + RX; }

	// -------------------------------------------------------
	// Edge-based topology accessors.
	// FLOOR CELLS REPRESENT SPACE. EDGES REPRESENT WALLS.
	// A wall/partition lives on the EDGE between two cells; it never
	// consumes a cell. Openings are traversable edge sections.
	// -------------------------------------------------------

	// Canonical edge id for the edge from (RX,RY) toward Dir.
	// Interior edges are stored once (owned by the West/South cell);
	// region-seam edges live in a dedicated border block. INDEX_NONE if invalid.
	int32 EdgeId(int32 RX, int32 RY, EGridDir Dir) const;

	// State of the canonical edge toward Dir from (RX,RY): one of EFloorEdgeState.
	uint8 EdgeStateBetween(int32 RX, int32 RY, EGridDir Dir) const;

	// Space LocalId of a region-local cell (every floor cell belongs to a space).
	int32 SpaceAt(int32 RX, int32 RY) const;

	// True if the edge toward Dir is opened for traversal.
	bool HasOpeningBetween(int32 RX, int32 RY, EGridDir Dir) const;

	// True if the edge toward Dir is a solid or partial boundary.
	bool HasWallBetween(int32 RX, int32 RY, EGridDir Dir) const;

private:
	// Space LocalId per region-local floor cell — every cell is floor.
	TArray<int32> CellSpaceGrid;

	// EFloorEdgeState per canonical edge.
	// Layout: interior 2 slots per cell (East/North owned by cell) +
	//         border seam block of 4 * RegionSizeCells slots (W/E/S/N edges).
	TArray<uint8> EdgeStates;
};

FORCEINLINE int32 UBackroomsFloorPlan::EdgeId(int32 RX, int32 RY, EGridDir Dir) const
{
	const int32 RS = RegionSizeCells;
	const int32 Base = RS * RS * 2;
	if (RX < 0 || RX >= RS || RY < 0 || RY >= RS)
	{
		return INDEX_NONE;
	}
	switch (Dir)
	{
		case EGridDir::East:
			return (RX + 1 < RS) ? (CellIndex(RX, RY) * 2 + 0) : (Base + 1 * RS + RY);
		case EGridDir::West:
			return (RX - 1 >= 0) ? (CellIndex(RX - 1, RY) * 2 + 0) : (Base + 0 * RS + RY);
		case EGridDir::North:
			return (RY + 1 < RS) ? (CellIndex(RX, RY) * 2 + 1) : (Base + 3 * RS + RX);
		case EGridDir::South:
			return (RY - 1 >= 0) ? (CellIndex(RX, RY - 1) * 2 + 1) : (Base + 2 * RS + RX);
		default:
			return INDEX_NONE;
	}
}

FORCEINLINE uint8 UBackroomsFloorPlan::EdgeStateBetween(int32 RX, int32 RY, EGridDir Dir) const
{
	const int32 Id = EdgeId(RX, RY, Dir);
	return (Id != INDEX_NONE && EdgeStates.IsValidIndex(Id)) ? EdgeStates[Id] : (uint8)EFloorEdgeState::Wall;
}

FORCEINLINE int32 UBackroomsFloorPlan::SpaceAt(int32 RX, int32 RY) const
{
	if (RX < 0 || RX >= RegionSizeCells || RY < 0 || RY >= RegionSizeCells)
	{
		return INDEX_NONE;
	}
	const int32 Idx = CellIndex(RX, RY);
	return CellSpaceGrid.IsValidIndex(Idx) ? CellSpaceGrid[Idx] : INDEX_NONE;
}

FORCEINLINE bool UBackroomsFloorPlan::HasOpeningBetween(int32 RX, int32 RY, EGridDir Dir) const
{
	const int32 Id = EdgeId(RX, RY, Dir);
	return (Id != INDEX_NONE && EdgeStates.IsValidIndex(Id)) && EdgeStates[Id] == (uint8)EFloorEdgeState::Opening;
}

FORCEINLINE bool UBackroomsFloorPlan::HasWallBetween(int32 RX, int32 RY, EGridDir Dir) const
{
	const int32 Id = EdgeId(RX, RY, Dir);
	if (Id == INDEX_NONE || !EdgeStates.IsValidIndex(Id))
	{
		return true; // out of domain edge — treat as solid
	}
	const uint8 S = EdgeStates[Id];
	return S == (uint8)EFloorEdgeState::Wall || S == (uint8)EFloorEdgeState::Partition;
}
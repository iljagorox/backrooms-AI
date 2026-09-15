#pragma once

#include "CoreMinimal.h"
#include "BackroomsChunkCoord.h"
#include "BackroomsTopology.h"
#include "BackroomsHandles.h"
#include "BackroomsBoundaryContract.generated.h"

// -------------------------------------------------------
// Step C: Canonical BoundaryContract generation
// 
// Shared boundary has its OWN deterministic seed.
// For any adjacent Region A and Region B:
//   BoundaryKey = WorldSeed + LevelIndex + CanonicalMin(RegionA, RegionB) + CanonicalMax(RegionA, RegionB) + BoundaryAxis
//   BoundaryContract = GenerateBoundaryContract(BoundaryKey)
// 
// Result: A.East == B.West by construction. No comparison/retry needed.
// Region generation consumes immutable BoundaryContracts.
// Region Attempt may change INTERNAL layout only — never external boundary contracts.
// -------------------------------------------------------

// -------------------------------------------------------
// FBoundaryContract — the immutable contract data for a shared edge between two regions.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FBoundaryContract
{
	GENERATED_BODY()

	// Portal positions along the shared boundary (grid coordinates).
	// These are in the local space of the region edge, not world space.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoundaryContract")
	TArray<FGridPoint> PortalPositions;  // grid cells where portals open

	// Portal widths (in cells). Typically 2 cells wide for main connections.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoundaryContract")
	TArray<float> PortalWidths;

	// Connection type: Door, Archway, LargeOpening, Emergency
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoundaryContract")
	TArray<EGridDir> ConnectionTypes;  // one per portal

	// Primary flow direction information (0 = no preference, 1 = primary flow, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoundaryContract")
	TArray<float> FlowPriorities;

	// Whether this boundary is the "main" exit-facing side, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoundaryContract")
	bool bIsExitFacing = false;

	// Deterministic contract key for debugging/verification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoundaryContract")
	FString ContractKey;

	FBoundaryContract() = default;

	// String representation for logging/debugging
	FString ToString() const;
};

// -------------------------------------------------------
// ToString — string representation for logging/debugging.
// -------------------------------------------------------
FORCEINLINE FString FBoundaryContract::ToString() const
{
	return FString::Printf(TEXT("BoundaryContract(Portals=%d, Widths=%d, Types=%d, Flow=%d, ExitFacing=%d)"),
		PortalPositions.Num(), PortalWidths.Num(), ConnectionTypes.Num(), FlowPriorities.Num(), bIsExitFacing ? 1 : 0);
}

// -------------------------------------------------------
// GenerateBoundaryContract — creates the canonical contract
// for a shared boundary between two regions.
// 
// BoundaryKey components:
//   WorldSeed       — global deterministic seed
//   LevelIndex      — current level (0..16)
//   CanonicalMin/Max — ensures (A,B) and (B,A) produce same key
//   BoundaryAxis    — which edge (±X, ±Y)
// 
// The contract is deterministic: same inputs → same output.
// Region generation CONSUMES this contract; Attempt never changes it.
// -------------------------------------------------------

// -------------------------------------------------------
// FBackroomsBoundaryContractGenerator — static class for generating
// canonical boundary contracts between regions.
// -------------------------------------------------------
class BACKROOMS_API FBackroomsBoundaryContractGenerator
{
public:
	// Generates the canonical boundary contract for the shared edge
	// between RegionA and RegionB at the specified axis.
	// 
	// @param WorldSeed       — global world seed
	// @param LevelIndex      — current level index
	// @param RegionA         — first region coordinate
	// @param RegionB         — second region coordinate (adjacent to A)
	// @param BoundaryAxis    — which edge separates them (EEdgeID)
	// @param OutContract     — generated contract (filled in)
	// @return true if successful
	static bool GenerateBoundaryContract(
		int32 WorldSeed,
		int32 LevelIndex,
		const FIntPoint& RegionA,
		const FIntPoint& RegionB,
		EEdgeID BoundaryAxis,
		int32 RegionSizeCells,
		FBoundaryContract& OutContract);

	// Canonical key computation — same as used in GenerateBoundaryContract
	static FString ComputeBoundaryKey(
		int32 WorldSeed,
		int32 LevelIndex,
		const FIntPoint& RegionA,
		const FIntPoint& RegionB,
		EEdgeID BoundaryAxis);

	// Verifies that two regions sharing an edge have matching contracts.
	// In the new model this always passes by construction, but kept for
	// editorial/logging purposes.
	static bool VerifyContractsMatch(const FBoundaryContract& A, const FBoundaryContract& B);
};

// -------------------------------------------------------
// ComputeBoundaryKey — human-readable key for debugging.
// -------------------------------------------------------
FORCEINLINE FString FBackroomsBoundaryContractGenerator::ComputeBoundaryKey(
	int32 WorldSeed,
	int32 LevelIndex,
	const FIntPoint& RegionA,
	const FIntPoint& RegionB,
	EEdgeID BoundaryAxis)
{
	FIntPoint Min = CanonicalMin(RegionA, RegionB);
	FIntPoint Max = CanonicalMax(RegionA, RegionB);

	return FString::Printf(
		TEXT("Seed=%d Level=%d RegionMin=(%d,%d) RegionMax=(%d,%d) Axis=%d"),
		WorldSeed, LevelIndex, Min.X, Min.Y, Max.X, Max.Y, (int32)BoundaryAxis);
}

// -------------------------------------------------------
// VerifyContractsMatch — always true in canonical model, but kept
// for logging/editorial verification. NOTE: A and B are expected to
// be generated from the same canonical (RegionA,RegionB) pair, so
// their portal positions are comparable cell-by-cell.
// -------------------------------------------------------
FORCEINLINE bool FBackroomsBoundaryContractGenerator::VerifyContractsMatch(const FBoundaryContract& A, const FBoundaryContract& B)
{
	bool Match = A.PortalPositions.Num() == B.PortalPositions.Num();
	if (Match)
	{
		for (int32 i = 0; i < A.PortalPositions.Num(); i++)
		{
			Match &= A.PortalPositions[i] == B.PortalPositions[i];
			Match &= A.PortalWidths[i] == B.PortalWidths[i];
			Match &= A.ConnectionTypes[i] == B.ConnectionTypes[i];
			Match &= FMath::Abs(A.FlowPriorities[i] - B.FlowPriorities[i]) < 0.001f;
		}
	}
	return Match;
}

// -------------------------------------------------------
// GenerateBoundaryContract — deterministic canonical contract for the
// shared edge between RegionA and RegionB.
//
// Portal positions are stored in the LOCAL coordinate of the shared edge:
//   E/W edge  -> P.X carries the coordinate along the edge (local Y of both regions)
//   N/S edge  -> P.Y carries the coordinate along the edge (local X of both regions)
// Because adjacent regions share the same extent along the boundary axis,
// these coordinates are identical in both regions' local space.
// -------------------------------------------------------
FORCEINLINE bool FBackroomsBoundaryContractGenerator::GenerateBoundaryContract(
	int32 WorldSeed,
	int32 LevelIndex,
	const FIntPoint& RegionA,
	const FIntPoint& RegionB,
	EEdgeID BoundaryAxis,
	int32 RegionSizeCells,
	FBoundaryContract& OutContract)
{
	OutContract.PortalPositions.Reset();
	OutContract.PortalWidths.Reset();
	OutContract.ConnectionTypes.Reset();
	OutContract.FlowPriorities.Reset();
	OutContract.bIsExitFacing = false;
	OutContract.ContractKey.Empty();

	const FIntPoint Min = CanonicalMin(RegionA, RegionB);
	const FIntPoint Max = CanonicalMax(RegionA, RegionB);

	// Deterministic boundary seed — independent of region internal generation.
	uint32 BoundaryHash = (uint32)WorldSeed;
	BoundaryHash = HashCombine(BoundaryHash, (uint32)LevelIndex);
	BoundaryHash = HashCombine(BoundaryHash, (uint32)Min.X);
	BoundaryHash = HashCombine(BoundaryHash, (uint32)Min.Y);
	BoundaryHash = HashCombine(BoundaryHash, (uint32)Max.X);
	BoundaryHash = HashCombine(BoundaryHash, (uint32)Max.Y);
	BoundaryHash = HashCombine(BoundaryHash, (uint32)(int32)BoundaryAxis);

	FRandomStream Stream((int32)BoundaryHash);

	// 1..3 portals per edge; deterministically biased by seed.
	const int32 PortalCount = 1 + Stream.RandRange(0, 2);

	// Keep a 2-cell margin from the corners so portals never produce
	// zero-width stubs.
	const int32 EdgeLo = 2;
	const int32 EdgeHi = FMath::Max(EdgeLo + 1, RegionSizeCells - 3);

	TArray<int32> Positions;
	int32 Guard = 0;
	while (Positions.Num() < PortalCount && Guard < 128)
	{
		Guard++;
		const int32 Pos = Stream.RandRange(EdgeLo, EdgeHi);
		bool bOk = true;
		for (int32 i = 0; i < Positions.Num(); i++)
		{
			if (FMath::Abs(Positions[i] - Pos) < 4)
			{
				bOk = false;
				break;
			}
		}
		if (bOk)
		{
			Positions.Add(Pos);
		}
	}

	for (int32 i = 0; i < Positions.Num(); i++)
	{
		FGridPoint P;
		if (BoundaryAxis == EEdgeID::EastEdge || BoundaryAxis == EEdgeID::WestEdge)
		{
			// Vertical boundary: coordinate lies along local Y of both regions.
			P.X = Positions[i];
			P.Y = 0;
		}
		else
		{
			// Horizontal boundary: coordinate lies along local X of both regions.
			P.X = 0;
			P.Y = Positions[i];
		}
		OutContract.PortalPositions.Add(P);

		// Width in cells: most portals are 2 cells wide.
		const float Width = Stream.FRand() < 0.7f ? 2.f : 1.f;
		OutContract.PortalWidths.Add(Width);

		const float Roll = Stream.FRand();
		EGridDir TypeTag = EGridDir::East;
		if (Roll < 0.55f)      TypeTag = EGridDir::East;   // door
		else if (Roll < 0.85f) TypeTag = EGridDir::North;  // archway
		else                   TypeTag = EGridDir::West;   // large opening
		OutContract.ConnectionTypes.Add(TypeTag);

		OutContract.FlowPriorities.Add(Stream.FRand());
	}

	OutContract.ContractKey = ComputeBoundaryKey(WorldSeed, LevelIndex, RegionA, RegionB, BoundaryAxis);
	return OutContract.PortalPositions.Num() > 0;
}
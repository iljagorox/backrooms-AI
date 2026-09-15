#pragma once

#include "CoreMinimal.h"
#include "BackroomsChunkCoord.h"
#include "BackroomsTopology.generated.h"

// -------------------------------------------------------
// Step A: Core integer topology structures
// Architecture represented in integer/grid/topological coordinates.
// World-space float coordinates derived later.
// -------------------------------------------------------

// -------------------------------------------------------
// FGridCell — exact integer cell coordinate.
// Used as topological authority for boundaries, connectivity,
// portal ownership, footprint representation.
// Never use FVector2D equality for these purposes.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FGridCell
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridCell")
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridCell")
	int32 Y = 0;

	FGridCell() = default;
	FGridCell(int32 InX, int32 InY) : X(InX), Y(InY) {}

	bool operator==(const FGridCell& Other) const { return X == Other.X && Y == Other.Y; }
	bool operator!=(const FGridCell& Other) const { return !(*this == Other); }

	bool operator<(const FGridCell& Other) const
	{
		return (Y != Other.Y) ? (Y < Other.Y) : (X < Other.X);
	}

	FGridCell operator+(const FGridCell& Other) const { return FGridCell(X + Other.X, Y + Other.Y); }
	FGridCell operator-(const FGridCell& Other) const { return FGridCell(X - Other.X, Y - Other.Y); }
};

// -------------------------------------------------------
// FGridBox — integer bounding box in cell coordinates.
// Broad-phase only; topology uses exact GridCell comparisons,
// not FBox2D equality.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FGridBox
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridBox")
	FGridCell Min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridBox")
	FGridCell Max;

	FGridBox() = default;
	FGridBox(int32 XMin, int32 YMin, int32 XMax, int32 YMax)
		: Min(FGridCell(XMin, YMin)), Max(FGridCell(XMax, YMax)) {}

	bool operator==(const FGridBox& Other) const { return Min == Other.Min && Max == Other.Max; }
	bool operator!=(const FGridBox& Other) const { return !(*this == Other); }

	int32 Width() const { return Max.X - Min.X + 1; }
	int32 Height() const { return Max.Y - Min.Y + 1; }
	int32 Area() const { return Width() * Height(); }

	FGridCell Center() const { return FGridCell((Min.X + Max.X) / 2, (Min.Y + Max.Y) / 2); }
};

// -------------------------------------------------------
// EGridDir — four cardinal directions in grid topology.
// Used for boundary edges, neighbor discovery, portal orientation.
// -------------------------------------------------------
UENUM(BlueprintType)
enum class EGridDir : uint8
{
	East  = 0,
	West  = 1,
	North = 2,
	South = 3,

	Count
};

FORCEINLINE FGridCell GridDirToVector(EGridDir Dir)
{
	switch (Dir)
	{
		case EGridDir::East:  return FGridCell(1, 0);
		case EGridDir::West:  return FGridCell(-1, 0);
		case EGridDir::North: return FGridCell(0, 1);
		case EGridDir::South: return FGridCell(0, -1);
		default: return FGridCell();
	}
}

// -------------------------------------------------------
// FGridPoint — exact point in integer grid topology.
// Used for footprint vertices, opening positions, column placements.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FGridPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridPoint")
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridPoint")
	int32 Y = 0;

	FGridPoint() = default;
	FGridPoint(int32 InX, int32 InY) : X(InX), Y(InY) {}

	bool operator==(const FGridPoint& Other) const { return X == Other.X && Y == Other.Y; }
	bool operator!=(const FGridPoint& Other) const { return !(*this == Other); }

	FGridPoint operator+(const FGridPoint& Other) const { return FGridPoint(X + Other.X, Y + Other.Y); }
	FGridPoint operator-(const FGridPoint& Other) const { return FGridPoint(X - Other.X, Y - Other.Y); }

	friend inline uint32 GetTypeHash(const FGridPoint& P)
	{
		return HashCombine(GetTypeHash(P.X), GetTypeHash(P.Y));
	}
};

// -------------------------------------------------------
// Canonical region minimization for boundary contracts.
// Ensures (RegionA, RegionB) always produces same key regardless of argument order.
// -------------------------------------------------------
FORCEINLINE FIntPoint CanonicalMin(const FIntPoint& A, const FIntPoint& B)
{
	return A.X < B.X || (A.X == B.X && A.Y < B.Y) ? A : B;
}

FORCEINLINE FIntPoint CanonicalMax(const FIntPoint& A, const FIntPoint& B)
{
	return A.X > B.X || (A.X == B.X && A.Y > B.Y) ? A : B;
}

// -------------------------------------------------------
// EEdgeID — identifier for a boundary edge of a space/region.
// Used in canonical boundary contract keys.
// -------------------------------------------------------
UENUM(BlueprintType)
enum class EEdgeID : uint8
{
	EastEdge = 0,   // +X direction
	WestEdge  = 1,  // -X direction
	NorthEdge = 2,  // +Y direction
	SouthEdge = 3,  // -Y direction

	Count
};

// -------------------------------------------------------
// FEdgeKey — half of a boundary contract key.
// Encodes which edge of which region, canonicalized.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FEdgeKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EdgeKey")
	FIntPoint RegionCoord;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EdgeKey")
	EEdgeID Edge = EEdgeID::EastEdge;

	FEdgeKey() = default;
	FEdgeKey(const FIntPoint& InRegionCoord, EEdgeID InEdge)
		: RegionCoord(InRegionCoord), Edge(InEdge) {}

	bool operator==(const FEdgeKey& Other) const
	{
		return RegionCoord == Other.RegionCoord && Edge == Other.Edge;
	}
	bool operator!=(const FEdgeKey& Other) const { return !(*this == Other); }
};

// -------------------------------------------------------
// Hash support for FEdgeKey (if needed in TSet/TMap)
// -------------------------------------------------------
inline uint32 GetTypeHash(const FEdgeKey& Key)
{
	return GetTypeHash(Key.RegionCoord) ^ (uint32)Key.Edge;
}
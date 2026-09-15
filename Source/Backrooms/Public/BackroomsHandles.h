#pragma once

#include "CoreMinimal.h"
#include "BackroomsChunkCoord.h"
#include "BackroomsTopology.h"
#include "BackroomsHandles.generated.h"

// -------------------------------------------------------
// Step B: Stable IDs/handles — replaces raw pointers in
// persistent FloorPlan/SpatialGraph structures.
// Must survive: TArray reallocations, serialization,
// region unload/load, streaming, save/load.
// -------------------------------------------------------

// -------------------------------------------------------
// FBackroomsSpaceHandle — stable handle to a Space in a FloorPlan.
// RegionCoord identifies which region the space lives in.
// LocalId is the index of the space within that region's space array.
// Together they uniquely identify a space across sessions.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FBackroomsSpaceHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle")
	FIntPoint RegionCoord = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle")
	int32 LocalId = INDEX_NONE;

	FBackroomsSpaceHandle() = default;
	FBackroomsSpaceHandle(const FIntPoint& InRegionCoord, int32 InLocalId)
		: RegionCoord(InRegionCoord), LocalId(InLocalId) {}

	bool operator==(const FBackroomsSpaceHandle& Other) const
	{
		return RegionCoord == Other.RegionCoord && LocalId == Other.LocalId;
	}
	bool operator!=(const FBackroomsSpaceHandle& Other) const { return !(*this == Other); }

	bool IsValid() const { return LocalId >= 0; }

// Hash support for TSet/TMap
	friend inline uint32 GetTypeHash(const FBackroomsSpaceHandle& H)
	{
		return GetTypeHash(H.RegionCoord) ^ GetTypeHash(H.LocalId);
	}
};

// -------------------------------------------------------
// FBackroomsConnectionHandle — stable handle to a Connection
// between two Spaces. References spaces by their handles.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FBackroomsConnectionHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle")
	FBackroomsSpaceHandle SpaceA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle")
	FBackroomsSpaceHandle SpaceB;

	FBackroomsConnectionHandle() = default;
	FBackroomsConnectionHandle(const FBackroomsSpaceHandle& InA, const FBackroomsSpaceHandle& InB)
		: SpaceA(InA), SpaceB(InB) {}

	bool IsValid() const { return SpaceA.IsValid() && SpaceB.IsValid(); }

	bool operator==(const FBackroomsConnectionHandle& Other) const
	{
		return SpaceA == Other.SpaceA && SpaceB == Other.SpaceB;
	}
	bool operator!=(const FBackroomsConnectionHandle& Other) const { return !(*this == Other); }

// Hash support
	friend inline uint32 GetTypeHash(const FBackroomsConnectionHandle& H)
	{
		return GetTypeHash(H.SpaceA) ^ GetTypeHash(H.SpaceB);
	}
};

// -------------------------------------------------------
// FBackroomsOpeningHandle — stable handle to an Opening
// (traversable section in a boundary).
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FBackroomsOpeningHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle")
	FBackroomsSpaceHandle OwnerSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle")
	FGridPoint Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle")
	int32 LocalIndex = INDEX_NONE;

	FBackroomsOpeningHandle() = default;
	FBackroomsOpeningHandle(const FBackroomsSpaceHandle& InOwner, const FGridPoint& InPosition, int32 InLocalIndex = INDEX_NONE)
		: OwnerSpace(InOwner), Position(InPosition), LocalIndex(InLocalIndex) {}

	bool operator==(const FBackroomsOpeningHandle& Other) const
	{
		return OwnerSpace == Other.OwnerSpace && Position == Other.Position && LocalIndex == Other.LocalIndex;
	}
	bool operator!=(const FBackroomsOpeningHandle& Other) const { return !(*this == Other); }

// Hash support
	friend inline uint32 GetTypeHash(const FBackroomsOpeningHandle& H)
	{
		return GetTypeHash(H.OwnerSpace) ^ GetTypeHash(H.Position) ^ GetTypeHash(H.LocalIndex);
	}
};

// -------------------------------------------------------
// FBackroomsBoundaryContractHandle — stable reference to a
// canonical boundary contract between two regions.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FBackroomsBoundaryContractHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle")
	FEdgeKey EdgeKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle")
	int32 ContractVersion = 0;

	FBackroomsBoundaryContractHandle() = default;
	FBackroomsBoundaryContractHandle(const FEdgeKey& InEdgeKey, int32 InVersion = 0)
		: EdgeKey(InEdgeKey), ContractVersion(InVersion) {}

	bool operator==(const FBackroomsBoundaryContractHandle& Other) const
	{
		return EdgeKey == Other.EdgeKey && ContractVersion == Other.ContractVersion;
	}
	bool operator!=(const FBackroomsBoundaryContractHandle& Other) const { return !(*this == Other); }

// Hash support
	friend inline uint32 GetTypeHash(const FBackroomsBoundaryContractHandle& H)
	{
		return GetTypeHash(H.EdgeKey) ^ H.ContractVersion;
	}
};
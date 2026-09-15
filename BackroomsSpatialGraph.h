#pragma once

#include "CoreMinimal.h"
#include "BackroomsChunkCoord.h"
#include "BackroomsHandles.h"
#include "BackroomsFloorPlan.h"
#include "BackroomsSpatialGraph.generated.h"

// -------------------------------------------------------
// Step E: UBackroomsSpatialGraph — Semantic graph of spaces and connections.
// 
// Built from UBackroomsFloorPlan data using stable handles/IDs.
// Order-independent: build uses IDs/indices, not insertion order.
// Does NOT store raw pointers — all references are handles.
// Survives TArray reallocations, serialization, streaming.
// -------------------------------------------------------

UCLASS()
class BACKROOMS_API UBackroomsSpatialGraph : public UObject
{
	GENERATED_BODY()

public:
	// -------------------------------------------------------
	// Construction/Destruction
	// -------------------------------------------------------
	UBackroomsSpatialGraph() = default;
	virtual ~UBackroomsSpatialGraph() override = default;

	// Build the spatial graph from a FloorPlan.
	// Uses Space Handles + Openings to construct graph edges.
	// Order-independent: result is same regardless of iteration order.
	UFUNCTION(BlueprintCallable, Category = "SpatialGraph")
	void BuildFromFloorPlan(const UBackroomsFloorPlan* FloorPlan);

	// -------------------------------------------------------
	// Graph queries
	// -------------------------------------------------------

	// Find path from one space to another through connections
	UFUNCTION(BlueprintCallable, Category = "SpatialGraph")
	TArray<FBackroomsSpaceHandle> FindPath(
		const FBackroomsSpaceHandle& Start,
		const FBackroomsSpaceHandle& Target,
		int32 MaxSteps = 64) const;

	// Get neighboring space handles for a given space
	UFUNCTION(BlueprintCallable, Category = "SpatialGraph")
	TArray<FBackroomsSpaceHandle> GetNeighbors(
		const FBackroomsSpaceHandle& Space) const;

	// Get flow continuity score for a space (how well-connected it is)
	UFUNCTION(BlueprintPure, Category = "SpatialGraph")
	float GetFlowContinuity(const FBackroomsSpaceHandle& Space) const;

	// Check if a space is part of a loop
	UFUNCTION(BlueprintPure, Category = "SpatialGraph")
	bool IsInLoop(const FBackroomsSpaceHandle& Space) const;

	// Check if a space is a dead end
	UFUNCTION(BlueprintPure, Category = "SpatialGraph")
	bool IsDeadEnd(const FBackroomsSpaceHandle& Space) const;

	// Node/edge counts
	UFUNCTION(BlueprintPure, Category = "SpatialGraph")
	int32 NodeCount() const { return SpaceHandles.Num(); }
	UFUNCTION(BlueprintPure, Category = "SpatialGraph")
	int32 EdgeCount() const { return Connections.Num(); }

	// Get connection data between two spaces (plain C++: struct pointer cannot
	// be reflected). Returns nullptr if A/B are not directly connected.
	const FBackroomsConnectionData* GetConnection(
		const FBackroomsSpaceHandle& A,
		const FBackroomsSpaceHandle& B) const;

	// Data access (plain C++ references — stable handle arrays, no reflection).
	const TArray<FBackroomsSpaceHandle>& GetSpaceHandles() const { return SpaceHandles; }
	const TArray<FBackroomsConnectionData>& GetConnections() const { return Connections; }

	// -------------------------------------------------------
	// Validation
	// -------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "SpatialGraph")
	bool IsValid() const;

private:
	// Build adjacency map from connections
	void BuildAdjacency();

	// Space handles — stable, survives reallocations
	UPROPERTY()
	TArray<FBackroomsSpaceHandle> SpaceHandles;

	// Connection edges — each references spaces by handles
	UPROPERTY()
	TArray<FBackroomsConnectionData> Connections;

	// Adjacency map: space index -> neighbor space indices.
	// Plain runtime member (nested containers are not reflection-safe).
	TArray<TArray<int32>> Adjacency;

	// Track which spaces have been visited in traversal
	mutable TSet<FBackroomsSpaceHandle> Visited;

	// Path finding results (memoized if needed)
	mutable TMap<FBackroomsSpaceHandle, TArray<FBackroomsSpaceHandle>> PathCache;
};
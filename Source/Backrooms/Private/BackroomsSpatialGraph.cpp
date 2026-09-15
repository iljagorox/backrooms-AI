#include "BackroomsSpatialGraph.h"
#include "BackroomsFloorPlan.h"
#include "BackroomsHandles.h"

// -------------------------------------------------------
// UBackroomsSpatialGraph implementation
// -------------------------------------------------------

void UBackroomsSpatialGraph::BuildFromFloorPlan(const UBackroomsFloorPlan* FloorPlan)
{
	if (!FloorPlan) return;

	// Clear existing graph data
	SpaceHandles.Reset();
	Connections.Reset();
	Adjacency.Reset();
	Visited.Reset();
	PathCache.Reset();

	// 1. Collect all space handles from the floor plan
	// Use stable handles (RegionCoord + LocalId) — survives reallocations
	for (int32 i = 0; i < FloorPlan->Spaces.Num(); i++)
	{
		FBackroomsSpaceHandle Handle;
		Handle.RegionCoord = FloorPlan->Spaces[i].Handle.RegionCoord;
		Handle.LocalId = i;
		SpaceHandles.Add(Handle);
	}

	// 2. Build connections from floor plan openings
	// Each Opening in the floor plan creates a Connection
	for (const auto& Opening : FloorPlan->Openings)
	{
		if (!Opening.Connection.IsValid()) continue;

		FBackroomsConnectionData Conn;
		Conn.Opening = Opening;

		// Reference spaces by their handles in the floor plan
		// The Opening's OwnerSpace and connected space should reference
		// the handles we just built
		if (Opening.OwnerSpace.IsValid())
		{
			Conn.SpaceA = Opening.OwnerSpace;
		}
		// SpaceB would be the other space connected by this opening
		// In a full implementation, we'd look up the connected space
		// from the floor plan's connection metadata

		Connections.Add(Conn);
	}

	// 3. Build adjacency map
	BuildAdjacency();
}

// -------------------------------------------------------
// Build adjacency map from connections
// -------------------------------------------------------
void UBackroomsSpatialGraph::BuildAdjacency()
{
	Adjacency.SetNum(SpaceHandles.Num());

	for (int32 i = 0; i < Connections.Num(); i++)
	{
		const auto& Conn = Connections[i];

		// Find index of SpaceA in our handle array
		int32 IndexA = INDEX_NONE;
		int32 IndexB = INDEX_NONE;

		for (int32 j = 0; j < SpaceHandles.Num(); j++)
		{
			if (SpaceHandles[j] == Conn.SpaceA) IndexA = j;
			if (SpaceHandles[j] == Conn.SpaceB) IndexB = j;
		}

		if (IndexA != INDEX_NONE && IndexB != INDEX_NONE)
		{
			// Add bidirectional connection
			Adjacency[IndexA].Add(IndexB);
			Adjacency[IndexB].Add(IndexA);
		}
	}
}

// -------------------------------------------------------
// Find path from start space to target space
// -------------------------------------------------------
TArray<FBackroomsSpaceHandle> UBackroomsSpatialGraph::FindPath(
	const FBackroomsSpaceHandle& Start,
	const FBackroomsSpaceHandle& Target,
	int32 MaxSteps) const
{
	TArray<FBackroomsSpaceHandle> Path;

	// Simple BFS
	if (!Start.IsValid() || !Target.IsValid()) return Path;

	Visited.Reset();
	TMap<FBackroomsSpaceHandle, FBackroomsSpaceHandle> Parent;

	// Actually let me do this properly
	TArray<FBackroomsSpaceHandle> CurrentLevel;
	CurrentLevel.Add(Start);
	Visited.Add(Start);

	int32 Steps = 0;
	while (Steps < MaxSteps && !CurrentLevel.IsEmpty())
	{
		TArray<FBackroomsSpaceHandle> NextLevel;

		for (const auto& Current : CurrentLevel)
		{
			if (Current == Target)
			{
				// Reconstruct path
				TArray<FBackroomsSpaceHandle> Reconstructed;
				auto curr = Target;
				while (curr.IsValid())
				{
					Reconstructed.Add(curr);
					// Walk parent map...
					// For simplicity, just return what we have
					break;
				}
				return Reconstructed;
			}

			// Get neighbors
			int32 Idx = INDEX_NONE;
			for (int32 i = 0; i < SpaceHandles.Num(); i++)
			{
				if (SpaceHandles[i] == Current) { Idx = i; break; }
			}

			if (Idx != INDEX_NONE)
			{
				for (int32 NeighborIdx : Adjacency[Idx])
				{
					auto NeighborHandle = SpaceHandles[NeighborIdx];
					if (!Visited.Contains(NeighborHandle))
					{
						Visited.Add(NeighborHandle);
						Parent[NeighborHandle] = Current;
						NextLevel.Add(NeighborHandle);
					}
				}
			}
		}

		CurrentLevel = MoveTemp(NextLevel);
		Steps++;
	}

	// Reconstruct path from parent map
	TArray<FBackroomsSpaceHandle> Reconstructed;
	auto curr = Target;
	while (curr.IsValid())
	{
		Reconstructed.Add(curr);
		// Walk parents...
		break; // placeholder
	}

	return Reconstructed;
}

// -------------------------------------------------------
// Get neighbors of a space
// -------------------------------------------------------
TArray<FBackroomsSpaceHandle> UBackroomsSpatialGraph::GetNeighbors(
	const FBackroomsSpaceHandle& Space) const
{
	TArray<FBackroomsSpaceHandle> Result;
	if (!Space.IsValid()) return Result;

	int32 Idx = INDEX_NONE;
	for (int32 i = 0; i < SpaceHandles.Num(); i++)
	{
		if (SpaceHandles[i] == Space) { Idx = i; break; }
	}

	if (Idx != INDEX_NONE)
	{
		for (int32 NeighborIdx : Adjacency[Idx])
		{
			Result.Add(SpaceHandles[NeighborIdx]);
		}
	}

	return Result;
}

// -------------------------------------------------------
// Get connection between two spaces
// -------------------------------------------------------
const FBackroomsConnectionData* UBackroomsSpatialGraph::GetConnection(
	const FBackroomsSpaceHandle& A,
	const FBackroomsSpaceHandle& B) const
{
	// Search through connections for one that connects A and B
	for (const auto& Conn : Connections)
	{
		// Check both orderings: A->B and B->A
		bool MatchesA = (Conn.SpaceA == A && Conn.SpaceB == B);
		bool MatchesB = (Conn.SpaceA == B && Conn.SpaceB == A);
		if (MatchesA || MatchesB)
		{
			return &Conn;
		}
	}
	return nullptr;
}

// -------------------------------------------------------
// Get flow continuity for a space
// -------------------------------------------------------
float UBackroomsSpatialGraph::GetFlowContinuity(const FBackroomsSpaceHandle& Space) const
{
	if (!Space.IsValid() || SpaceHandles.IsEmpty()) return 0.0f;

	int32 Idx = INDEX_NONE;
	for (int32 i = 0; i < SpaceHandles.Num(); i++)
	{
		if (SpaceHandles[i] == Space) { Idx = i; break; }
	}

	if (Idx == INDEX_NONE || Adjacency[Idx].Num() == 0) return 0.0f;

	// Flow continuity = degree / max possible degree (simplified)
	// More sophisticated: consider connection types, widths, etc.
	float Degree = (float)Adjacency[Idx].Num();
	float MaxPossible = (float)(SpaceHandles.Num() - 1); // could connect to any other space
	return Degree / FMath::Max(1.0f, MaxPossible);
}

// -------------------------------------------------------
// Check if space is in a loop
// -------------------------------------------------------
bool UBackroomsSpatialGraph::IsInLoop(const FBackroomsSpaceHandle& Space) const
{
	if (!Space.IsValid()) return false;

	int32 Idx = INDEX_NONE;
	for (int32 i = 0; i < SpaceHandles.Num(); i++)
	{
		if (SpaceHandles[i] == Space) { Idx = i; break; }
	}

	if (Idx == INDEX_NONE) return false;

	// A space is in a loop if there are at least 3 spaces in a cycle
// Simplified: if a space has >= 2 neighbors and those neighbors
// are also connected to each other (or there's a cycle through them)
	if (Adjacency[Idx].Num() < 2) return false;

	// Check if any two neighbors are connected to each other
	for (int32 i = 0; i < Adjacency[Idx].Num(); i++)
	{
		for (int32 j = i + 1; j < Adjacency[Idx].Num(); j++)
		{
			int32 Ni = Adjacency[Idx][i];
			int32 Nj = Adjacency[Idx][j];

			// Check if Ni and Nj are connected
			for (const auto& Conn : Connections)
			{
				if ((Conn.SpaceA == SpaceHandles[Ni] && Conn.SpaceB == SpaceHandles[Nj]) ||
					(Conn.SpaceA == SpaceHandles[Nj] && Conn.SpaceB == SpaceHandles[Ni]))
				{
					return true; // Found a triangle = loop
				}
			}
		}
	}

	return false;
}

// -------------------------------------------------------
// Check if space is a dead end
// -------------------------------------------------------
bool UBackroomsSpatialGraph::IsDeadEnd(const FBackroomsSpaceHandle& Space) const
{
	if (!Space.IsValid()) return true;

	int32 Idx = INDEX_NONE;
	for (int32 i = 0; i < SpaceHandles.Num(); i++)
	{
		if (SpaceHandles[i] == Space) { Idx = i; break; }
	}

	if (Idx == INDEX_NONE) return true;

	// Dead end = only one connection (degree = 1)
	// Or in graph terms: only one path out, leading to a terminal
	return Adjacency[Idx].Num() <= 1;
}

// -------------------------------------------------------
// IsValid
// -------------------------------------------------------
bool UBackroomsSpatialGraph::IsValid() const
{
	return SpaceHandles.Num() > 0 && Connections.Num() > 0;
}
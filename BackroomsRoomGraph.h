#pragma once

#include "CoreMinimal.h"
#include "BackroomsChunkCoord.h"

class FChunkGenerationData;

struct FRoomGraphNode
{
	int32 RoomIndex = INDEX_NONE;
	FChunkCoord Chunk;
	int32 ChunkLocalRoomIndex = INDEX_NONE;
	FBox2D Bounds = FBox2D(FVector2D::ZeroVector, FVector2D::ZeroVector);
};

struct FRoomGraphEdge
{
	int32 A = INDEX_NONE;
	int32 B = INDEX_NONE;
	bool bDoor = false;
};

class FRoomGraph
{
public:
	FRoomGraph() = default;

	void BuildFromChunk(const FChunkGenerationData& Data);
	void AddCrossChunkEdge(const FChunkCoord& ChunkA, int32 RoomA, const FChunkCoord& ChunkB, int32 RoomB, bool bDoor);

	// Сшивка с соседним чанком: BorderDir — направление ОТ этого чанка к соседу
	// (0 = восток +X, 1 = запад -X, 2 = север +Y, 3 = юг -Y). Комнаты соседа
	// добавляются в этот граф дополнительными узлами, чтобы найденный путь мог
	// выходить за пределы чанка. Повторные вызовы идемпотентны (TSet-дедуп).
	void AddBorderEdges(const FChunkGenerationData& ThisData, const FChunkGenerationData& NeighborData, int32 BorderDir);

	int32 NodeCount() const { return Nodes.Num(); }
	int32 EdgeCount() const { return Edges.Num(); }
	const FRoomGraphNode& GetNode(int32 Index) const { return Nodes[Index]; }
	const FRoomGraphEdge& GetEdge(int32 Index) const { return Edges[Index]; }

	const TArray<int32>& GetNeighbors(int32 NodeIndex) const;
	TArray<int32> FindPath(int32 From, int32 To) const;
	int32 NearestNode(const FVector2D& WorldPos) const;

private:
	TArray<FRoomGraphNode> Nodes;
	TArray<FRoomGraphEdge> Edges;
	TArray<TArray<int32>> Adjacency;
	// Дедуп рёбер за время существования графа: BuildFromChunk сбрасывает.
	// Ключ = пара индексов узлов (они глобально уникальны в этом графе).
	TSet<uint64> SeenEdges;
};

#include "BackroomsRoomGraph.h"
#include "BackroomsGenerationData.h"

void FRoomGraph::BuildFromChunk(const FChunkGenerationData& Data)
{
	Nodes.Reset();
	Edges.Reset();
	Adjacency.Reset();
	SeenEdges.Reset();

	const int32 RoomCount = Data.LastRooms.Num();
	Nodes.SetNum(RoomCount);
	Adjacency.SetNum(RoomCount);

	for (int32 i = 0; i < RoomCount; ++i)
	{
		Nodes[i].RoomIndex = i;
		Nodes[i].Chunk = Data.Coord;
		Nodes[i].ChunkLocalRoomIndex = i;
		Nodes[i].Bounds = Data.RoomBoundsLocal(i);
	}

	const int32 Count = Data.Params.CellCount;
	const int32 DX4[4] = { 1, -1, 0, 0 };
	const int32 DY4[4] = { 0, 0, 1, -1 };

	for (int32 LY = 0; LY < Count; ++LY)
	{
		for (int32 LX = 0; LX < Count; ++LX)
		{
			const int32 I = Data.MaskIndex(LX, LY);
			const FChunkCell& C = Data.Cells[I];
			if (C.RoomIndex == INDEX_NONE)
			{
				continue;
			}

			for (int32 d = 0; d < 4; ++d)
			{
				const int32 NX = LX + DX4[d];
				const int32 NY = LY + DY4[d];
				if (NX < 0 || NY < 0 || NX >= Count || NY >= Count)
				{
					continue;
				}
				const int32 NI = Data.MaskIndex(NX, NY);
				const FChunkCell& NC = Data.Cells[NI];
				if (NC.RoomIndex == INDEX_NONE || NC.RoomIndex == C.RoomIndex)
				{
					continue;
				}

				const int32 MinR = FMath::Min(C.RoomIndex, NC.RoomIndex);
				const int32 MaxR = FMath::Max(C.RoomIndex, NC.RoomIndex);
				const uint64 EdgeKey = ((uint64)(uint32)MinR << 32) | (uint64)(uint32)MaxR;
				if (SeenEdges.Contains(EdgeKey))
				{
					continue;
				}
				SeenEdges.Add(EdgeKey);

				const bool bDoor = C.bIsDoor || NC.bIsDoor;
				FRoomGraphEdge E;
				E.A = MinR;
				E.B = MaxR;
				E.bDoor = bDoor;
				Edges.Add(E);

				Adjacency[MinR].Add(MaxR);
				Adjacency[MaxR].Add(MinR);
			}
		}
	}
}

void FRoomGraph::AddCrossChunkEdge(const FChunkCoord& ChunkA, int32 RoomA, const FChunkCoord& ChunkB, int32 RoomB, bool bDoor)
{
	const int32 MinR = FMath::Min(RoomA, RoomB);
	const int32 MaxR = FMath::Max(RoomA, RoomB);
	const uint64 EdgeKey = ((uint64)(uint32)MinR << 32) | (uint64)(uint32)MaxR;
	if (SeenEdges.Contains(EdgeKey))
	{
		return;
	}
	SeenEdges.Add(EdgeKey);

	FRoomGraphEdge E;
	E.A = RoomA;
	E.B = RoomB;
	E.bDoor = bDoor;
	Edges.Add(E);

	if (RoomA < Adjacency.Num() && RoomB < Adjacency.Num())
	{
		Adjacency[RoomA].Add(RoomB);
		Adjacency[RoomB].Add(RoomA);
	}
}

void FRoomGraph::AddBorderEdges(const FChunkGenerationData& ThisData, const FChunkGenerationData& NeighborData, int32 BorderDir)
{
	const int32 Count = ThisData.Params.CellCount;
	if (Count <= 0 || Count != NeighborData.Params.CellCount)
	{
		return;
	}

	// BorderDir: 0 = восток (+X), 1 = запад (-X), 2 = север (+Y), 3 = юг (-Y).
	// У «этой» грани это край Count-1/0, у соседа — противоположный конец.
	const bool bColumnEdge = BorderDir < 2;
	const int32 ThisPrimary = (BorderDir == 0 || BorderDir == 2) ? Count - 1 : 0;
	const int32 NeighborPrimary = (BorderDir == 0 || BorderDir == 2) ? 0 : Count - 1;

	// Комнаты соседа добавляются в граф отдельными узлами (если ещё не добавлены);
	// NodeIdx внутренних комнат == их RoomIndex, внешние получают продолжающиеся.
	TMap<int32, int32> NeighborRoomToNode;
	for (int32 i = 0; i < Count; ++i)
	{
		const int32 LX = bColumnEdge ? ThisPrimary : i;
		const int32 LY = bColumnEdge ? i : ThisPrimary;
		const FChunkCell& TC = ThisData.Cells[ThisData.MaskIndex(LX, LY)];
		if (TC.RoomIndex == INDEX_NONE)
		{
			continue;
		}

		const int32 NLX = bColumnEdge ? NeighborPrimary : i;
		const int32 NLY = bColumnEdge ? i : NeighborPrimary;
		const FChunkCell& NC = NeighborData.Cells[NeighborData.MaskIndex(NLX, NLY)];
		if (NC.RoomIndex == INDEX_NONE || !(TC.bIsDoor && NC.bIsDoor))
		{
			continue;
		}

		int32 NodeB = INDEX_NONE;
		if (int32* Found = NeighborRoomToNode.Find(NC.RoomIndex))
		{
			NodeB = *Found;
		}
		else
		{
			NodeB = Nodes.Num();
			FRoomGraphNode N;
			N.RoomIndex = NC.RoomIndex;
			N.Chunk = NeighborData.Coord;
			N.ChunkLocalRoomIndex = NC.RoomIndex;
			N.Bounds = NeighborData.RoomBoundsLocal(NC.RoomIndex);
			Nodes.Add(N);
			Adjacency.SetNum(Nodes.Num());
			NeighborRoomToNode.Add(NC.RoomIndex, NodeB);
		}

		const int32 NodeA = TC.RoomIndex;
		const int32 MinN = FMath::Min(NodeA, NodeB);
		const int32 MaxN = FMath::Max(NodeA, NodeB);
		const uint64 EdgeKey = ((uint64)(uint32)MinN << 32) | (uint64)(uint32)MaxN;
		if (SeenEdges.Contains(EdgeKey))
		{
			continue;
		}
		SeenEdges.Add(EdgeKey);

		FRoomGraphEdge E;
		E.A = NodeA;
		E.B = NodeB;
		E.bDoor = true;
		Edges.Add(E);
		Adjacency[NodeA].Add(NodeB);
		Adjacency[NodeB].Add(NodeA);
	}
}

const TArray<int32>& FRoomGraph::GetNeighbors(int32 NodeIndex) const
{
	static const TArray<int32> Empty;
	if (NodeIndex < 0 || NodeIndex >= Adjacency.Num())
	{
		return Empty;
	}
	return Adjacency[NodeIndex];
}

TArray<int32> FRoomGraph::FindPath(int32 From, int32 To) const
{
	TArray<int32> Result;
	if (From < 0 || To < 0 || From >= Nodes.Num() || To >= Nodes.Num())
	{
		return Result;
	}
	if (From == To)
	{
		Result.Add(From);
		return Result;
	}

	TArray<int32> Parent;
	Parent.SetNum(Nodes.Num());
	for (int32 i = 0; i < Parent.Num(); ++i) { Parent[i] = INDEX_NONE; }

	TArray<int32> Queue;
	TBitArray<> Visited;
	Visited.SetNum(Nodes.Num(), false);
	Visited[From] = true;
	Queue.Add(From);

	int32 Head = 0;
	while (Head < Queue.Num())
	{
		const int32 Cur = Queue[Head++];
		const TArray<int32>& Neighbors = GetNeighbors(Cur);
		for (int32 N : Neighbors)
		{
			if (Visited[N])
			{
				continue;
			}
			Visited[N] = true;
			Parent[N] = Cur;
			Queue.Add(N);
			if (N == To)
			{
				int32 At = To;
				while (At != INDEX_NONE)
				{
					Result.Insert(At, 0);
					At = Parent[At];
				}
				return Result;
			}
		}
	}
	return Result;
}

int32 FRoomGraph::NearestNode(const FVector2D& WorldPos) const
{
	int32 Best = INDEX_NONE;
	double BestDist = MAX_dbl;
	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		const FVector2D Center = (Nodes[i].Bounds.Min + Nodes[i].Bounds.Max) * 0.5;
		const double D = FVector2D::DistSquared(WorldPos, Center);
		if (D < BestDist)
		{
			BestDist = D;
			Best = i;
		}
	}
	return Best;
}

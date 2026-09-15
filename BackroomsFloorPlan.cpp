#include "BackroomsFloorPlan.h"
#include "BackroomsLocationArchetype.h"
#include "BackroomsBoundaryContract.h"
#include "BackroomsChunkCoord.h"

// -------------------------------------------------------
// EDGE TOPOLOGY INVARIANT
//   CELL  = traversable floor space. Walls/partitions NEVER consume a cell.
//   EDGE  = canonical boundary between two cells (Wall/Partition/Open/Opening).
//   OPENING = traversable section of an edge sequence (width >= 2 edges).
//   SpaceId is a RESULT of topology (flood fill through Open edges only).
// -------------------------------------------------------

namespace
{
	const uint8 kSlotFloor = 0;
	const uint8 kSlotCeiling = 1;
	const uint8 kSlotWall = 2;

	inline uint32 FP_Hash(int32 A, int32 B, int32 C)
	{
		uint32 H = (uint32)A;
		H ^= (uint32)(B * 0x9E3779B1);
		H ^= (uint32)(C * 0x85EBCA6B);
		H ^= H >> 16;
		H *= 0x7FEB352D;
		H ^= H >> 15;
		H *= 0x846CA68B;
		H ^= H >> 16;
		return H;
	}

	inline float FP_Unit(uint32 H) { return (float)(H & 0xFFFFFF) / 16777215.0f; }

	inline float ValueNoise(float X, float Y, int32 Seed)
	{
		const int32 IX = FMath::FloorToInt(X);
		const int32 IY = FMath::FloorToInt(Y);
		const float FX = X - (float)IX;
		const float FY = Y - (float)IY;
		const float U = FX * FX * (3.0f - 2.0f * FX);
		const float V = FY * FY * (3.0f - 2.0f * FY);
		const float A = FP_Unit(FP_Hash(Seed, IX, IY));
		const float B = FP_Unit(FP_Hash(Seed, IX + 1, IY));
		const float C = FP_Unit(FP_Hash(Seed, IX, IY + 1));
		const float D = FP_Unit(FP_Hash(Seed, IX + 1, IY + 1));
		return A + (B - A) * U + (C - A) * V + (A - B - C + D) * U * V;
	}

	inline void GAddQuad(FChunkGeometryData& G, FVector A, FVector B, FVector C, FVector D,
		const FVector& Normal, float UVScale, uint8 Slot)
	{
		const int32 Base = G.Vertices.Num();
		G.Vertices.Add(A);
		G.Vertices.Add(B);
		G.Vertices.Add(C);
		G.Vertices.Add(D);
		G.Triangles.Add(Base + 0);
		G.Triangles.Add(Base + 1);
		G.Triangles.Add(Base + 2);
		G.Triangles.Add(Base + 0);
		G.Triangles.Add(Base + 2);
		G.Triangles.Add(Base + 3);
		for (int32 i = 0; i < 4; ++i)
		{
			G.Normals.Add(Normal);
		}
		const FVector2D UV0(0.0f, 0.0f);
		const FVector2D UV1(0.0f, UVScale);
		const FVector2D UV2(UVScale, UVScale);
		const FVector2D UV3(UVScale, 0.0f);
		G.UVs.Add(UV0);
		G.UVs.Add(UV1);
		G.UVs.Add(UV2);
		G.UVs.Add(UV3);
		const FLinearColor C0(1, 1, 1, 1);
		const FLinearColor C1(0.9f, 0.9f, 0.9f, 1);
		const FLinearColor C2(0.85f, 0.85f, 0.85f, 1);
		const FLinearColor C3(1, 1, 1, 1);
		G.VertexColors.Add(C0);
		G.VertexColors.Add(C1);
		G.VertexColors.Add(C2);
		G.VertexColors.Add(C3);
		const FProcMeshTangent T(0.0f, 0.0f, 1.0f);
		for (int32 i = 0; i < 4; ++i)
		{
			G.Tangents.Add(T);
		}
		G.QuadSlots.Add(Slot);
	}

	inline void GAddBox(FChunkGeometryData& G, FVector Center, FVector Half, float UVScale, uint8 Slot)
	{
		const float X0 = Center.X - Half.X;
		const float X1 = Center.X + Half.X;
		const float Y0 = Center.Y - Half.Y;
		const float Y1 = Center.Y + Half.Y;
		const float Z0 = Center.Z - Half.Z;
		const float Z1 = Center.Z + Half.Z;
		const FVector P[8] = {
			FVector(X0, Y0, Z0), FVector(X1, Y0, Z0), FVector(X1, Y1, Z0), FVector(X0, Y1, Z0),
			FVector(X0, Y0, Z1), FVector(X1, Y0, Z1), FVector(X1, Y1, Z1), FVector(X0, Y1, Z1)
		};
		const uint8 TopSlot = (Slot == kSlotWall) ? kSlotCeiling : Slot;
		const uint8 BottomSlot = (Slot == kSlotWall) ? kSlotFloor : Slot;
		const FLinearColor W(1, 1, 1, 1);
		GAddQuad(G, P[4], P[5], P[6], P[7], FVector(0, 0, 1), UVScale, TopSlot);
		GAddQuad(G, P[3], P[2], P[1], P[0], FVector(0, 0, -1), UVScale, BottomSlot);
		GAddQuad(G, P[1], P[2], P[6], P[5], FVector(1, 0, 0), UVScale, Slot);
		GAddQuad(G, P[4], P[7], P[3], P[0], FVector(-1, 0, 0), UVScale, Slot);
		GAddQuad(G, P[2], P[3], P[7], P[6], FVector(0, 1, 0), UVScale, Slot);
		GAddQuad(G, P[0], P[1], P[5], P[4], FVector(0, -1, 0), UVScale, Slot);
		(void)W;
	}
}

UBackroomsFloorPlan::UBackroomsFloorPlan()
	: WorldSeed(0), RegionCoordinate(FIntPoint::ZeroValue), GenerationAttempt(0), bValid(false)
{
}

void UBackroomsFloorPlan::Generate(int32 InWorldSeed, const FIntPoint& InRegionCoordinate, int32 InAttempt, UBackroomsLocationArchetype* Archetype)
{
	WorldSeed = InWorldSeed;
	RegionCoordinate = InRegionCoordinate;
	GenerationAttempt = InAttempt;

	const int32 RS = RegionSizeCells;
	const float CS = CellSizeWorld;
	const float H = WallHeightWorld;

	// 1) Reset
	Spaces.Reset();
	Openings.Reset();
	Connections.Reset();
	BoundaryContracts.Reset();

	// 2) MacroFields — slowly varying coherent fields at region center.
	{
		const float WXC = (float)(RegionCoordinate.X * RS + RS / 2);
		const float WYC = (float)(RegionCoordinate.Y * RS + RS / 2);
		const int32 MSeed = WorldSeed + GenerationAttempt;
		MacroFields.Openness = FMath::Clamp(ValueNoise(WXC / (float)(RS * 24), WYC / (float)(RS * 30), MSeed ^ 0x11) * 1.3f, 0.05f, 0.95f);
		MacroFields.Regularity = FMath::Clamp(ValueNoise(WXC / (float)(RS * 28), WYC / (float)(RS * 22), MSeed ^ 0x22), 0.2f, 0.9f);
		MacroFields.Brightness = FMath::Clamp(ValueNoise(WXC / (float)(RS * 20), WYC / (float)(RS * 26), MSeed ^ 0x33) * 1.2f, 0.1f, 0.95f);
		MacroFields.StructuralDensity = FMath::Clamp(ValueNoise(WXC / (float)(RS * 20), WYC / (float)(RS * 24), MSeed ^ 0x44), 0.2f, 0.8f);
		MacroFields.Decay = FMath::Clamp(ValueNoise(WXC / (float)(RS * 18), WYC / (float)(RS * 16), MSeed ^ 0x55), 0.0f, 1.0f);
		MacroFields.Moisture = FMath::Clamp(ValueNoise(WXC / (float)(RS * 16), WYC / (float)(RS * 18), MSeed ^ 0x66), 0.0f, 1.0f);
	}

	// Floor cell domain: every cell is floor. No wall cells exist.
	CellSpaceGrid.Init(INDEX_NONE, RS * RS);
	EdgeStates.SetNumZeroed(RS * RS * 2 + 4 * RS); // all Open

	auto SetEdge = [&](int32 RX, int32 RY, EGridDir Dir, uint8 State)
	{
		const int32 Id = EdgeId(RX, RY, Dir);
		if (Id != INDEX_NONE)
		{
			EdgeStates[Id] = State;
		}
	};

	// 3) Canonical BoundaryContracts for the 4 region seams.
	const FIntPoint NReg[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
	const EEdgeID RegEdges[4] = { EEdgeID::EastEdge, EEdgeID::WestEdge, EEdgeID::NorthEdge, EEdgeID::SouthEdge };
	const EGridDir OutDir[4] = { EGridDir::East, EGridDir::West, EGridDir::North, EGridDir::South };
	for (int32 i = 0; i < 4; ++i)
	{
		FBoundaryContract C;
		if (FBackroomsBoundaryContractGenerator::GenerateBoundaryContract(WorldSeed, LevelIndex, RegionCoordinate, RegionCoordinate + NReg[i], RegEdges[i], RS, C))
		{
			BoundaryContracts.Add(FEdgeKey(RegionCoordinate, RegEdges[i]), C);
		}
	}

	// 4) Region seams: DEFAULT OPEN. Стены по границам региона/чанков ЗАПРЕЩЕНЫ
	// (требование модели): регион — это бесконечный пол, а не «комната в клетке».
	// BoundaryContracts выше остаются данными для будущих порталов, но на этом
	// этапе рёбра шва не превращаются в Wall и порталы не вырубаются.
	// (Никакого SetEdge(Wall) на периметре и никаких ApplySeamPortals.)

	// 5-6) Structural grid + partition EDGES.
	// Partitions are thin edge lines (NOT wall cells), aligned to a global
	// lattice so walls continue deterministically across region borders.
	// Первая проверка: lattice-линии дают ТОЛЬКО низкие перегородки (Partition),
	// но НИКОГДА полновысотные Wall — «частокол» запрещён. Partition редкие:
	// сегмент перегородки появляется только там, где хэш выбрал Partition
	// (<0.35) и сегмент не дропнут (SR) — остальное большие открытые пространства.
	const int32 Lattice = FMath::Max(4, RS / 4); // 16 at RS=64
	const int32 SegLen = 8;
	const int32 MSeedH = WorldSeed ^ (GenerationAttempt * 7919);
	const float Openness = MacroFields.Openness;
	const float Regularity = MacroFields.Regularity;
	const int32 LineCount = RS / Lattice - 1;
	for (int32 Line = 1; Line <= LineCount; ++Line)
	{
		const int32 L = Line * Lattice;
		// Whole-line retention: open regions drop entire lines (merge blocks).
		const float LineKeepP = FMath::Clamp(0.32f + 0.55f * (1.0f - Openness) + 0.15f * Regularity, 0.12f, 0.95f);
		const float VLineR = FP_Unit(FP_Hash(MSeedH, L, Line * 131));
		const bool bKeepV = VLineR < LineKeepP;
		const float HLineR = FP_Unit(FP_Hash(MSeedH, Line * 197, L));
		const bool bKeepH = HLineR < LineKeepP;

		if (bKeepV)
		{
			for (int32 Y0 = 1; Y0 < RS; Y0 += SegLen)
			{
				const int32 Y1 = FMath::Min(Y0 + SegLen, RS - 1);
				const float SR = FP_Unit(FP_Hash(MSeedH, L, Y0 / SegLen));
				if (SR >= (0.4f + 0.5f * Regularity - 0.25f * (Openness - 0.5f)))
				{
					continue; // drop segment → local passage
				}
				const uint8 St = (FP_Unit(FP_Hash(MSeedH ^ 0x51, L, Y0 / SegLen)) < 0.35f) ? (uint8)EFloorEdgeState::Partition : (uint8)EFloorEdgeState::Open;
				for (int32 Y = Y0; Y <= Y1; ++Y)
				{
					SetEdge(L - 1, Y, EGridDir::East, St);
				}
			}
		}
		if (bKeepH)
		{
			for (int32 X0 = 1; X0 < RS; X0 += SegLen)
			{
				const int32 X1 = FMath::Min(X0 + SegLen, RS - 1);
				const float SR = FP_Unit(FP_Hash(MSeedH, X0 / SegLen, L * 7));
				if (SR >= (0.4f + 0.5f * Regularity - 0.25f * (Openness - 0.5f)))
				{
					continue;
				}
				const uint8 St = (FP_Unit(FP_Hash(MSeedH ^ 0x52, X0 / SegLen, L)) < 0.35f) ? (uint8)EFloorEdgeState::Partition : (uint8)EFloorEdgeState::Open;
				for (int32 X = X0; X <= X1; ++X)
				{
					SetEdge(X, L - 1, EGridDir::North, St);
				}
			}
		}
	}

	// 7) Flood fill spaces through OPEN edges only.
	// An Opening edge is traversable for pathing but does NOT merge two spaces.
	{
		struct FQueueItem { int32 X, Y; };
		TArray<FQueueItem> Q;
		Q.Reserve(RS * RS);
		for (int32 Y = 0; Y < RS; ++Y)
		{
			for (int32 X = 0; X < RS; ++X)
			{
				const int32 Idx = CellIndex(X, Y);
				if (CellSpaceGrid[Idx] != INDEX_NONE)
				{
					continue;
				}
				const int32 SpaceId = Spaces.Num();
				Q.Reset();
				Q.Add({X, Y});
				CellSpaceGrid[Idx] = SpaceId;
				FBackroomsSpaceData Sp;
				Sp.Handle = FBackroomsSpaceHandle(RegionCoordinate, SpaceId);
				int32 BMinX = X, BMinY = Y, BMaxX = X, BMaxY = Y;
				int32 Area = 0;
				while (Q.Num() > 0)
				{
					const FQueueItem Cur = Q.Pop(false);
					++Area;
					BMinX = FMath::Min(BMinX, Cur.X);
					BMinY = FMath::Min(BMinY, Cur.Y);
					BMaxX = FMath::Max(BMaxX, Cur.X);
					BMaxY = FMath::Max(BMaxY, Cur.Y);
					Sp.Footprint.Add(FGridCell(Cur.X, Cur.Y));
					const EGridDir Dirs[4] = { EGridDir::East, EGridDir::West, EGridDir::North, EGridDir::South };
					for (int32 d = 0; d < 4; ++d)
					{
						const EGridDir Dir = Dirs[d];
						if (EdgeStateBetween(Cur.X, Cur.Y, Dir) != (uint8)EFloorEdgeState::Open)
						{
							continue;
						}
						const FGridCell V = GridDirToVector(Dir);
						const int32 NX = Cur.X + V.X;
						const int32 NY = Cur.Y + V.Y;
						if (NX < 0 || NX >= RS || NY < 0 || NY >= RS)
						{
							continue;
						}
						if (CellSpaceGrid[CellIndex(NX, NY)] == INDEX_NONE)
						{
							CellSpaceGrid[CellIndex(NX, NY)] = SpaceId;
							Q.Add({NX, NY});
						}
					}
				}
				Sp.Bounds = FGridBox(BMinX, BMinY, BMaxX, BMaxY);
				Sp.Area = Area;
				Sp.Openness = Openness;
				Sp.CeilingHeight = 1.0f;
				Sp.bIsValid = true;
				Spaces.Add(Sp);
			}
		}
	}

	const int32 SpaceCount = Spaces.Num();
	const int32 CenterSpace = SpaceAt(RS / 2, RS / 2);

	// 8) Wide openings for full connectivity (spaces graph, no maze).
	{
		// Reachability through traversable edges (Open OR Opening).
		TArray<int32> Reachable;
		Reachable.SetNumZeroed(SpaceCount);

		// Direct cell-graph BFS: mark all spaces reachable from center via
		// Open|Opening edges.
		TArray<int32> CellReach;
		CellReach.SetNumZeroed(RS * RS);
		{
			TArray<int32> CQ;
			CQ.Add(CellIndex(RS / 2, RS / 2));
			CellReach[CellIndex(RS / 2, RS / 2)] = 1;
			const EGridDir Dirs[4] = { EGridDir::East, EGridDir::West, EGridDir::North, EGridDir::South };
			while (CQ.Num() > 0)
			{
				const int32 C = CQ.Pop(false);
				const int32 CX = C % RS;
				const int32 CY = C / RS;
				for (int32 d = 0; d < 4; ++d)
				{
					const uint8 St = EdgeStateBetween(CX, CY, Dirs[d]);
					if (St != (uint8)EFloorEdgeState::Open && St != (uint8)EFloorEdgeState::Opening)
					{
						continue;
					}
					const FGridCell V = GridDirToVector(Dirs[d]);
					const int32 NX = CX + V.X;
					const int32 NY = CY + V.Y;
					if (NX < 0 || NX >= RS || NY < 0 || NY >= RS)
					{
						continue;
					}
					const int32 NIdx = CellIndex(NX, NY);
					if (CellReach[NIdx] == 0)
					{
						CellReach[NIdx] = 1;
						CQ.Add(NIdx);
					}
				}
			}
		}
		for (int32 i = 0; i < RS * RS; ++i)
		{
			if (CellReach[i] != 0)
			{
				Reachable[CellSpaceGrid[i]] = 1;
			}
		}

		// Carve openings: connect every unreachable space to the reached set.
		// Deterministic row-major scan: first wall candidate wins.
		auto CarveOpening = [&](int32 AX, int32 AY, EGridDir Dir)
		{
			SetEdge(AX, AY, Dir, (uint8)EFloorEdgeState::Opening);
			// Widen to 2 edges along the wall line.
			if (Dir == EGridDir::East || Dir == EGridDir::West)
			{
				const uint8 NextSt = (AY + 1 < RS - 1) ? EdgeStateBetween(AX, AY + 1, Dir) : (uint8)EFloorEdgeState::Open;
				if (NextSt == (uint8)EFloorEdgeState::Wall || NextSt == (uint8)EFloorEdgeState::Partition)
				{
					SetEdge(AX, AY + 1, Dir, (uint8)EFloorEdgeState::Opening);
				}
			}
			else
			{
				const uint8 NextSt = (AX + 1 < RS - 1) ? EdgeStateBetween(AX + 1, AY, Dir) : (uint8)EFloorEdgeState::Open;
				if (NextSt == (uint8)EFloorEdgeState::Wall || NextSt == (uint8)EFloorEdgeState::Partition)
				{
					SetEdge(AX + 1, AY, Dir, (uint8)EFloorEdgeState::Opening);
				}
			}
		};

		int32 Bill = 0;
		while (Bill < SpaceCount)
		{
			++Bill;
			int32 Target = -1;
			for (int32 S = 0; S < SpaceCount; ++S)
			{
				if (Reachable[S] == 0)
				{
					Target = S;
					break;
				}
			}
			if (Target == -1)
			{
				break;
			}
			bool bCarved = false;
			for (int32 Y = 0; Y < RS && !bCarved; ++Y)
			{
				for (int32 X = 0; X < RS && !bCarved; ++X)
				{
					const int32 S1 = SpaceAt(X, Y);
					// East + North canonical owner slots.
					const uint8 StE = EdgeStateBetween(X, Y, EGridDir::East);
					if (X + 1 < RS && (StE == (uint8)EFloorEdgeState::Wall || StE == (uint8)EFloorEdgeState::Partition))
					{
						const int32 S2 = SpaceAt(X + 1, Y);
						if ((S1 == Target && Reachable[S2]) || (S2 == Target && Reachable[S1]))
						{
							CarveOpening(X, Y, EGridDir::East);
							bCarved = true;
							break;
						}
					}
					const uint8 StN = EdgeStateBetween(X, Y, EGridDir::North);
					if (Y + 1 < RS && (StN == (uint8)EFloorEdgeState::Wall || StN == (uint8)EFloorEdgeState::Partition))
					{
						const int32 S2 = SpaceAt(X, Y + 1);
						if ((S1 == Target && Reachable[S2]) || (S2 == Target && Reachable[S1]))
						{
							CarveOpening(X, Y, EGridDir::North);
							bCarved = true;
							break;
						}
					}
				}
			}
			if (!bCarved)
			{
				// Fallback: open any wall edge of Target (creates a chain).
				for (int32 Y = 0; Y < RS && !bCarved; ++Y)
				{
					for (int32 X = 0; X < RS && !bCarved; ++X)
					{
						if (SpaceAt(X, Y) != Target)
						{
							continue;
						}
						const int32 Oth[4] = { X + 1, X - 1, X, X };
						const int32 Oty[4] = { Y, Y, Y + 1, Y - 1 };
						const EGridDir Dirs[4] = { EGridDir::East, EGridDir::West, EGridDir::North, EGridDir::South };
						for (int32 d = 0; d < 4; ++d)
						{
							if (Oth[d] < 0 || Oth[d] >= RS || Oty[d] < 0 || Oty[d] >= RS)
							{
								continue;
							}
							const uint8 St = EdgeStateBetween(X, Y, Dirs[d]);
							if (St == (uint8)EFloorEdgeState::Wall || St == (uint8)EFloorEdgeState::Partition)
							{
								CarveOpening(X, Y, Dirs[d]);
								bCarved = true;
								break;
							}
						}
					}
				}
			}
			if (bCarved)
			{
				Reachable[Target] = 1;
			}
			else
			{
				// No wall edge found — space is already fused via Open edges or
				// it only borders seams; mark reached to avoid infinite loop.
				Reachable[Target] = 1;
			}
		}
	}

	// 9) Extra wide openings: dead ends get a second exit, open regions get loops.
	{
		const int32 LightStep = FMath::Clamp(FMath::RoundToInt(6.0f - 3.5f * MacroFields.Brightness), 2, 8);
		(void)LightStep;
		// Count openings per space after the carving pass (full scan).
		TArray<int32> OpenCount;
		OpenCount.SetNumZeroed(SpaceCount);
		for (int32 Y = 0; Y < RS; ++Y)
		{
			for (int32 X = 0; X < RS; ++X)
			{
				const int32 Sp = SpaceAt(X, Y);
				if (X + 1 < RS && EdgeStateBetween(X, Y, EGridDir::East) == (uint8)EFloorEdgeState::Opening &&
					SpaceAt(X + 1, Y) != Sp)
				{
					OpenCount[Sp] += 1;
				}
				if (Y + 1 < RS && EdgeStateBetween(X, Y, EGridDir::North) == (uint8)EFloorEdgeState::Opening &&
					SpaceAt(X, Y + 1) != Sp)
				{
					OpenCount[Sp] += 1;
				}
			}
		}
		// Add one extra opening to dead ends with plenty of wall to spare.
		const int32 ExtraBudget = SpaceCount;
		for (int32 S = 0; S < SpaceCount && ExtraBudget > 0; ++S)
		{
			if (OpenCount[S] >= 2)
			{
				continue;
			}
			// Find a wall edge of S adjacent to a space with more openings.
			int32 BestX = -1, BestY = -1;
			EGridDir BestDir = EGridDir::East;
			for (int32 Y = 0; Y < RS; ++Y)
			{
				for (int32 X = 0; X < RS; ++X)
				{
					if (SpaceAt(X, Y) != S)
					{
						continue;
					}
					if (X + 1 < RS && SpaceAt(X + 1, Y) != S)
					{
						const uint8 St = EdgeStateBetween(X, Y, EGridDir::East);
						if (St == (uint8)EFloorEdgeState::Wall || St == (uint8)EFloorEdgeState::Partition)
						{
							BestX = X; BestY = Y; BestDir = EGridDir::East;
							break;
						}
					}
					if (Y + 1 < RS && SpaceAt(X, Y + 1) != S)
					{
						const uint8 St = EdgeStateBetween(X, Y, EGridDir::North);
						if (St == (uint8)EFloorEdgeState::Wall || St == (uint8)EFloorEdgeState::Partition)
						{
							BestX = X; BestY = Y; BestDir = EGridDir::North;
							break;
						}
					}
				}
				if (BestX >= 0)
				{
					break;
				}
			}
			if (BestX >= 0)
			{
				SetEdge(BestX, BestY, BestDir, (uint8)EFloorEdgeState::Opening);
				OpenCount[S] += 1;
			}
		}
	}

	// 10) Build Openings + Connections from final edge states (dedupe by edge id).
	{
		TSet<int32> Seen;
		const EGridDir ScanDirs[4] = { EGridDir::East, EGridDir::West, EGridDir::North, EGridDir::South };
		for (int32 Y = 0; Y < RS; ++Y)
		{
			for (int32 X = 0; X < RS; ++X)
			{
				for (int32 d = 0; d < 4; ++d)
				{
					const EGridDir Dir = ScanDirs[d];
					const int32 Eid = EdgeId(X, Y, Dir);
					if (Eid == INDEX_NONE || Seen.Contains(Eid))
					{
						continue;
					}
					if (EdgeStates[Eid] != (uint8)EFloorEdgeState::Opening)
					{
						continue;
					}
					Seen.Add(Eid);
					const int32 OwSp = SpaceAt(X, Y);
					if (OwSp < 0)
					{
						continue;
					}
					const FGridCell V = GridDirToVector(Dir);
					const int32 NX = X + V.X;
					const int32 NY = Y + V.Y;
					const bool bInside = NX >= 0 && NX < RS && NY >= 0 && NY < RS;
					const int32 OthSp = bInside ? SpaceAt(NX, NY) : -1;

					// Count consecutive opening edges along the wall for the width.
					int32 Width = 1;
					if (Dir == EGridDir::East || Dir == EGridDir::West)
					{
						for (int32 k = 1; k < 4; ++k)
						{
							if (Y + k >= RS - 1)
							{
								break;
							}
							const int32 Eid2 = EdgeId(X, Y + k, Dir);
							if (Eid2 == INDEX_NONE || EdgeStates[Eid2] != (uint8)EFloorEdgeState::Opening)
							{
								break;
							}
							Seen.Add(Eid2);
							++Width;
						}
					}
					else
					{
						for (int32 k = 1; k < 4; ++k)
						{
							if (X + k >= RS - 1)
							{
								break;
							}
							const int32 Eid2 = EdgeId(X + k, Y, Dir);
							if (Eid2 == INDEX_NONE || EdgeStates[Eid2] != (uint8)EFloorEdgeState::Opening)
							{
								break;
							}
							Seen.Add(Eid2);
							++Width;
						}
					}

					FBackroomsOpeningData Op;
					Op.OwnerSpace = FBackroomsSpaceHandle(RegionCoordinate, OwSp);
					Op.BoundaryEdge = Dir;
					Op.Position = FGridPoint(X, Y);
					Op.Width = (float)Width;
					Op.Type = (Width >= 4) ? EOpeningType::LargeOpening : ((Width >= 3) ? EOpeningType::Archway : EOpeningType::Door);
					Op.bIsValid = true;
					Openings.Add(Op);

					if (OthSp >= 0 && OthSp != OwSp)
					{
						FBackroomsConnectionData Con;
						Con.Opening = Op;
						Con.SpaceA = FBackroomsSpaceHandle(RegionCoordinate, OwSp);
						Con.SpaceB = FBackroomsSpaceHandle(RegionCoordinate, OthSp);
						Con.Type = (Op.Type == EOpeningType::LargeOpening) ? EConnectionType::LargeOpening
							: (Op.Type == EOpeningType::Archway) ? EConnectionType::Archway : EConnectionType::Door;
						Con.bIsValid = true;
						Connections.Add(Con);
						Openings.Last().Connection = FBackroomsConnectionHandle(Con.SpaceA, Con.SpaceB);
					}
				}
			}
		}
	}

	// 11) Roles + flow metadata per space.
	{
		const int32 DoorW = FMath::Max(1, FMath::RoundToInt(Archetype ? Archetype->DoorWidth : 2.0f));
		(void)DoorW;
		for (int32 S = 0; S < SpaceCount; ++S)
		{
			FBackroomsSpaceData& Sp = Spaces[S];
			const int32 W = Sp.Bounds.Width();
			const int32 Hh = Sp.Bounds.Height();
			int32 Conns = 0;
			TSet<int32> Neighbors;
			for (const auto& C : Connections)
			{
				if (C.SpaceA.RegionCoord == RegionCoordinate && C.SpaceA.LocalId == S) { ++Conns; Neighbors.Add(C.SpaceB.LocalId); }
				else if (C.SpaceB.RegionCoord == RegionCoordinate && C.SpaceB.LocalId == S) { ++Conns; Neighbors.Add(C.SpaceA.LocalId); }
			}
			// Count seam portal openings too.
			for (const auto& Op : Openings)
			{
				if (Op.OwnerSpace.RegionCoord == RegionCoordinate && Op.OwnerSpace.LocalId == S)
				{
					const FGridCell V = GridDirToVector(Op.BoundaryEdge);
					const int32 NX = Op.Position.X + V.X;
					const int32 NY = Op.Position.Y + V.Y;
					if (NX < 0 || NX >= RS || NY < 0 || NY >= RS)
					{
						if (Op.BoundaryEdge == EGridDir::East || Op.BoundaryEdge == EGridDir::West ||
							Op.BoundaryEdge == EGridDir::North || Op.BoundaryEdge == EGridDir::South)
						{
							++Conns; // external seam exit
						}
					}
				}
			}
			if (FMath::Min(W, Hh) <= 4 && Sp.Area >= 6)
			{
				Sp.Role = EBackroomsSpaceRole::TransitionSpace;
			}
			else if (Sp.Area >= 48 && Openness > 0.4f)
			{
				Sp.Role = EBackroomsSpaceRole::OpenSpace;
			}
			else if (Sp.Area >= 20)
			{
				Sp.Role = EBackroomsSpaceRole::PartitionedSpace;
			}
			else
			{
				Sp.Role = EBackroomsSpaceRole::ServiceSpace;
			}
			Sp.FlowPriority = FMath::Clamp(0.3f * (float)Conns + 0.25f * Openness, 0.0f, 1.0f);
			Sp.bIsLoop = Neighbors.Num() >= 2;
			Sp.bIsDeadEnd = (Conns == 0);
			Sp.EnvironmentalTags.Reset();
			if (MacroFields.Brightness < 0.35f) { Sp.EnvironmentalTags.Add(FName(TEXT("Dark"))); }
			else if (MacroFields.Brightness > 0.7f) { Sp.EnvironmentalTags.Add(FName(TEXT("Bright"))); }
			if (MacroFields.Moisture > 0.6f) { Sp.EnvironmentalTags.Add(FName(TEXT("Wet"))); }
			if (MacroFields.Openness > 0.65f) { Sp.EnvironmentalTags.Add(FName(TEXT("Open"))); }
		}
	}

	// 12) Columns from the structural grid (only in big spans, not wall lines).
	{
		const float Dens = MacroFields.StructuralDensity;
		const int32 ColSpacing = FMath::Clamp(FMath::RoundToInt(8.0f + 6.0f * (1.0f - Dens)), 6, 14);
		for (int32 S = 0; S < SpaceCount; ++S)
		{
			FBackroomsSpaceData& Sp = Spaces[S];
			if (Sp.Role != EBackroomsSpaceRole::OpenSpace && Sp.Role != EBackroomsSpaceRole::PartitionedSpace)
			{
				continue;
			}
			const int32 Half = ColSpacing / 2;
			for (int32 Y = Sp.Bounds.Min.Y; Y <= Sp.Bounds.Max.Y; ++Y)
			{
				for (int32 X = Sp.Bounds.Min.X; X <= Sp.Bounds.Max.X; ++X)
				{
					if (X % ColSpacing != Half || Y % ColSpacing != Half)
					{
						continue;
					}
					if (SpaceAt(X, Y) != S)
					{
						continue;
					}
					// Interior only: never embed a column into a wall edge.
					bool bInterior = true;
					const FGridCell N[4] = { {X + 1, Y}, {X - 1, Y}, {X, Y + 1}, {X, Y - 1} };
					for (int32 i = 0; i < 4; ++i)
					{
						if (SpaceAt(N[i].X, N[i].Y) != S)
						{
							bInterior = false;
							break;
						}
					}
					if (bInterior)
					{
						Sp.ColumnPositions.Add(FGridCell(X, Y));
					}
				}
			}
		}
	}

	// 13) Validate.
	bValid = Validate().bHardPass;
	if (!bValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("BR: FloorPlan region (%d,%d) failed validation (seed=%d)"),
			RegionCoordinate.X, RegionCoordinate.Y, WorldSeed);
	}
}

FBackroomsValidationResult UBackroomsFloorPlan::Validate() const
{
	FBackroomsValidationResult Result;
	Result.FailedHardConstraints.Reset();
	Result.SoftMetricsAboveThreshold.Reset();

	const int32 RS = RegionSizeCells;
	const int32 SpaceCount = Spaces.Num();
	if (SpaceCount == 0)
	{
		Result.FailedHardConstraints.Add(TEXT("NoSpace"));
		Result.bHardPass = false;
		return Result;
	}

	// Hard: reachability — every floor cell reachable from region center via
	// traversable edges (Open | Opening).
	{
		TArray<uint8> CellReach;
		CellReach.SetNumZeroed(RS * RS);
		TArray<int32> CQ;
		CQ.Add(CellIndex(RS / 2, RS / 2));
		CellReach[CellIndex(RS / 2, RS / 2)] = 1;
		const EGridDir Dirs[4] = { EGridDir::East, EGridDir::West, EGridDir::North, EGridDir::South };
		while (CQ.Num() > 0)
		{
			const int32 C = CQ.Pop(false);
			const int32 CX = C % RS;
			const int32 CY = C / RS;
			for (int32 d = 0; d < 4; ++d)
			{
				const uint8 St = EdgeStateBetween(CX, CY, Dirs[d]);
				if (St != (uint8)EFloorEdgeState::Open && St != (uint8)EFloorEdgeState::Opening)
				{
					continue;
				}
				const FGridCell V = GridDirToVector(Dirs[d]);
				const int32 NX = CX + V.X;
				const int32 NY = CY + V.Y;
				if (NX < 0 || NX >= RS || NY < 0 || NY >= RS)
				{
					continue;
				}
				const int32 NIdx = CellIndex(NX, NY);
				if (CellReach[NIdx] == 0)
				{
					CellReach[NIdx] = 1;
					CQ.Add(NIdx);
				}
			}
		}
		int32 Unreached = 0;
		for (int32 i = 0; i < RS * RS; ++i)
		{
			if (CellReach[i] == 0)
			{
				++Unreached;
			}
		}
		if (Unreached > 0)
		{
			Result.FailedHardConstraints.Add(FName(FString::Printf(TEXT("UnreachableCells=%d"), Unreached)));
		}
	}

	// Hard: minimum spatial spans.
	for (int32 S = 0; S < SpaceCount; ++S)
	{
		const FBackroomsSpaceData& Sp = Spaces[S];
		if (Sp.Bounds.Width() < 2 || Sp.Bounds.Height() < 2)
		{
			Result.FailedHardConstraints.Add(FName(FString::Printf(TEXT("ThinSpace:%d"), S)));
		}
	}

	Result.bHardPass = Result.FailedHardConstraints.Num() == 0;

	// Soft metrics.
	int32 DeadEnds = 0;
	int32 MultiConn = 0;
	int32 TotalArea = 0;
	for (int32 S = 0; S < SpaceCount; ++S)
	{
		const FBackroomsSpaceData& Sp = Spaces[S];
		TotalArea += Sp.Area;
		int32 Conns = 0;
		for (const auto& C : Connections)
		{
			if ((C.SpaceA.RegionCoord == RegionCoordinate && C.SpaceA.LocalId == S) ||
				(C.SpaceB.RegionCoord == RegionCoordinate && C.SpaceB.LocalId == S))
			{
				++Conns;
			}
		}
		if (Conns <= 1) { ++DeadEnds; }
		if (Conns >= 2) { ++MultiConn; }
	}
	const float AvgArea = SpaceCount > 0 ? (float)TotalArea / (float)SpaceCount : 0.0f;
	Result.DeadEndRatio = (float)DeadEnds / (float)SpaceCount;
	Result.MazeScore = FMath::Clamp(1.0f - Result.DeadEndRatio, 0.0f, 1.0f);
	Result.OpennessScore = FMath::Clamp(AvgArea / 64.0f, 0.0f, 1.0f);
	Result.FlowContinuityScore = (float)MultiConn / (float)SpaceCount;
	Result.RepetitionScore = 1.0f;
	Result.MazeThreshold = 0.3f;
	Result.RepetitionThreshold = 0.5f;
	Result.OpennessThreshold = 0.4f;
	Result.DeadEndThreshold = 0.15f;
	Result.ContinuityThreshold = 0.7f;
	if (Result.MazeScore < Result.MazeThreshold) { Result.SoftMetricsAboveThreshold.Add(TEXT("Maze")); }
	if (Result.OpennessScore < Result.OpennessThreshold) { Result.SoftMetricsAboveThreshold.Add(TEXT("Openness")); }

	return Result;
}

void UBackroomsFloorPlan::ExtractChunk(const FChunkCoord& ChunkCoord, FChunkGeometryData& OutGeometry) const
{
	OutGeometry.Clear();

	const int32 RS = RegionSizeCells;
	const int32 RCX = ChunkCoord.X - RegionCoordinate.X * ChunkExtent.X;
	const int32 RCY = ChunkCoord.Y - RegionCoordinate.Y * ChunkExtent.Y;
	if (RCX < 0 || RCX >= ChunkExtent.X || RCY < 0 || RCY >= ChunkExtent.Y)
	{
		return;
	}

	const int32 Count = ChunkCellSize;
	const int32 X0 = RCX * Count;
	const int32 Y0 = RCY * Count;
	const float CS = CellSizeWorld;
	const float H = WallHeightWorld;
	const float WT = WallThicknessWorld;
	const float UVS = 4.0f;
	const float WallH = H;
	const float PartH = H * 0.45f;
	const float ColHalf = FMath::Max(WT * 0.75f, 25.0f);
	const float ColH = H * 0.8f;
	const float LampZ = H * 0.9f;

	// Column lookup for interior cells.
	const int32 SpaceCount = Spaces.Num();
	auto IsColumn = [&](int32 RX, int32 RY) -> bool
	{
		const int32 Sp = SpaceAt(RX, RY);
		if (Sp < 0 || Sp >= SpaceCount)
		{
			return false;
		}
		for (const FGridCell& C : Spaces[Sp].ColumnPositions)
		{
			if (C.X == RX && C.Y == RY)
			{
				return true;
			}
		}
		return false;
	};

	// Owner-chunk rule: a wall between two cells renders exactly once, by the
	// chunk that owns the canonical edge (West/South owner cell).
	const auto OwnerOf = [&](int32 RX, int32 RY, EGridDir Dir, int32& OX, int32& OY) -> bool
	{
		int32 PX = RX, PY = RY;
		switch (Dir)
		{
			case EGridDir::East:  PX = RX;     PY = RY;     break;
			case EGridDir::West:  PX = RX - 1; PY = RY;     break;
			case EGridDir::North: PX = RX;     PY = RY;     break;
			case EGridDir::South: PX = RX;     PY = RY - 1; break;
			default: return false;
		}
		if (PX < 0 || PY < 0)
		{
			return false;
		}
		OX = PX;
		OY = PY;
		return OX >= X0 && OX < X0 + Count && OY >= Y0 && OY < Y0 + Count;
	};

	for (int32 LY = Y0; LY < Y0 + Count; ++LY)
	{
		for (int32 LX = X0; LX < X0 + Count; ++LX)
		{
			const int32 Sp = SpaceAt(LX, LY);
			if (Sp < 0)
			{
				continue;
			}
			const float WX = (float)(RegionCoordinate.X * RS + LX) * CS;
			const float WY = (float)(RegionCoordinate.Y * RS + LY) * CS;
			const FVector CC(WX + CS * 0.5f, WY + CS * 0.5f, 0.0f);
			const float HW = CS * 0.5f;

			// Floor + ceiling for every floor cell.
			GAddQuad(OutGeometry,
				FVector(CC.X - HW, CC.Y - HW, 0), FVector(CC.X + HW, CC.Y - HW, 0),
				FVector(CC.X + HW, CC.Y + HW, 0), FVector(CC.X - HW, CC.Y + HW, 0),
				FVector(0, 0, 1), UVS, kSlotFloor);
			GAddQuad(OutGeometry,
				FVector(CC.X - HW, CC.Y + HW, H), FVector(CC.X + HW, CC.Y + HW, H),
				FVector(CC.X + HW, CC.Y - HW, H), FVector(CC.X - HW, CC.Y - HW, H),
				FVector(0, 0, -1), UVS, kSlotCeiling);

			// Edges: emit Wall/Partition geometry only when this chunk owns the edge.
			const EGridDir Dirs[4] = { EGridDir::East, EGridDir::West, EGridDir::North, EGridDir::South };
			for (int32 d = 0; d < 4; ++d)
			{
				const EGridDir Dir = Dirs[d];
				int32 OX = 0, OY = 0;
				if (!OwnerOf(LX, LY, Dir, OX, OY))
				{
					continue;
				}
				const uint8 St = EdgeStateBetween(LX, LY, Dir);
				if (St == (uint8)EFloorEdgeState::Open)
				{
					continue;
				}
				if (St == (uint8)EFloorEdgeState::Opening)
				{
					const FGridCell V = GridDirToVector(Dir);
					OutGeometry.DoorSocketPositions.Add(FVector(CC.X + V.X * HW, CC.Y + V.Y * HW, H * 0.6f));
					OutGeometry.DoorSocketNormals.Add(FVector((float)V.X, (float)V.Y, 0.0f));
					continue;
				}
				const bool bPart = St == (uint8)EFloorEdgeState::Partition;
				const float WallHeight = bPart ? PartH : WallH;
				FVector BoxC;
				FVector Half;
				if (Dir == EGridDir::East)
				{
					BoxC = FVector(CC.X + HW, CC.Y, WallHeight * 0.5f);
					Half = FVector(WT * 0.5f, HW, WallHeight * 0.5f);
				}
				else if (Dir == EGridDir::West)
				{
					BoxC = FVector(CC.X - HW, CC.Y, WallHeight * 0.5f);
					Half = FVector(WT * 0.5f, HW, WallHeight * 0.5f);
				}
				else if (Dir == EGridDir::North)
				{
					BoxC = FVector(CC.X, CC.Y + HW, WallHeight * 0.5f);
					Half = FVector(HW, WT * 0.5f, WallHeight * 0.5f);
				}
				else
				{
					BoxC = FVector(CC.X, CC.Y - HW, WallHeight * 0.5f);
					Half = FVector(HW, WT * 0.5f, WallHeight * 0.5f);
				}
				GAddBox(OutGeometry, BoxC, Half, UVS, kSlotWall);
			}
		}
	}

	// Columns: structural grid geometry (interior cells only).
	for (int32 S = 0; S < SpaceCount; ++S)
	{
		for (const FGridCell& C : Spaces[S].ColumnPositions)
		{
			if (C.X < X0 || C.X >= X0 + Count || C.Y < Y0 || C.Y >= Y0 + Count)
			{
				continue;
			}
			const float WX = (float)(RegionCoordinate.X * RS + C.X) * CS;
			const float WY = (float)(RegionCoordinate.Y * RS + C.Y) * CS;
			GAddBox(OutGeometry, FVector(WX + CS * 0.5f, WY + CS * 0.5f, ColH * 0.5f),
				FVector(ColHalf, ColHalf, ColH * 0.5f), UVS, kSlotWall);
		}
	}

	// Lights: ceiling grid derived from Brightness. Deterministic.
	{
		const int32 LightStep = FMath::Clamp(FMath::RoundToInt(6.0f - 3.5f * MacroFields.Brightness), 2, 8);
		const int32 HalfStep = LightStep / 2;
		for (int32 LY = Y0; LY < Y0 + Count; ++LY)
		{
			for (int32 LX = X0; LX < X0 + Count; ++LX)
			{
				const int32 Sp = SpaceAt(LX, LY);
				if (Sp < 0 || LX % LightStep != HalfStep || LY % LightStep != HalfStep)
				{
					continue;
				}
				// Interior of the space (not shoved against a wall/opening).
				const FGridCell N[4] = { {LX + 1, LY}, {LX - 1, LY}, {LX, LY + 1}, {LX, LY - 1} };
				bool bInterior = true;
				for (int32 i = 0; i < 4; ++i)
				{
					if (SpaceAt(N[i].X, N[i].Y) != Sp)
					{
						bInterior = false;
						break;
					}
				}
				if (!bInterior || IsColumn(LX, LY))
				{
					continue;
				}
				const float WX = (float)(RegionCoordinate.X * RS + LX) * CS;
				const float WY = (float)(RegionCoordinate.Y * RS + LY) * CS;
				float B = MacroFields.Brightness;
				// Sparse broken lights add character.
				const float Fl = FP_Unit(FP_Hash(WorldSeed, LX, LY));
				if (Fl < 0.035f * (1.0f + 2.0f * (1.0f - B)))
				{
					continue; // broken light — keep the spot dark
				}
				FLinearColor Col = FLinearColor(1.0f, 0.96f, 0.88f) * FMath::Clamp(0.7f + 0.5f * B, 0.7f, 1.2f);
				OutGeometry.LightPositions.Add(FVector(WX + CS * 0.5f, WY + CS * 0.5f, LampZ));
				OutGeometry.LightColors.Add(Col);
			}
		}
	}

	// Props: contextual slots for service/partitioned spaces (deterministic).
	{
		for (int32 S = 0; S < SpaceCount; ++S)
		{
			const FBackroomsSpaceData& Sp2 = Spaces[S];
			if (Sp2.Role != EBackroomsSpaceRole::ServiceSpace && Sp2.Role != EBackroomsSpaceRole::PartitionedSpace)
			{
				continue;
			}
			const int32 Cap = FMath::Clamp(Sp2.Area / 32, 1, 6);
			int32 Added = 0;
			for (const FGridCell& C : Sp2.Footprint)
			{
				if (C.X < X0 || C.X >= X0 + Count || C.Y < Y0 || C.Y >= Y0 + Count)
				{
					continue;
				}
				const float Fr = FP_Unit(FP_Hash(WorldSeed ^ 0xAB, C.X, C.Y));
				if (Fr < 0.12f && !IsColumn(C.X, C.Y) && Added < Cap)
				{
					const float WX = (float)(RegionCoordinate.X * RS + C.X) * CS;
					const float WY = (float)(RegionCoordinate.Y * RS + C.Y) * CS;
					OutGeometry.PropPositions.Add(FVector(WX + CS * 0.5f, WY + CS * 0.5f, 0.0f));
					OutGeometry.PropTypes.Add(0);
					++Added;
				}
				if (Added >= Cap)
				{
					break;
				}
			}
		}
	}
}

const FBackroomsSpaceData* UBackroomsFloorPlan::GetSpaceByHandle(const FBackroomsSpaceHandle& Handle) const
{
	if (Handle.RegionCoord == RegionCoordinate && Handle.IsValid() && Handle.LocalId < Spaces.Num())
	{
		return &Spaces[Handle.LocalId];
	}
	return nullptr;
}

const FBoundaryContract* UBackroomsFloorPlan::GetBoundaryContract(const FIntPoint& RegionCoord2, EEdgeID Edge) const
{
	const FBoundaryContract* Found = BoundaryContracts.Find(FEdgeKey(RegionCoord2, Edge));
	return Found;
}

void FChunkGeometryData::Clear()
{
	Vertices.Reset();
	Triangles.Reset();
	Normals.Reset();
	UVs.Reset();
	VertexColors.Reset();
	Tangents.Reset();
	QuadSlots.Reset();
	DoorSocketPositions.Reset();
	DoorSocketNormals.Reset();
	PropPositions.Reset();
	PropTypes.Reset();
	LightPositions.Reset();
	LightColors.Reset();
	ColumnPositions.Reset();
}

void FBackroomsSpaceData::ComputeArea()
{
	int32 Count = 0;
	for (const auto& Cell : Footprint)
	{
		if (Cell.X >= Bounds.Min.X && Cell.X <= Bounds.Max.X &&
			Cell.Y >= Bounds.Min.Y && Cell.Y <= Bounds.Max.Y)
		{
			++Count;
		}
	}
	Area = Count;
}

bool FBackroomsSpaceData::ContainsCell(const FGridCell& Cell) const
{
	return Cell.X >= Bounds.Min.X && Cell.X <= Bounds.Max.X &&
		Cell.Y >= Bounds.Min.Y && Cell.Y <= Bounds.Max.Y;
}

#if WITH_EDITOR
FString UBackroomsFloorPlan::GetDebugInfo() const
{
	return FString::Printf(
		TEXT("FloorPlan Region=(%d,%d) seed=%d attempt=%d\n  Spaces=%d Openings=%d Connections=%d Valid=%s\n  Openness=%.2f Regularity=%.2f Brightness=%.2f Structural=%.2f"),
		RegionCoordinate.X, RegionCoordinate.Y, WorldSeed, GenerationAttempt,
		Spaces.Num(), Openings.Num(), Connections.Num(), bValid ? TEXT("YES") : TEXT("NO"),
		MacroFields.Openness, MacroFields.Regularity, MacroFields.Brightness, MacroFields.StructuralDensity);
}
#endif
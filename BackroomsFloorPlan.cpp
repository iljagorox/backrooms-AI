#include "BackroomsFloorPlan.h"
#include "BackroomsLocationArchetype.h"
#include "BackroomsBoundaryContract.h"
#include "BackroomsChunkCoord.h"

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

	inline float FP_Unit(uint32 H)
	{
		return (float)(H & 0xFFFFFFu) / 16777215.0f;
	}

	inline int32 FP_Choice(int32 Seed, int32 X, int32 Y, int32 Count)
	{
		return Count > 0 ? (int32)(FP_Hash(Seed, X, Y) % (uint32)Count) : 0;
	}

	inline void GAddQuad(FChunkGeometryData& G, FVector A, FVector B, FVector C, FVector D,
		const FVector& Normal, float UVScale, uint8 Slot)
	{
		const int32 Base = G.Vertices.Num();
		G.Vertices.Add(A); G.Vertices.Add(B); G.Vertices.Add(C); G.Vertices.Add(D);
		G.Triangles.Add(Base + 0); G.Triangles.Add(Base + 1); G.Triangles.Add(Base + 2);
		G.Triangles.Add(Base + 0); G.Triangles.Add(Base + 2); G.Triangles.Add(Base + 3);
		for (int32 i = 0; i < 4; ++i) G.Normals.Add(Normal);
		G.UVs.Add(FVector2D(0,0)); G.UVs.Add(FVector2D(0,UVScale));
		G.UVs.Add(FVector2D(UVScale,UVScale)); G.UVs.Add(FVector2D(UVScale,0));
		G.VertexColors.Add(FLinearColor(1,1,1,1)); G.VertexColors.Add(FLinearColor(.9f,.9f,.9f,1));
		G.VertexColors.Add(FLinearColor(.85f,.85f,.85f,1)); G.VertexColors.Add(FLinearColor(1,1,1,1));
		const FProcMeshTangent T(0,0,1);
		for (int32 i = 0; i < 4; ++i) G.Tangents.Add(T);
		G.QuadSlots.Add(Slot);
	}

	inline void GAddBox(FChunkGeometryData& G, FVector Center, FVector Half, float UVScale, uint8 Slot)
	{
		const float X0 = Center.X-Half.X, X1 = Center.X+Half.X;
		const float Y0 = Center.Y-Half.Y, Y1 = Center.Y+Half.Y;
		const float Z0 = Center.Z-Half.Z, Z1 = Center.Z+Half.Z;
		const FVector P[8] = {
			FVector(X0,Y0,Z0),FVector(X1,Y0,Z0),FVector(X1,Y1,Z0),FVector(X0,Y1,Z0),
			FVector(X0,Y0,Z1),FVector(X1,Y0,Z1),FVector(X1,Y1,Z1),FVector(X0,Y1,Z1)};
		const uint8 TopSlot = Slot == kSlotWall ? kSlotCeiling : Slot;
		const uint8 BottomSlot = Slot == kSlotWall ? kSlotFloor : Slot;
		GAddQuad(G,P[4],P[5],P[6],P[7],FVector(0,0,1),UVScale,TopSlot);
		GAddQuad(G,P[3],P[2],P[1],P[0],FVector(0,0,-1),UVScale,BottomSlot);
		GAddQuad(G,P[1],P[2],P[6],P[5],FVector(1,0,0),UVScale,Slot);
		GAddQuad(G,P[4],P[7],P[3],P[0],FVector(-1,0,0),UVScale,Slot);
		GAddQuad(G,P[2],P[3],P[7],P[6],FVector(0,1,0),UVScale,Slot);
		GAddQuad(G,P[0],P[1],P[5],P[4],FVector(0,-1,0),UVScale,Slot);
	}

	enum class ERoomTemplate : uint8
	{
		Rect = 0,
		Wide = 1,
		LShape = 2,
		Split = 3,
		OpenHall = 4
	};

	inline ERoomTemplate PickTemplate(int32 Seed, int32 MX, int32 MY)
	{
		const int32 R = FP_Choice(Seed ^ 0x71A9, MX, MY, 100);
		if (R < 28) return ERoomTemplate::Rect;
		if (R < 48) return ERoomTemplate::Wide;
		if (R < 68) return ERoomTemplate::LShape;
		if (R < 88) return ERoomTemplate::Split;
		return ERoomTemplate::OpenHall;
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
	const int32 Module = FMath::Max(8, RS / 4);
	const int32 Modules = FMath::Max(1, RS / Module);
	const int32 Seed = WorldSeed;
	const float OpennessBias = 0.35f + 0.65f * FP_Unit(FP_Hash(Seed ^ 0x11, RegionCoordinate.X, RegionCoordinate.Y));
	const float Brightness = 0.25f + 0.75f * FP_Unit(FP_Hash(Seed ^ 0x22, RegionCoordinate.X, RegionCoordinate.Y));
	const float Moisture = FP_Unit(FP_Hash(Seed ^ 0x33, RegionCoordinate.X, RegionCoordinate.Y));

	Spaces.Reset();
	Openings.Reset();
	Connections.Reset();
	BoundaryContracts.Reset();
	CellSpaceGrid.Init(INDEX_NONE, RS * RS);
	// Cell interiors are open by default. Walls/partitions are explicitly authored
	// below; defaulting every edge to Wall creates the old grid/sticks maze.
	EdgeStates.Init((uint8)EFloorEdgeState::Open, RS * RS * 2 + 4 * RS);

	auto SetEdge = [&](int32 X, int32 Y, EGridDir Dir, EFloorEdgeState State)
	{
		const int32 Id = EdgeId(X, Y, Dir);
		if (Id != INDEX_NONE && EdgeStates.IsValidIndex(Id)) EdgeStates[Id] = (uint8)State;
	};

	const FIntPoint NReg[4] = {{1,0},{-1,0},{0,1},{0,-1}};
	const EEdgeID RegEdges[4] = {EEdgeID::EastEdge,EEdgeID::WestEdge,EEdgeID::NorthEdge,EEdgeID::SouthEdge};
	for (int32 i=0;i<4;++i)
	{
		FBoundaryContract C;
		if (FBackroomsBoundaryContractGenerator::GenerateBoundaryContract(
			WorldSeed, LevelIndex, RegionCoordinate, RegionCoordinate + NReg[i], RegEdges[i], RS, C))
		{
			BoundaryContracts.Add(FEdgeKey(RegionCoordinate, RegEdges[i]), C);
		}
	}
	for (int32 y=0;y<RS;++y)
	{
		SetEdge(0,y,EGridDir::West,EFloorEdgeState::Open);
		SetEdge(RS-1,y,EGridDir::East,EFloorEdgeState::Open);
	}
	for (int32 x=0;x<RS;++x)
	{
		SetEdge(x,0,EGridDir::South,EFloorEdgeState::Open);
		SetEdge(x,RS-1,EGridDir::North,EFloorEdgeState::Open);
	}

	for (int32 MY=0; MY<Modules; ++MY)
	{
		for (int32 MX=0; MX<Modules; ++MX)
		{
			const ERoomTemplate Template = PickTemplate(Seed + InAttempt * 7919, MX, MY);
			const int32 SpaceId = Spaces.Num();
			FBackroomsSpaceData Sp;
			Sp.Handle = FBackroomsSpaceHandle(RegionCoordinate, SpaceId);
			Sp.Bounds = FGridBox(MX*Module, MY*Module, FMath::Min(RS-1,(MX+1)*Module-1), FMath::Min(RS-1,(MY+1)*Module-1));
			Sp.Openness = OpennessBias;
			Sp.CeilingHeight = 1.0f;
			Sp.bIsValid = true;
			Sp.ValidationScore = 1.0f;

			switch (Template)
			{
				case ERoomTemplate::Rect: Sp.Role = EBackroomsSpaceRole::ServiceSpace; break;
				case ERoomTemplate::Wide: Sp.Role = EBackroomsSpaceRole::OpenSpace; break;
				case ERoomTemplate::LShape: Sp.Role = EBackroomsSpaceRole::PartitionedSpace; break;
				case ERoomTemplate::Split: Sp.Role = EBackroomsSpaceRole::PartitionedSpace; break;
				default: Sp.Role = EBackroomsSpaceRole::TransitionSpace; break;
			}

			for (int32 y=Sp.Bounds.Min.Y;y<=Sp.Bounds.Max.Y;++y)
			{
				for (int32 x=Sp.Bounds.Min.X;x<=Sp.Bounds.Max.X;++x)
				{
					Sp.Footprint.Add(FGridCell(x,y));
					CellSpaceGrid[CellIndex(x,y)] = SpaceId;
				}
			}
			Sp.Area = Sp.Footprint.Num();
			Sp.FlowPriority = Template == ERoomTemplate::OpenHall ? 0.9f : 0.45f;
			if (Brightness < 0.32f) Sp.EnvironmentalTags.Add(FName(TEXT("Dark")));
			else if (Brightness > 0.72f) Sp.EnvironmentalTags.Add(FName(TEXT("Bright")));
			if (Moisture > 0.62f) Sp.EnvironmentalTags.Add(FName(TEXT("Wet")));
			if (Template == ERoomTemplate::OpenHall) Sp.EnvironmentalTags.Add(FName(TEXT("Open")));
			Spaces.Add(Sp);

			const int32 X0 = MX*Module;
			const int32 Y0 = MY*Module;
			const int32 X1 = FMath::Min(RS-1,(MX+1)*Module-1);
			const int32 Y1 = FMath::Min(RS-1,(MY+1)*Module-1);
			const int32 MidX = X0 + Module/2;
			const int32 MidY = Y0 + Module/2;
			const int32 Door = 2 + (FP_Choice(Seed ^ 0xA1, MX, MY, 3));

			if (Template == ERoomTemplate::Split)
			{
				for (int32 y=Y0+2;y<=Y1-2;++y)
				{
					if (FMath::Abs(y-MidY) > Door) SetEdge(MidX-1,y,EGridDir::East,EFloorEdgeState::Wall);
				}
			}
			else if (Template == ERoomTemplate::LShape)
			{
				for (int32 y=MidY;y<=Y1-2;++y)
				{
					if (FMath::Abs(y-(MidY+Door)) > 1) SetEdge(MidX-1,y,EGridDir::East,EFloorEdgeState::Wall);
				}
				for (int32 x=X0+2;x<=MidX-1;++x)
				{
					if (FMath::Abs(x-(X0+Door+2)) > 1) SetEdge(x,MidY-1,EGridDir::North,EFloorEdgeState::Wall);
				}
			}
			else if (Template == ERoomTemplate::Wide)
			{
				const bool bVertical = FP_Choice(Seed ^ 0xA2, MX, MY, 2) == 0;
				if (bVertical)
				{
					for (int32 y=Y0+3;y<=Y0+Module/2-1;++y) SetEdge(MidX-1,y,EGridDir::East,EFloorEdgeState::Partition);
				}
				else
				{
					for (int32 x=X0+3;x<=X0+Module/2-1;++x) SetEdge(x,MidY-1,EGridDir::North,EFloorEdgeState::Partition);
				}
			}
		}
	}

	const int32 SpaceCount = Spaces.Num();

	auto OpenBetweenModules = [&](int32 A, int32 B, EGridDir Dir)
	{
		const int32 AX = (A % Modules) * Module;
		const int32 AY = (A / Modules) * Module;
		const int32 BX = (B % Modules) * Module;
		const int32 BY = (B / Modules) * Module;
		if (Dir == EGridDir::East)
		{
			const int32 Y = FMath::Clamp(AY + Module/2 + (FP_Choice(Seed ^ 0xB1,A,B,3)-1)*2, AY+2, AY+Module-3);
			SetEdge(AX+Module-1,Y,EGridDir::East,EFloorEdgeState::Opening);
			SetEdge(AX+Module-1,Y+1,EGridDir::East,EFloorEdgeState::Opening);
		}
		else
		{
			const int32 X = FMath::Clamp(AX + Module/2 + (FP_Choice(Seed ^ 0xB2,A,B,3)-1)*2, AX+2, AX+Module-3);
			SetEdge(X,AY+Module-1,EGridDir::North,EFloorEdgeState::Opening);
			SetEdge(X+1,AY+Module-1,EGridDir::North,EFloorEdgeState::Opening);
		}
	};

	for (int32 my=0;my<Modules;++my)
	{
		for (int32 mx=0;mx<Modules;++mx)
		{
			const int32 S = my*Modules+mx;
			if (mx>0 && (my==0 || FP_Choice(Seed ^ 0xC1,mx,my,2)==0))
				OpenBetweenModules(S,S-1,EGridDir::West);
			else if (my>0)
				OpenBetweenModules(S,S-Modules,EGridDir::South);
		}
	}
	for (int32 my=0;my<Modules;++my)
	{
		for (int32 mx=0;mx<Modules;++mx)
		{
			const int32 S=my*Modules+mx;
			if (mx+1<Modules && FP_Unit(FP_Hash(Seed ^ 0xC2,mx,my)) < 0.28f)
				OpenBetweenModules(S,S+1,EGridDir::East);
			if (my+1<Modules && FP_Unit(FP_Hash(Seed ^ 0xC3,mx,my)) < 0.24f)
				OpenBetweenModules(S,S+Modules,EGridDir::North);
		}
	}

	for (int32 side=0;side<4;++side)
	{
		if (FP_Unit(FP_Hash(Seed ^ 0xD0,RegionCoordinate.X+side,RegionCoordinate.Y-side)) > 0.45f) continue;
		const int32 P = FMath::Clamp(FP_Choice(Seed ^ 0xD1,RegionCoordinate.X+side*17,RegionCoordinate.Y-side*11,RS-2)+1,1,RS-2);
		if (side==0) { SetEdge(RS-1,P,EGridDir::East,EFloorEdgeState::Opening); SetEdge(RS-1,P+1,EGridDir::East,EFloorEdgeState::Opening); }
		if (side==1) { SetEdge(0,P,EGridDir::West,EFloorEdgeState::Opening); SetEdge(0,P+1,EGridDir::West,EFloorEdgeState::Opening); }
		if (side==2) { SetEdge(P,RS-1,EGridDir::North,EFloorEdgeState::Opening); SetEdge(P+1,RS-1,EGridDir::North,EFloorEdgeState::Opening); }
		if (side==3) { SetEdge(P,0,EGridDir::South,EFloorEdgeState::Opening); SetEdge(P+1,0,EGridDir::South,EFloorEdgeState::Opening); }
	}

	for (int32 S=0;S<SpaceCount;++S)
	{
		FBackroomsSpaceData& Sp=Spaces[S];
		if (Sp.Role != EBackroomsSpaceRole::OpenSpace) continue;
		const int32 MX=S%Modules, MY=S/Modules;
		const int32 Variant=FP_Choice(Seed ^ 0xE1,MX,MY,5);
		const int32 Count=(Variant==0 || Variant==1) ? 1 : 0;
		for (int32 c=0;c<Count;++c)
		{
			const int32 X=Sp.Bounds.Min.X+Module/2 + (c?3:0);
			const int32 Y=Sp.Bounds.Min.Y+Module/2 + (Variant==1?3:0);
			if (X>Sp.Bounds.Min.X+2 && X<Sp.Bounds.Max.X-2 && Y>Sp.Bounds.Min.Y+2 && Y<Sp.Bounds.Max.Y-2)
				Sp.ColumnPositions.Add(FGridCell(X,Y));
		}
	}

	{
		TSet<int32> Seen;
		for (int32 y=0;y<RS;++y)
		{
			for (int32 x=0;x<RS;++x)
			{
				const EGridDir Dirs[2]={EGridDir::East,EGridDir::North};
				for (EGridDir Dir:Dirs)
				{
					const int32 Eid=EdgeId(x,y,Dir);
					if (Eid==INDEX_NONE || Seen.Contains(Eid) || !EdgeStates.IsValidIndex(Eid) || EdgeStates[Eid]!=(uint8)EFloorEdgeState::Opening) continue;
					Seen.Add(Eid);
					const FGridCell V=GridDirToVector(Dir);
					const int32 NX=x+V.X, NY=y+V.Y;
					const int32 A=SpaceAt(x,y);
					const int32 B=(NX>=0&&NX<RS&&NY>=0&&NY<RS)?SpaceAt(NX,NY):-1;
					FBackroomsOpeningData Op;
					Op.OwnerSpace=FBackroomsSpaceHandle(RegionCoordinate,A);
					Op.BoundaryEdge=Dir;
					Op.Position=FGridPoint(x,y);
					Op.Width=2.0f;
					Op.Type=EOpeningType::Door;
					Op.bIsValid=true;
					Openings.Add(Op);
					if (B>=0 && B!=A)
					{
						FBackroomsConnectionData C;
						C.Opening=Op;
						C.SpaceA=FBackroomsSpaceHandle(RegionCoordinate,A);
						C.SpaceB=FBackroomsSpaceHandle(RegionCoordinate,B);
						C.Type=EConnectionType::Door;
						C.bIsValid=true;
						Connections.Add(C);
						Openings.Last().Connection=FBackroomsConnectionHandle(C.SpaceA,C.SpaceB);
					}
				}
			}
		}
	}

	for (int32 S=0;S<SpaceCount;++S)
	{
		FBackroomsSpaceData& Sp=Spaces[S];
		TSet<int32> Neighbors;
		for (const FBackroomsConnectionData& C:Connections)
		{
			if (C.SpaceA.LocalId==S) Neighbors.Add(C.SpaceB.LocalId);
			else if (C.SpaceB.LocalId==S) Neighbors.Add(C.SpaceA.LocalId);
		}
		Sp.bIsLoop=Neighbors.Num()>=2;
		Sp.bIsDeadEnd=Neighbors.Num()==0;
		Sp.FlowPriority=FMath::Clamp(0.25f*(float)Neighbors.Num() + (Sp.Role==EBackroomsSpaceRole::TransitionSpace?0.25f:0.0f),0.0f,1.0f);
	}

	MacroFields.Openness=OpennessBias;
	MacroFields.Regularity=0.55f + 0.35f*FP_Unit(FP_Hash(Seed ^ 0xF1,RegionCoordinate.X,RegionCoordinate.Y));
	MacroFields.Brightness=Brightness;
	MacroFields.StructuralDensity=0.12f;
	MacroFields.Decay=FP_Unit(FP_Hash(Seed ^ 0xF2,RegionCoordinate.X,RegionCoordinate.Y));
	MacroFields.Moisture=Moisture;

	bValid=Validate().bHardPass;
	if (!bValid)
	{
		UE_LOG(LogTemp,Warning,TEXT("BR: deterministic room-template FloorPlan failed validation region=(%d,%d) seed=%d"),RegionCoordinate.X,RegionCoordinate.Y,WorldSeed);
	}
	(void)Archetype;
}

FBackroomsValidationResult UBackroomsFloorPlan::Validate() const
{
	FBackroomsValidationResult Result;
	Result.FailedHardConstraints.Reset();
	Result.SoftMetricsAboveThreshold.Reset();
	const int32 RS=RegionSizeCells;
	const int32 SpaceCount=Spaces.Num();
	if (SpaceCount==0)
	{
		Result.FailedHardConstraints.Add(TEXT("NoSpace"));
		Result.bHardPass=false;
		return Result;
	}

	TArray<uint8> Reach; Reach.SetNumZeroed(RS*RS);
	TArray<int32> Q; Q.Add(CellIndex(RS/2,RS/2)); Reach[CellIndex(RS/2,RS/2)]=1;
	const EGridDir Dirs[4]={EGridDir::East,EGridDir::West,EGridDir::North,EGridDir::South};
	while(Q.Num()>0)
	{
		const int32 C=Q.Pop(false); const int32 X=C%RS, Y=C/RS;
		for(EGridDir D:Dirs)
		{
			const uint8 St=EdgeStateBetween(X,Y,D);
			if(St!=(uint8)EFloorEdgeState::Open && St!=(uint8)EFloorEdgeState::Opening) continue;
			const FGridCell V=GridDirToVector(D); const int32 NX=X+V.X, NY=Y+V.Y;
			if(NX<0||NX>=RS||NY<0||NY>=RS) continue;
			const int32 I=CellIndex(NX,NY);
			if(!Reach[I]) { Reach[I]=1; Q.Add(I); }
		}
	}
	int32 Unreached=0;
	for(uint8 R:Reach) if(!R) ++Unreached;
	if(Unreached>0) Result.FailedHardConstraints.Add(FName(FString::Printf(TEXT("UnreachableCells=%d"),Unreached)));
	for(int32 S=0;S<SpaceCount;++S)
	{
		const FBackroomsSpaceData& Sp=Spaces[S];
		if(Sp.Bounds.Width()<4 || Sp.Bounds.Height()<4) Result.FailedHardConstraints.Add(FName(FString::Printf(TEXT("ThinSpace:%d"),S)));
	}
	Result.bHardPass=Result.FailedHardConstraints.Num()==0;

	int32 DeadEnds=0, Multi=0, TotalArea=0;
	for(int32 S=0;S<SpaceCount;++S)
	{
		TotalArea+=Spaces[S].Area; int32 Conns=0;
		for(const FBackroomsConnectionData& C:Connections)
			if(C.SpaceA.LocalId==S || C.SpaceB.LocalId==S) ++Conns;
		if(Conns<=1) ++DeadEnds; if(Conns>=2) ++Multi;
	}
	const float AvgArea=SpaceCount?(float)TotalArea/(float)SpaceCount:0.0f;
	Result.DeadEndRatio=SpaceCount?(float)DeadEnds/(float)SpaceCount:0.0f;
	Result.MazeScore=FMath::Clamp(1.0f-Result.DeadEndRatio,0.0f,1.0f);
	Result.OpennessScore=FMath::Clamp(AvgArea/256.0f,0.0f,1.0f);
	Result.FlowContinuityScore=SpaceCount?(float)Multi/(float)SpaceCount:0.0f;
	Result.RepetitionScore=1.0f;
	Result.MazeThreshold=0.3f; Result.RepetitionThreshold=0.5f; Result.OpennessThreshold=0.4f;
	Result.DeadEndThreshold=0.15f; Result.ContinuityThreshold=0.7f;
	if(Result.MazeScore<Result.MazeThreshold) Result.SoftMetricsAboveThreshold.Add(TEXT("Maze"));
	if(Result.OpennessScore<Result.OpennessThreshold) Result.SoftMetricsAboveThreshold.Add(TEXT("Openness"));
	return Result;
}

void UBackroomsFloorPlan::ExtractChunk(const FChunkCoord& ChunkCoord, FChunkGeometryData& OutGeometry) const
{
	OutGeometry.Clear();
	const int32 RS=RegionSizeCells;
	const int32 RCX=ChunkCoord.X-RegionCoordinate.X*ChunkExtent.X;
	const int32 RCY=ChunkCoord.Y-RegionCoordinate.Y*ChunkExtent.Y;
	if(RCX<0||RCX>=ChunkExtent.X||RCY<0||RCY>=ChunkExtent.Y) return;
	const int32 Count=ChunkCellSize, X0=RCX*Count, Y0=RCY*Count;
	const float CS=CellSizeWorld,H=WallHeightWorld,WT=WallThicknessWorld,UVS=4.0f;
	const float WallH=H,PartH=H*0.45f,ColHalf=FMath::Max(WT*0.75f,25.0f),ColH=H*0.8f,LampZ=H*0.9f;
	const int32 SpaceCount=Spaces.Num();
	auto IsColumn=[&](int32 X,int32 Y)->bool
	{
		const int32 S=SpaceAt(X,Y); if(S<0||S>=SpaceCount) return false;
		for(const FGridCell& C:Spaces[S].ColumnPositions) if(C.X==X&&C.Y==Y) return true;
		return false;
	};
	auto OwnerOf=[&](int32 X,int32 Y,EGridDir D,int32& OX,int32& OY)->bool
	{
		OX=X; OY=Y;
		if(D==EGridDir::West) --OX; else if(D==EGridDir::South) --OY;
		if(OX<0||OY<0) return false;
		return OX>=X0&&OX<X0+Count&&OY>=Y0&&OY<Y0+Count;
	};

	for(int32 Y=Y0;Y<Y0+Count;++Y)
	{
		for(int32 X=X0;X<X0+Count;++X)
		{
			if(SpaceAt(X,Y)<0) continue;
			const float WX=(float)(RegionCoordinate.X*RS+X)*CS, WY=(float)(RegionCoordinate.Y*RS+Y)*CS;
			const FVector CC(WX+CS*0.5f,WY+CS*0.5f,0), HW=FVector(CS*0.5f,CS*0.5f,0);
			GAddQuad(OutGeometry,FVector(CC.X-HW.X,CC.Y-HW.Y,0),FVector(CC.X+HW.X,CC.Y-HW.Y,0),FVector(CC.X+HW.X,CC.Y+HW.Y,0),FVector(CC.X-HW.X,CC.Y+HW.Y,0),FVector(0,0,1),UVS,kSlotFloor);
			GAddQuad(OutGeometry,FVector(CC.X-HW.X,CC.Y+HW.Y,H),FVector(CC.X+HW.X,CC.Y+HW.Y,H),FVector(CC.X+HW.X,CC.Y-HW.Y,H),FVector(CC.X-HW.X,CC.Y-HW.Y,H),FVector(0,0,-1),UVS,kSlotCeiling);
			const EGridDir Dirs[4]={EGridDir::East,EGridDir::West,EGridDir::North,EGridDir::South};
			for(EGridDir D:Dirs)
			{
				int32 OX=0,OY=0; if(!OwnerOf(X,Y,D,OX,OY)) continue;
				const uint8 St=EdgeStateBetween(X,Y,D); if(St==(uint8)EFloorEdgeState::Open) continue;
				if(St==(uint8)EFloorEdgeState::Opening)
				{
					const FGridCell V=GridDirToVector(D);
					OutGeometry.DoorSocketPositions.Add(FVector(CC.X+V.X*CS*0.5f,CC.Y+V.Y*CS*0.5f,H*0.6f));
					OutGeometry.DoorSocketNormals.Add(FVector((float)V.X,(float)V.Y,0));
					continue;
				}
				const bool bPart=St==(uint8)EFloorEdgeState::Partition; const float WH=bPart?PartH:WallH;
				FVector BC,Half;
				if(D==EGridDir::East){BC=FVector(CC.X+CS*.5f,CC.Y,WH*.5f);Half=FVector(WT*.5f,CS*.5f,WH*.5f);}
				else if(D==EGridDir::West){BC=FVector(CC.X-CS*.5f,CC.Y,WH*.5f);Half=FVector(WT*.5f,CS*.5f,WH*.5f);}
				else if(D==EGridDir::North){BC=FVector(CC.X,CC.Y+CS*.5f,WH*.5f);Half=FVector(CS*.5f,WT*.5f,WH*.5f);}
				else {BC=FVector(CC.X,CC.Y-CS*.5f,WH*.5f);Half=FVector(CS*.5f,WT*.5f,WH*.5f);}
				GAddBox(OutGeometry,BC,Half,UVS,kSlotWall);
			}
		}
	}

	for(int32 S=0;S<SpaceCount;++S)
	{
		for(const FGridCell& C:Spaces[S].ColumnPositions)
		{
			if(C.X<X0||C.X>=X0+Count||C.Y<Y0||C.Y>=Y0+Count) continue;
			const float WX=(float)(RegionCoordinate.X*RS+C.X)*CS,WY=(float)(RegionCoordinate.Y*RS+C.Y)*CS;
			GAddBox(OutGeometry,FVector(WX+CS*.5f,WY+CS*.5f,ColH*.5f),FVector(ColHalf,ColHalf,ColH*.5f),UVS,kSlotWall);
		}
	}

	for(int32 S=0;S<SpaceCount;++S)
	{
		const FBackroomsSpaceData& Sp=Spaces[S];
		if(Sp.Bounds.Max.X<X0||Sp.Bounds.Min.X>=X0+Count||Sp.Bounds.Max.Y<Y0||Sp.Bounds.Min.Y>=Y0+Count) continue;
		const int32 MX=S%FMath::Max(1,RS/ChunkCellSize/2), MY=S/FMath::Max(1,RS/ChunkCellSize/2);
		const int32 CX=Sp.Bounds.Min.X+Sp.Bounds.Width()/2,CY=Sp.Bounds.Min.Y+Sp.Bounds.Height()/2;
		const float WX=(float)(RegionCoordinate.X*RS+CX)*CS,WY=(float)(RegionCoordinate.Y*RS+CY)*CS;
		if(CX>=X0&&CX<X0+Count&&CY>=Y0&&CY<Y0+Count)
		{
			FLinearColor Col=FLinearColor(1.0f,0.96f,0.88f)*FMath::Clamp(0.65f+0.55f*MacroFields.Brightness,0.65f,1.2f);
			OutGeometry.LightPositions.Add(FVector(WX+CS*.5f,WY+CS*.5f,LampZ));
			OutGeometry.LightColors.Add(Col);
		}
		(void)MX; (void)MY;
	}

	for(int32 S=0;S<SpaceCount;++S)
	{
		const FBackroomsSpaceData& Sp=Spaces[S];
		const int32 Cap=(Sp.Role==EBackroomsSpaceRole::OpenSpace)?1:((Sp.Role==EBackroomsSpaceRole::TransitionSpace)?2:3);
		int32 Added=0;
		for(const FGridCell& C:Sp.Footprint)
		{
			if(C.X<X0||C.X>=X0+Count||C.Y<Y0||C.Y>=Y0+Count||Added>=Cap||IsColumn(C.X,C.Y)) continue;
			if(FP_Unit(FP_Hash(WorldSeed ^ 0xAB,C.X,C.Y))<0.045f)
			{
				const float WX=(float)(RegionCoordinate.X*RS+C.X)*CS,WY=(float)(RegionCoordinate.Y*RS+C.Y)*CS;
				OutGeometry.PropPositions.Add(FVector(WX+CS*.5f,WY+CS*.5f,0));
				OutGeometry.PropTypes.Add(0); ++Added;
			}
		}
	}
}

const FBackroomsSpaceData* UBackroomsFloorPlan::GetSpaceByHandle(const FBackroomsSpaceHandle& Handle) const
{
	if(Handle.RegionCoord==RegionCoordinate && Handle.IsValid() && Handle.LocalId>=0 && Handle.LocalId<Spaces.Num()) return &Spaces[Handle.LocalId];
	return nullptr;
}

const FBoundaryContract* UBackroomsFloorPlan::GetBoundaryContract(const FIntPoint& RegionCoord2,EEdgeID Edge) const
{
	return BoundaryContracts.Find(FEdgeKey(RegionCoord2,Edge));
}

void FChunkGeometryData::Clear()
{
	Vertices.Reset(); Triangles.Reset(); Normals.Reset(); UVs.Reset(); VertexColors.Reset(); Tangents.Reset(); QuadSlots.Reset();
	DoorSocketPositions.Reset(); DoorSocketNormals.Reset(); PropPositions.Reset(); PropTypes.Reset();
	LightPositions.Reset(); LightColors.Reset(); ColumnPositions.Reset();
}

void FBackroomsSpaceData::ComputeArea()
{
	Area=0;
	for(const FGridCell& Cell:Footprint)
		if(Cell.X>=Bounds.Min.X&&Cell.X<=Bounds.Max.X&&Cell.Y>=Bounds.Min.Y&&Cell.Y<=Bounds.Max.Y) ++Area;
}

bool FBackroomsSpaceData::ContainsCell(const FGridCell& Cell) const
{
	return Cell.X>=Bounds.Min.X&&Cell.X<=Bounds.Max.X&&Cell.Y>=Bounds.Min.Y&&Cell.Y<=Bounds.Max.Y;
}

#if WITH_EDITOR
FString UBackroomsFloorPlan::GetDebugInfo() const
{
	return FString::Printf(TEXT("FloorPlan Region=(%d,%d) seed=%d attempt=%d Spaces=%d Openings=%d Connections=%d Valid=%s Openness=%.2f Regularity=%.2f Brightness=%.2f Structural=%.2f"),
		RegionCoordinate.X,RegionCoordinate.Y,WorldSeed,GenerationAttempt,Spaces.Num(),Openings.Num(),Connections.Num(),bValid?TEXT("YES"):TEXT("NO"),MacroFields.Openness,MacroFields.Regularity,MacroFields.Brightness,MacroFields.StructuralDensity);
}
#endif

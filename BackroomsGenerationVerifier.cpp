#include "BackroomsGenerationVerifier.h"
#include "BackroomsGenerationData.h"
#include "BackroomsChunkCell.h"
#include "BackroomsCellType.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarGenVerify(
	TEXT("BR.Gen.Verify"),
	1,
	TEXT("Generation verification mode: 0=Off, 1=Log (default), 2=Strict."),
	ECVF_Default);

namespace
{
	FORCEINLINE uint32 HashMix(uint32 H, uint32 V)
	{
		H ^= V;
		H *= 16777619u;
		return H;
	}

	FORCEINLINE uint32 Quantize(float V)
	{
		const float S = FMath::IsFinite(V) ? V : 0.0f;
		return (uint32)(int32)FMath::RoundToInt(S * 1000000.0f);
	}

	FORCEINLINE uint32 CellHash(uint32 H, const FChunkCell& C)
	{
		H = HashMix(H, Quantize(C.Density));
		H = HashMix(H, (uint32)(uint8)C.CellType);
		H = HashMix(H, C.bIsWall ? 1u : 0u);
		H = HashMix(H, C.bIsDoor ? 1u : 0u);
		H = HashMix(H, C.bIsScatter ? 1u : 0u);
		H = HashMix(H, C.bIsLivingRoom ? 1u : 0u);
		H = HashMix(H, (uint32)C.RoomIndex);
		return H;
	}

	// Collects actionable, cell-level differences between two generations.
	void CompareCells(const FChunkGenerationData& A, const FChunkGenerationData& B,
		TArray<FString>& Out, int32 MaxDiffs)
	{
		const int32 Count = A.Params.CellCount;
		if (A.Cells.Num() != B.Cells.Num())
		{
			Out.Add(FString::Printf(TEXT("cell count differs: %d vs %d"), A.Cells.Num(), B.Cells.Num()));
			return;
		}
		int32 Diffs = 0;
		for (int32 LY = 0; LY < Count && Diffs < MaxDiffs; ++LY)
		{
			for (int32 LX = 0; LX < Count && Diffs < MaxDiffs; ++LX)
			{
				const int32 I = LY * Count + LX;
				const FChunkCell& CA = A.Cells[I];
				const FChunkCell& CB = B.Cells[I];
				if (Quantize(CA.Density) != Quantize(CB.Density)
					|| CA.bIsWall != CB.bIsWall || CA.bIsDoor != CB.bIsDoor
					|| CA.CellType != CB.CellType || CA.RoomIndex != CB.RoomIndex)
				{
					Out.Add(FString::Printf(
						TEXT("cell(%d,%d): wall %d/%d door %d/%d type %d/%d room %d/%d dens %.4f/%.4f"),
						LX, LY, CA.bIsWall, CB.bIsWall, CA.bIsDoor, CB.bIsDoor,
						(int32)CA.CellType, (int32)CB.CellType,
						CA.RoomIndex, CB.RoomIndex, CA.Density, CB.Density));
					++Diffs;
				}
			}
		}
	}
}

EGenerationVerifyMode FChunkGenerationVerifier::GetMode()
{
	const int32 V = CVarGenVerify.GetValueOnAnyThread();
	if (V <= 0) { return EGenerationVerifyMode::Off; }
	if (V >= 2) { return EGenerationVerifyMode::Strict; }
	return EGenerationVerifyMode::Log;
}

const TCHAR* FChunkGenerationVerifier::StageName(EGenerationStage Stage)
{
	switch (Stage)
	{
	case EGenerationStage::Density: return TEXT("Density");
	case EGenerationStage::Walls:   return TEXT("Walls");
	case EGenerationStage::Doors:   return TEXT("Doors");
	case EGenerationStage::Rooms:   return TEXT("Rooms");
	case EGenerationStage::Masks:   return TEXT("Masks");
	default:                        return TEXT("Unknown");
	}
}

uint32 FChunkGenerationVerifier::HashCells(const FChunkGenerationData& Data)
{
	uint32 H = 2166136261u;
	H = HashMix(H, (uint32)Data.Seed);
	H = HashMix(H, (uint32)Data.Coord.X);
	H = HashMix(H, (uint32)Data.Coord.Y);
	H = HashMix(H, (uint32)Data.Params.CellCount);
	H = HashMix(H, (uint32)Data.Params.Pattern);
	for (const FChunkCell& C : Data.Cells)
	{
		H = CellHash(H, C);
	}
	return H;
}

void FChunkGenerationVerifier::VerifyStage(EGenerationStage Stage, const FChunkGenerationData& Data, FStageVerifyResult& Out)
{
	Out.Stage = Stage;
	Out.bPassed = true;

	const int32 Count = Data.Params.CellCount;
	const int32 Expected = Count * Count;

	if (Data.Cells.Num() != Expected)
	{
		Out.Failures.Add(FString::Printf(TEXT("cells=%d expected=%d"), Data.Cells.Num(), Expected));
		Out.bPassed = false;
		Out.NumChecks += 1;
		return;
	}

	switch (Stage)
	{
	case EGenerationStage::Density:
	{
		float MinD = TNumericLimits<float>::Max();
		float MaxD = TNumericLimits<float>::Lowest();
		for (int32 I = 0; I < Expected; ++I)
		{
			const float D = Data.Cells[I].Density;
			++Out.NumChecks;
			if (!FMath::IsFinite(D))
			{
				Out.Failures.Add(FString::Printf(TEXT("cell %d density not finite"), I));
			}
			else if (D < -0.001f || D > 1.001f)
			{
				Out.Failures.Add(FString::Printf(TEXT("cell %d density %.4f out of [0,1]"), I, D));
			}
			MinD = FMath::Min(MinD, D);
			MaxD = FMath::Max(MaxD, D);
		}
		if (FMath::IsFinite(MinD) && (MaxD - MinD) < 1e-4f)
		{
			Out.Warnings.Add(FString::Printf(TEXT("density is degenerate (range %.6f)"), MaxD - MinD));
		}
		break;
	}
	case EGenerationStage::Walls:
	{
		int32 Walls = 0;
		for (int32 I = 0; I < Expected; ++I)
		{
			const FChunkCell& C = Data.Cells[I];
			++Out.NumChecks;
			const bool bTypeIsWall = (C.CellType == ECellType::Wall || C.CellType == ECellType::Door);
			if (C.bIsWall && !bTypeIsWall)
			{
				Out.Failures.Add(FString::Printf(TEXT("cell %d bIsWall but type %d"), I, (int32)C.CellType));
			}
			if (!C.bIsWall && bTypeIsWall)
			{
				Out.Failures.Add(FString::Printf(TEXT("cell %d not wall but type %d"), I, (int32)C.CellType));
			}
			if (C.bIsWall) { ++Walls; }
		}
		++Out.NumChecks;
		if (Walls == 0)
		{
			Out.Warnings.Add(TEXT("no wall cells in chunk"));
		}
		else if (Walls == Expected)
		{
			Out.Failures.Add(TEXT("every cell is a wall"));
		}
		break;
	}
	case EGenerationStage::Doors:
	{
		for (int32 I = 0; I < Expected; ++I)
		{
			const FChunkCell& C = Data.Cells[I];
			++Out.NumChecks;
			if (C.bIsDoor && !C.bIsWall)
			{
				Out.Failures.Add(FString::Printf(TEXT("cell %d door without wall"), I));
			}
			if (C.bIsDoor && C.CellType != ECellType::Door)
			{
				Out.Failures.Add(FString::Printf(TEXT("cell %d door but type %d"), I, (int32)C.CellType));
			}
		}
		break;
	}
	case EGenerationStage::Rooms:
	{
		const int32 NumRooms = Data.LastRooms.Num();
		TArray<int32> Counts;
		Counts.SetNumZeroed(FMath::Max(1, NumRooms));
		for (int32 I = 0; I < Expected; ++I)
		{
			const FChunkCell& C = Data.Cells[I];
			++Out.NumChecks;
			const bool bPassable = (!C.bIsWall || C.bIsDoor);
			if (bPassable)
			{
				if (C.RoomIndex < 0 || C.RoomIndex >= NumRooms)
				{
					Out.Failures.Add(FString::Printf(TEXT("passable cell %d room index %d out of [0,%d)"), I, C.RoomIndex, NumRooms));
				}
				else
				{
					++Counts[C.RoomIndex];
				}
			}
			else if (C.RoomIndex != INDEX_NONE)
			{
				Out.Failures.Add(FString::Printf(TEXT("solid cell %d has room index %d"), I, C.RoomIndex));
			}
		}
		for (int32 R = 0; R < NumRooms; ++R)
		{
			++Out.NumChecks;
			if (Data.LastRooms[R].CellCount != Counts[R])
			{
				Out.Failures.Add(FString::Printf(TEXT("room %d cell count %d != actual %d"),
					R, Data.LastRooms[R].CellCount, Counts[R]));
			}
		}
		break;
	}
	default:
		break;
	}

	Out.bPassed = Out.Failures.Num() == 0;
}

bool FChunkGenerationVerifier::VerifyMasks(const FChunkGenerationData& Data, const TArray<uint8>& WallMask,
	const TArray<uint8>& DoorMask, TArray<FString>& OutFailures, int32& OutChecks)
{
	OutChecks = 0;
	const int32 Expected = Data.Params.CellCount * Data.Params.CellCount;
	if (Data.Cells.Num() != Expected || WallMask.Num() != Expected || DoorMask.Num() != Expected)
	{
		OutFailures.Add(FString::Printf(TEXT("mask size mismatch cells=%d wall=%d door=%d expected=%d"),
			Data.Cells.Num(), WallMask.Num(), DoorMask.Num(), Expected));
		++OutChecks;
		return false;
	}
	for (int32 I = 0; I < Expected; ++I)
	{
		++OutChecks;
		const uint8 WantWall = Data.Cells[I].bIsWall ? 1 : 0;
		const uint8 WantDoor = Data.Cells[I].bIsDoor ? 1 : 0;
		if (WallMask[I] != WantWall)
		{
			OutFailures.Add(FString::Printf(TEXT("wall mask %d = %d expected %d"), I, WallMask[I], WantWall));
		}
		if (DoorMask[I] != WantDoor)
		{
			OutFailures.Add(FString::Printf(TEXT("door mask %d = %d expected %d"), I, DoorMask[I], WantDoor));
		}
	}
	return OutFailures.Num() == 0;
}

FGenerationVerifyReport FChunkGenerationVerifier::RunAll(const FChunkGenerationData& Data, int32 Replays)
{
	FGenerationVerifyReport Report;
	Report.ContentHash = HashCells(Data);
	Report.ReplayHash = Report.ContentHash;
	Report.ReplayCount = Replays;
	Report.bDeterministicReplay = true;

	for (int32 R = 0; R < Replays; ++R)
	{
		FChunkGenerationData Copy(Data.Coord, Data.Seed, Data.Params);
		Copy.WorldSeed = Data.WorldSeed;
		Copy.bUseWorldSeed = Data.bUseWorldSeed;
		Copy.Generate();
		const uint32 H = HashCells(Copy);
		Report.ReplayHash = H;
		if (H != Report.ContentHash)
		{
			Report.bDeterministicReplay = false;
			CompareCells(Data, Copy, Report.ReplayFailures, 8);
			break;
		}
	}

	const int32 LastStage = (int32)EGenerationStage::Masks;
	for (int32 S = 0; S < LastStage; ++S)
	{
		FStageVerifyResult StageResult;
		VerifyStage((EGenerationStage)S, Data, StageResult);
		Report.Stages.Add(StageResult);
	}

	Report.bPassed = Report.bDeterministicReplay;
	for (const FStageVerifyResult& S : Report.Stages)
	{
		if (!S.bPassed) { Report.bPassed = false; }
	}
	return Report;
}

FString FChunkGenerationVerifier::FormatStage(const FStageVerifyResult& Result)
{
	return FString::Printf(TEXT("%s: %s (%d checks, %d fail, %d warn)"),
		StageName(Result.Stage), Result.bPassed ? TEXT("PASS") : TEXT("FAIL"),
		Result.NumChecks, Result.Failures.Num(), Result.Warnings.Num());
}

FString FGenerationVerifyReport::ToString() const
{
	FString S;
	S += TEXT("=== Generation verification ===\n");
	S += FString::Printf(TEXT("content hash : %08X\n"), ContentHash);
	S += FString::Printf(TEXT("replay hash  : %08X (%d replays)\n"), ReplayHash, ReplayCount);
	S += FString::Printf(TEXT("deterministic: %s\n"), bDeterministicReplay ? TEXT("YES") : TEXT("NO"));
	for (const FString& F : ReplayFailures)
	{
		S += FString::Printf(TEXT("  replay diff: %s\n"), *F);
	}
	for (const FStageVerifyResult& Stage : Stages)
	{
		S += FString::Printf(TEXT("  %s\n"), *FChunkGenerationVerifier::FormatStage(Stage));
		for (const FString& F : Stage.Failures)
		{
			S += FString::Printf(TEXT("    FAIL: %s\n"), *F);
		}
		for (const FString& W : Stage.Warnings)
		{
			S += FString::Printf(TEXT("    warn: %s\n"), *W);
		}
	}
	S += FString::Printf(TEXT("result: %s\n"), bPassed ? TEXT("PASS") : TEXT("FAIL"));
	return S;
}

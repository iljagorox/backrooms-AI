#pragma once

#include "CoreMinimal.h"

class FChunkGenerationData;

// ---- Layer: generation verification ----
// A reusable, engine-facing verification pass for the whole chunk pipeline.
// It works only on the pure generation data (FChunkGenerationData), so the
// same checks apply to any level/profile: the level changes thresholds and
// layout, not the invariants the world must satisfy.
//
// Generation stages. A check runs right after each stage so a failure is
// attributed to the exact step that produced it instead of being lost in the
// final result. Extend the enum when the pipeline grows (geometry, props, city).
enum class EGenerationStage : uint8
{
	Density = 0,
	Walls,
	Doors,
	Rooms,
	Masks,
	Count
};

// Verification mode:
//   Off    - checks disabled (release/performance);
//   Log    - checks run, violations are only logged (default, non-fatal);
//   Strict - any violation marks the generation as failed (tests/dev).
enum class EGenerationVerifyMode : uint8
{
	Off = 0,
	Log,
	Strict
};

struct FStageVerifyResult
{
	EGenerationStage Stage = EGenerationStage::Density;
	bool bPassed = true;
	int32 NumChecks = 0;
	TArray<FString> Failures;
	TArray<FString> Warnings;
};

struct FGenerationVerifyReport
{
	bool bPassed = true;
	uint32 ContentHash = 0;
	uint32 ReplayHash = 0;
	bool bDeterministicReplay = false;
	int32 ReplayCount = 0;
	TArray<FString> ReplayFailures;
	TArray<FStageVerifyResult> Stages;

	FString ToString() const;
};

class FChunkGenerationVerifier
{
public:
	// Reads the BR.Gen.Verify console variable. Public so the generator can
	// cheaply skip work when verification is off.
	static EGenerationVerifyMode GetMode();

	static const TCHAR* StageName(EGenerationStage Stage);

	// FNV-1a over the pure cell data. Identical inputs must yield an identical
	// hash; this is the contract the replay check enforces.
	static uint32 HashCells(const FChunkGenerationData& Data);

	// Invariant checks for a single stage against already-filled data.
	static void VerifyStage(EGenerationStage Stage, const FChunkGenerationData& Data, FStageVerifyResult& Out);

	// Actor-level stage: the wall/door masks must match the generation data.
	static bool VerifyMasks(const FChunkGenerationData& Data, const TArray<uint8>& WallMask,
		const TArray<uint8>& DoorMask, TArray<FString>& OutFailures, int32& OutChecks);

	// Full report: regenerate the chunk several times and compare hashes/cells
	// (determinism), then run every stage check on the provided data.
	static FGenerationVerifyReport RunAll(const FChunkGenerationData& Data, int32 Replays = 2);

	static FString FormatStage(const FStageVerifyResult& Result);
};

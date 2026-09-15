#include "BackroomsValidator.h"
#include "BackroomsFloorPlan.h"
#include "BackroomsLocationArchetype.h"

FBackroomsValidationResult UBackroomsValidator::Validate(const UBackroomsFloorPlan* FloorPlan, const UBackroomsLocationArchetype* Archetype)
{
	if (!FloorPlan)
	{
		FBackroomsValidationResult Result;
		Result.bHardPass = false;
		Result.FailedHardConstraints.Add(TEXT("NoFloorPlan"));
		return Result;
	}

	FBackroomsValidationResult Result = FloorPlan->Validate();

	// Пороги из архетипа имеют приоритет над дефолтными из FloorPlan.
	if (Archetype)
	{
		Result.MazeThreshold = Archetype->MazeScoreThreshold;
		Result.RepetitionThreshold = Archetype->RepetitionScoreThreshold;
		Result.OpennessThreshold = Archetype->OpennessScoreThreshold;
		Result.DeadEndThreshold = Archetype->DeadEndRatioThreshold;
		Result.ContinuityThreshold = Archetype->FlowContinuityThreshold;
	}

	return Result;
}

TArray<FHardConstraintCheckResult> UBackroomsValidator::CheckHardConstraints(const UBackroomsFloorPlan* FloorPlan)
{
	TArray<FHardConstraintCheckResult> Checks;

	if (!FloorPlan)
	{
		FHardConstraintCheckResult& NoPlan = Checks.AddDefaulted_GetRef();
		NoPlan.ConstraintName = TEXT("NoFloorPlan");
		NoPlan.bPassed = false;
		NoPlan.Details = TEXT("FloorPlan is null");
		return Checks;
	}

	const FBackroomsValidationResult Result = FloorPlan->Validate();

	if (Result.FailedHardConstraints.Num() == 0)
	{
		FHardConstraintCheckResult& Passed = Checks.AddDefaulted_GetRef();
		Passed.ConstraintName = TEXT("AllHardConstraints");
		Passed.bPassed = true;
		Passed.Details = TEXT("All hard constraints satisfied");
		return Checks;
	}

	for (const FName& Name : Result.FailedHardConstraints)
	{
		FHardConstraintCheckResult& Check = Checks.AddDefaulted_GetRef();
		Check.ConstraintName = Name.ToString();
		Check.bPassed = false;
		Check.Details = Name.ToString();
	}

	return Checks;
}

FSoftMetricResults UBackroomsValidator::ScoreSoftMetrics(const UBackroomsFloorPlan* FloorPlan, const UBackroomsLocationArchetype* Archetype)
{
	FSoftMetricResults Metrics;

	if (!FloorPlan)
	{
		return Metrics;
	}

	const FBackroomsValidationResult Result = FloorPlan->Validate();

	Metrics.MazeScore = Result.MazeScore;
	Metrics.RepetitionScore = Result.RepetitionScore;
	Metrics.OpennessScore = Result.OpennessScore;
	Metrics.DeadEndRatio = Result.DeadEndRatio;
	Metrics.FlowContinuityScore = Result.FlowContinuityScore;

	if (Archetype)
	{
		Metrics.MazeThreshold = Archetype->MazeScoreThreshold;
		Metrics.RepetitionThreshold = Archetype->RepetitionScoreThreshold;
		Metrics.OpennessThreshold = Archetype->OpennessScoreThreshold;
		Metrics.DeadEndThreshold = Archetype->DeadEndRatioThreshold;
		Metrics.ContinuityThreshold = Archetype->FlowContinuityThreshold;
	}

	return Metrics;
}
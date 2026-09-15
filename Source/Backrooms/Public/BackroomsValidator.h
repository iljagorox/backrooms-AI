#pragma once

#include "CoreMinimal.h"
#include "BackroomsFloorPlan.h"
#include "BackroomsLocationArchetype.h"
#include "BackroomsValidator.generated.h"

// -------------------------------------------------------
// Step H: FBackroomsValidation — HARD constraints + SOFT quality metrics
// 
// Two categories as per Correction 11:
//   HARD constraints — if any fail, layout is rejected outright.
//   SOFT quality metrics — scored, rejection only if exceeding
//   configurable archetype thresholds.
// -------------------------------------------------------

// -------------------------------------------------------
// FHardConstraintCheckResult — result of an individual hard constraint check
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FHardConstraintCheckResult
{
	GENERATED_BODY()

	UPROPERTY()
	FString ConstraintName;

	UPROPERTY()
	bool bPassed;

	UPROPERTY()
	FString Details; // human-readable failure reason
};

// -------------------------------------------------------
// FSoftMetricResults — soft metric scoring results
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FSoftMetricResults
{
	GENERATED_BODY()

	UPROPERTY()
	float MazeScore = 1.0f;       // 0.0 = heavy maze/dungeon, 1.0 = open flow

	UPROPERTY()
	float RepetitionScore = 1.0f; // 0.0 = highly repetitive, 1.0 = varied

	UPROPERTY()
	float OpennessScore = 0.5f;   // 0.0 = cramped, 1.0 = open

	UPROPERTY()
	float DeadEndRatio = 0.0f;    // 0.0 = no dead ends, 1.0 = all dead ends

	UPROPERTY()
	float FlowContinuityScore = 1.0f; // 0.0 = broken flow, 1.0 = continuous flow

	// For reference only — actual comparison done by caller with archetype thresholds
	UPROPERTY()
	float MazeThreshold = 0.3f;

	UPROPERTY()
	float RepetitionThreshold = 0.5f;

	UPROPERTY()
	float OpennessThreshold = 0.4f;

	UPROPERTY()
	float DeadEndThreshold = 0.15f;

	UPROPERTY()
	float ContinuityThreshold = 0.7f;
};

// -------------------------------------------------------
// UBackroomsValidator — Validation component
// -------------------------------------------------------
UCLASS()
class BACKROOMS_API UBackroomsValidator : public UObject
{
	GENERATED_BODY()

public:
	// -------------------------------------------------------
	// Validate a FloorPlan against HARD constraints and SOFT metrics.
	// -------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Validation")
	static FBackroomsValidationResult Validate(
		const UBackroomsFloorPlan* FloorPlan,
		const UBackroomsLocationArchetype* Archetype = nullptr);

	// -------------------------------------------------------
	// Validate individual hard constraints, returning details.
	// Individual checks can be enabled/disabled per archetype.
	// -------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Validation")
	TArray<FHardConstraintCheckResult> CheckHardConstraints(
		const UBackroomsFloorPlan* FloorPlan);

	// -------------------------------------------------------
	// Soft metric scoring — results are stored, actual rejection
	// decision uses configurable thresholds from archetype.
	// -------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Validation")
	FSoftMetricResults ScoreSoftMetrics(
		const UBackroomsFloorPlan* FloorPlan,
		const UBackroomsLocationArchetype* Archetype = nullptr);
};

// -------------------------------------------------------
// Validation check results — used internally
// -------------------------------------------------------
UENUM(BlueprintType)
enum class EValidationCheck : uint8
{
	// Topology
	CheckConnectivity           UMETA(DisplayName = "Connectivity"),
	CheckMinimumDimensions      UMETA(DisplayName = "MinDimensions"),
	CheckNoOverlaps             UMETA(DisplayName = "NoOverlaps"),

	// Boundary contracts
	CheckContractsSatisfied     UMETA(DisplayName = "ContractsSatisfied"),
	CheckValidOpenings          UMETA(DisplayName = "ValidOpenings"),

	// Structural/spatial
	CheckMinimumSpan            UMETA(DisplayName = "MinSpan"),
	CheckStructuralPlausibility UMETA(DisplayName = "StructuralPlausibility"),

	// Count
	CheckCount
};

// String names for validation checks
inline const FString& ValidationCheckName(EValidationCheck Check)
{
	static const FString Names[] = {
		TEXT("Connectivity"),
		TEXT("MinDimensions"),
		TEXT("NoOverlaps"),
		TEXT("ContractsSatisfied"),
		TEXT("ValidOpenings"),
		TEXT("MinSpan"),
		TEXT("StructuralPlausibility")
	};
	return Names[(int32)Check];
}


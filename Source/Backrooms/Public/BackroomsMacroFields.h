#pragma once

#include "CoreMinimal.h"
#include "BackroomsMacroFields.generated.h"

// -------------------------------------------------------
// FBackroomsMacroFields — slowly varying coherent fields sampled
// over world coordinates. "Still the same place, but gradually
// changing": no square biome borders, openness decays across
// hundreds of meters instead of snapping at a region boundary.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FBackroomsMacroFields
{
	GENERATED_BODY()

	// 0.0 = cramped, 1.0 = huge open halls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MacroFields")
	float Openness = 0.5f;

	// 0.0 = chaotic, 1.0 = strict architectural grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MacroFields")
	float Regularity = 0.5f;

	// 0.0 = dark, 1.0 = bright (affects light density)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MacroFields")
	float Brightness = 0.5f;

	// Wear and tear of the environment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MacroFields")
	float Decay = 0.0f;

	// Wet surfaces, stains, damp air
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MacroFields")
	float Moisture = 0.0f;

	// 0.0 = sparse supports, 1.0 = dense column forest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MacroFields")
	float StructuralDensity = 0.5f;

	FBackroomsMacroFields() = default;
};
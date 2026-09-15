#pragma once

#include "CoreMinimal.h"
#include "BackroomsHandles.h"
#include "BackroomsSurfaceType.generated.h"

// -------------------------------------------------------
// EBackroomsSurfaceType — semantic type of a wall/floor/ceiling surface.
// Used by the material system to look up appropriate master materials.
// Architecture defines the type; material palette assigns the visual.
// -------------------------------------------------------
UENUM(BlueprintType)
enum class EBackroomsSurfaceType : uint8
{
	Floor       UMETA(DisplayName = "Floor"),
	Wall        UMETA(DisplayName = "Wall"),
	Ceiling     UMETA(DisplayName = "Ceiling"),
	Column      UMETA(DisplayName = "Column"),
	DoorFrame   UMETA(DisplayName = "DoorFrame"),
	Trim        UMETA(DisplayName = "Trim"),
	Structural  UMETA(DisplayName = "Structural"),
	WetSurface  UMETA(DisplayName = "WetSurface"),

	Count
};

// -------------------------------------------------------
// FGeneratedSurface — surface data generated during mesh building,
// sourced from FloorPlan semantics. Does NOT contain architectural
// topology — only rendering primitives with semantic tags.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FGeneratedSurface
{
	GENERATED_BODY()

	// Semantic type of this surface
	UPROPERTY()
	EBackroomsSurfaceType Type = EBackroomsSurfaceType::Wall;

	// Owner space — stable handle, survives reallocations/serialization
	UPROPERTY()
	FBackroomsSpaceHandle Space;

	// Unique surface ID within the space
	UPROPERTY()
	int32 SurfaceId = INDEX_NONE;

	// World position of the surface (derived from FloorPlan geometry)
	UPROPERTY()
	FVector WorldPosition;

	// Surface normal (derived from FloorPlan geometry)
	UPROPERTY()
	FVector Normal;

	// -------------------------------------------------------
	// Surface parameters for material lookup
	// -------------------------------------------------------
	UPROPERTY()
	float Age = 0.0f;         // how "old" the surface is (from MacroFields)

	UPROPERTY()
	float Dirt = 0.0f;        // dirt amount (0.0 = clean, 1.0 = very dirty)

	UPROPERTY()
	float Moisture = 0.0f;    // moisture/wetness (0.0 = dry, 1.0 = wet)

	UPROPERTY()
	float Damage = 0.0f;      // damage level

	UPROPERTY()
	float Discoloration = 0.0f;  // color fading/aging

	UPROPERTY()
float Roughness = 0.5f;   // roughness 0.0 = mirror, 1.0 = rough
};
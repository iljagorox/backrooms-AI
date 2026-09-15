#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BackroomsChunkCoord.h"
#include "BackroomsTopology.h"
#include "BackroomsMacroFields.h"
#include "BackroomsLocationArchetype.generated.h"

// -------------------------------------------------------
// EBackroomsSpaceRole — minimal architectural roles for Level 0.
// Do NOT over-classify into: OpenOffice, BreakRoom, Lobby, CubicleFarm, etc.
// Instead use ambiguous architectural roles that describe shape/function,
// not modern office job functions. Specialization can be added later by
// grammar expansion, not by rewriting FloorPlan/SpatialGraph.
// -------------------------------------------------------
UENUM(BlueprintType)
enum class EBackroomsSpaceRole : uint8
{
	// Large readable areas — the "main" spaces people walk through
	OpenSpace       UMETA(DisplayName = "OpenSpace"),

	// Spaces divided into sections, possibly with partitions or columns
	PartitionedSpace UMETA(DisplayName = "PartitionedSpace"),

	// Connecting areas between larger spaces — corridors, passages, transitional zones
	TransitionSpace UMETA(DisplayName = "TransitionSpace"),

	// Utility / functional areas — storage, service rooms, mechanical, etc.
	ServiceSpace    UMETA(DisplayName = "ServiceSpace"),

	// Void spaces for structural support — columns, load-bearing areas, voids
	StructuralVoid  UMETA(DisplayName = "StructuralVoid"),

	Count
};

// -------------------------------------------------------
// FSpaceRoleDef — definition of a space role with generation rules
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FSpaceRoleDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceRoleDef")
	EBackroomsSpaceRole Role = EBackroomsSpaceRole::OpenSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceRoleDef")
	float PreferredMinCells = 4;      // min cells for this role

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceRoleDef")
	float PreferredMaxCells = 64;     // preferred max cells

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceRoleDef")
	float MaxCells = 128;             // hard max

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceRoleDef")
	float ColumnSpacing = 0.0f;       // 0 = no columns preferred

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceRoleDef")
	float bPrefersColumns = 0.0f;     // 0.0 = no, 1.0 = yes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceRoleDef")
	float OpennessBias = 0.5f;        // 0.0 = cramped, 1.0 = open

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceRoleDef")
	float CeilingHeightBias = 0.5f;   // 0.0 = low, 1.0 = high

	FSpaceRoleDef() = default;
};

// -------------------------------------------------------
// FMacroFieldConfig — configuration for a slowly varying coherent field
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FMacroFieldConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MacroFieldConfig")
	FName FieldName;                  // e.g., "Openness", "Regularity", "Brightness"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MacroFieldConfig")
	float FieldStrength = 1.0f;       // how strongly this field influences generation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MacroFieldConfig")
	float CorrelationLength = 32.0f;  // how many regions influence each other (slow variation)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MacroFieldConfig")
	float WorldScale = 1.0f;          // scaling of world coords for the field

	FMacroFieldConfig() = default;
};

// -------------------------------------------------------
// FBackroomsLocationArchetype — Level 0 grammar / generation rules
// 
// Contains:
// - Space role definitions (5 minimal roles, not over-classified)
// - Macro field configs (slowly varying coherent fields)
// - Opening generation rules
// - Structural support rules
// - All thresholds are configurable — not hardcoded design truths
// -------------------------------------------------------
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew)
class BACKROOMS_API UBackroomsLocationArchetype : public UDataAsset
{
	GENERATED_BODY()

public:
	// -------------------------------------------------------
	// Space role definitions — 5 minimal roles for Level 0
	// -------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Archetype|SpaceRoles")
	TArray<FSpaceRoleDef> SpaceRoleDefinitions;

	// -------------------------------------------------------
	// Macro field configs — slowly varying coherent fields
	// Ensure: "This is still the same place, but it is gradually changing."
	// Do NOT produce abrupt random biome changes at every Region boundary.
	// -------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Archetype|MacroFields")
	TArray<FMacroFieldConfig> MacroFieldConfigs;

	// -------------------------------------------------------
	// Opening generation rules
	// -------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Archetype|Openings")
	float MinOpeningWidth = 2.0f;     // minimum portal width in cells

	UPROPERTY(EditAnywhere, Category = "Archetype|Openings")
	float MaxOpeningWidth = 8.0f;     // maximum portal width in cells

	UPROPERTY(EditAnywhere, Category = "Archetype|Openings")
	float DoorWidth = 2.0f;           // standard door width

	UPROPERTY(EditAnywhere, Category = "Archetype|Openings")
	float ArchwayWidth = 4.0f;        // archway width

	// -------------------------------------------------------
	// Structural support rules
	// -------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Archetype|Structure")
	float ColumnSpacingDefault = 8.0f;     // default column spacing in cells

	UPROPERTY(EditAnywhere, Category = "Archetype|Structure")
	float MaxSpanWithoutColumn = 6.0f;     // spans > this need columns

	UPROPERTY(EditAnywhere, Category = "Archetype|Structure")
	float ColumnHeightFraction = 0.8f;     // fraction of WallHeight

	// -------------------------------------------------------
	// Validation thresholds (soft metrics — configurable, not hard truths)
	// -------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Archetype|Validation")
	float MazeScoreThreshold = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Archetype|Validation")
	float RepetitionScoreThreshold = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Archetype|Validation")
	float OpennessScoreThreshold = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Archetype|Validation")
	float DeadEndRatioThreshold = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Archetype|Validation")
	float FlowContinuityThreshold = 0.7f;

	// -------------------------------------------------------
	// Level identity
	// -------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Identity")
	FString LevelName = TEXT("L0");

	UPROPERTY(EditAnywhere, Category = "Identity")
	FString DisplayName = TEXT("Liminal Office");

	// -------------------------------------------------------
	// Generated role lookup by index (plain C++: struct-by-ref cannot
	// be a UFUNCTION return, and role iteration is a floor-plan concern).
	// -------------------------------------------------------
	const FSpaceRoleDef& GetRoleDef(EBackroomsSpaceRole Role) const
	{
		static const FSpaceRoleDef Fallback;
		for (const FSpaceRoleDef& Def : SpaceRoleDefinitions)
		{
			if (Def.Role == Role)
			{
				return Def;
			}
		}
		return Fallback;
	}

	// -------------------------------------------------------
	// Get role def by index (for iteration)
	// -------------------------------------------------------
	int32 GetRoleDefCount() const { return SpaceRoleDefinitions.Num(); }

	// -------------------------------------------------------
	// Get role name display string (plain C++: string lookup helper)
	// -------------------------------------------------------
	FString RoleName(EBackroomsSpaceRole Role) const
	{
		switch (Role)
		{
			case EBackroomsSpaceRole::OpenSpace: return TEXT("OpenSpace");
			case EBackroomsSpaceRole::PartitionedSpace: return TEXT("PartitionedSpace");
			case EBackroomsSpaceRole::TransitionSpace: return TEXT("TransitionSpace");
			case EBackroomsSpaceRole::ServiceSpace: return TEXT("ServiceSpace");
			case EBackroomsSpaceRole::StructuralVoid: return TEXT("StructuralVoid");
			default: return TEXT("Unknown");
		}
	}
};
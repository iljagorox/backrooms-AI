#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PropDatabase.generated.h"

UENUM(BlueprintType)
enum class EPropPlacement : uint8
{
	Floor     UMETA(DisplayName = "Floor"),
	Wall      UMETA(DisplayName = "Wall"),
	Ceiling   UMETA(DisplayName = "Ceiling"),
	Corner    UMETA(DisplayName = "Corner")
};

USTRUCT(BlueprintType)
struct FPropRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PropID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPropPlacement Placement = EPropPlacement::Floor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Probability = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bBlocking = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D ScaleMin = FVector2D(1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D ScaleMax = FVector2D(1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxDensity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> RequiredRoomTags;
};

UCLASS(BlueprintType)
class BACKROOMS_API UPropDatabase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
	TArray<FPropRule> Props;

	const FPropRule* FindPropByName(FName PropID) const;
	TArray<const FPropRule*> GetPropsForPlacement(EPropPlacement Placement) const;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("PropDatabase", FName("Default"));
	}
};

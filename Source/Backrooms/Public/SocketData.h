#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SocketData.generated.h"

UENUM(BlueprintType)
enum class ESocketDirection : uint8
{
	North  UMETA(DisplayName = "North (+Y)"),
	South  UMETA(DisplayName = "South (-Y)"),
	East   UMETA(DisplayName = "East (+X)"),
	West   UMETA(DisplayName = "West (-X)")
};

UENUM(BlueprintType)
enum class ERoomSocketType : uint8
{
	Door        UMETA(DisplayName = "Door"),
	Corridor    UMETA(DisplayName = "Corridor"),
	Threshold   UMETA(DisplayName = "Threshold"),
	Event       UMETA(DisplayName = "Event")
};

USTRUCT(BlueprintType)
struct FSocketDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SocketID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESocketDirection Direction = ESocketDirection::North;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERoomSocketType Type = ERoomSocketType::Door;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 CellOffset = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Width = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Height = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> FrameMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> DoorMesh;
};

UCLASS(BlueprintType)
class BACKROOMS_API USocketData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	FName SocketID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	ESocketDirection Direction = ESocketDirection::North;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	ERoomSocketType Type = ERoomSocketType::Door;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	TArray<FSocketDefinition> Variants;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	float SpawnWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	bool bRequiresLight = true;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("SocketData", SocketID);
	}
};

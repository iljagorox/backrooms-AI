#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BackroomsCellType.h"
#include "RoomData.generated.h"

UENUM(BlueprintType)
enum class ERoomSize : uint8
{
	Small    UMETA(DisplayName = "Small (1-4 cells)"),
	Medium   UMETA(DisplayName = "Medium (5-9 cells)"),
	Large    UMETA(DisplayName = "Large (10-16 cells)")
};

USTRUCT(BlueprintType)
struct FRoomPropEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Probability = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bBlocking = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D OffsetMin = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D OffsetMax = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct FRoomLightEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Intensity = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Radius = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ZOffset = 250.f;
};

UCLASS(BlueprintType)
class BACKROOMS_API URoomData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	FName RoomID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	ERoomSize Size = ERoomSize::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TArray<ECellType> AllowedCellTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
	TArray<FRoomPropEntry> Props;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lights")
	TArray<FRoomLightEntry> Lights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Events")
	TArray<FName> PossibleEvents;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	float SpawnWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	bool bCanHaveDoors = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	int32 MinAdjacentRooms = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	int32 MaxAdjacentRooms = 4;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("RoomData", RoomID);
	}
};

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoomArchetype.generated.h"

class URoomData;

USTRUCT(BlueprintType)
struct FArchetypeRoomWeight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<URoomData> RoomData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight = 1.0f;
};

UCLASS(BlueprintType)
class BACKROOMS_API URoomArchetype : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Archetype")
	FName ArchetypeID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Archetype")
	FName DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
	TArray<FArchetypeRoomWeight> RoomWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
	int32 MinRoomsPerChunk = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
	int32 MaxRoomsPerChunk = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
	float DoorChance = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
	float CorridorLengthMin = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
	float CorridorLengthMax = 6.f;

	const URoomData* PickRoom(FRandomStream& Rng) const;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("RoomArchetype", ArchetypeID);
	}
};

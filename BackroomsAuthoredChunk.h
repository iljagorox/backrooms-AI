#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "BackroomsAuthoredChunk.generated.h"

UENUM(BlueprintType)
enum class EBackroomsRoomIdea : uint8
{
	OrdinaryOffice,
	BrokenOffice,
	EmptyRoom,
	RestRoom,
	Archive,
	Storage,
	Technical,
	LongCorridor,
	FurnitureRoom,
	EventRoom,
	Custom
};

UENUM(BlueprintType)
enum class EBackroomsSocketKind : uint8
{
	Trash,
	Important,
	Furniture,
	Light,
	Event,
	Monster,
	Entrance,
	Exit
};

USTRUCT(BlueprintType)
struct FBackroomsAuthoredSocket
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Socket")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Socket")
	EBackroomsSocketKind Kind = EBackroomsSocketKind::Trash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Socket")
	FTransform LocalTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Socket")
	FName Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Socket")
	bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct FBackroomsAuthoredRoom
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
	EBackroomsRoomIdea Idea = EBackroomsRoomIdea::OrdinaryOffice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
	FBox LocalBounds = FBox(EForceInit::ForceInitToZero);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
	TArray<FBackroomsAuthoredSocket> Sockets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
	TArray<FName> AllowedEventTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
	bool bCanSpawnMonster = false;
};

UCLASS(BlueprintType)
class BACKROOMS_API UBackroomsAuthoredChunk : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunk")
	FName ChunkId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunk")
	int32 LevelIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunk")
	FIntPoint SizeCells = FIntPoint(8, 8);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunk")
	float CellSize = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunk")
	float FloorZ = 0.0f;

	// The mesh/actors are authored in the asset. Generation only selects the
	// variant and fills its explicit sockets.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunk")
	TSoftObjectPtr<UStaticMesh> PreviewMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rooms")
	TArray<FBackroomsAuthoredRoom> Rooms;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Connections")
	TArray<FBackroomsAuthoredSocket> BoundarySockets;

	bool SupportsLevel(int32 InLevelIndex) const { return LevelIndex == InLevelIndex; }
};

USTRUCT()
struct FBackroomsSeedContext
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Seed = 0;

	UPROPERTY()
	int32 LevelIndex = 0;

	UPROPERTY()
	FIntPoint ChunkCoord = FIntPoint::ZeroValue;

	uint32 MakeStreamSeed(uint32 Purpose) const;
};

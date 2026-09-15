#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BackroomsAuthoredChunk.h"
#include "BackroomsAuthoredChunkLibrary.generated.h"

/**
 * Per-level authored chunk catalogue.
 *
 * Geometry is never generated here. The catalogue only exposes authored
 * variants and performs deterministic selection from the single world seed.
 */
UCLASS(BlueprintType)
class BACKROOMS_API UBackroomsAuthoredChunkLibrary : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Authored Chunks")
	TArray<TSoftObjectPtr<UBackroomsAuthoredChunk>> Chunks;

	/** Returns loaded variants belonging to the requested level. */
	void GetCandidates(int32 InLevelIndex, TArray<UBackroomsAuthoredChunk*>& OutCandidates) const;

	/**
	 * Deterministically selects one authored variant.
	 * Purpose must identify the selection stage (architecture, event room, etc.).
	 */
	UBackroomsAuthoredChunk* Select(int32 InSeed, int32 InLevelIndex, const FIntPoint& InChunkCoord, uint32 Purpose) const;
};

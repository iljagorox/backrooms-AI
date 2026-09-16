#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BackroomsAuthoredChunk.h"
#include "BackroomsAuthoredChunkLibrary.generated.h"

/** Deterministic catalogue of complete authored chunk templates. */
UCLASS(BlueprintType)
class BACKROOMS_API UBackroomsAuthoredChunkLibrary : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Authored Chunks")
    TArray<TSoftObjectPtr<UBackroomsAuthoredChunk>> Chunks;

    /** Returns only valid templates for the requested level. */
    void GetCandidates(int32 InLevelIndex, TArray<UBackroomsAuthoredChunk*>& OutCandidates) const;

    /** Deterministic selection. The seed is the only source of variation. */
    UBackroomsAuthoredChunk* Select(int32 InSeed, int32 InLevelIndex, const FIntPoint& InChunkCoord, uint32 Purpose) const;

    /** Validates the complete authored catalogue and returns human-readable errors. */
    bool Validate(FString& OutReport) const;

private:
    static bool ValidateChunk(const UBackroomsAuthoredChunk* Chunk, FString& OutError);
};

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BackroomsAuthoredRuntimeSubsystem.generated.h"

class ABackroomsChunkActor;
class UBackroomsAuthoredChunk;
class UBackroomsAuthoredChunkLibrary;

UCLASS()
class BACKROOMS_API UBackroomsAuthoredRuntimeSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return true; }
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

private:
    void ProcessChunk(ABackroomsChunkActor* Chunk);
    UBackroomsAuthoredChunkLibrary* LoadLibrary();
    bool BuildAuthoredChunk(ABackroomsChunkActor* Chunk, UBackroomsAuthoredChunk* Authored);
    void FillSockets(ABackroomsChunkActor* Chunk, UBackroomsAuthoredChunk* Authored);
    void ClearLegacyOwnedActors(ABackroomsChunkActor* Chunk) const;

    UPROPERTY()
    TMap<TObjectPtr<ABackroomsChunkActor>, FName> ProcessedChunks;

    UPROPERTY()
    TObjectPtr<UBackroomsAuthoredChunkLibrary> CachedLibrary;

    bool bLibraryLoadAttempted = false;
};

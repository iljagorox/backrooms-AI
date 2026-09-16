#include "BackroomsAuthoredChunkLibrary.h"

namespace
{
    static bool HasBoundaryKind(const UBackroomsAuthoredChunk* Chunk, EBackroomsSocketKind Kind)
    {
        for (const FBackroomsAuthoredSocket& Socket : Chunk->BoundarySockets)
        {
            if (Socket.bEnabled && Socket.Kind == Kind)
            {
                return true;
            }
        }
        return false;
    }
}

bool UBackroomsAuthoredChunkLibrary::ValidateChunk(const UBackroomsAuthoredChunk* Chunk, FString& OutError)
{
    if (!Chunk)
    {
        OutError = TEXT("Null chunk reference");
        return false;
    }

    if (Chunk->ChunkId.IsNone())
    {
        OutError = TEXT("ChunkId is empty");
        return false;
    }

    if (Chunk->LevelIndex < 0 || Chunk->SizeCells.X <= 0 || Chunk->SizeCells.Y <= 0 || Chunk->CellSize <= 0.0f)
    {
        OutError = FString::Printf(TEXT("Invalid dimensions or level: %s"), *Chunk->ChunkId.ToString());
        return false;
    }

    if (!Chunk->PreviewMesh.IsValid() && Chunk->PreviewMesh.IsNull())
    {
        OutError = FString::Printf(TEXT("Missing geometry mesh: %s"), *Chunk->ChunkId.ToString());
        return false;
    }

    if (!HasBoundaryKind(Chunk, EBackroomsSocketKind::Entrance) || !HasBoundaryKind(Chunk, EBackroomsSocketKind::Exit))
    {
        OutError = FString::Printf(TEXT("Chunk must have enabled Entrance and Exit sockets: %s"), *Chunk->ChunkId.ToString());
        return false;
    }

    for (const FBackroomsAuthoredRoom& Room : Chunk->Rooms)
    {
        if (Room.Id.IsNone())
        {
            OutError = FString::Printf(TEXT("Room with empty Id in chunk: %s"), *Chunk->ChunkId.ToString());
            return false;
        }
    }

    return true;
}

void UBackroomsAuthoredChunkLibrary::GetCandidates(int32 InLevelIndex, TArray<UBackroomsAuthoredChunk*>& OutCandidates) const
{
    OutCandidates.Reset();

    for (const TSoftObjectPtr<UBackroomsAuthoredChunk>& SoftChunk : Chunks)
    {
        UBackroomsAuthoredChunk* Chunk = SoftChunk.LoadSynchronous();
        FString Error;
        if (Chunk && Chunk->SupportsLevel(InLevelIndex) && ValidateChunk(Chunk, Error))
        {
            OutCandidates.Add(Chunk);
        }
    }

    OutCandidates.Sort([](const UBackroomsAuthoredChunk& A, const UBackroomsAuthoredChunk& B)
    {
        return A.ChunkId.ToString() < B.ChunkId.ToString();
    });
}

UBackroomsAuthoredChunk* UBackroomsAuthoredChunkLibrary::Select(int32 InSeed, int32 InLevelIndex, const FIntPoint& InChunkCoord, uint32 Purpose) const
{
    TArray<UBackroomsAuthoredChunk*> Candidates;
    GetCandidates(InLevelIndex, Candidates);
    if (Candidates.IsEmpty())
    {
        return nullptr;
    }

    const FBackroomsSeedContext Context{InSeed, InLevelIndex, InChunkCoord};
    FRandomStream Stream(Context.MakeStreamSeed(Purpose));
    return Candidates[Stream.RandRange(0, Candidates.Num() - 1)];
}

bool UBackroomsAuthoredChunkLibrary::Validate(FString& OutReport) const
{
    TArray<FString> Errors;
    TSet<FString> SeenIds;

    for (const TSoftObjectPtr<UBackroomsAuthoredChunk>& SoftChunk : Chunks)
    {
        UBackroomsAuthoredChunk* Chunk = SoftChunk.LoadSynchronous();
        FString Error;
        if (!ValidateChunk(Chunk, Error))
        {
            Errors.Add(Error);
            continue;
        }

        const FString Id = Chunk->ChunkId.ToString();
        if (SeenIds.Contains(Id))
        {
            Errors.Add(FString::Printf(TEXT("Duplicate ChunkId: %s"), *Id));
        }
        SeenIds.Add(Id);
    }

    if (Errors.IsEmpty())
    {
        OutReport = FString::Printf(TEXT("Authored catalogue valid: %d templates"), Chunks.Num());
        return true;
    }

    OutReport = FString::Join(Errors, TEXT("\n"));
    return false;
}

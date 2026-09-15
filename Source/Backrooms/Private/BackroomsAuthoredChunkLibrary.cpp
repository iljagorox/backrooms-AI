#include "BackroomsAuthoredChunkLibrary.h"

void UBackroomsAuthoredChunkLibrary::GetCandidates(
	int32 InLevelIndex,
	TArray<UBackroomsAuthoredChunk*>& OutCandidates) const
{
	OutCandidates.Reset();

	for (const TSoftObjectPtr<UBackroomsAuthoredChunk>& SoftChunk : Chunks)
	{
		UBackroomsAuthoredChunk* Chunk = SoftChunk.LoadSynchronous();
		if (Chunk && Chunk->SupportsLevel(InLevelIndex))
		{
			OutCandidates.Add(Chunk);
		}
	}

	// Asset-array order must not become hidden randomness. Keep the candidate
	// order stable so the same world seed keeps selecting the same authored
	// variant even after assets are rearranged in the DataAsset.
	OutCandidates.Sort([](const UBackroomsAuthoredChunk& A, const UBackroomsAuthoredChunk& B)
	{
		return A.ChunkId.ToString() < B.ChunkId.ToString();
	});
}

UBackroomsAuthoredChunk* UBackroomsAuthoredChunkLibrary::Select(
	int32 InSeed,
	int32 InLevelIndex,
	const FIntPoint& InChunkCoord,
	uint32 Purpose) const
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

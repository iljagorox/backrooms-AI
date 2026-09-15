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

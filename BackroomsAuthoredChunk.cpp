#include "BackroomsAuthoredChunk.h"

uint32 FBackroomsSeedContext::MakeStreamSeed(uint32 Purpose) const
{
	// All randomness in authored generation derives from the single world seed.
	// Level/chunk/purpose only derive independent deterministic streams from it.
	uint32 H = HashCombineFast(::GetTypeHash(Seed), ::GetTypeHash(LevelIndex));
	H = HashCombineFast(H, ::GetTypeHash(ChunkCoord));
	H = HashCombineFast(H, Purpose);
	return H;
}

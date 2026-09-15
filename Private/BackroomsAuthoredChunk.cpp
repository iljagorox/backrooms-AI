#include "BackroomsAuthoredChunk.h"

uint32 FBackroomsSeedContext::MakeStreamSeed(uint32 Purpose) const
{
	// The world seed is the only entropy source. Level/chunk/purpose merely
	// derive independent deterministic streams from that same seed.
	uint32 H = HashCombineFast(::GetTypeHash(Seed), ::GetTypeHash(LevelIndex));
	H = HashCombineFast(H, ::GetTypeHash(ChunkCoord));
	H = HashCombineFast(H, Purpose);
	return H;
}

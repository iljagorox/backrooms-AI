#include "RoomArchetype.h"
#include "RoomData.h"

const URoomData* URoomArchetype::PickRoom(FRandomStream& Rng) const
{
	if (RoomWeights.Num() == 0)
	{
		return nullptr;
	}

	float TotalWeight = 0.f;
	for (const FArchetypeRoomWeight& RW : RoomWeights)
	{
		if (RW.RoomData)
		{
			TotalWeight += RW.Weight;
		}
	}

	if (TotalWeight <= 0.f)
	{
		return nullptr;
	}

	float Roll = Rng.FRand() * TotalWeight;
	for (const FArchetypeRoomWeight& RW : RoomWeights)
	{
		if (!RW.RoomData)
		{
			continue;
		}
		Roll -= RW.Weight;
		if (Roll <= 0.f)
		{
			return RW.RoomData;
		}
	}

	return RoomWeights.Last().RoomData;
}

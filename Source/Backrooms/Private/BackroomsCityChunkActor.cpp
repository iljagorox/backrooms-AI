#include "BackroomsCityChunkActor.h"
#include "BackroomsChunkActor.h"

ABackroomsCityChunkActor::ABackroomsCityChunkActor()
{
}

void ABackroomsCityChunkActor::InitCityOnly()
{
}

void ABackroomsCityChunkActor::BuildCityGeometry()
{
}

void ABackroomsCityChunkActor::PlaceCityProps(int32 CellX, int32 CellY, FRandomStream& Random)
{
}

void ABackroomsCityChunkActor::ClearCityData()
{
}

// Город-остров целиком строит базовый ABackroomsChunkActor (BuildSpawnPlatform
// при bSpawnPlatform=true) — как в HEAD до выделения городского класса. Здесь
// только делегирование в базу: свой city-конвейер пока не реализован.
void ABackroomsCityChunkActor::BuildChunk(int32 InChunkX, int32 InChunkY, int32 InSeed, const TObjectPtr<UPropDatabase>& InDatabase)
{
	ABackroomsChunkActor::BuildChunk(InChunkX, InChunkY, InSeed, InDatabase);
}

void ABackroomsCityChunkActor::BuildChunkFromProfile(int32 InChunkX, int32 InChunkY, int32 InSeed, const ULevelGeneratorProfile* InProfile)
{
	ABackroomsChunkActor::BuildChunkFromProfile(InChunkX, InChunkY, InSeed, InProfile);
}
#pragma once

#include "CoreMinimal.h"
#include "BackroomsChunkActor.h"
#include "BackroomsCityChunkActor.generated.h"

UCLASS()
class BACKROOMS_API ABackroomsCityChunkActor : public ABackroomsChunkActor
{
	GENERATED_BODY()

public:
	ABackroomsCityChunkActor();

	// City-specific initialization - minimal, no backrooms generation
	virtual void InitCityOnly();

	// Build only city layout: roads, sidewalks, buildings
	// Completely skips backrooms wall/room/props generation
	virtual void BuildCityGeometry();

	// Place only city props (street lamps, benches, etc.)
	// Skips backrooms prop database, scatter mask, room logic
	virtual void PlaceCityProps(int32 CellX, int32 CellY, FRandomStream& Random);

	// Clear only city-specific mesh data
	virtual void ClearCityData();

	// City layout parameters
	UPROPERTY(EditAnywhere, Category = "City")
	float CityRoadWidth = 400.0f;

	UPROPERTY(EditAnywhere, Category = "City")
	float CitySidewalkWidth = 180.0f;

	UPROPERTY(EditAnywhere, Category = "City")
	float CityBlockSize = 1800.0f;

	// Half extent of the city chunk (used for metrics calculations)
	UPROPERTY(EditAnywhere, Category = "City")
	float HalfExtent = 5000.0f;

	// City prop placement
	UPROPERTY(EditAnywhere, Category = "City")
	float CityPropChance = 0.3f;

	// Overrides for city-only generation - skip backrooms density/room/props
	virtual void BuildChunk(int32 InChunkX, int32 InChunkY, int32 InSeed, const TObjectPtr<UPropDatabase>& InDatabase) override;
	virtual void BuildChunkFromProfile(int32 InChunkX, int32 InChunkY, int32 InSeed, const ULevelGeneratorProfile* InProfile) override;
};
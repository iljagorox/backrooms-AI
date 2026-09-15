#pragma once

#include "CoreMinimal.h"
#include "BackroomsCellType.generated.h"

UENUM(BlueprintType)
enum class ECellType : uint8
{
	Empty      UMETA(DisplayName = "Empty"),
	Floor      UMETA(DisplayName = "Floor"),
	Wall       UMETA(DisplayName = "Wall"),
	Door       UMETA(DisplayName = "Door"),
	Corridor   UMETA(DisplayName = "Corridor"),
	Room       UMETA(DisplayName = "Room"),
	LivingRoom UMETA(DisplayName = "LivingRoom"),
	Threshold  UMETA(DisplayName = "Threshold")
};

inline bool CellTypeIsSolid(ECellType Type)
{
	return Type == ECellType::Wall;
}

inline bool CellTypeIsPassable(ECellType Type)
{
	return Type == ECellType::Floor || Type == ECellType::Door
		|| Type == ECellType::Corridor || Type == ECellType::Room
		|| Type == ECellType::LivingRoom;
}

inline bool CellTypeIsDoor(ECellType Type)
{
	return Type == ECellType::Door;
}

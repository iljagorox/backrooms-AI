#pragma once

#include "CoreMinimal.h"
#include "BackroomsCellType.h"
#include "BackroomsChunkCell.generated.h"

USTRUCT(BlueprintType)
struct FChunkCell
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float Density = 0.f;

	UPROPERTY(BlueprintReadOnly)
	ECellType CellType = ECellType::Empty;

	UPROPERTY(BlueprintReadOnly)
	bool bIsWall = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDoor = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsScatter = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLivingRoom = false;

	UPROPERTY(BlueprintReadOnly)
	int32 RoomIndex = INDEX_NONE;

	// Вид комнаты (ERoomVariant) для этой клетки. Не UPROPERTY: это производная
	// «точка зрения» на мир (pure function от координаты), а не сериализуемые
	// данные. Хранится байтом, чтобы не тянуть UHT-зависимость на enum.
	uint8 RoomVariant = 0;
};

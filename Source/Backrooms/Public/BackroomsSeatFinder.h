#pragma once

#include "CoreMinimal.h"

// Точка/место, куда монстр может сесть: найдено авто-детекцией по инстанс-мешам.
struct FBackroomsSeat
{
	// Валидное ли место.
	bool bValid = false;
	// Мировой центр сиденья (верхняя грань объекта).
	FVector Center = FVector::ZeroVector;
	// Точка на полу ПЕРЕД объектом (монстр идёт сюда, затем садится).
	FVector ApproachPoint = FVector::ZeroVector;
	// Высота сиденья над полом (см).
	float SeatHeightCm = 0.0f;
	// Мировая Z верхней грани сиденья (см).
	float SeatTopZ = 0.0f;
	// Направление «вперёд» от сиденья (куда монстр должен смотреть, сев).
	float FacingYaw = 0.0f;
};

namespace BackroomsSeatFinder
{
	// Ищет ближайшую «сидячую» мебель в радиусе: по инстанс-мешам в мире
	// (стул/скамейка/диван/стол/коробка — высота верхней грани 45..85 см).
	// Возвращает ближайшее к Origin валидное место либо bValid=false.
	BACKROOMS_API FBackroomsSeat FindSeatNear(UWorld* World, const FVector& Origin, float SearchRadius);
}
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BackroomsCameraProp.generated.h"

class UStaticMeshComponent;

// Тело персонажа (view-model) от первого лица: статичная модель,
// прикреплённая к CameraComponent игрока. Пока модель не переигнута
// в скелетную — анимационное дерево на неё навесить нельзя (см. справку).
UCLASS()
class BACKROOMS_API ABackroomsCameraProp : public AActor
{
	GENERATED_BODY()

public:
	ABackroomsCameraProp();

	// Меш тела.
	UPROPERTY(VisibleAnywhere, Category = "Body")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	// Ассет тела (по умолчанию — импортированный персонаж SM_Character).
	UPROPERTY(EditAnywhere, Category = "Body")
	TSoftObjectPtr<UStaticMesh> BodyMeshAsset;

	// Смещение/поворот относительно камеры игрока.
	UPROPERTY(EditAnywhere, Category = "Body")
	FTransform CameraOffset;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void TryAttachToPlayer();
	bool bAttached = false;
};
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BackroomsExitActor.generated.h"

class UBoxComponent;
class UPointLightComponent;
class UStaticMeshComponent;

// Обычная на вид дверь, которая становится переходом только после того, как
// режиссёр пространства разместил её в ещё не показанном игроку чанке.
UCLASS()
class BACKROOMS_API ABackroomsExitActor : public AActor
{
	GENERATED_BODY()

public:
	ABackroomsExitActor();

	void ConfigureExit(int32 InNextLevelIndex);

protected:
	UFUNCTION()
	void OnExitOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Trigger;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> GuidanceLight;

	UPROPERTY(EditAnywhere, Category = "Exit")
	int32 NextLevelIndex = 1;

	bool bConsumed = false;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BackroomsHeldItem.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UItemDataAsset;

// Реальный 3D-предмет в руке (Аналог BP_HeldItem): спавнится при экипировке
// слота, крепится к сокету руки/камере и плавно «вырастает» из нуля до
// HeldScale (0.25 c). Меш и флаги берутся из UItemDataAsset.
UCLASS()
class BACKROOMS_API ABackroomsHeldItem : public AActor
{
	GENERATED_BODY()

public:
	ABackroomsHeldItem();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Статданные экипированного предмета (меш, масштаб, флаги).
	UPROPERTY(BlueprintReadOnly, Category = "Held")
	TObjectPtr<UItemDataAsset> ItemData;

	// Можно ли осматривать, держа предмет в руке (Alt).
	UPROPERTY(BlueprintReadOnly, Category = "Held")
	bool bCanBeInspected = true;

	// Можно ли выбросить предмет из руки (ПКМ).
	UPROPERTY(BlueprintReadOnly, Category = "Held")
	bool bCanBeThrown = true;

	// Длительность плавного появления предмета в руке (сек).
	UPROPERTY(EditDefaultsOnly, Category = "Held")
	float AppearDuration = 0.25f;

	// Меш предмета (корень): сортаются коллизия/физика, крутится при осмотре.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// Точка вращения при осмотре (под мешем, без перекоса масштаба).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Held")
	TObjectPtr<USceneComponent> Pivot;

	// Настроить предмет по данным и запустить появление.
	void Initialize(UItemDataAsset* Data);

private:
	FVector TargetScale = FVector::OneVector;
	bool bAppearing = false;
	float AppearTimer = 0.0f;
};
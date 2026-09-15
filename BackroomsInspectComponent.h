#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BackroomsInspectComponent.generated.h"

class UItemDataAsset;
class UStaticMeshComponent;
class USceneComponent;
class ABackroomsPlayerCharacter;

// Осмотр предмета — режим-стейт на персонаже (не отдельный мир). Предмет
// сажается на InspectRoot, прицепленный к камере, ввод движения блокируется;
// мышь вращает предмет, колесо меняет дистанцию. Оборван осмотром-тайником не
// является: любой урон/атака во время осмотра вызывает ForceCancelInspect.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BACKROOMS_API UBackroomsInspectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBackroomsInspectComponent();

	// Начать осмотр предмета по ID каталога (UItemDataAsset). true — осмотр идёт.
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	bool StartInspect(FName ItemId);

	// Закончить осмотр (отпускание клавиши / ESC / Interact).
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void EndInspect();

	// Принудительное прерывание: вызывается уроном/атакой сущности.
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void ForceCancelInspect();

	UFUNCTION(BlueprintPure, Category = "Inspect")
	bool IsInspecting() const { return bActive; }

	UFUNCTION(BlueprintPure, Category = "Inspect")
	FName GetItemId() const { return ItemId; }

	UFUNCTION(BlueprintPure, Category = "Inspect")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintPure, Category = "Inspect")
	FString GetDescription() const { return Description; }

	// Оси ввода: мышь крутит предмет, колесо — дистанция. Вызываются из
	// Turn/LookUp/Zoom игрока (вместо камеры).
	void InputYaw(float Value);
	void InputPitch(float Value);
	void InputZoom(float Value);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// Событие окончания осмотра (любым путём), парметр — принудительное ли.
	DECLARE_MULTICAST_DELEGATE_OneParam(FBackroomsInspectEnded, bool);
	FBackroomsInspectEnded OnInspectEnded;

	// Дистанция осмотра (см).
	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	float MinDistance = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	float MaxDistance = 80.0f;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	float ZoomStep = 20.0f;          // см за «щелчок» колеса

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	float PitchClamp = 80.0f;        // ограничение тангажа (±)

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	float MouseRotateSpeed = 0.35f;

	void EnsureRig(ABackroomsPlayerCharacter* Owner);
	void ApplyPlacement(ABackroomsPlayerCharacter* Owner);

	bool bActive = false;
	FName ItemId = NAME_None;
	FText DisplayName;
	FString Description;

	TObjectPtr<USceneComponent> InspectRoot;
	TObjectPtr<UStaticMeshComponent> ItemMesh;
	FRotator Rotation = FRotator::ZeroRotator;
	float Distance = 50.0f;

	// Здоровье на момент старта: упало — осмотр срывается (не safe-room).
	float HealthAtInspectStart = -1.0f;
};
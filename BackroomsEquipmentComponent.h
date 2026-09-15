#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BackroomsEquipmentComponent.generated.h"

class UItemDataAsset;
class ABackroomsPlayerCharacter;

// Объявление о смене экипировки (надели/сняли предмет).
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBackroomsEquipChanged);

// Состояние «предмет в руках» (§8.3): какая вещь экипирована, монтажи Equip/Use
// из данных предмета и фонарик-предмет (bFlashlight). Сам игрок по-прежнему
// владеет HeldItemActor (спавн/уничтожение) и батарейкой; компонент дополняет
// экипировку данными и реакциями, чтобы логика руки была в одном месте.
UCLASS(ClassGroup = (Backrooms), meta = (BlueprintSpawnableComponent))
class BACKROOMS_API UBackroomsEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBackroomsEquipmentComponent();

	// Экипировать слот: запомнить предмет, сыграть EquipMontage (если задан),
	// для фонаря-предмета включить свет. HeldActor — уже прикреплённый к руке
	// ABackroomsHeldItem (или nullptr для не-акторных слотов).
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipItem(FName ItemId, UItemDataAsset* Data, AActor* HeldActor);

	// Снять предмет из рук: погасить фонарь-предмет, сбросить состояние.
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void Unequip();

	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FBackroomsEquipChanged OnEquipChanged;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsEquipped() const { return bEquipped && EquippedData != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Equipment")
	FName GetEquippedItemId() const { return EquippedItemId; }

	// Фонарик как предмет: экипирован ли такой и включён ли его свет.
	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsFlashlightItemEquipped() const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsFlashlightItemOn() const { return bFlashlightItemOn; }

	// Вкл/выкл фонаря-предмета (кнопка фонарика при экипированном фонарике).
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void ToggleFlashlightItem();

	// Принудительно погасить (батарея села, режим сменился).
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void ForceFlashlightItemOff();

	// Монтаж использования при применении расходника (UseMontage из данных).
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void PlayUseMontage();

private:
	// Пробросить видимость камерного луча игроку (через его SetFlashlightFromItem).
	void UpdateEquippedFlashlight();
	ABackroomsPlayerCharacter* GetPlayer() const;

	UPROPERTY()
	TObjectPtr<UItemDataAsset> EquippedData;

	UPROPERTY()
	TObjectPtr<AActor> EquippedHeldActor;

	bool bEquipped = false;
	bool bFlashlightItemOn = false;
	FName EquippedItemId = NAME_None;
};
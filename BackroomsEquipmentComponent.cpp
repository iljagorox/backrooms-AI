#include "BackroomsEquipmentComponent.h"

#include "BackroomsPlayerCharacter.h"
#include "BackroomsInventoryData.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/UObjectGlobals.h"

UBackroomsEquipmentComponent::UBackroomsEquipmentComponent()
{
	// Сам игрок гоняет drain батареи в своём Tick; тут события по требованию.
	PrimaryComponentTick.bCanEverTick = false;
}

bool UBackroomsEquipmentComponent::IsFlashlightItemEquipped() const
{
	return IsEquipped() && EquippedData->bFlashlight;
}

ABackroomsPlayerCharacter* UBackroomsEquipmentComponent::GetPlayer() const
{
	return Cast<ABackroomsPlayerCharacter>(GetOwner());
}

void UBackroomsEquipmentComponent::UpdateEquippedFlashlight()
{
	if (ABackroomsPlayerCharacter* P = GetPlayer())
	{
		P->SetFlashlightFromItem(bFlashlightItemOn);
	}
}

void UBackroomsEquipmentComponent::EquipItem(FName ItemId, UItemDataAsset* Data, AActor* HeldActor)
{
	bEquipped = true;
	EquippedItemId = ItemId;
	EquippedData = Data;
	EquippedHeldActor = HeldActor;

	// EquipMontage — опциональный ассет в данных предмета; без него ничего не
	// играем (движение руки уже задано сокетом/мешем).
	if (Data && !Data->EquipMontage.IsNull())
	{
		if (UAnimMontage* Montage = Data->EquipMontage.LoadSynchronous())
		{
			if (ABackroomsPlayerCharacter* P = GetPlayer())
			{
				if (UAnimInstance* Anim = P->GetMesh() ? P->GetMesh()->GetAnimInstance() : nullptr)
				{
					Anim->Montage_Play(Montage, 1.0f);
				}
			}
		}
	}

	// Фонарик-предмет: экипировка сразу включает свет (пока батарея позволяет).
	if (IsFlashlightItemEquipped())
	{
		bFlashlightItemOn = true;
		UpdateEquippedFlashlight();
	}
	OnEquipChanged.Broadcast();
}

void UBackroomsEquipmentComponent::Unequip()
{
	if (!bEquipped)
	{
		return;
	}
	if (bFlashlightItemOn)
	{
		bFlashlightItemOn = false;
		UpdateEquippedFlashlight();
	}
	bEquipped = false;
	EquippedData = nullptr;
	EquippedHeldActor = nullptr;
	EquippedItemId = NAME_None;
	OnEquipChanged.Broadcast();
}

void UBackroomsEquipmentComponent::ToggleFlashlightItem()
{
	if (!IsFlashlightItemEquipped())
	{
		return;
	}
	bFlashlightItemOn = !bFlashlightItemOn;
	UpdateEquippedFlashlight();
}

void UBackroomsEquipmentComponent::ForceFlashlightItemOff()
{
	if (bFlashlightItemOn)
	{
		bFlashlightItemOn = false;
		UpdateEquippedFlashlight();
	}
}

void UBackroomsEquipmentComponent::PlayUseMontage()
{
	if (!EquippedData || EquippedData->UseMontage.IsNull())
	{
		return;
	}
	if (UAnimMontage* Montage = EquippedData->UseMontage.LoadSynchronous())
	{
		if (ABackroomsPlayerCharacter* P = GetPlayer())
		{
			if (UAnimInstance* Anim = P->GetMesh() ? P->GetMesh()->GetAnimInstance() : nullptr)
			{
				Anim->Montage_Play(Montage, 1.0f);
			}
		}
	}
}
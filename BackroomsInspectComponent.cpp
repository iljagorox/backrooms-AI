#include "BackroomsInspectComponent.h"
#include "BackroomsPlayerCharacter.h"
#include "BackroomsItemSystem.h"
#include "BackroomsInventoryData.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Engine/StaticMesh.h"

UBackroomsInspectComponent::UBackroomsInspectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UBackroomsInspectComponent::EnsureRig(ABackroomsPlayerCharacter* Owner)
{
	if (!Owner || !Owner->GetCameraComponent())
	{
		return;
	}

	if (!InspectRoot)
	{
		InspectRoot = NewObject<USceneComponent>(Owner, TEXT("InspectRoot"));
		InspectRoot->SetupAttachment(Owner->GetCameraComponent());
		InspectRoot->RegisterComponentWithWorld(Owner->GetWorld());
		InspectRoot->SetRelativeLocation(FVector::ZeroVector);
		InspectRoot->SetRelativeRotation(FRotator::ZeroRotator);
	}

	if (!ItemMesh)
	{
		ItemMesh = NewObject<UStaticMeshComponent>(Owner, TEXT("InspectItemMesh"));
		ItemMesh->SetupAttachment(InspectRoot);
		ItemMesh->RegisterComponentWithWorld(Owner->GetWorld());
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetGenerateOverlapEvents(false);
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetMobility(EComponentMobility::Movable);
	}
}

bool UBackroomsInspectComponent::StartInspect(FName Id)
{
	if (bActive || Id.IsNone())
	{
		return false;
	}

	ABackroomsPlayerCharacter* Owner = Cast<ABackroomsPlayerCharacter>(GetOwner());
	if (!Owner)
	{
		return false;
	}

	UItemDataAsset* Asset = UBackroomsItemSystem::GetItemDataAsset(Id);
	UStaticMesh* Mesh = Asset ? Asset->GetWorldMesh() : nullptr;
	if (!Mesh)
	{
		return false;
	}

	EnsureRig(Owner);
	if (!InspectRoot || !ItemMesh)
	{
		return false;
	}

	// Блок ввода движения: мышь и колесо теперь уходят в осмотр, камера стоит.
	if (AController* C = Owner->GetController())
	{
		C->SetIgnoreMoveInput(true);
	}

	ItemMesh->SetStaticMesh(Mesh);
	// Приводим предмет к «витринному» размеру ~30 см по наибольшей оси.
	UStaticMesh* SM = Mesh;
	const FBoxSphereBounds B = SM->GetBounds();
	const float MaxExtent = FMath::Max3(B.BoxExtent.X, B.BoxExtent.Y, B.BoxExtent.Z);
	const float FitScale = (MaxExtent > 1.0f) ? 15.0f / MaxExtent : 0.2f;
	ItemMesh->SetRelativeScale3D(FVector(FitScale));
	ItemMesh->SetUsingAbsoluteRotation(false);
	ItemMesh->SetUsingAbsoluteLocation(false);

	Rotation = FRotator::ZeroRotator;
	Distance = FMath::Clamp(((MinDistance + MaxDistance) * 0.5f), MinDistance, MaxDistance);
	ItemMesh->SetRelativeRotation(Rotation);
	ItemMesh->SetRelativeLocation(FVector(Distance, 0.0f, -10.0f));
	ItemMesh->SetVisibility(true, true);

	bActive = true;
	ItemId = Id;
	DisplayName = Asset->DisplayName;
	Description = Asset->Description.ToString();
	HealthAtInspectStart = Owner->GetHealth();

	SetComponentTickEnabled(true);
	return true;
}

void UBackroomsInspectComponent::EndInspect()
{
	if (!bActive)
	{
		return;
	}

	bActive = false;
	SetComponentTickEnabled(false);

	ABackroomsPlayerCharacter* Owner = Cast<ABackroomsPlayerCharacter>(GetOwner());
	if (Owner)
	{
		if (AController* C = Owner->GetController())
		{
			C->SetIgnoreMoveInput(false);
		}
	}

	if (ItemMesh)
	{
		ItemMesh->SetVisibility(false, true);
	}
	ItemId = NAME_None;
	DisplayName = FText::GetEmpty();
	Description.Empty();

	// Сначала гасим уведомлением того, кто ждал (в т.ч. PlayerCharacter).
	OnInspectEnded.Broadcast(false);
}

void UBackroomsInspectComponent::ForceCancelInspect()
{
	// Тот же путь, но с флагом «принудительно» — урон/атака.
	if (!bActive)
	{
		return;
	}
	bActive = false;
	SetComponentTickEnabled(false);

	ABackroomsPlayerCharacter* Owner = Cast<ABackroomsPlayerCharacter>(GetOwner());
	if (Owner)
	{
		if (AController* C = Owner->GetController())
		{
			C->SetIgnoreMoveInput(false);
		}
	}
	if (ItemMesh)
	{
		ItemMesh->SetVisibility(false, true);
	}
	ItemId = NAME_None;
	DisplayName = FText::GetEmpty();
	Description.Empty();

	OnInspectEnded.Broadcast(true);
}

void UBackroomsInspectComponent::InputYaw(float Value)
{
	if (!bActive)
	{
		return;
	}
	Rotation.Yaw += Value * MouseRotateSpeed * 120.0f;
	if (ItemMesh)
	{
		ItemMesh->SetRelativeRotation(Rotation);
	}
}

void UBackroomsInspectComponent::InputPitch(float Value)
{
	if (!bActive)
	{
		return;
	}
	Rotation.Pitch = FMath::Clamp(Rotation.Pitch + Value * MouseRotateSpeed * 120.0f, -PitchClamp, PitchClamp);
	if (ItemMesh)
	{
		ItemMesh->SetRelativeRotation(Rotation);
	}
}

void UBackroomsInspectComponent::InputZoom(float Value)
{
	if (!bActive)
	{
		return;
	}
	Distance = FMath::Clamp(Distance - Value * ZoomStep, MinDistance, MaxDistance);
	if (ItemMesh)
	{
		ItemMesh->SetRelativeLocation(FVector(Distance, 0.0f, -10.0f));
	}
}

void UBackroomsInspectComponent::ApplyPlacement(ABackroomsPlayerCharacter* Owner)
{
	// Камера двигается (head bob/дыхание) — тянем предмет следом каждый кадр.
	if (ItemMesh)
	{
		ItemMesh->SetRelativeLocation(FVector(Distance, 0.0f, -10.0f));
		ItemMesh->SetRelativeRotation(Rotation);
	}
}

void UBackroomsInspectComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ABackroomsPlayerCharacter* Owner = Cast<ABackroomsPlayerCharacter>(GetOwner());
	if (!Owner || !bActive)
	{
		SetComponentTickEnabled(false);
		return;
	}

	// Осмотр не должен быть safe-room: урон во время осмотра срывает его.
	if (HealthAtInspectStart > 0.0f && Owner->GetHealth() < HealthAtInspectStart - 0.01f)
	{
		ForceCancelInspect();
		return;
	}

	ApplyPlacement(Owner);
}
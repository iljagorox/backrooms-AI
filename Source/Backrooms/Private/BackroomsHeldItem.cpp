#include "BackroomsHeldItem.h"
#include "BackroomsInventoryData.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ABackroomsHeldItem::ABackroomsHeldItem()
{
	PrimaryActorTick.bCanEverTick = true;

	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	SetRootComponent(Pivot);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(Pivot);
	// Коллизия и физика отключаются в BeginPlay (предмет — только визуал в руке).
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABackroomsHeldItem::BeginPlay()
{
	Super::BeginPlay();

	// Предмет в руке не должен ни с чем сталкиваться и подчиняться физике.
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetEnableGravity(false);
}

void ABackroomsHeldItem::Initialize(UItemDataAsset* Data)
{
	ItemData = Data;
	if (!ItemData)
	{
		bCanBeInspected = false;
		bCanBeThrown = false;
		return;
	}

	UStaticMesh* Mesh = ItemData->GetHeldMesh();
	if (Mesh)
	{
		MeshComp->SetStaticMesh(Mesh);
	}

	// Масштаб в руке: HeldScale из данных, умноженный на нормализацию меша
	// (если ассет огромный — приводим к ~30 см, чтобы у лица не торчал шкаф).
	TargetScale = ItemData->HeldScale;
	if (Mesh)
	{
		const FBoxSphereBounds Bounds = Mesh->GetBounds();
		const float MaxDim = 2.0f * Bounds.BoxExtent.GetMax();
		if (MaxDim > 30.0f)
		{
			const float Normalizer = 30.0f / MaxDim;
			TargetScale *= Normalizer;
		}
	}
	TargetScale.X = FMath::Max(TargetScale.X, 0.01f);
	TargetScale.Y = FMath::Max(TargetScale.Y, 0.01f);
	TargetScale.Z = FMath::Max(TargetScale.Z, 0.01f);

	MeshComp->SetRelativeScale3D(TargetScale * 0.001f);
	bAppearing = true;
	AppearTimer = 0.0f;
}

void ABackroomsHeldItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bAppearing)
	{
		return;
	}
	AppearTimer += DeltaTime;
	const float K = FMath::Clamp(AppearTimer / FMath::Max(0.001f, AppearDuration), 0.0f, 1.0f);
	// Быстрый старт, плавное завершение: предмет «вырастает» в руке.
	const float Smooth = K * K * (3.0f - 2.0f * K);
	MeshComp->SetRelativeScale3D(FMath::Lerp(TargetScale * 0.001f, TargetScale, Smooth));
	if (K >= 1.0f)
	{
		MeshComp->SetRelativeScale3D(TargetScale);
		bAppearing = false;
	}
}
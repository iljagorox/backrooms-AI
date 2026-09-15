#include "BackroomsCameraProp.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ABackroomsCameraProp::ABackroomsCameraProp()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.2f;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	RootComponent = BodyMesh;
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetCastShadow(false);
	BodyMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	// Импортированная модель персонажа.
	BodyMeshAsset = TSoftObjectPtr<UStaticMesh>(
		FSoftObjectPath(TEXT("/Game/Custom/Character/SM_Character.SM_Character")));

	// Тело «под камерой»: слегка вперёд/вниз и чуть наклонено (ползущая фигура).
	// Поворот по Yaw=90 (модель лежит вдоль своей Y-оси) прижимает её к взгляду.
	CameraOffset = FTransform(
		FRotator(40.0f, 90.0f, 0.0f),
		FVector(30.0f, 0.0f, -58.0f),
		FVector::OneVector);
}

void ABackroomsCameraProp::BeginPlay()
{
	Super::BeginPlay();
	TryAttachToPlayer();
}

void ABackroomsCameraProp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bAttached)
	{
		TryAttachToPlayer();
	}
}

void ABackroomsCameraProp::TryAttachToPlayer()
{
	if (bAttached)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn)
	{
		return;
	}

	UCameraComponent* Camera = Pawn->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		return;
	}

	if (!BodyMesh->GetStaticMesh())
	{
		if (UStaticMesh* Mesh = BodyMeshAsset.LoadSynchronous())
		{
			BodyMesh->SetStaticMesh(Mesh);
		}
	}

	AttachToComponent(Camera, FAttachmentTransformRules::SnapToTargetIncludingScale);
	SetActorRelativeTransform(CameraOffset);
	bAttached = true;
}
#include "BackroomsExitActor.h"

#include "BackroomsLevelBook.h"
#include "BackroomsProgression.h"
#include "BackroomsSaveSubsystem.h"
#include "BackroomsWorldGenerator.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ABackroomsExitActor::ABackroomsExitActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);
	// QueryAndPhysics на CDO дергает физический материал до инициализации
	// GEngine (native CDO construction) и заливает лог ошибками. Коллизию
	// выставляем на реальных инстансах — там движок уже готов.
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DoorMesh->SetCollisionObjectType(ECC_WorldStatic);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DoorFinder(TEXT("/Game/Props/Door_Prop.Door_Prop"));
	if (DoorFinder.Succeeded())
	{
		DoorMesh->SetStaticMesh(DoorFinder.Object);
	}

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitTrigger"));
	Trigger->SetupAttachment(DoorMesh);
	Trigger->SetBoxExtent(FVector(120.0f, 90.0f, 150.0f));
	Trigger->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ABackroomsExitActor::OnExitOverlap);

	GuidanceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GuidanceLight"));
	GuidanceLight->SetupAttachment(DoorMesh);
	GuidanceLight->SetRelativeLocation(FVector(0.0f, 0.0f, 190.0f));
	GuidanceLight->SetIntensity(350.0f);
	GuidanceLight->SetAttenuationRadius(550.0f);
	GuidanceLight->SetLightColor(FLinearColor(1.0f, 0.82f, 0.45f));
	GuidanceLight->SetCastShadows(true);
}

void ABackroomsExitActor::ConfigureExit(int32 InNextLevelIndex)
{
	NextLevelIndex = FMath::Max(0, InNextLevelIndex);
}

void ABackroomsExitActor::OnExitOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bConsumed || !OtherActor)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	bConsumed = true;
	if (ABackroomsWorldGenerator* Generator = Cast<ABackroomsWorldGenerator>(
		UGameplayStatics::GetActorOfClass(this, ABackroomsWorldGenerator::StaticClass())))
	{
		// Забег завершён: зафиксировать очки/лидерборд до перехода.
		if (UGameInstance* GI = Generator->GetGameInstance())
		{
			if (UBackroomsProgression* Prog = GI->GetSubsystem<UBackroomsProgression>())
			{
				Prog->NotifyDescended(NextLevelIndex);
				Prog->EndRun(true);
				Prog->BeginRun(Generator->Seed, NextLevelIndex);
			}

			// Сохраняем прогресс ДО перехода: на фиксированных картах SetLevel
			// генератора отключён guard'ом — продолжение живёт в сейве.
			if (UBackroomsSaveSubsystem* SaveSys = GI->GetSubsystem<UBackroomsSaveSubsystem>())
			{
				SaveSys->SaveProgress(Generator->Seed, NextLevelIndex);
			}
		}

		// L0..L9 — отдельные карты (/Game/Runtime/Lvl_L*): переход через OpenLevel
		// с seed в URL (см. generator BeginPlay). Старшие уровни без карт (L10+) —
		// legacy SetLevel в той же карте.
		if (NextLevelIndex >= 0 && NextLevelIndex <= 9)
		{
			const FString Options = FString::Printf(TEXT("seed=%d&skipmenu"), Generator->Seed);
			UGameplayStatics::OpenLevel(this, *UBackroomsLevelBook::GetMapName(NextLevelIndex), true, *Options);
		}
		else
		{
			Generator->SetLevel(NextLevelIndex);
		}
	}
	Destroy();
}

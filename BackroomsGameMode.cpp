#include "BackroomsGameMode.h"
#include "BackroomsPlayerCharacter.h"
#include "BackroomsMainMenuWidget.h"
#include "BackroomsChunkActor.h"
#include "BackroomsRiggedMonster.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"

namespace
{
	static void StabilizeGeneratedWorld(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		// FloorPlan currently builds the floor as a procedural mesh. Keep a thin,
		// deterministic simple floor collider under each generated Backrooms chunk.
		// This is deliberately separate from visual geometry: it guarantees that
		// ACharacter floor detection cannot lose the floor because of procedural
		// mesh collision cooking/section state.
		for (TActorIterator<ABackroomsChunkActor> It(World); It; ++It)
		{
			ABackroomsChunkActor* Chunk = *It;
			if (!Chunk || Chunk->bSpawnPlatform)
			{
				continue;
			}

			UBoxComponent* FloorCollider = nullptr;
			for (UActorComponent* Component : Chunk->GetComponents())
			{
				if (UBoxComponent* Box = Cast<UBoxComponent>(Component))
				{
					if (Box->ComponentTags.Contains(TEXT("BackroomsFloorCollision")))
					{
						FloorCollider = Box;
						break;
					}
				}
			}

			if (!FloorCollider)
			{
				FloorCollider = NewObject<UBoxComponent>(Chunk, TEXT("GeneratedFloorCollision"));
				if (!FloorCollider)
				{
					continue;
				}
				FloorCollider->ComponentTags.Add(TEXT("BackroomsFloorCollision"));
				FloorCollider->SetupAttachment(Chunk->GetRootComponent());
				FloorCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				FloorCollider->SetCollisionObjectType(ECC_WorldStatic);
				FloorCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
				FloorCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
				FloorCollider->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
				FloorCollider->SetGenerateOverlapEvents(false);
				FloorCollider->RegisterComponent();
			}

			const float HalfSize = FMath::Max(50.0f, Chunk->CellSize * Chunk->ChunkSizeCells * 0.5f);
			FloorCollider->SetBoxExtent(FVector(HalfSize, HalfSize, 5.0f));
			FloorCollider->SetRelativeLocation(FVector(0.0f, 0.0f, -5.0f));
		}

		// The imported Karelia skeletal mesh is sometimes authored with its long
		// body axis in X/Y instead of UE's Z-up. Detect that from the mesh bounds
		// and rotate only the visual mesh; the Character capsule stays upright.
		for (TActorIterator<ABackroomsRiggedMonster> It(World); It; ++It)
		{
			ABackroomsRiggedMonster* Monster = *It;
			if (!Monster || Monster->IsVanished())
			{
				continue;
			}

			if (USkeletalMeshComponent* Mesh = Monster->GetMesh())
			{
				if (USkeletalMesh* Asset = Mesh->GetSkeletalMeshAsset())
				{
					const FBoxSphereBounds B = Asset->GetBounds();
					const FVector E = B.BoxExtent;
					FRotator Correction = FRotator::ZeroRotator;
					if (E.X > E.Z * 1.35f && E.X >= E.Y)
					{
						Correction = FRotator(-90.0f, 0.0f, 0.0f);
					}
					else if (E.Y > E.Z * 1.35f && E.Y > E.X)
					{
						Correction = FRotator(0.0f, 0.0f, 90.0f);
					}
					if (!Correction.IsNearlyZero())
					{
						Mesh->SetRelativeRotation(Correction);
					}
				}
			}

			// Repair an invalid Falling spawn after the generator's initial floor
			// trace. Start below the ceiling so the trace cannot hit the ceiling first.
			UCharacterMovementComponent* Move = Monster->GetCharacterMovement();
			if (!Move || !Move->IsFalling())
			{
				continue;
			}

			const FVector P = Monster->GetActorLocation();
			const float TraceTop = 250.0f;
			const FVector Start(P.X, P.Y, TraceTop);
			const FVector End(P.X, P.Y, -100.0f);
			FHitResult Hit;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(BackroomsMonsterFloorRepair), false, Monster);
			if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params) && Hit.bBlockingHit)
			{
				const float HalfHeight = Monster->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				const FVector SafeLocation = Hit.ImpactPoint + FVector(0.0f, 0.0f, HalfHeight + 2.0f);
				Monster->SetActorLocation(SafeLocation, false, nullptr, ETeleportType::TeleportPhysics);
				Move->SetMovementMode(MOVE_Walking);
				Move->StopMovementImmediately();
			}
		}
	}
}

ABackroomsGameMode::ABackroomsGameMode()
{
	DefaultPawnClass = ABackroomsPlayerCharacter::StaticClass();
}

void ABackroomsGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		// Chunks are generated asynchronously after GameMode::BeginPlay. Recheck
		// briefly while generation is active; the operation is idempotent.
		FTimerHandle StabilizeTimer;
		World->GetTimerManager().SetTimer(
			StabilizeTimer,
			FTimerDelegate::CreateLambda([World]() { StabilizeGeneratedWorld(World); }),
			0.20f,
			true);

		if (World->URL.HasOption(TEXT("skipmenu")))
		{
			return;
		}
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		MainMenuWidget = CreateWidget<UBackroomsMainMenuWidget>(PC, UBackroomsMainMenuWidget::StaticClass());
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport(1000);
			MainMenuWidget->ShowMenu(true);
		}
	}
}

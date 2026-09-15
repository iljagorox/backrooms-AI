#include "BackroomsRiggedMonster.h"
#include "BackroomsMonsterAIController.h"
#include "BackroomsSenseComponent.h"
#include "BackroomsProceduralAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Animation/AnimBlueprint.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/WidgetComponent.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

ABackroomsRiggedMonster::ABackroomsRiggedMonster()
{
	// ACharacter создаёт скелетный меш корневым (CharacterMesh0). Чтобы
	// ригинг/анимации модели сидели именно на нём, ссылаемся на него.
	SkeletalMeshComp = GetMesh();
	if (SkeletalMeshComp)
	{
		SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SkeletalMeshComp->SetGenerateOverlapEvents(true);
	}

	// Модель Karelia (женский Smiler, импортирован из glow karelia...glb через
	// конвертацию в FBX) по умолчанию. В модели рост абсурдно большой — в
	// ApplyEnemyVisuals нормализуем масштаб под MonsterVisualHeightCm.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> KareliaMesh(
		TEXT("/Game/Monsters/Karelia/karelia_the_backrooms_smiler_nsfw.karelia_the_backrooms_smiler_nsfw"));
	if (KareliaMesh.Succeeded())
	{
		EnemySkeletalMesh = KareliaMesh.Object;
	}
	else
	{
		// Фолбэк: старая модель Smiler.
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> SmilerMesh(
			TEXT("/Game/Monsters/Smiler/Smiler"));
		if (SmilerMesh.Succeeded())
		{
			EnemySkeletalMesh = SmilerMesh.Object;
		}
	}

	// Компактный капсульный коллайдер.
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	// Движение по нав-мешу: мягкая скорость по умолчанию.
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true;
		Move->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
		Move->MaxWalkSpeed = PatrolWalkSpeed;
	}

	// Плавающий ярлык состояния над головой.
	StateWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("StateName"));
	StateWidget->SetupAttachment(GetRootComponent());
	StateWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 190.0f));
	StateWidget->SetDrawSize(FVector2D(420.0f, 64.0f));
	StateWidget->SetGeometryMode(EWidgetGeometryMode::Plane);
	StateWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StateWidget->SetCastShadow(false);

	// Без этого монстр стоял «болваном»: контроллер никогда не садился на
	// павн и вся система чувств/AI не запускалась.
	AIControllerClass = ABackroomsMonsterAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABackroomsRiggedMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ApplyEnemyVisuals();
}

void ABackroomsRiggedMonster::BeginPlay()
{
	Super::BeginPlay();
	EnableProceduralAnimation(bProceduralAnimation);

	// Слэйт-ярлык (без UMG-ассета, как ABackroomsNoiseMarker::BeginPlay).
	SAssignNew(StateTextLabel, STextBlock)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
		.ColorAndOpacity(FLinearColor(1.0f, 0.95f, 0.80f, 0.0f))
		.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
		.ShadowOffset(FVector2D(2.0f, 2.0f))
		.Justification(ETextJustify::Center);
	if (StateWidget && StateTextLabel.IsValid())
	{
		StateWidget->SetSlateWidget(StateTextLabel.ToSharedRef());
		StateLabelBaseZ = StateWidget->GetRelativeLocation().Z;
	}
}

void ABackroomsRiggedMonster::ApplyEnemyVisuals()
{
	if (!SkeletalMeshComp)
	{
		return;
	}

	if (EnemySkeletalMesh)
	{
		// Ставим скачанную модельку (с ригингом/скелетом) на монстра.
		SkeletalMeshComp->SetSkeletalMesh(EnemySkeletalMesh);
	}

	// Нормализация масштаба: огромная модель Karelia -> рост как у игрока.
	// Ставим точкой опоры низ капсулы: меш выравниваем по ступням.
	if (bProceduralAnimation && EnemySkeletalMesh)
	{
		const float K = FMath::Max(0.01f, MonsterVisualHeightCm) / FMath::Max(1.0f, KareliaBodyHeightCm);
		SkeletalMeshComp->SetRelativeScale3D(FVector(K));
		const FBoxSphereBounds B = EnemySkeletalMesh->GetBounds();
		const float FeetLocal = B.Origin.Z - B.BoxExtent.Z; // низ меша в локальных координатах
		const float CapsuleHalf = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		SkeletalMeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -CapsuleHalf - FeetLocal * K));
	}

	// Процедурная анимация: свой AnimInstance (UBackroomsProceduralAnimInstance)
	// сам пишет ВСЮ позу кодом (женские походка/бег/осмотр) — без AnimBP.
	if (bProceduralAnimation)
	{
		SkeletalMeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		SkeletalMeshComp->SetAnimInstanceClass(UBackroomsProceduralAnimInstance::StaticClass());
		return;
	}

	// Основной путь (не-процедурный) — Animation Blueprint (как у игрока):
	// AnimBP сам берёт скорость павна и переключает Idle/Walk/Chase.
	if (!AnimBlueprint.IsNull())
	{
		if (UAnimBlueprint* ABP = AnimBlueprint.LoadSynchronous())
		{
			if (UClass* AnimClass = ABP->GeneratedClass)
			{
				SkeletalMeshComp->SetAnimInstanceClass(AnimClass);
				return;
			}
		}
	}

	// Фолбэк без AnimBP: простая зацикленная анимация покоя.
	if (IdleAnimation)
	{
		// Проигрываем анимацию покоя в цикле на скелете.
		SkeletalMeshComp->PlayAnimation(IdleAnimation, /*bLoop*/ true);
	}
}

void ABackroomsRiggedMonster::SetAggressiveMode(bool bAggressive)
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = bAggressive ? ApproachWalkSpeed : PatrolWalkSpeed;
	}
}

void ABackroomsRiggedMonster::HearNoise(const FVector& Location, float Loudness)
{
	if (bVanished)
	{
		return;
	}
	if (ABackroomsMonsterAIController* AI = Cast<ABackroomsMonsterAIController>(GetController()))
	{
		if (AI->Sense)
		{
			AI->Sense->ReportNoise(Location, Loudness);
		}
	}
}

void ABackroomsRiggedMonster::Vanish()
{
	if (bVanished)
	{
		return;
	}
	bVanished = true;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
	}
}

void ABackroomsRiggedMonster::ReappearAt(const FVector& Location)
{
	bVanished = false;
	SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetMovementMode(MOVE_Walking);
	}
}

void ABackroomsRiggedMonster::TakePunch(const FVector& HitLocation, const FVector& Direction, float Force)
{
	if (Force <= 0.0f)
	{
		return;
	}

	const FVector Dir = Direction.IsNearlyZero() ? GetActorForwardVector() : Direction.GetSafeNormal();

	// Включаем окно отброса: остаток силы будет слабее с каждым кадром.
	bKnockbackActive = true;
	KnockbackDir = Dir;
	KnockbackHitPoint = HitLocation;
	KnockbackForce = FMath::Max(KnockbackForce, Force);
	KnockbackRecoverTimer = 0.0f;

	// Анимация: меш на секунду чуть отклоняется назад при ударе.
	RecoilLean = 1.0f;

	// Лёгкий пинок капсулы (существо мощное — не уронить, только толкнуть).
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->AddImpulse(Dir * (Force * 0.002f), /*bVelocityChange*/ true);
	}

	// Если скелет уже симулируется (рагдолл) — дополнительно тянем физикой.
	if (SkeletalMeshComp && SkeletalMeshComp->IsSimulatingPhysics())
	{
		bKnockbackPhysics = true;
		SkeletalMeshComp->AddForceToAllBodiesBelow(
			Dir * (Force * 150.0f), NAME_None, /*bAccelChange*/ false, /*bIncludeSelf*/ true);
	}
}

void ABackroomsRiggedMonster::UpdateKnockback(float DeltaSeconds)
{
	if (!bKnockbackActive)
	{
		return;
	}

	KnockbackRecoverTimer += DeltaSeconds;

	// Импульс каждому кадру: остаток силы тает экспоненциально.
	if (bKnockbackPhysics && SkeletalMeshComp && SkeletalMeshComp->IsSimulatingPhysics())
	{
		const FVector Step = KnockbackDir * (KnockbackForce * DeltaSeconds * KnockbackApplyScale);
		SkeletalMeshComp->AddForceToAllBodiesBelow(
			Step * 60.0f, NAME_None, /*bAccelChange*/ false, /*bIncludeSelf*/ true);
	}

	// Затухание: F *= e^-rate*dt. При больших силах монстр проседает дольше.
	KnockbackForce *= FMath::Exp(-KnockbackDecayRate * DeltaSeconds);

	// Пока отброс активен — монстра нельзя «вести» ходьбой (он ошарашен).
	if (KnockbackRecoverTimer < KnockbackRecoverDelay)
	{
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->StopMovementImmediately();
		}
	}

	if (KnockbackForce < 1.0f || KnockbackRecoverTimer >= KnockbackRecoverDelay + 0.4f)
	{
		bKnockbackActive = false;
		KnockbackForce = 0.0f;
		DisableKnockbackPhysics();
	}
}

void ABackroomsRiggedMonster::DisableKnockbackPhysics()
{
	bKnockbackPhysics = false;
	KnockbackDir = FVector::ZeroVector;
	KnockbackRecoverTimer = 0.0f;

	if (SkeletalMeshComp && SkeletalMeshComp->IsSimulatingPhysics())
	{
		SkeletalMeshComp->SetSimulatePhysics(false);
	}
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetMovementMode(MOVE_Walking);
	}
}

void ABackroomsRiggedMonster::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Процедурная женская анимация: прокидываем режим осмотра в AnimInstance
	// (походку/бег он ведёт сам из скорости павна).
	if (bProceduralAnimation && SkeletalMeshComp && !bVanished)
	{
		if (UBackroomsProceduralAnimInstance* PAI =
			Cast<UBackroomsProceduralAnimInstance>(SkeletalMeshComp->GetAnimInstance()))
		{
			PAI->SetInspecting(bInspecting);
		}
	}

	// Плавающий ярлык состояния: текст, покачивание, билборд к камере.
	if (StateWidget && StateTextLabel.IsValid() && !bVanished)
	{
		const FText Label = FText::FromString(StateDisplayName);
		if (!Label.EqualTo(StateTextLabel->GetText()))
		{
			StateTextLabel->SetText(Label);
			StateLabelAlpha = 1.0f;
		}
		StateLabelAlpha = FMath::Max(0.45f, StateLabelAlpha - DeltaSeconds * 1.6f);
		StateTextLabel->SetColorAndOpacity(
			FLinearColor(1.0f, 0.95f, 0.80f, StateLabelAlpha));

		const float WorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		StateWidget->SetRelativeLocation(FVector(
			0.0f, 0.0f, StateLabelBaseZ + FMath::Sin(WorldTime * 1.7f) * 5.0f));

		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		FVector CamLoc = GetActorLocation();
		if (PC && PC->PlayerCameraManager)
		{
			CamLoc = PC->PlayerCameraManager->GetCameraLocation();
		}
		else if (PC && PC->GetPawn())
		{
			CamLoc = PC->GetPawn()->GetActorLocation();
		}
		const FVector ToCam = CamLoc - StateWidget->GetComponentLocation();
		if (ToCam.SizeSquared() > 1.0f)
		{
			StateWidget->SetWorldRotation(ToCam.Rotation());
		}
	}

	if (bKnockbackActive)
	{
		UpdateKnockback(DeltaSeconds);
	}

	// Пружинный возврат наклона: «отклонился» → встал прямо.
	if (RecoilLean > 0.0f)
	{
		RecoilLean = FMath::Max(0.0f, RecoilLean - DeltaSeconds * RecoilRecoveryRate);
	}
	if (SkeletalMeshComp)
	{
		const FRotator Base = SkeletalMeshComp->GetRelativeRotation();
		SkeletalMeshComp->SetRelativeRotation(
			FRotator(RecoilLean * RecoilPitchMax, Base.Yaw, Base.Roll),
			false, nullptr, ETeleportType::TeleportPhysics);
	}
}

// ---------------------------------------------------------------- осмотр

void ABackroomsRiggedMonster::SetInspectMode(bool bOn)
{
	bInspecting = bOn;
}

void ABackroomsRiggedMonster::SetSitting(bool bInSit, float InSeatHeightCm)
{
	bSittingAnim = bInSit;
	// Высота сиденья считается от низа капсулы; таз опускаем настолько,
	// чтобы бёдра легли на объект. ~100 см — высота бедра стоя у фигуры.
	SitDropCm = bInSit ? FMath::Clamp(100.0f - InSeatHeightCm, 0.0f, 70.0f) : 0.0f;

	if (!bProceduralAnimation || !SkeletalMeshComp)
	{
		return;
	}
	if (UBackroomsProceduralAnimInstance* PAI =
		Cast<UBackroomsProceduralAnimInstance>(SkeletalMeshComp->GetAnimInstance()))
	{
		PAI->SetSitting(bInSit, SitDropCm);
	}
}

void ABackroomsRiggedMonster::SetStateDisplayName(const FString& InName)
{
	StateDisplayName = InName;
}

void ABackroomsRiggedMonster::EnableProceduralAnimation(bool bEnable)
{
	bProceduralAnimation = bEnable;
	// Переприменяем визуалы: при включении ставится процедурный AnimInstance,
	// при выключении — обычный AnimBP/фолбэк.
	ApplyEnemyVisuals();
}

// Конец процедурки.

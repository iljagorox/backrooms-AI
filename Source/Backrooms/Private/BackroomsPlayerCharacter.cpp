#include "BackroomsPlayerCharacter.h"
#include "BackroomsPauseMenuWidget.h"
#include "BackroomsGameOverWidget.h"
#include "BackroomsPhysProp.h"
#include "BackroomsItemPickup.h"
#include "BackroomsItemSystem.h"
#include "BackroomsStatusComponent.h"
#include "BackroomsInspectComponent.h"
#include "BackroomsEquipmentComponent.h"
#include "BackroomsHUDWidget.h"
#include "BackroomsInventoryWidget.h"
#include "BackroomsInventoryComponent.h"
#include "BackroomsInventoryData.h"
#include "BackroomsHeldItem.h"
#include "BackroomsAchievements.h"
#include "BackroomsProgression.h"
#include "BackroomsChunkActor.h"
#include "BackroomsWorldGenerator.h"
#include "BackroomsRiggedMonster.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Math/UnrealMathUtility.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "BackroomsInputSettings.h"
#include "BackroomsLocalization.h"
#include "BackroomsQualitySettings.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/EngineTypes.h"
#include "Materials/MaterialInterface.h"

static int32 ComputeRoomID(int32 CellX, int32 CellY)
{
	// 32-битный хеш пары мировых клеток. Строка Y*10000+X коллизит на дистанциях
	// 10000 клеток в одной оси (u: (0,1) и (10000,0) дают один RoomID), а пары
	// упаковываются в int32 линейно. Перемешивание Уанга асимметрично и без
	// линейных коллизий, при этом детерминированно на произвольных координатах
	// (включая отрицательные, т.к. CellX/CellY уже в мировом пространстве).
	uint32 H = (uint32)CellX * 0x9E3779B1u ^ (uint32)CellY * 0x85EBCA6Bu;
	H ^= H >> 16;
	H *= 0x7FEB352Du;
	H ^= H >> 13;
	return (int32)H;
}

ABackroomsPlayerCharacter::ABackroomsPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);

	// Поворот капсулы от контроллера (первое лицо).
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = false;
		Move->bUseControllerDesiredRotation = true;
		Move->MaxWalkSpeed = WalkSpeed;
		Move->JumpZVelocity = 420.0f;
		Move->AirControl = 0.2f;
		// Присед: тише и ниже, чтобы прятаться от монстра.
		Move->NavAgentProps.bCanCrouch = true;
		Move->MaxWalkSpeedCrouched = WalkSpeed * 0.5f;
		Move->SetCrouchedHalfHeight(52.0f);
	}

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(CameraBaseOffset);
	Camera->bUsePawnControlRotation = true;
	// Маленькая ближняя плоскость отсечения: модель тела крепится к капсуле у
	// глаз, большая Near Clip Plane резала её пополам. Ставим в BeginPlay (в
	// конструкторе нет контроллера).

	// Точка крепления взятых предметов: перед камерой по её вперёд (+X relative),
	// чтобы взглядом вращать предмет вокруг, рассматривая со всех сторон.
	HoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HoldPoint"));
	HoldPoint->SetupAttachment(Camera);
	HoldPoint->SetRelativeLocation(FVector(HoldOffset, 0.0f, 0.0f));

	// Фонарик: пятно света вперёд по взгляду. По умолчанию выключен (F).
	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(Camera);
	Flashlight->SetRelativeLocation(FVector(10.0f, 0.0f, -8.0f));
	Flashlight->SetIntensity(9000.0f);
	Flashlight->SetAttenuationRadius(3200.0f);
	Flashlight->SetInnerConeAngle(16.0f);
	Flashlight->SetOuterConeAngle(34.0f);
	Flashlight->SetLightColor(FLinearColor(1.0f, 0.97f, 0.86f));
	Flashlight->SetCastShadows(true);
	Flashlight->SetVisibility(false);

	// Класс меню паузы по умолчанию — иначе ESC/P не открывали ничего.
	PauseMenuClass = UBackroomsPauseMenuWidget::StaticClass();

	// Экран смерти.
	GameOverWidgetClass = UBackroomsGameOverWidget::StaticClass();

	// Система состояний: живая среда (перегрев/радиация/шум/адреналин/защита).
	StatusComponent = CreateDefaultSubobject<UBackroomsStatusComponent>(TEXT("StatusComponent"));

	InspectComponent = CreateDefaultSubobject<UBackroomsInspectComponent>(TEXT("InspectComponent"));

	// Экипировка: состояние руки, монтажи Equip/Use, фонарик-предмет.
	EquipmentComponent = CreateDefaultSubobject<UBackroomsEquipmentComponent>(TEXT("EquipmentComponent"));

	// HUD (иконки состояний + шкалы) по умолчанию.
	HUDWidgetClass = UBackroomsHUDWidget::StaticClass();

// Инвентарь-бестиарий по умолчанию.
	InventoryWidgetClass = UBackroomsInventoryWidget::StaticClass();

	// Компонент инвентаря: слоты (источник правды по предметам игрока).
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	// Класс «предмета в руке» по умолчанию (BP_HeldItem).
	HeldItemClass = ABackroomsHeldItem::StaticClass();

	// Тело персонажа (скелетное, с анимациями) — видно руки и ноги.
	// Крепим к капсуле (не к камере), корень скелета на полу (Z=0): ноги/стопы
	// стоят на земле, руки машут при ходьбе, а не «плывут» вместе с наклоном
	// взгляда. Поворот капсула берёт от контроллера (первое лицо).
	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetCastShadow(true);
	BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	BodyMesh->SetRelativeRotation(FRotator::ZeroRotator);

	// Статическая пользовательская модель — единственное запасное тело игрока.
	// Она крепится к капсуле, а не перед камерой: в первом лице не заслоняет
	// экран и не дублируется с анимированным мешем.
	StaticBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticBodyMesh"));
	StaticBodyMesh->SetupAttachment(GetCapsuleComponent());
	StaticBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticBodyMesh->SetCastShadow(true);
	// Видно владельцу: в первом лице должны быть видны тело и ноги. Голова
	// геометрически совпадает с камерой — внутренние (задние) грани меша
	// отсекаются, поэтому обзор она не загораживает.
	StaticBodyMesh->SetOwnerNoSee(false);
	StaticBodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	StaticBodyMesh->SetRelativeRotation(FRotator::ZeroRotator);
	StaticBodyMesh->SetRelativeScale3D(FVector(0.9f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CharMeshFinder(TEXT("/Game/Custom/Character/SM_Character.SM_Character"));
	if (CharMeshFinder.Succeeded())
	{
		StaticBodyMesh->SetStaticMesh(CharMeshFinder.Object);
	}
	// Ригнутый пользовательский персонаж (Mixamo FBX), импортирован в проект.
	// Анимации водятся в single-node режиме (см. UpdateBodyAnimation), т.к.
	// AnimBlueprint под этот скелет отсутствует.
	BodyMeshAsset = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(
		TEXT("/Game/Custom/Character/Rigged/t_pose.t_pose")));
	AnimBlueprint = TSoftObjectPtr<UAnimBlueprint>();

	// Материал тела (текстура Mixamo). Правка слота 0 в ассете не сохранялась,
	// поэтому назначаем материал из кода при загрузке меша.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BodyMatFinder(
		TEXT("/Game/Custom/Character/M_Character.M_Character"));
	if (BodyMatFinder.Succeeded())
	{
		BodyMaterial = BodyMatFinder.Object;
	}

	IdleAnim = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
		TEXT("/Game/Custom/Character/Rigged/AS_Idle.AS_Idle")));
	WalkAnim = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
		TEXT("/Game/Custom/Character/Rigged/AS_Walk.AS_Walk")));
	RunAnim = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
		TEXT("/Game/Custom/Character/Rigged/AS_Run.AS_Run")));
	JumpUpAnim = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
		TEXT("/Game/Custom/Character/Rigged/AS_JumpUp.AS_JumpUp")));
	JumpDownAnim = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
		TEXT("/Game/Custom/Character/Rigged/AS_JumpDown.AS_JumpDown")));
	CrawlAnim = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
		TEXT("/Game/Custom/Character/Rigged/AS_Crawl.AS_Crawl")));
	PunchAnim = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
		TEXT("/Game/Custom/Character/Rigged/AS_Punch.AS_Punch")));
	DieAnim = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
		TEXT("/Game/Custom/Character/Rigged/AS_Die.AS_Die")));

	// Скелетную модель не подменяем чужим манекеном из демо-пака. Если у
	// пользователя есть ригнутый персонаж, он назначается в BodyMeshAsset в
	// классе/Blueprint и становится единственным анимированным телом.
}

void ABackroomsPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Near clip plane 1 cm: SetNearClipPlane удалён в UE5.7.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->ConsoleCommand(TEXT("r.NearClipPlane 0.5"));
	}

	// Диагностика спавна.
	const FVector Loc = GetActorLocation();
	const FRotator Rot = GetActorRotation();
	const UCapsuleComponent* Cap = GetCapsuleComponent();
	const float CapHalf = Cap ? Cap->GetScaledCapsuleHalfHeight() : 0.0f;
	const float CapRadius = Cap ? Cap->GetScaledCapsuleRadius() : 0.0f;
	UE_LOG(LogTemp, Log, TEXT("Backrooms: Character spawned at %.0f,%.0f,%.0f  Rot=%.0f  Capsule=(r=%.0f h=%.0f)"),
		Loc.X, Loc.Y, Loc.Z, Rot.Yaw, CapRadius, CapHalf);
	UE_LOG(LogTemp, Log, TEXT("Backrooms: BodyMesh=%s  StaticBodyMesh=%s"),
		BodyMesh && BodyMesh->GetSkeletalMeshAsset() ? TEXT("has SK") : TEXT("no SK"),
		StaticBodyMesh && StaticBodyMesh->GetStaticMesh() ? TEXT("has SM") : TEXT("no SM"));

LoadBodyVisuals();
	StepAccumulator = 0.0f;
	bWasMoving = false;

	// Клетка пробы OnPlayerEnteredRoom должна совпадать с клеткой генератора
	// (CellSize активного профиля), иначе RoomID не соответствует геометрии:
	// режет каждые 50 см вместо входа в новую комнату. 500 = фолбэк для tile 0.5 м.
	for (TActorIterator<ABackroomsWorldGenerator> It(GetWorld()); It; ++It)
	{
		RoomProbeCellSize = It->GetActiveRoomCellSize();
		ProbedLevelIndex = It->LevelIndex;
		ProbedChunkSizeCells = It->ChunkSizeCells;
		break;
	}

	// Перки: применить бонусы сразу и обновлять при каждом открытии нового.
	ApplyPerkBonuses();
	if (UBackroomsProgression* Prog = GetProgression())
	{
		Prog->OnPerkUnlocked.AddDynamic(this, &ABackroomsPlayerCharacter::OnPerkUnlockedHandler);
	}

	// Осмотр: компонент сигналит при любом конце (включая срыв уроном).
	if (InspectComponent)
	{
		InspectComponent->OnInspectEnded.AddUObject(this, &ABackroomsPlayerCharacter::HandleInspectEnded);
	}

	// Фонарик: грязная линза, осязаемый луч в тумане (если ассеты/туман есть).
	ApplyFlashlightRealism();

	// Рендерер: Lumen GI/отражения, Virtual Shadow Maps, контактные тени под
	// подошвами. Слои постпроцесса камеры (жёлтый Lumen-свет по стенам).
	ApplyRendererDefaults();

// Инвентарь и статы: живут на игроке. Каталог предметов заполняется по
	// умолчанию, слоты — в InventoryComponent (источник правды).
	if (!ItemSystem)
	{
		ItemSystem = NewObject<UBackroomsItemSystem>(this, TEXT("ItemSystem"));
		ItemSystem->InitializeDefaultItems();
	}
	ItemSystem->SetInventoryComponent(InventoryComponent);
	// Батарейка пополняет фонарик; инвентарь меняется — пересобираем хотбар.
	ItemSystem->OnBatteryUsed.AddDynamic(this, &ABackroomsPlayerCharacter::AddFlashlightCharge);
	ItemSystem->OnInventoryChanged.AddDynamic(this, &ABackroomsPlayerCharacter::RebuildHotbar);
	RebuildHotbar();

// HUD: состояния (иконки) + рассудок/фонарик. Не перехватывает ввод.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<UBackroomsHUDWidget>(PC, HUDWidgetClass);
			if (HUDWidget)
			{
				HUDWidget->SetPlayer(this);
				HUDWidget->AddToViewport(10);
				// В главном меню HUD скрыт — включается на старте игры.
				// При переходе между картами (skipmenu) HUD виден сразу.
				const bool bSkipMenu = GetWorld() && GetWorld()->URL.HasOption(TEXT("skipmenu"));
				HUDWidget->SetVisibility(bSkipMenu ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
			}
		}
	}

	// Убираем «двойника»: в уровень мог быть вручную поставлен актёр с той же
	// моделью SM_Character. Иначе он стоит рядом с игроком как вторая фигура.
	// Сканируем ВСЕ актёры (не только AStaticMeshActor): дубль может быть любого
	// класса. Свой павн и его компоненты пропускаем.
	if (StaticBodyMesh && StaticBodyMesh->GetStaticMesh())
	{
		UStaticMesh* PlayerMesh = StaticBodyMesh->GetStaticMesh();
		TArray<AActor*> ToDestroy;
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor || Actor == this)
			{
				continue;
			}
			TArray<UStaticMeshComponent*> Comps;
			Actor->GetComponents<UStaticMeshComponent>(Comps);
			for (UStaticMeshComponent* Comp : Comps)
			{
				if (Comp && Comp->GetStaticMesh() == PlayerMesh)
				{
					ToDestroy.Add(Actor);
					break;
				}
			}
		}
		for (AActor* Actor : ToDestroy)
		{
			UE_LOG(LogTemp, Log, TEXT("Backrooms: removed duplicate SM_Character actor %s"), *Actor->GetName());
			Actor->Destroy();
		}
	}
}

void ABackroomsPlayerCharacter::LoadBodyVisuals()
{
	// 1) Ригнутая модель (Mannequin) — приоритетный видимый корпус. Именно она
	//    даёт руки/ноги и штатные анимации. Статическое тело тогда скрываем,
	//    чтобы не было «второго» кривого меша под ногами.
	bool bSkeletalLoaded = false;
	// Скелетную модель не подменяем чужим манекеном из демо-пака.
	if (USkeletalMesh* SkeletalMesh = BodyMeshAsset.LoadSynchronous())
	{
		BodyMesh->SetSkeletalMeshAsset(SkeletalMesh);
		BodyMesh->SetVisibility(true);
		bSkeletalLoaded = true;

		// Ориентация: Mixamo forward = +Y, у UE forward = +X — доворачиваем меш.
		BodyMesh->SetRelativeRotation(BodyMeshRelativeRotation);

		// Материал из кода (слот 0), иначе меш рендерится серым без текстуры.
		// Раздаём на все слоты: у некоторых ригов их несколько, и пустой слот
		// дал бы серые части тела.
		if (BodyMaterial)
		{
			const int32 NumSlots = FMath::Max(1, BodyMesh->GetNumMaterials());
			for (int32 Slot = 0; Slot < NumSlots; ++Slot)
			{
				BodyMesh->SetMaterial(Slot, BodyMaterial);
			}
		}

		const float CapsuleHalf = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FBoxSphereBounds Bounds = SkeletalMesh->GetBounds();
		const float MeshBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
		BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -CapsuleHalf - MeshBottom));

		UE_LOG(LogTemp, Log, TEXT("Backrooms: player skeletal mesh loaded — %s"), *SkeletalMesh->GetName());
	}
	else
	{
		BodyMesh->SetVisibility(false);
		UE_LOG(LogTemp, Log, TEXT("Backrooms: no skeletal mesh assigned; falling back to static body."));
	}

	// 2) Статическое тело — только если скелетного нет.
	if (StaticBodyMesh && StaticBodyMesh->GetStaticMesh())
	{
		StaticBodyMesh->SetVisibility(!bSkeletalLoaded);
		if (!bSkeletalLoaded)
		{
			const float CapsuleHalf = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			const FBoxSphereBounds Bounds = StaticBodyMesh->GetStaticMesh()->GetBounds();
			const float ScaleZ = StaticBodyMesh->GetRelativeScale3D().Z;
			const float MeshBottom = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * ScaleZ;
			StaticBodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -CapsuleHalf - MeshBottom));
		}
	}

	// 3) Применяем Animation Blueprint (Idle/Walk/Run/Jump), если задан.
	if (!AnimBlueprint.IsNull())
	{
		if (UAnimBlueprint* ABP = AnimBlueprint.LoadSynchronous())
		{
			if (UClass* AnimClass = ABP->GeneratedClass)
			{
				BodyMesh->SetAnimInstanceClass(AnimClass);
			}
		}
	}
	else if (bSkeletalLoaded)
	{
		// Нет AnimBP — водим одиночными UAnimSequence из кода (UpdateBodyAnimation).
		BodyMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		CurrentBodyAnim = nullptr;
		UpdateBodyAnimation(0.0f);
	}

	UE_LOG(LogTemp, Log, TEXT("Backrooms: LoadBodyVisuals done. Skeletal=%s, StaticBody=%s"),
		bSkeletalLoaded ? TEXT("OK") : TEXT("NULL"),
		StaticBodyMesh && StaticBodyMesh->GetStaticMesh() ? TEXT("present") : TEXT("NULL"));
}

void ABackroomsPlayerCharacter::UpdateBodyAnimation(float DeltaTime)
{
	if (!BodyMesh || !BodyMesh->GetSkeletalMeshAsset())
	{
		return;
	}

	// Одноразовая анимация (удар/смерть) держит экран до конца проигрывания.
	if (bBodyOneShot)
	{
		BodyOneShotTimeLeft -= DeltaTime;
		if (BodyOneShotTimeLeft > 0.0f)
		{
			return;
		}
		bBodyOneShot = false;
		CurrentBodyAnim = nullptr;
	}

	// После смерти тело остаётся в финальной позе.
	if (bDead)
	{
		return;
	}

	const UCharacterMovementComponent* Move = GetCharacterMovement();
	const float Speed = GetVelocity().Size();
	const bool bFalling = Move && Move->IsFalling();
	const float VerticalVel = Move ? Move->Velocity.Z : 0.0f;

	UAnimSequence* Desired = nullptr;
	float Rate = 1.0f;

	if (bFalling)
	{
		// Запомнили точку отрыва от земли (Z начала падения).
		if (FallStartZ == 0.0f)
		{
			FallStartZ = GetActorLocation().Z;
		}
		// Падение ниже порога MinFallAnimHeight — это обычный прыжок на ящик,
		// а не падение в пропасть: показываем только взлёт (JumpUp), падение
		// (JumpDown) не включаем.
		const float FallHeight = FallStartZ - GetActorLocation().Z;
		if (VerticalVel > 0.0f)
		{
			Desired = JumpUpAnim.LoadSynchronous();
		}
		else if (FallHeight > MinFallAnimHeight)
		{
			Desired = JumpDownAnim.LoadSynchronous();
		}
		// Иначе оставляем текущую (бег/шаг/idle) — игрок в воздухе миг, а
		// анимация «полёта вниз» мелькнёт и разорвёт бег при приземлении.
	}
	else
	{
		// На земле — сбрасываем точку отсчёта следующего падения.
		FallStartZ = 0.0f;

		if (bIsCrouched)
		{
			Desired = CrawlAnim.LoadSynchronous();
		}
		else if (Speed > 20.0f)
		{
			const bool bRunning = bSprintHeld || Speed > WalkSpeed * 1.05f;
			Desired = bRunning ? RunAnim.LoadSynchronous() : WalkAnim.LoadSynchronous();
			Rate = FMath::Clamp(Speed / FMath::Max(1.0f, WalkSpeed), 0.55f, 2.2f);
			if (!bRunning)
			{
				Rate = FMath::Clamp(Rate, 0.55f, 1.15f);
			}
		}
		else
		{
			Desired = IdleAnim.LoadSynchronous();
		}
	}

	if (Desired && Desired != CurrentBodyAnim)
	{
		CurrentBodyAnim = Desired;
		BodyMesh->PlayAnimation(Desired, true);
	}

	if (CurrentBodyAnim)
	{
		BodyMesh->SetPlayRate(Rate);
	}
}

void ABackroomsPlayerCharacter::PlayBodyOneShot(UAnimSequence* Seq)
{
	if (!Seq || !BodyMesh || !BodyMesh->GetSkeletalMeshAsset())
	{
		return;
	}
	bBodyOneShot = true;
	BodyOneShotTimeLeft = Seq->GetPlayLength();
	CurrentBodyAnim = Seq;
	BodyMesh->PlayAnimation(Seq, false);
}

void ABackroomsPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Статы (голод/жажда/здоровье/рассудок) убывают и обновляют инвентарь.
	// Состояния (StatusComponent тикает сам) множат убывание рассудка и добавляют
	// прямой урон от облучения/отравления/перегрева.
	if (ItemSystem)
	{
ItemSystem->TickStats(DeltaTime);

		// Бонусы перков (множители расхода) — держим в кэше, недёшево каждый кадр
		// ходить за подсистемой.
		if (UBackroomsProgression* Prog = GetProgression())
		{
			const FBackroomsPerkBonuses PerkB = Prog->GetBonuses();
			PerkSanityDrainMult = PerkB.SanityDrainMult;
			PerkFearGrowthMult = PerkB.FearGrowthMult;
			ItemSystem->HungerDrainRate = 0.5f * PerkB.SurvivalDrainMult;
			ItemSystem->ThirstDrainRate = 0.8f * PerkB.SurvivalDrainMult;
		}

		if (StatusComponent)
		{
			ItemSystem->SanityDrainMultiplier = StatusComponent->GetSanityDrainMultiplier() * PerkSanityDrainMult;

			const float ExtraSanity = StatusComponent->GetExtraSanityDrainPerSecond() * DeltaTime;
			if (ExtraSanity > 0.0f)
			{
				ItemSystem->Sanity = FMath::Clamp(ItemSystem->Sanity - ExtraSanity, 0.0f, ItemSystem->MaxSanity);
			}
			const float HealthDrain = StatusComponent->GetHealthDrainPerSecond() * DeltaTime;
			if (HealthDrain > 0.0f)
			{
				ItemSystem->Health = FMath::Clamp(ItemSystem->Health - HealthDrain, 0.0f, ItemSystem->MaxHealth);
			}
		}

// Смерть при нулевом здоровье.
		if (!bDead && ItemSystem->Health <= 0.0f)
		{
			Die();
		}
	}

	// Удержание «использовать» (UseTime): копим прогресс, ловим прерывания.
	UpdateUseHold(DeltaTime);

	// Заряд кидания предмета из инвентаря: копим силу удержания; если рука
	// опустела (смена слота/смерть) — сбрасываем.
	if (bThrowCharging)
	{
		if (HeldItemActor)
		{
			ThrowHoldTime += DeltaTime;
		}
		else
		{
			bThrowCharging = false;
			ThrowHoldTime = 0.0f;
		}
	}

	// Подсветка пикапа под прицелом (золотой glow на ближайшем предмете).
	UpdateFocusedPickup(DeltaTime);

	// Достижения: время и путь в Бэкрумсе копятся, пока игрок под землёй.
	// В городе прогресс не идёт — «матчасть» начинается только в Бэкрумсе.
if (!IsInCity())
	{
		if (UBackroomsAchievements* Ach = GetAchievements())
		{
			Ach->NotifyTimeInBackrooms(DeltaTime);
			if (!bDead)
			{
				Ach->NotifyDistance(GetVelocity().Size() * DeltaTime);
			}
		}
		// Время забега копится только под землёй (в городе — «матчасть» ещё нет).
		if (!bDead)
		{
			if (UBackroomsProgression* Prog = GetProgression())
			{
				Prog->TickRun(DeltaTime);
			}
		}
	}

	// Страх от среды: темнота и вид монстра роняют рассудок, свет лечит.
	UpdateFear(DeltaTime);
	UpdateSanityPostProcess(DeltaTime);

	// Выносливость бега (может снять bSprintHeld при истощении).
	UpdateStamina(DeltaTime);

	// Скорость с учётом состояний (замедление от влаги/сонливости, ускорение
	// от адреналина). Ставим ДО расчёта боба, он зависит от MaxWalkSpeed.
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		const float Base = bSprintHeld ? RunSpeed : WalkSpeed;
		const float Mult = StatusComponent ? StatusComponent->GetSpeedMultiplier() : 1.0f;
		Move->MaxWalkSpeed = Base * Mult;
	}

	const float Speed = GetVelocity().Size();
	const bool bIsMoving = Speed > 20.0f;

	// Анимация тела: idle/walk/jump/crawl (single-node, без AnimBP).
	UpdateBodyAnimation(DeltaTime);

	// --- Head bob (только при движении) ---
	if (bIsMoving)
	{
		const float MaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
		const float SpeedRatio = FMath::Clamp(Speed / MaxWalkSpeed, 0.1f, 1.0f);
		const float Frequency = BobFrequencyBase * SpeedRatio;
		BobTimer += DeltaTime * Frequency;
	}
	else
	{
		BobTimer = 0.0f;
	}

	const float BobZ = FMath::Sin(BobTimer * 2.0f * PI) * BobAmplitude;
	const float BobY = FMath::Sin(BobTimer * PI) * BobSideAmplitude;

	// --- Breathing (постоянно, медленно) ---
	BreathTimer += DeltaTime * BreathFrequency;
	const float BreathOffset = FMath::Sin(BreathTimer * 2.0f * PI) * BreathAmplitude;

	// --- Дрейф камеры от состояний (сонливость/отравление/облучение) ---
	float DriftZ = 0.0f;
	float DriftY = 0.0f;
	if (StatusComponent)
	{
		const float Drift = StatusComponent->GetCameraDrift();
		if (Drift > 0.001f)
		{
			CameraDriftTimer += DeltaTime * (1.0f + Drift * 3.0f);
			DriftZ = FMath::Sin(CameraDriftTimer * 1.7f) * Drift * 3.5f;
			DriftY = FMath::Sin(CameraDriftTimer * 1.1f + 1.0f) * Drift * 2.5f;
		}
	}

	// --- Итоговое смещение камеры (базовая + боб + дыхание + дрейф) ---
	// В третьем лице позиция дальше и выше; боб/дрейф приглушены, чтобы
	// камера на плече не «тряслась».
	const float ThirdScale = bThirdPerson ? 0.25f : 1.0f;
	FVector BaseOffset = bThirdPerson ? ThirdPersonOffset : CameraBaseOffset;
	if (bThirdPerson)
	{
		BaseOffset.X = -ThirdPersonDistance;
	}
// Близость монстра (0..1): наполняем каждый кадр для вибрации страха.
	MonsterProx = 0.0f;
	{
		float MinDistSq = MonsterShakeRadius * MonsterShakeRadius;
		bool bAnyMonster = false;
		for (TActorIterator<ABackroomsRiggedMonster> It(GetWorld()); It; ++It)
		{
			if (It->IsVanished())
			{
				continue;
			}
			const float DistSq = FVector::DistSquared(GetActorLocation(), It->GetActorLocation());
			if (DistSq < MinDistSq)
			{
				MinDistSq = DistSq;
				bAnyMonster = true;
			}
		}
		if (bAnyMonster)
		{
			const float Dist = FMath::Sqrt(MinDistSq);
			MonsterProx = FMath::Clamp(1.0f - Dist / MonsterShakeRadius, 0.0f, 1.0f);
		}
	}

	// Касание земли: сила тряски зависит от высоты падения.
	if (UCharacterMovementComponent* FallMove = GetCharacterMovement())
	{
		const bool bNowFalling = FallMove->IsFalling();
		if (bNowFalling)
		{
			bWasFallingShake = true;
			FallImpactMax = FMath::Max(FallImpactMax, FMath::Abs(FallMove->Velocity.Z));
		}
		else if (bWasFallingShake)
		{
			bWasFallingShake = false;
			TriggerFallShake();
			FallImpactMax = 0.0f;
		}
	}

	const FVector FinalOffset = BaseOffset + FVector(0.0f, (BobY + DriftY) * ThirdScale, (BobZ + BreathOffset + DriftZ) * ThirdScale);
	const float Speed01 = FMath::Clamp(Speed / GetCharacterMovement()->MaxWalkSpeed, 0.0f, 1.0f);
	const FVector ShakeOffset = ComputeShakeOffset(DeltaTime, Speed01) * ThirdScale;
	FVector CamRelative = FinalOffset + ShakeOffset;

	// Третье лицо: камера за спиной не должна проваливаться в стены/геометрию.
	// Трассируем от тела к желаемой позиции камеры и прижимаем её к преграде
	// (эффект «spring arm»), чтобы за спиной стена не резала обзор.
	if (bThirdPerson && GetWorld())
	{
		const FVector CapsuleLoc = GetActorLocation();
		const FRotator CapsuleRot = GetActorRotation();
		const FVector Start = CapsuleLoc + CapsuleRot.RotateVector(FVector(-30.0f, 0.0f, 70.0f));
		const FVector End = CapsuleLoc + CapsuleRot.RotateVector(CamRelative);
		FHitResult CamHit;
		FCollisionQueryParams QueryParams(FName(TEXT("ThirdPersonCam")), /*bTraceComplex=*/false, this);
		if (GetWorld()->LineTraceSingleByChannel(CamHit, Start, End, ECC_Camera, QueryParams))
		{
			const FVector Dir = (Start - End).GetSafeNormal();
			const FVector Clamped = CamHit.Location - Dir * 12.0f;
			CamRelative = CapsuleRot.UnrotateVector(Clamped - CapsuleLoc);
		}
	}

	Camera->SetRelativeLocation(CamRelative);

	// --- Footsteps ---
	if (bIsMoving)
	{
		StepAccumulator += DeltaTime;
		if (StepAccumulator >= GetCurrentStepInterval())
		{
			Footstep();
			StepAccumulator = 0.0f;
		}
		bWasMoving = true;
	}
	else if (bWasMoving)
	{
		StepAccumulator = 0.0f;
		bWasMoving = false;
	}

// --- OnPlayerEnteredRoom: лёгкий probe текущей мировой клетки ---
	// Клетка та же, что у генератора (RoomProbeCellSize = профильная CellSize,
	// см. BeginPlay), RoomID — хеш пары клеток вместо ломающейся строки Y*10000+X.
	const FVector Loc = GetActorLocation();
	const int32 CellX = FMath::FloorToInt(Loc.X / FMath::Max(1.0f, RoomProbeCellSize));
	const int32 CellY = FMath::FloorToInt(Loc.Y / FMath::Max(1.0f, RoomProbeCellSize));
const int32 RoomID = ComputeRoomID(CellX, CellY);
	if (RoomID != LastRoomID)
	{
		LastRoomID = RoomID;
		OnPlayerEnteredRoom.Broadcast(RoomID);

		// XP за исследование: новая комната и новый чанк — один раз за забег по
		// миру. Никаких «шариков»: награда идёт за сам факт продвижения вглубь.
		if (!IsInCity())
		{
			if (UBackroomsProgression* Prog = GetProgression())
			{
				Prog->NotifyRoomDiscovered(RoomID, ProbedLevelIndex);
				const int32 ChunkCells = FMath::Max(1, ProbedChunkSizeCells);
				Prog->NotifyChunkDiscovered(CellX / ChunkCells, CellY / ChunkCells, ProbedLevelIndex);
			}
		}
	}

	// --- Фонарик: расход заряда, мерцание на исходе, автоотключение на нуле ---
	if (bFlashlightOn && Flashlight)
	{
		FlashlightBattery = FMath::Max(0.0f, FlashlightBattery - FlashlightDrainPerSecond * DeltaTime);
if (FlashlightBattery <= 0.0f)
		{
			bFlashlightOn = false;
			Flashlight->SetVisibility(false);
			// Фонарик-предмет тоже гаснет от разряда (кнопкой уже не вернуть).
			if (EquipmentComponent)
			{
				EquipmentComponent->ForceFlashlightItemOff();
			}
		}
		else if (FlashlightBattery < 20.0f)
		{
			const float TimeMod = FMath::Fmod(GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f, 0.22f);
			Flashlight->SetVisibility(TimeMod < 0.13f);
		}
		else
		{
			Flashlight->SetVisibility(true);
		}
	}

	// --- HUD (состояния + шкалы) ---
	if (HUDWidget)
	{
		HUDWidget->Refresh();
	}
}

void ABackroomsPlayerCharacter::Footstep()
{
	// Точка подключения звука/вибрации шага.
	UE_LOG(LogTemp, Verbose, TEXT("Footstep! Speed: %.0f"), GetVelocity().Size());

	// Громкость шага: бег громче ходьбы, присед — тише. Монстры это слышат.
	const float Speed = GetVelocity().Size();
	const float MaxWalk = GetCharacterMovement() ? GetCharacterMovement()->MaxWalkSpeed : WalkSpeed;
	const float Ratio = FMath::Clamp(Speed / FMath::Max(1.0f, MaxWalk), 0.0f, 1.0f);
	float Loudness = FMath::Lerp(0.3f, 0.85f, Ratio);
	if (bIsCrouched)
	{
		Loudness *= CrouchNoiseScale;
	}
	EmitNoise(Loudness);
}

void ABackroomsPlayerCharacter::EmitNoise(float Loudness)
{
	if (Loudness <= 0.0f || !GetWorld())
	{
		return;
	}
	const FVector Loc = GetActorLocation();
	for (TActorIterator<ABackroomsRiggedMonster> It(GetWorld()); It; ++It)
	{
		It->HearNoise(Loc, Loudness);
	}
}

void ABackroomsPlayerCharacter::StartCrouch()
{
	Crouch();
}

void ABackroomsPlayerCharacter::StopCrouch()
{
	UnCrouch();
}

void ABackroomsPlayerCharacter::UpdateFear(float DeltaTime)
{
	if (!ItemSystem || bDead)
	{
		FearValue = FMath::FInterpTo(FearValue, 0.0f, DeltaTime, 1.5f);
		return;
	}

	const FVector Loc = GetActorLocation();

// Освещённость: фонарик в руке ИЛИ комнатный свет неподалёку.
	const bool bLightInHand = bFlashlightOn && FlashlightBattery > 0.0f;
	bool bLit = bLightInHand;
	bool bRoomLight = false;
	if (GetWorld())
	{
		for (TActorIterator<ABackroomsChunkActor> It(GetWorld()); It; ++It)
		{
			if (It->IsPointLit(Loc, SafeLightRadius))
			{
				bRoomLight = true;
				bLit = true;
				break;
			}
		}
	}
	bCurrentlyLit = bRoomLight;

	// Монстр: близость и попадание в поле зрения.
	bool bMonsterVisible = false;
	float NearestMonster = TNumericLimits<float>::Max();
	if (GetWorld())
	{
		const FVector ViewDir = Camera ? Camera->GetForwardVector() : GetActorForwardVector();
		for (TActorIterator<ABackroomsRiggedMonster> It(GetWorld()); It; ++It)
		{
			const ABackroomsRiggedMonster* Monster = *It;
			if (!Monster || Monster->IsVanished())
			{
				continue;
			}
			const FVector MonsterLoc = Monster->GetActorLocation();
			const FVector ToMonster = MonsterLoc - Loc;
			const float Dist = ToMonster.Size();
			NearestMonster = FMath::Min(NearestMonster, Dist);
			if (Dist > 3000.0f || Dist < 1.0f)
			{
				continue;
			}
			const float Dot = FVector::DotProduct(ViewDir, ToMonster / Dist);
			if (Dot > FMath::Cos(FMath::DegreesToRadians(30.0f)))
			{
				bMonsterVisible = true;
			}
		}
	}

	const bool bMonsterNear = NearestMonster < 1400.0f;

	// Убывание/восстановление рассудка.
	float Sanity = ItemSystem->Sanity;
	if (bMonsterVisible)
	{
		Sanity -= MonsterSightSanityDrain * DeltaTime;
	}
	if (!bLit)
	{
		Sanity -= DarknessSanityDrain * DeltaTime;
	}
	if (bLit && !bMonsterNear && !bMonsterVisible)
	{
		Sanity += SafeSanityRegen * DeltaTime;
	}
	ItemSystem->Sanity = FMath::Clamp(Sanity, 0.0f, ItemSystem->MaxSanity);

	// Целевой «страх» = нехватка рассудка + прямой взгляд на монстра.
	const float LowSanity = 1.0f - FMath::Clamp(ItemSystem->Sanity / FMath::Max(1.0f, ItemSystem->MaxSanity), 0.0f, 1.0f);
	const float SightFear = bMonsterVisible ? 0.6f : (bMonsterNear ? 0.3f : 0.0f);
const float TargetFear = FMath::Clamp((LowSanity * 0.7f + SightFear) * PerkFearGrowthMult, 0.0f, 1.0f);
	FearValue = FMath::FInterpTo(FearValue, TargetFear, DeltaTime, 2.5f);
}

void ABackroomsPlayerCharacter::UpdateSanityPostProcess(float DeltaTime)
{
	if (!Camera || !GetWorld())
	{
		return;
	}

	FPostProcessSettings& PP = Camera->PostProcessSettings;
	Camera->PostProcessBlendWeight = 1.0f;

	const float F = FMath::Clamp(FearValue, 0.0f, 1.0f);

	// --- Базовый «камерный» слой: видеокамера, а не глаз (см. совет по реализму). ---
	// Мягкое свечение ламп: блум за порогом яркости, чтобы светились плафоны и
	// пересветы, а не весь экран.
	PP.bOverride_BloomIntensity = true;
	PP.BloomIntensity = 0.8f;
	PP.bOverride_BloomThreshold = true;
	PP.BloomThreshold = 1.25f;
	// Рассеянный отражённый свет (Indirect Lighting): жёлтая лиминаль Lumen
	// «налипает» на стены и в углы — свет идёт от ламп, а не «из ниоткуда».
	PP.bOverride_IndirectLightingIntensity = true;
	PP.IndirectLightingIntensity = 1.35f;
	// Контактное затенение: «густота» в стыках ботинок/пол и углах стен.
	PP.bOverride_AmbientOcclusionIntensity = true;
	PP.AmbientOcclusionIntensity = 0.45f;
	PP.bOverride_AmbientOcclusionRadius = true;
	PP.AmbientOcclusionRadius = 120.0f;
	// Тело-камера: лёгкий MotionBlur убирает «пластиковую» резкость стоп-кадра.
	PP.bOverride_MotionBlurAmount = true;
	PP.MotionBlurAmount = 0.22f;
	PP.bOverride_MotionBlurMax = true;
	PP.MotionBlurMax = 0.5f;

	// Уровень света: комнатная люминесцентка = 1.0, только луч фонарика ~0.35,
	// чистая тьма = 0.0. Всё ниже — адаптация глаза/матрицы к этому свету.
	// Город — дневная карта: «ламп» там нет, но улица освещена солнцем
	// (иначе тьма-постпроцесс придавливает яркий город до тусклой каши).
	const bool bLamp = bCurrentlyLit || IsInCity();
	const bool bFlashlight = bFlashlightOn && FlashlightBattery > 0.0f;
	const float LightLevel = bLamp ? 1.0f : (bFlashlight ? 0.35f : 0.0f);

	// Насыщенность: палочки/колбочки — при слабом свете цветовое зрение гаснет,
	// страх обесцвечивает дополнительно.
	PP.bOverride_ColorSaturation = true;
	const float DarkDesat = FMath::Lerp(1.0f, 0.30f, 1.0f - LightLevel);
	const float FearDesat = FMath::Lerp(1.0f, 0.35f, F);
	const float Sat = DarkDesat * FearDesat;
	PP.ColorSaturation = FVector4(Sat, Sat, Sat, 1.0f);

	// Туннельное зрение: в темноте края проваливаются, стресс сжимает обзор.
	const float DarkVig = FMath::Lerp(0.35f, 0.90f, (1.0f - LightLevel) * 0.5f);
	const float FearVig = FMath::Lerp(0.35f, 0.95f, F);
	PP.bOverride_VignetteIntensity = true;
	PP.VignetteIntensity = FMath::Max(DarkVig, FearVig);

	// Баланс белого глаза/камеры: тёплый вольфрам фонарика, жёлтая
	// люминесцентная лиминала, холодное сумеречное зрение в темноте.
	FLinearColor WB;
	if (bFlashlight)
	{
		WB = FLinearColor(1.08f, 1.0f, 0.88f);
	}
	else
	{
		WB = FMath::Lerp(
			FLinearColor(0.90f, 0.95f, 1.08f),
			FLinearColor(1.04f, 1.01f, 0.94f),
			LightLevel);
	}

	// Мерцание люминесцентной лампы (только когда она реально горит):
	// медленный бит двух частот — как строб 100 Гц.
	const float TimeNow = GetWorld()->GetTimeSeconds();
	const float Flicker = 1.0f + 0.015f * FMath::Sin(TimeNow * 12.566f) * FMath::Sin(TimeNow * 9.425f + 1.3f);
	const float LampFlicker = bLamp ? Flicker : 1.0f;

	// Страх уводит в холодный болезненно-серый тон.
	const FLinearColor Gain = FMath::Lerp(
		WB * LampFlicker,
		FLinearColor(0.85f, 0.87f, 0.95f),
		F);
	PP.bOverride_ColorGain = true;
	PP.ColorGain = FVector4(Gain.R, Gain.G, Gain.B, 1.0f);

	// ISO-шум матрицы: база у любой камеры (убирает «пластик» цифрового
	// изображения) + усиление в темноте от агрессивного ISO + страх.
	float Grain = 0.15f;
	if (LightLevel < 1.0f)
	{
		Grain = FMath::Max(Grain, FMath::Lerp(0.0f, 0.35f, 1.0f - LightLevel));
	}
	Grain += F * 0.2f;
	PP.bOverride_FilmGrainIntensity = true;
	PP.FilmGrainIntensity = FMath::Clamp(Grain, 0.0f, 0.6f);

	// Хроматическая аберрация дешёвой оптики: лёгкая база «плывущих» краёв +
	// сильнее в темноте (шире зрачок, хуже коррекция) и при дрожании от страха.
	PP.bOverride_SceneFringeIntensity = true;
	PP.SceneFringeIntensity = 3.0f;
	PP.bOverride_ChromaticAberrationStartOffset = true;
	PP.ChromaticAberrationStartOffset = 0.1f;
}

float ABackroomsPlayerCharacter::GetCurrentStepInterval() const
{
	const float Speed = GetVelocity().Size();
	const float MaxWalk = GetCharacterMovement()->MaxWalkSpeed;
	const float Ratio = FMath::Clamp(Speed / MaxWalk, 0.0f, 1.0f);
	return FMath::Lerp(WalkStepInterval, RunStepInterval, Ratio);
}

void ABackroomsPlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Применяем переназначенные клавиши (если есть) поверх DefaultInput.ini.
	BackroomsInput::ApplyBindings();

	// Базовое движение остаётся legacy-совместимым, поскольку проект уже
	// использует ActionMappings. Без этих четырёх осей pawn был создан, но не
	// получал WASD/мышь от DefaultInput.ini.
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ABackroomsPlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ABackroomsPlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ABackroomsPlayerCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ABackroomsPlayerCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("Zoom"), this, &ABackroomsPlayerCharacter::ZoomView);

	// Прыжок (Space) — ACharacter::Jump/StopJumping.
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
	// Бег (Shift, удержание).
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &ABackroomsPlayerCharacter::StartSprint);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &ABackroomsPlayerCharacter::StopSprint);
	// Присед (Ctrl, удержание): тише шаги, ниже камера.
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &ABackroomsPlayerCharacter::StartCrouch);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &ABackroomsPlayerCharacter::StopCrouch);
	// Фонарик (F).
	PlayerInputComponent->BindAction(TEXT("Flashlight"), IE_Pressed, this, &ABackroomsPlayerCharacter::ToggleFlashlight);
	// Переключение вида 1-е / 3-е лицо (V).
	PlayerInputComponent->BindAction(TEXT("View"), IE_Pressed, this, &ABackroomsPlayerCharacter::ToggleView);
	// Удар кулаком (Q).
	PlayerInputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &ABackroomsPlayerCharacter::Attack);

	// Привязка действий взятия/толчка/броса. Enhanced Input в проекте настроен
	// (EnhancedPlayerInput/EnhancedInputComponent в DefaultInput.ini); здесь же
	// дополнительно оставляем legacy ActionMappings (Grab/Push/Throw в
	// DefaultInput.ini), чтобы взаимодействие работало и без ассетов.
PlayerInputComponent->BindAction(TEXT("Grab"), IE_Pressed, this, &ABackroomsPlayerCharacter::TakeToHand);
	PlayerInputComponent->BindAction(TEXT("Push"), IE_Pressed, this, &ABackroomsPlayerCharacter::PushHeld);
	// Бросок: удержание копит заряд для предмета из инвентаря, отпускание кидает;
	// физические пропы — мгновенный бросок при нажатии.
	PlayerInputComponent->BindAction(TEXT("Throw"), IE_Pressed, this, &ABackroomsPlayerCharacter::StartThrowCharge);
	PlayerInputComponent->BindAction(TEXT("Throw"), IE_Released, this, &ABackroomsPlayerCharacter::FireThrowCharge);
	PlayerInputComponent->BindAction(TEXT("PauseMenu"), IE_Pressed, this, &ABackroomsPlayerCharacter::TogglePauseMenu).bExecuteWhenPaused = true;

// Инвентарь/хотбар: использовать выбранный предмет (R, удержание — UseTime),
	// слоты 1..4.
	PlayerInputComponent->BindAction(TEXT("Use"), IE_Pressed, this, &ABackroomsPlayerCharacter::UseSelectedItemPressed);
	PlayerInputComponent->BindAction(TEXT("Use"), IE_Released, this, &ABackroomsPlayerCharacter::UseSelectedItemReleased);
	PlayerInputComponent->BindAction(TEXT("Slot1"), IE_Pressed, this, &ABackroomsPlayerCharacter::SelectHotbarSlot1);
	PlayerInputComponent->BindAction(TEXT("Slot2"), IE_Pressed, this, &ABackroomsPlayerCharacter::SelectHotbarSlot2);
	PlayerInputComponent->BindAction(TEXT("Slot3"), IE_Pressed, this, &ABackroomsPlayerCharacter::SelectHotbarSlot3);
	PlayerInputComponent->BindAction(TEXT("Slot4"), IE_Pressed, this, &ABackroomsPlayerCharacter::SelectHotbarSlot4);
	// Осмотр предмета в руке: удерживаем Alt и крутим мышью.
	PlayerInputComponent->BindAction(TEXT("Inventory"), IE_Pressed, this, &ABackroomsPlayerCharacter::ToggleInventory);
	PlayerInputComponent->BindAction(TEXT("Inspect"), IE_Pressed, this, &ABackroomsPlayerCharacter::StartInspect);
	PlayerInputComponent->BindAction(TEXT("Inspect"), IE_Released, this, &ABackroomsPlayerCharacter::StopInspect);
}

void ABackroomsPlayerCharacter::MoveForward(float Value)
{
	if (Controller && !FMath::IsNearlyZero(Value))
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
	}
}

void ABackroomsPlayerCharacter::MoveRight(float Value)
{
	if (Controller && !FMath::IsNearlyZero(Value))
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
	}
}

void ABackroomsPlayerCharacter::Turn(float Value)
{
	// Осмотр предметом в руке (компонент): мышь вращает предмет, не камеру.
	if (InspectComponent && InspectComponent->IsInspecting())
	{
		InspectComponent->InputYaw(Value);
		return;
	}

	// Осмотр: мышь вращает предмет в руке (физический или из инвентаря), а не камеру.
	if (bInspecting && (HeldProp || HeldItemActor))
	{
		InspectRotation.Yaw += Value * MouseSensitivity * 120.0f;
		if (HeldProp)
		{
			HeldProp->SetActorRelativeRotation(InspectRotation);
		}
		else if (HeldItemActor)
		{
			HeldItemActor->SetActorRelativeRotation(InspectRotation);
		}
		return;
	}
	AddControllerYawInput(Value * MouseSensitivity);
}

void ABackroomsPlayerCharacter::LookUp(float Value)
{
	// Осмотр предметом в руке (компонент): мышь вращает предмет, не камеру.
	if (InspectComponent && InspectComponent->IsInspecting())
	{
		InspectComponent->InputPitch(Value);
		return;
	}

	// Осмотр: мышь вращает предмет в руке по тангажу.
	if (bInspecting && (HeldProp || HeldItemActor))
	{
		InspectRotation.Pitch += Value * MouseSensitivity * 120.0f;
		if (HeldProp)
		{
			HeldProp->SetActorRelativeRotation(InspectRotation);
		}
		else if (HeldItemActor)
		{
			HeldItemActor->SetActorRelativeRotation(InspectRotation);
		}
		return;
	}
	AddControllerPitchInput(Value * MouseSensitivity);
}

void ABackroomsPlayerCharacter::SetMouseSensitivity(float InSensitivity)
{
	MouseSensitivity = FMath::Clamp(InSensitivity, 0.05f, 10.0f);
}

void ABackroomsPlayerCharacter::SetHUDVisible(bool bVisible)
{
	if (HUDWidget)
	{
		HUDWidget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

UBackroomsAchievements* ABackroomsPlayerCharacter::GetAchievements() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UBackroomsAchievements>();
	}
	return nullptr;
}

UBackroomsProgression* ABackroomsPlayerCharacter::GetProgression() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UBackroomsProgression>();
	}
	return nullptr;
}

void ABackroomsPlayerCharacter::OnPerkUnlockedHandler(FName PerkId, const FText& Name, int32 PointsLeft)
{
	// Очко потрачено — сразу пересобрать статы, без перезапуска.
	ApplyPerkBonuses();
	if (HUDWidget)
	{
		HUDWidget->ShowToast(FString::Printf(TEXT("%s  ·  +%d"), *Name.ToString(), PointsLeft));
	}
}

void ABackroomsPlayerCharacter::ApplyPerkBonuses()
{
	UBackroomsProgression* Prog = GetProgression();
	if (!Prog)
	{
		return;
	}
	const FBackroomsPerkBonuses B = Prog->GetBonuses();

	// Максимумы. ItemSystem/MaxStamina — источник правды; перки добавляют сверху
	// базы, а не поверх уже увеличенного значения (идемпотентно при повторе).
	if (ItemSystem)
	{
		ItemSystem->MaxHealth = 100.0f + B.MaxHealthAdd;
		ItemSystem->MaxSanity = 100.0f + B.MaxSanityAdd;
		ItemSystem->Health = FMath::Min(ItemSystem->Health, ItemSystem->MaxHealth);
		ItemSystem->Sanity = FMath::Min(ItemSystem->Sanity, ItemSystem->MaxSanity);
	}
	MaxStamina = 100.0f + B.MaxStaminaAdd;
	Stamina = FMath::Min(Stamina, MaxStamina);

	// Расход и скорость.
	StaminaDrainPerSecond = 7.5f * B.StaminaDrainMult;
	FlashlightDrainPerSecond = 0.55f * B.FlashlightDrainMult;
	WalkSpeed = 350.0f * B.SpeedMult;
	RunSpeed = 700.0f * B.SpeedMult;

	// Слоты инвентаря (рюкзак расширяется перками).
	if (InventoryComponent)
	{
		InventoryComponent->Capacity = 24 + FMath::RoundToInt(B.InventorySlotsAdd);
	}
}

bool ABackroomsPlayerCharacter::IsInCity() const
{
	// Генератор — единственный, кто знает, спустился ли игрок в Бэкрумс.
	// Если генератора в сцене нет (тестовая карта), считаем, что мы не в городе:
	// HUD со шкалами лучше показать, чем спрятать.
	for (TActorIterator<ABackroomsWorldGenerator> It(GetWorld()); It; ++It)
	{
		return !It->IsPlayerInBackrooms();
	}
	return false;
}

void ABackroomsPlayerCharacter::StartSprint()
{
	if (bStaminaExhausted || Stamina <= 0.0f || bDead)
	{
		return;
	}
	bSprintHeld = true;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = RunSpeed;
	}
}

void ABackroomsPlayerCharacter::StopSprint()
{
	bSprintHeld = false;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = WalkSpeed;
	}
}

void ABackroomsPlayerCharacter::UpdateStamina(float DeltaTime)
{
	if (bDead)
	{
		return;
	}

	const float Speed = GetVelocity().Size();
	const bool bBurning = bSprintHeld && !bIsCrouched && Speed > 20.0f && Stamina > 0.0f;

	if (bBurning)
	{
		Stamina = FMath::Max(0.0f, Stamina - StaminaDrainPerSecond * DeltaTime);
		StaminaRegenDelayTimer = StaminaRegenDelay;
		if (Stamina <= 0.0f)
		{
			bStaminaExhausted = true;
			StopSprint();
		}
		return;
	}

	if (StaminaRegenDelayTimer > 0.0f)
	{
		StaminaRegenDelayTimer = FMath::Max(0.0f, StaminaRegenDelayTimer - DeltaTime);
	}
	else
	{
		Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenPerSecond * DeltaTime);
	}

	if (bStaminaExhausted && Stamina >= StaminaRecoverThreshold)
	{
		bStaminaExhausted = false;
	}
}

bool ABackroomsPlayerCharacter::IsFlashlightItemEquipped() const
{
	return EquipmentComponent && EquipmentComponent->IsFlashlightItemEquipped();
}

void ABackroomsPlayerCharacter::SetFlashlightFromItem(bool bOn)
{
	// Фонарь-предмет не зажжётся на пустой батарейке (нужны запасные батарейки).
	if (bOn && FlashlightBattery <= 0.0f)
	{
		return;
	}
	bFlashlightOn = bOn;
	if (Flashlight)
	{
		Flashlight->SetVisibility(bOn);
	}
}

void ABackroomsPlayerCharacter::ApplyFlashlightRealism()
{
	if (!Flashlight)
	{
		return;
	}

	// «Грязная линза» — Light Function с дефектами стекла и неровным пятном.
	// Материал задаётся в CDO/BP (FlashlightLensMaterial); отсутствие ассета в
	// контенте не ломает фонарик — остаётся чистый круглый конус.
	if (!FlashlightLensMaterial.IsNull())
	{
		if (UMaterialInterface* Lens = FlashlightLensMaterial.LoadSynchronous())
		{
			Flashlight->SetLightFunctionMaterial(Lens);
			// Дальше ~60 м функция блёкнет, чтобы не вылезал «резаный» квадрат
			// мапинга функции на 90º.
			Flashlight->SetLightFunctionFadeDistance(6000.0f);
		}
	}

	// Луч «режет» воздух: вес света в объёмном тумане.
	Flashlight->SetVolumetricScatteringIntensity(FlashlightVolumetricScattering);

	// Контактная тень: тёмный стык подошвы/пола прямо под светом (запускается
	// r.ContactShadows, см. ApplyRendererDefaults; для всех primitives, не только
	// помеченных — чтобы тень была и от капсулы/ботинок).
	Flashlight->ContactShadowLength = 1.5f;
	Flashlight->ContactShadowLengthInWS = 1;
	Flashlight->ContactShadowNonCastingIntensity = 1.0f;

	// Объёмный туман в мире: без него рассеивание не видно. Включаем для уже
	// поставленных ExponentialHeightFog (единожды в BeginPlay).
	if (bFlashlightVolumetricFog && GetWorld())
	{
		for (TActorIterator<AExponentialHeightFog> It(GetWorld()); It; ++It)
		{
			UExponentialHeightFogComponent* Fog = It->GetComponent();
			if (Fog && !Fog->bEnableVolumetricFog)
			{
				Fog->SetVolumetricFog(true);
				// Клочковатый (а не «молочный») туман: рассеивание чуть выше
				// дефолта, чтобы луч был осязаемым, но воздух оставался тёмным.
				Fog->SetVolumetricFogScatteringDistribution(0.35f);
			}
		}
	}
}

// Настройки рендера кладутся консольными командами, т.к. правка Project
	// Settings из C++ на лету не пишется, а переключение в UI — руками.
void ABackroomsPlayerCharacter::ApplyRendererDefaults()
{
	UWorld* World = GetWorld();
	if (!World || !GEngine)
	{
		return;
	}

	auto Exec = [World](const TCHAR* Cmd)
	{
		GEngine->Exec(World, Cmd);
	};

	// --- Lumen: GI и отражения (свет «живёт» на лампах, а не приходит из ниоткуда). ---
	if (bEnableLumen)
	{
		Exec(TEXT("r.DynamicGlobalIlluminationMethod 1"));   // Lumen GI
		Exec(TEXT("r.Lumen.DiffuseIndirect.Allow 1"));
		Exec(TEXT("r.ReflectionMethod 1"));                  // Lumen Reflections
		Exec(TEXT("r.Lumen.Reflections.Allow 1"));
	}

	// --- Virtual Shadow Maps: чёткая физичная тень под подошвой, без «левитации». ---
	if (bEnableVirtualShadowMaps)
	{
		Exec(TEXT("r.Shadow.Virtual.Enable 1"));
	}
	else
	{
		Exec(TEXT("r.Shadow.Virtual.Enable 0"));
	}

	// --- Контактные тени: тёмный стык ботинки/пол рядом с источниками света. ---
	Exec(bEnableContactShadows ? TEXT("r.ContactShadows 1") : TEXT("r.ContactShadows 0"));

	// --- «Картофельный режим» поверх дефолтов (если включён в настройках). ---
	BackroomsQuality::ApplyPotato(BackroomsQuality::IsPotatoMode(), World);
}

void ABackroomsPlayerCharacter::ToggleFlashlight()
{
	// Экипирован фонарик-предмет: F включает/гасит именно его, а не камерный.
	if (IsFlashlightItemEquipped() && EquipmentComponent)
	{
		EquipmentComponent->ToggleFlashlightItem();
		return;
	}
	// На пустой батарейке фонарик не включить: нужна батарейка.
	if (!bFlashlightOn && FlashlightBattery <= 0.0f)
	{
		return;
	}
	bFlashlightOn = !bFlashlightOn;
	if (Flashlight)
	{
		Flashlight->SetVisibility(bFlashlightOn);
	}
}

void ABackroomsPlayerCharacter::ToggleView()
{
	// V: переключение первого/третьего лица. В третьем камера уходит назад и
	// чуть вверх; тело остаётся видимым (третье лицо). В первом возвращаемся к
	// уровню глаз.
	bThirdPerson = !bThirdPerson;
	// В третьем лице капсула больше не наклоняется за взглядом (иначе смещение
	// «назад» уезжает в пол/потолок). Камера по-прежнему смотрит по контроллеру.
	bUseControllerRotationPitch = !bThirdPerson;
	if (Camera)
	{
		Camera->SetRelativeLocation(bThirdPerson ? FVector(-ThirdPersonDistance, ThirdPersonOffset.Y, ThirdPersonOffset.Z) : CameraBaseOffset);
	}
	// В третьем лице тело должно быть видно владельцу (иначе видно «из глаз»).
	if (BodyMesh)
	{
		BodyMesh->SetOwnerNoSee(false);
	}
}

void ABackroomsPlayerCharacter::ZoomView(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}
	// Осмотр: колесо меняет дистанцию до предмета (30–80 см).
	if (InspectComponent && InspectComponent->IsInspecting())
	{
		InspectComponent->InputZoom(Value);
		return;
	}
	// Колесо мыши в первом лице переключает активный слот хотбара (экипировка).
	if (!bThirdPerson)
	{
		CycleHotbarSlot(Value > 0.0f ? 1 : -1);
		return;
	}
	// В третьем лице колесо приближает/отдаляет камеру.
	ThirdPersonDistance = FMath::Clamp(ThirdPersonDistance - Value * 30.0f, MinThirdPersonDistance, MaxThirdPersonDistance);
	if (Camera)
	{
		Camera->SetRelativeLocation(FVector(-ThirdPersonDistance, ThirdPersonOffset.Y, ThirdPersonOffset.Z));
	}
}

void ABackroomsPlayerCharacter::TakeToHand()
{
	// Держим предмет из инвентаря? Убираем его из руки (E — «Снять»).
	if (HeldItemActor)
	{
		UnequipHeldItem();
		return;
	}

	// Уже держим? Кладём обратно.
	if (HeldProp)
	{
		HeldProp->MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeldProp->MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		HeldProp->MeshComp->SetSimulatePhysics(true);
		HeldProp->MeshComp->SetEnableGravity(true);
		HeldProp->bCarried = false;
		OnItemDropped.Broadcast(HeldProp);
		HeldProp = nullptr;
		return;
	}

	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * GrabDistance;
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, FCollisionQueryParams(FName(TEXT("GrabTrace")), false, this)))
	{
		ABackroomsPhysProp* Prop = Cast<ABackroomsPhysProp>(Hit.GetActor());
		if (Prop)
		{
			HeldProp = Prop;
			Prop->bCarried = true;
			// Взяли в руку: физика выключается, предмет приклеивается к HoldPoint
			// (перед камерой) — можно рассматривать, вращая взгляд.
			Prop->MeshComp->SetSimulatePhysics(false);
			Prop->MeshComp->SetEnableGravity(false);
			Prop->MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Prop->AttachToComponent(HoldPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			// Событие: предмет взят (слой игрока) + уведомить чанк (сбор сцены).
			OnItemPickedUp.Broadcast(Prop);
			Prop->NotifyPickedUp();
			return;
		}
	}

	// Физического предмета перед собой нет — пробуем подобрать расходник
	// (банка/аптечка/вода). Раньше он подбирался при касании; теперь только по
	// кнопке «Взять», при этом берём ближайший в небольшом радиусе у ног.
	ABackroomsItemPickup* Nearest = nullptr;
	float NearestSq = FMath::Square(220.0f);
	for (TActorIterator<ABackroomsItemPickup> It(GetWorld()); It; ++It)
	{
		const float D2 = FVector::DistSquared(It->GetActorLocation(), Start);
		if (D2 < NearestSq)
		{
			NearestSq = D2;
			Nearest = *It;
		}
	}
// Счётчик и уничтожение ведёт сам пикап (принял всё — исчез, невлезшее
	// осталось лежать в мире).
	if (Nearest)
	{
		Nearest->TryPickupBy(this);
	}
}

FString ABackroomsPlayerCharacter::GetInteractPrompt() const
{
	// Подсказка строится по фактическому переназначению: BackroomsInput отдаёт
	// текущую клавишу, поэтому после смены привязки текст меняется сам.
	auto Key = [](const TCHAR* BindId, const TCHAR* Def) -> FString
	{
		return BackroomsInput::KeyDisplayName(BackroomsInput::GetEffectiveKey(BindId, Def));
	};

// Раньше всего — предмет в руках: там набор действий другой.
	if (HeldProp)
	{
return FString::Printf(TEXT("%s — %s   %s — %s   %s — %s"),
			*Key(TEXT("Inspect"), TEXT("LeftAlt")),
			*BackroomsLoc::Get(TEXT("Interact.Inspect")),
			*Key(TEXT("Throw"), TEXT("RightMouseButton")),
			*BackroomsLoc::Get(TEXT("Interact.Throw")),
			*Key(TEXT("Grab"), TEXT("E")),
			*BackroomsLoc::Get(TEXT("Interact.Drop")));
	}

	// Предмет из инвентаря в руке: осмотр, выброс, снять.
	if (HeldItemActor)
	{
		const FString InspectPart = HeldItemActor->bCanBeInspected
			? FString::Printf(TEXT("%s — %s   "), *Key(TEXT("Inspect"), TEXT("LeftAlt")), *BackroomsLoc::Get(TEXT("Interact.Inspect")))
			: FString();
		const FString ThrowPart = HeldItemActor->bCanBeThrown
			? FString::Printf(TEXT("%s — %s   "), *Key(TEXT("Throw"), TEXT("RightMouseButton")), *BackroomsLoc::Get(TEXT("Interact.Throw")))
			: FString();
		return FString::Printf(TEXT("%s%s%s — %s"), *InspectPart, *ThrowPart,
			*Key(TEXT("Grab"), TEXT("E")), *BackroomsLoc::Get(TEXT("Interact.TakeOff")));
	}

// Пикап под прицелом: подобрать / использовать / осмотреть (если держим в руке).
	if (ABackroomsItemPickup* Pickup = GetFocusedPickup())
	{
		FString UsePart;
		if (ItemSystem)
		{
			FBackroomsItemDef Def;
			if (ItemSystem->GetItemDef(Pickup->ItemId, Def) &&
				Def.UseType == EBackroomsItemUseType::Consume)
			{
				UsePart = FString::Printf(TEXT("%s — %s   "), *Key(TEXT("Use"), TEXT("R")), *BackroomsLoc::Get(TEXT("Interact.Use")));
			}
		}
		return FString::Printf(TEXT("%s%s — %s"), *UsePart, *Key(TEXT("Grab"), TEXT("E")), *BackroomsLoc::Get(TEXT("Interact.Pickup")));
	}

	// Предмет под прицелом: взять / толкнуть / использовать.
	if (GetFocusedProp())
	{
		return FString::Printf(TEXT("%s — %s   %s — %s"),
			*Key(TEXT("Grab"), TEXT("E")), *BackroomsLoc::Get(TEXT("Interact.Take")),
			*Key(TEXT("Push"), TEXT("LeftMouseButton")), *BackroomsLoc::Get(TEXT("Interact.Push")));
	}

	return FString();
}

bool ABackroomsPlayerCharacter::GetInteractPromptWorldPoint(FVector& OutPoint) const
{
	// Точка под предметом, к которой HUD приклеивает текст подсказки.
	auto Anchor = [](const UPrimitiveComponent* C) -> FVector
	{
		if (!C)
		{
			return FVector::ZeroVector;
		}
		const FBoxSphereBounds B = C->Bounds;
		return B.Origin + FVector(0.0f, 0.0f, -B.BoxExtent.Z - 24.0f);
	};

	if (HeldProp)
	{
		OutPoint = Anchor(HeldProp->MeshComp);
		return true;
	}
	if (HeldItemActor)
	{
		OutPoint = Anchor(HeldItemActor->MeshComp);
		return true;
	}
	if (ABackroomsItemPickup* Pickup = GetFocusedPickup())
	{
		OutPoint = Anchor(Pickup->MeshComp);
		return true;
	}
	if (ABackroomsPhysProp* Prop = GetFocusedProp())
	{
		OutPoint = Anchor(Prop->MeshComp);
		return true;
	}
	return false;
}

ABackroomsItemPickup* ABackroomsPlayerCharacter::GetFocusedPickup() const
{
	// Подсказку/подсветку не показываем во время осмотра/инвентаря.
	if (!Camera || bInspecting || !GetWorld())
	{
		return nullptr;
	}
	const FVector CamPos = Camera->GetComponentLocation();
	const FVector Fwd = Camera->GetForwardVector();
	float Best = FMath::Square(GrabDistance);
	ABackroomsItemPickup* Found = nullptr;
	for (TActorIterator<ABackroomsItemPickup> It(GetWorld()); It; ++It)
	{
		ABackroomsItemPickup* P = *It;
		if (!IsValid(P) || !P->MeshComp || P->IsInFlight())
		{
			continue;
		}
		const FVector To = P->GetActorLocation() - CamPos;
		const float DistSq = To.SizeSquared();
		if (DistSq > Best || DistSq <= 1.0f)
		{
			continue;
		}
		// Предмет должен быть в поле зрения, а не за спиной.
		if (Fwd.Dot(To) <= 0.0f)
		{
			continue;
		}
		// Не подсвечиваем сквозь стену.
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, CamPos, P->GetActorLocation(), ECC_Visibility,
			FCollisionQueryParams(FName(TEXT("PickupFocus")), true, this)))
		{
			if (Hit.GetActor() != P)
			{
				continue;
			}
		}
		Best = DistSq;
		Found = P;
	}
	return Found;
}

void ABackroomsPlayerCharacter::UpdateFocusedPickup(float DeltaTime)
{
	FocusUpdateTimer -= DeltaTime;
	if (FocusUpdateTimer > 0.0f)
	{
		return;
	}
	FocusUpdateTimer = 0.12f;

	ABackroomsItemPickup* Cur = (bDead || bInspecting) ? nullptr : GetFocusedPickup();
	FocusedPickup = Cur;

	// Сменился предмет под прицелом — переключаем золотой glow.
	if (GlowedPickup.Get() != Cur)
	{
		if (ABackroomsItemPickup* Old = GlowedPickup.Get())
		{
			Old->SetGlowEnabled(false);
		}
		GlowedPickup = Cur;
		if (Cur)
		{
			Cur->SetGlowEnabled(true);
		}
	}
}

float ABackroomsPlayerCharacter::GetHealth() const
{
	return ItemSystem ? ItemSystem->Health : 1.0f;
}

ABackroomsPhysProp* ABackroomsPlayerCharacter::GetFocusedProp() const
{
	// Трассировка из камеры — та же, что при взятии, чтобы подсказка не врала.
	if (!Camera)
	{
		return nullptr;
	}
	// Подсказку не показываем во время инспекции/меню.
	if (bInspecting)
	{
		return nullptr;
	}
	if (!GetWorld())
	{
		return nullptr;
	}
	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * GrabDistance;
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility,
		FCollisionQueryParams(FName(TEXT("FocusTrace")), false, this)))
	{
		return Cast<ABackroomsPhysProp>(Hit.GetActor());
	}
	return nullptr;
}

void ABackroomsPlayerCharacter::PushHeld()
{
	// ЛКМ = «толкнуть»: предмет в руке швыряется вперёд, либо толкаем свободный
	// предмет по центру прицела. Импульс ослабляем весом.
	ABackroomsPhysProp* Target = HeldProp;
	if (!Target)
	{
		FVector Start = Camera->GetComponentLocation();
		FVector End = Start + Camera->GetForwardVector() * (GrabDistance + 140.0f);
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility,
			FCollisionQueryParams(FName(TEXT("PushTrace")), false, this)))
		{
			Target = Cast<ABackroomsPhysProp>(Hit.GetActor());
		}
	}
	if (!Target)
	{
		return;
	}

	// Предмет в руке: физика выключена — включаем её перед импульсом.
	if (Target->bCarried)
	{
		Target->MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Target->MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		Target->MeshComp->SetSimulatePhysics(true);
		Target->MeshComp->SetEnableGravity(true);
		Target->bCarried = false;
		HeldProp = nullptr;
	}

	const float MassFactor = FMath::Max(0.2f, 1.0f / FMath::Max(Target->WeightKg, 0.5f));
	Target->MeshComp->AddImpulse(Camera->GetForwardVector() * (PushImpulse * MassFactor), NAME_None, true);
	OnItemPushed.Broadcast(Target, Camera->GetForwardVector());
	EmitNoise(0.7f);
}

void ABackroomsPlayerCharacter::Attack()
{
	// Одноразовая анимация удара.
	PlayBodyOneShot(PunchAnim.LoadSynchronous());

// Толкаем физический проп строго перед камерой (короткая дистанция).
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * 180.0f;
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility,
		FCollisionQueryParams(FName(TEXT("AttackTrace")), false, this)))
	{
		if (ABackroomsPhysProp* Target = Cast<ABackroomsPhysProp>(Hit.GetActor()))
		{
			const float MassFactor = FMath::Max(0.2f, 1.0f / FMath::Max(Target->WeightKg, 0.5f));
			Target->MeshComp->AddImpulse(Camera->GetForwardVector() * (PushImpulse * 1.4f * MassFactor), NAME_None, true);
			OnItemPushed.Broadcast(Target, Camera->GetForwardVector());
		}
		else if (ABackroomsRiggedMonster* Monster = Cast<ABackroomsRiggedMonster>(Hit.GetActor()))
		{
			// Существо мощное — не убить, а чуть отбросить (анимац. наклон + пинок).
			const FVector Dir = (Start - Hit.ImpactPoint).GetSafeNormal();
			Monster->TakePunch(Hit.ImpactPoint, Dir, PunchKnockbackForce);
			TriggerPunchShake();
		}
	}
	EmitNoise(0.6f);
}

void ABackroomsPlayerCharacter::TriggerPunchShake()
{
	// Резкий короткий «толчок»: высокая частота, быстро гаснет.
	PunchShake.bActive = true;
	PunchShake.Age = 0.0f;
	PunchShake.Amplitude = PunchShakeAmplitude;
	PunchShake.Frequency = PunchShakeFrequency;
	PunchShake.Duration = PunchShakeDuration;
	PunchShake.Axis = FVector(1.0f, 0.5f, 0.4f);
}

void ABackroomsPlayerCharacter::TriggerFallShake()
{
	// Сила тряски от высоты падения; низкий глухой удар.
	const float Strength = FMath::Clamp(FallImpactMax / 1200.0f, 0.0f, 1.0f);
	const float Amp = FallShakeMaxAmplitude * Strength;
	if (Amp < 0.15f)
	{
		return;
	}
	FallShake.bActive = true;
	FallShake.Age = 0.0f;
	FallShake.Amplitude = Amp;
	FallShake.Frequency = FallShakeFrequency;
	FallShake.Duration = FallShakeDuration;
	FallShake.Axis = FVector(0.0f, 0.5f, 1.0f);
}

FVector ABackroomsPlayerCharacter::ComputeShakeOffset(float DeltaTime, float Speed01)
{
	ShakeClock += DeltaTime;
	FVector Offset = FVector::ZeroVector;

	// 1) Ходьба/бег: мелкая дрожь, ведомая фазой шага (BobTimer).
	const float MoveAmp = WalkShakeAmplitude * Speed01;
	Offset.X += FMath::Sin(BobTimer * 2.0f * PI) * MoveAmp * 0.3f;
	Offset.Y += FMath::Sin(BobTimer * PI) * MoveAmp * 0.8f;
	Offset.Z += FMath::Sin(BobTimer * 2.0f * PI + 0.7f) * MoveAmp * 0.5f;

	// 2) Монстр близко: низкая вибрация страха (дрожь, а не толчок).
	if (MonsterProx > 0.001f)
	{
		const float Amp = MonsterShakeMaxAmplitude * MonsterProx;
		const float Freq = MonsterShakeBaseFrequency + 8.0f * MonsterProx;
		Offset += FVector(0.3f, 0.8f, 1.0f) * (Amp * FMath::Sin(ShakeClock * Freq * 2.0f * PI));
	}

	// 3) Импульсы (удар/падение): амплитуда тает экспоненциально.
	const auto AddPulse = [&Offset, this](FShakePulse& Pulse, float DT)
	{
		if (!Pulse.bActive)
		{
			return;
		}
		Pulse.Age += DT;
		const float T = Pulse.Duration > KINDA_SMALL_NUMBER ? Pulse.Age / Pulse.Duration : 1.0f;
		if (T >= 1.0f)
		{
			Pulse.bActive = false;
			return;
		}
		const float Envelope = Pulse.Amplitude * FMath::Exp(-2.2f * T);
		const float Phase = ShakeClock * Pulse.Frequency * 2.0f * PI;
		Offset += Pulse.Axis * (Envelope * FMath::Sin(Phase));
	};
	AddPulse(PunchShake, DeltaTime);
	AddPulse(FallShake, DeltaTime);

	return Offset;
}

void ABackroomsPlayerCharacter::Die()
{
	if (bDead)
	{
		return;
	}
bDead = true;
	if (UBackroomsAchievements* Ach = GetAchievements())
	{
		Ach->NotifyDeath();
	}
	if (UBackroomsProgression* Prog = GetProgression())
	{
		// Смерть закрывает забег: запись уходит в лидерборд без бонуса выживания.
		Prog->EndRun(false);
	}
	PlayBodyOneShot(DieAnim.LoadSynchronous());

	// Блокируем движение и ввод.
	if (AController* C = GetController())
	{
		C->SetIgnoreMoveInput(true);
		C->SetIgnoreLookInput(true);
	}
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
	}

	// Экран смерти: рестарт того же уровня / выход.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (!GameOverWidget && GameOverWidgetClass)
		{
			GameOverWidget = CreateWidget<UBackroomsGameOverWidget>(PC, GameOverWidgetClass);
		}
		if (GameOverWidget)
		{
			GameOverWidget->AddToViewport(50);
			GameOverWidget->ShowGameOver();
		}
		// Скрываем HUD, чтобы не мешал экрану смерти.
		if (HUDWidget)
		{
			HUDWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

bool ABackroomsPlayerCharacter::UseHotbarSlot(int32 SlotIndex)
{
	if (!HotbarItemIds.IsValidIndex(SlotIndex))
	{
		return false;
	}
	const FName Id = HotbarItemIds[SlotIndex];
	if (Id.IsNone())
	{
		return false;
	}
	const bool bUsed = UseInventoryItem(Id);
	// Расходник мог обнулить экипированный слот — убираем предмет из руки.
	SyncHeldItemToInventory();
	return bUsed;
}

void ABackroomsPlayerCharacter::UseSelectedItemPressed()
{
	if (bUseHeld)
	{
		return;
	}
	// Нет выбранного слота — берём первый непустой.
	if (SelectedHotbarSlot < 0)
	{
		for (int32 i = 0; i < HotbarItemIds.Num(); ++i)
		{
			if (!HotbarItemIds[i].IsNone())
			{
				SelectedHotbarSlot = i;
				break;
			}
		}
	}
	if (SelectedHotbarSlot < 0 || !HotbarItemIds.IsValidIndex(SelectedHotbarSlot))
	{
		return;
	}
	const FName Id = HotbarItemIds[SelectedHotbarSlot];
	if (Id.IsNone())
	{
		return;
	}

	// Consume-предметы с UseTime требуют удержания клавиши (прогресс в HUD).
	// Всё остальное срабатывает мгновенно, как раньше.
	if (ItemSystem)
	{
		FBackroomsItemDef Def;
		if (ItemSystem->GetItemDef(Id, Def) &&
			Def.UseType == EBackroomsItemUseType::Consume &&
			Def.UseTime > 0.15f)
		{
			bUseHeld = true;
			UseHoldItemId = Id;
			UseHoldTime = 0.0f;
			UseHoldDuration = FMath::Max(0.15f, Def.UseTime);
			UseHoldStartHealth = ItemSystem->Health;
			return;
		}
	}
	UseHotbarSlot(SelectedHotbarSlot);
}

void ABackroomsPlayerCharacter::UseSelectedItemReleased()
{
	// Отпустили раньше, чем заполнился прогресс — отмена, предмет не тратится.
	if (!bUseHeld)
	{
		return;
	}
	bUseHeld = false;
	UseHoldItemId = NAME_None;
}

void ABackroomsPlayerCharacter::UpdateUseHold(float DeltaTime)
{
	if (!bUseHeld)
	{
		return;
	}
	if (!ItemSystem || UseHoldItemId.IsNone())
	{
		bUseHeld = false;
		UseHoldItemId = NAME_None;
		return;
	}

	// Прерывания: пошли/побежали — отпустили; получили урон — отменили.
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		if (Move->Velocity.Size2D() > 70.0f)
		{
			bUseHeld = false;
			UseHoldItemId = NAME_None;
			return;
		}
	}
	if (UseHoldStartHealth > 0.0f && ItemSystem->Health < UseHoldStartHealth - 0.01f)
	{
		bUseHeld = false;
		UseHoldItemId = NAME_None;
		return;
	}

	UseHoldTime += DeltaTime;
	if (UseHoldTime >= UseHoldDuration)
	{
		const FName Id = UseHoldItemId;
		bUseHeld = false;
		UseHoldItemId = NAME_None;
		UseInventoryItem(Id);
		// Расходник мог обнулить экипированный слот — убираем предмет из руки.
		SyncHeldItemToInventory();
	}
}

FString ABackroomsPlayerCharacter::GetUseHoldItemName() const
{
	if (!ItemSystem || UseHoldItemId.IsNone())
	{
		return FString();
	}
	FBackroomsItemDef Def;
	if (ItemSystem->GetItemDef(UseHoldItemId, Def))
	{
		const FString Key = FString::Printf(TEXT("Item.%s.Name"), *Def.Id.ToString());
		const FString LocName = BackroomsLoc::Get(*Key);
		return (LocName == Key) ? Def.DisplayName : LocName;
	}
	return FString();
}

void ABackroomsPlayerCharacter::SelectHotbarSlot(int32 HotbarIndex)
{
	// Отсекаем несуществующий слот.
	if (HotbarIndex < 0 || HotbarIndex >= HotbarItemIds.Num())
	{
		UnequipHeldItem();
		return;
	}
	SelectedHotbarSlot = HotbarIndex;

	// Хотбар — первые непустые слоты инвентаря: номер в хотбаре = номер
	// непустого слота в массиве инвентаря.
	const int32 InvSlot = NthNonEmptySlotIndex(HotbarIndex);
	if (InvSlot >= 0)
	{
		EquipItemFromSlot(InvSlot);
	}
	else
	{
		UnequipHeldItem();
	}
}

void ABackroomsPlayerCharacter::SelectHotbarSlot1() { SelectHotbarSlot(0); }
void ABackroomsPlayerCharacter::SelectHotbarSlot2() { SelectHotbarSlot(1); }
void ABackroomsPlayerCharacter::SelectHotbarSlot3() { SelectHotbarSlot(2); }
void ABackroomsPlayerCharacter::SelectHotbarSlot4() { SelectHotbarSlot(3); }

void ABackroomsPlayerCharacter::CycleHotbarSlot(int32 Direction)
{
	const int32 Num = HotbarItemIds.Num();
	if (Num <= 0)
	{
		return;
	}
	// Замыкаем выбор: последний слот через колесо переходит к первому.
	int32 Current = (SelectedHotbarSlot >= 0 && SelectedHotbarSlot < Num) ? SelectedHotbarSlot : FMath::Max(0, Num - 1);
	if (Current >= Num)
	{
		Current = 0;
	}
	Current = (Current + Direction + Num) % Num;
	SelectHotbarSlot(Current);
}

int32 ABackroomsPlayerCharacter::NthNonEmptySlotIndex(int32 N) const
{
	if (N < 0 || !InventoryComponent)
	{
		return -1;
	}
	int32 Seen = 0;
	for (int32 i = 0; i < InventoryComponent->GetNumSlots(); ++i)
	{
		if (!InventoryComponent->GetItem(i).IsEmpty())
		{
			if (Seen == N)
			{
				return i;
			}
			++Seen;
		}
	}
	return -1;
}

void ABackroomsPlayerCharacter::EquipItemFromSlot(int32 InventorySlotIndex)
{
	if (!InventoryComponent || !HeldItemClass || !GetWorld())
	{
		return;
	}

	const FInventoryItem Item = InventoryComponent->GetItem(InventorySlotIndex);
	if (Item.IsEmpty())
	{
		UnequipHeldItem();
		return;
	}

	// Тот же слот уже в руке — ничего не делаем.
	if (CurrentHeldSlotIndex == InventorySlotIndex && HeldItemActor)
	{
		return;
	}

	UnequipHeldItem();

	// Разрешаем мягкую ссылку на Data Asset (для встроенных предметов —
	// подставляется из каталога ItemSystem).
	FInventoryItem Mutable = Item;
	UItemDataAsset* Data = InventoryComponent->ResolveItemData(Mutable);
	if (!Data)
	{
		CurrentHeldSlotIndex = -1;
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	HeldItemActor = GetWorld()->SpawnActor<ABackroomsHeldItem>(
		HeldItemClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (!HeldItemActor)
	{
		CurrentHeldSlotIndex = -1;
		return;
	}

	HeldItemActor->Initialize(Data);
	HeldItemActor->SetActorRelativeRotation(FRotator::ZeroRotator);

	// Точка крепления: сокет на скелете тела (если есть) или HoldPoint у камеры.
	USceneComponent* Slot = HoldPoint;
	FName SocketName = ItemSocketName;
	if (Data && !Data->HandSocketName.IsNone() && BodyMesh && BodyMesh->DoesSocketExist(Data->HandSocketName))
	{
		SocketName = Data->HandSocketName;
	}
	if (BodyMesh && BodyMesh->DoesSocketExist(SocketName))
	{
		Slot = BodyMesh;
	}
	HeldItemActor->AttachToComponent(Slot, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

	CurrentHeldSlotIndex = InventorySlotIndex;
	bInspecting = false;

	// Компонент экипировки: монтаж (opt), фонарик-предмет — свет в руку.
	if (EquipmentComponent)
	{
		EquipmentComponent->EquipItem(Item.ItemID, Data, HeldItemActor);
	}
}

void ABackroomsPlayerCharacter::UnequipHeldItem()
{
	// Компонент экипировки: погасить фонарик-предмет, сбросить состояние руки.
	if (EquipmentComponent)
	{
		EquipmentComponent->Unequip();
	}
	if (HeldItemActor)
	{
		HeldItemActor->Destroy();
		HeldItemActor = nullptr;
	}
	// Осмотр оборван: меш осмотра снят с камеры, ввод движения разблокирован.
	if (InspectComponent && InspectComponent->IsInspecting())
	{
		InspectComponent->EndInspect();
	}
	CurrentHeldSlotIndex = -1;
	bInspecting = false;
}

void ABackroomsPlayerCharacter::SyncHeldItemToInventory()
{
	if (CurrentHeldSlotIndex < 0)
	{
		return;
	}
	if (!InventoryComponent || InventoryComponent->GetItem(CurrentHeldSlotIndex).IsEmpty())
	{
		UnequipHeldItem();
	}
}

void ABackroomsPlayerCharacter::StartInspect()
{
	// Предмет из инвентаря в руке: осмотр делает компонент (меш рядом с
	// камерой, движение блокируется, текст DisplayName+Description в HUD).
	if (InspectComponent && HeldItemActor && HeldItemActor->bCanBeInspected)
	{
		FName Id;
		if (CurrentHeldSlotIndex >= 0 && InventoryComponent)
		{
			Id = InventoryComponent->GetItem(CurrentHeldSlotIndex).ItemID;
		}
		if (!Id.IsNone() && InspectComponent->StartInspect(Id))
		{
			bInspecting = true;
			// Прячем маленький held-актор из руки: в осмотре работает свой меш.
			HeldItemActor->SetActorHiddenInGame(true);
			return;
		}
	}

	if (HeldProp)
	{
		bInspecting = true;
		// Начинаем осмотр с текущей ориентации предмета в руке.
		InspectRotation = HeldProp->GetRootComponent()->GetRelativeRotation();
	}
}

void ABackroomsPlayerCharacter::StopInspect()
{
	if (InspectComponent && InspectComponent->IsInspecting())
	{
		InspectComponent->EndInspect();
	}
	bInspecting = false;
}

bool ABackroomsPlayerCharacter::IsInspecting() const
{
	return bInspecting || (InspectComponent && InspectComponent->IsInspecting());
}

FName ABackroomsPlayerCharacter::GetInspectItemId() const
{
	return InspectComponent ? InspectComponent->GetItemId() : NAME_None;
}

FText ABackroomsPlayerCharacter::GetInspectItemName() const
{
	const FName Id = GetInspectItemId();
	if (Id.IsNone())
	{
		return FText::GetEmpty();
	}
	const FString Key = FString::Printf(TEXT("Item.%s.Name"), *Id.ToString());
	const FString LocName = BackroomsLoc::Get(*Key);
	return FText::FromString((LocName == Key)
		? (InspectComponent ? InspectComponent->GetDisplayName().ToString() : FString())
		: LocName);
}

FString ABackroomsPlayerCharacter::GetInspectItemDescription() const
{
	const FName Id = GetInspectItemId();
	if (Id.IsNone())
	{
		return FString();
	}
	const FString Key = FString::Printf(TEXT("Item.%s.Desc"), *Id.ToString());
	const FString LocDesc = BackroomsLoc::Get(*Key);
	return (LocDesc == Key)
		? (InspectComponent ? InspectComponent->GetDescription() : FString())
		: LocDesc;
}

class USceneComponent* ABackroomsPlayerCharacter::GetCameraComponent() const
{
	return Camera.Get();
}

void ABackroomsPlayerCharacter::HandleInspectEnded(bool bForced)
{
	// Любой конец осмотра (включая срыв уроном/атакой) возвращает предмет в руку.
	(void)bForced;
	if (HeldItemActor)
	{
		HeldItemActor->SetActorHiddenInGame(false);
	}
	bInspecting = false;
}

void ABackroomsPlayerCharacter::AddFlashlightCharge(float Amount)
{
	FlashlightBattery = FMath::Clamp(FlashlightBattery + Amount, 0.0f, 100.0f);
}

void ABackroomsPlayerCharacter::RebuildHotbar()
{
	HotbarItemIds.Reset();
	if (InventoryComponent)
	{
		// Хотбар — первые 4 непустых слота инвентаря.
		for (int32 i = 0; i < InventoryComponent->GetNumSlots(); ++i)
		{
			const FInventoryItem Item = InventoryComponent->GetItem(i);
			if (!Item.IsEmpty())
			{
				HotbarItemIds.Add(Item.ItemID);
				if (HotbarItemIds.Num() >= 4)
				{
					break;
				}
			}
		}
	}
	if (SelectedHotbarSlot >= HotbarItemIds.Num())
	{
		SelectedHotbarSlot = HotbarItemIds.Num() - 1;
	}
	// Экипированный слот мог опустеть/сместиться — убрать предмет из руки.
	SyncHeldItemToInventory();
}

void ABackroomsPlayerCharacter::StartThrowCharge()
{
	// Осмотр предмета из руки сорвался бы от броска — заканчиваем.
	if (InspectComponent && InspectComponent->IsInspecting())
	{
		InspectComponent->EndInspect();
	}

	// Предмет из инвентаря в руке: кидание с зарядом — ПКМ удержание копит
	// силу, отпускание вызывает FireThrowCharge (§8.4).
	if (HeldItemActor)
	{
		if (!HeldItemActor->bCanBeThrown)
		{
			return;
		}
		bThrowCharging = true;
		ThrowHoldTime = 0.0f;
		return;
	}

	// Физический проп (предмет уровня) — мгновенный бросок, как и раньше.
	ThrowHeld();
}

void ABackroomsPlayerCharacter::FireThrowCharge()
{
	if (!bThrowCharging)
	{
		return;
	}
	bThrowCharging = false;
	const float ChargedFor = ThrowHoldTime;
	ThrowHoldTime = 0.0f;

	if (!HeldItemActor || !HeldItemActor->bCanBeThrown || !ItemSystem)
	{
		return;
	}

	// Заряд удержания (0..1): короткое нажатие — лёгкий «подброс», полная
	// выдержка ~0.5 с — дальний бросок на ThrowImpulseRange.Y.
	const float Charge = FMath::Clamp(ChargedFor / ThrowChargeDuration, 0.0f, 1.0f);

	ABackroomsItemPickup* Pickup = nullptr;
	const FName ItemId = (CurrentHeldSlotIndex >= 0 && InventoryComponent)
		? InventoryComponent->GetItem(CurrentHeldSlotIndex).ItemID : NAME_None;
	if (ItemId.IsNone() || !ItemSystem->DropItem(ItemId, 1, true, Charge, Pickup))
	{
		return;
	}
	// Упавший предмет при ударе издаст шум — отдаём его слуху монстров.
	if (Pickup)
	{
		Pickup->OnItemNoise.AddDynamic(this, &ABackroomsPlayerCharacter::OnPickupNoise);
	}
	// Слот мог опустеть (единственный экземпляр улетел) — убрать из руки
	// (UnequipHeldItem + погасить фонарик-предмет).
	SyncHeldItemToInventory();
}

void ABackroomsPlayerCharacter::OnPickupNoise(FVector Location, float Radius)
{
	// Радиус шума предмета → громкость в системе слуха монстров.
	EmitNoise(FMath::Clamp(Radius / 2000.0f, 0.1f, 1.0f));
}

void ABackroomsPlayerCharacter::ThrowHeld()
{
	// Осмотр физического пропа сорвался бы от броска — заканчиваем.
	if (InspectComponent && InspectComponent->IsInspecting())
	{
		InspectComponent->EndInspect();
	}

	// Сюда попадаем только с физическим пропом: предметы из инвентаря кидаются
	// с зарядом через StartThrowCharge/FireThrowCharge.
	if (!HeldProp)
	{
		return;
	}
	const float BaseThrow = 900.0f;
	// Коэффициент от веса: табуретка (≈3 kg) летит далеко, шкаф (~60 kg) —
	// почти не сдвинуть. Тяжёлый не улетает по тому же импульсу.
	const float WeightFactor = FMath::Max(0.12f, 100.0f / (100.0f + HeldProp->WeightKg * 4.0f));
	const FVector ImpulseVector = Camera->GetForwardVector() * (BaseThrow * WeightFactor) + FVector::UpVector * 120.0f;
	// Перед броском возвращаем физику и коллизию.
	HeldProp->MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeldProp->MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	HeldProp->MeshComp->SetSimulatePhysics(true);
	HeldProp->MeshComp->SetEnableGravity(true);
	HeldProp->MeshComp->AddImpulse(ImpulseVector, NAME_None, true);
	HeldProp->bCarried = false;
	OnItemThrown.Broadcast(HeldProp, ImpulseVector);
	HeldProp = nullptr;
	EmitNoise(0.9f);
}

bool ABackroomsPlayerCharacter::PickUpInventoryItem(FName ItemId, int32 Count)
{
	if (!ItemSystem)
	{
		return false;
	}
int32 Leftover = 0;
	const bool bPicked = ItemSystem->PickUpItem(ItemId, Count, Leftover);
	// Невлезшее не теряем: вываливается у ног игрока (PickUpItem больше сам
	// остаток не спавнит — правда прежнего «Добавить → выпал» сохраняется).
	if (!bPicked && Leftover > 0)
	{
		UInventoryComponent* Inv = GetInventoryComponent();
		if (Inv)
		{
			Inv->SpawnLeftoverPickup(ItemId, Leftover);
			return false;
		}
	}
	if (bPicked)
	{
		if (UBackroomsAchievements* Ach = GetAchievements())
		{
			Ach->NotifyItemPickedUp();
		}
	}
	return bPicked;
}

bool ABackroomsPlayerCharacter::UseInventoryItem(FName ItemId)
{
	if (!ItemSystem)
	{
		return false;
	}

	FBackroomsItemDef Def;
	const bool bHasDef = ItemSystem->GetItemDef(ItemId, Def);
	const bool bUsed = ItemSystem->UseItem(ItemId);

	// Монтаж применения (UseMontage) — если у предмета задан и есть в контенте.
	if (bUsed && EquipmentComponent)
	{
		EquipmentComponent->PlayUseMontage();
	}

	// Использованный предмет влияет на состояния (лекарства — защита, вода —
	// снимает отравление и т.п.). Живая связь инвентаря и статусов.
	if (bUsed && bHasDef && StatusComponent)
	{
		StatusComponent->NotifyItemUsed(Def);
	}
	if (bUsed && bHasDef)
	{
		if (UBackroomsAchievements* Ach = GetAchievements())
		{
			// Категория идёт в достижения как int (совпадает с enum).
			Ach->NotifyItemUsed((int32)Def.Category);
		}
	}
	return bUsed;
}

void ABackroomsPlayerCharacter::ToggleInventory()
{
	// Осмотр и открытый инвентарь несовместимы.
	if (InspectComponent && InspectComponent->IsInspecting())
	{
		InspectComponent->EndInspect();
	}
	if (!InventoryWidget && InventoryWidgetClass)
	{
		InventoryWidget = CreateWidget<UBackroomsInventoryWidget>(
			Cast<APlayerController>(GetController()), InventoryWidgetClass);
	}
	if (!InventoryWidget)
	{
		return;
	}
	if (!InventoryWidget->IsInViewport())
	{
		InventoryWidget->AddToViewport(20);
	}
	InventoryWidget->ToggleInventory();
}

void ABackroomsPlayerCharacter::TogglePauseMenu()
{
	// Пауза прерывает осмотр (меш с камеры снимается, движение разблокируется).
	if (InspectComponent && InspectComponent->IsInspecting())
	{
		InspectComponent->EndInspect();
	}
	if (!PauseMenuWidget && PauseMenuClass)
	{
		PauseMenuWidget = CreateWidget<UBackroomsPauseMenuWidget>(GetWorld(), PauseMenuClass);
	}
	if (PauseMenuWidget)
	{
		const bool bVisible = PauseMenuWidget->IsVisible();
		PauseMenuWidget->ShowMenu(!bVisible);
		if (!bVisible)
		{
			APlayerController* PC = Cast<APlayerController>(GetController());
			if (PC) PC->SetPause(true);
		}
		else
		{
			APlayerController* PC = Cast<APlayerController>(GetController());
			if (PC)
			{
				PC->SetPause(false);
				FInputModeGameOnly Game;
				PC->SetInputMode(Game);
				PC->bShowMouseCursor = false;
			}
		}
	}
}

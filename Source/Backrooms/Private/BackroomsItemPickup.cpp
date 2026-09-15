#include "BackroomsItemPickup.h"
#include "BackroomsPlayerCharacter.h"
#include "BackroomsItemSystem.h"
#include "BackroomsInventoryData.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"

ABackroomsItemPickup::ABackroomsItemPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	RootComponent = Trigger;
	Trigger->InitSphereRadius(70.0f);
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionObjectType(ECC_WorldDynamic);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(Trigger);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetGenerateOverlapEvents(false);

	// Золотая лампа-подсветка при наведении (мягкий пульс в Tick).
	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(Trigger);
	GlowLight->SetLightColor(FLinearColor(1.0f, 0.78f, 0.15f));
	GlowLight->SetIntensity(0.0f);
	GlowLight->SetAttenuationRadius(180.0f);
	GlowLight->SetCastShadows(false);

	// Заглушка, если у предмета нет мирового меша (например, ассет не завезён).
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		PlaceholderMesh = CubeFinder.Object;
	}
}

void ABackroomsItemPickup::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ABackroomsItemPickup::OnTriggerBegin);
}

void ABackroomsItemPickup::Initialize(FName InItemId, int32 InCount, UStaticMesh* Mesh, float InScale)
{
	ItemId = InItemId;
	Count = FMath::Max(1, InCount);
	Scale = (InScale > 0.0f) ? InScale : 1.0f;

	UStaticMesh* UseMesh = Mesh ? Mesh : PlaceholderMesh;
	if (UseMesh && MeshComp)
	{
		MeshComp->SetStaticMesh(UseMesh);

		// Нормализация размера: у разных мешей (особенно Fab-сканов) габариты
		// гуляют в разы — банка пепси могла быть размером с человека. Приводим
		// предмет к «карманному» размеру ~35 см по наибольшей оси, сохраняя
		// заданный Scale как относительный.
		float FitScale = Scale;
		const FBoxSphereBounds B = UseMesh->GetBounds();
		const float MaxExtent = FMath::Max3(B.BoxExtent.X, B.BoxExtent.Y, B.BoxExtent.Z);
		if (MaxExtent > 1.0f)
		{
			const float TargetHalf = 17.5f; // ~35 см в поперечнике
			FitScale = Scale * (TargetHalf / MaxExtent);
		}
		else
		{
			// Заглушка-куб (100 см): уменьшаем сильнее.
			FitScale = Scale * 0.2f;
		}
		MeshComp->SetRelativeScale3D(FVector(FitScale));

		// Ставим так, чтобы низ предмета лежал на земле (учёт центра модели).
		const float BottomZ = (B.Origin.Z - B.BoxExtent.Z) * FitScale;
		MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -BottomZ));
	}

BaseLocation = GetActorLocation();

	// Радиус шума — в данные предмета (§6): бутылка ~8 м, канистра ~15 м.
	if (const UItemDataAsset* Asset = UBackroomsItemSystem::GetItemDataAsset(InItemId))
	{
		NoiseRadius = FMath::Max(50.0f, Asset->NoiseRadius);
	}
	State = EPickupState::Idle;
}

void ABackroomsItemPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Полёт: физика включается с задержкой, чтобы не застрять в спавн-точке.
	if (bAwaitingPhysics)
	{
		PhysicsDelay -= DeltaTime;
		if (PhysicsDelay <= 0.0f)
		{
			bAwaitingPhysics = false;
			if (MeshComp)
			{
				MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				MeshComp->SetSimulatePhysics(true);
				MeshComp->SetEnableGravity(true);
				MeshComp->SetNotifyRigidBodyCollision(true);
				MeshComp->AddImpulse(PendingImpulse, NAME_None, true);
			}
		}
		return;
	}

	// Летит: ждём, пока затихнет — переход в Rest (снова интерактивный).
	if (State == EPickupState::Thrown)
	{
		if (MeshComp)
		{
			const float Speed = MeshComp->GetPhysicsLinearVelocity().Size();
			if (Speed < 25.0f)
			{
				RestTimeout += DeltaTime;
				if (RestTimeout > 0.5f)
				{
					MeshComp->SetSimulatePhysics(false);
					MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					State = EPickupState::Rest;
				}
			}
			else
			{
				RestTimeout = 0.0f;
			}
		}
		return;
	}

	// Idle: парение + вращение — маркер, что предмет можно подобрать.
	if (State == EPickupState::Idle)
	{
		SpinPhase += DeltaTime * SpinSpeed;
		const float Bob = FMath::Sin(SpinPhase * PI / 45.0f) * BobAmplitude;
		SetActorLocation(BaseLocation + FVector(0.0f, 0.0f, HoverHeight + Bob));
		AddActorLocalRotation(FRotator(0.0f, SpinSpeed * DeltaTime, 0.0f));
	}

	// Золотая подсветка при наведении: мягкий пульс интенсивности.
	if (bGlow && GlowLight)
	{
		GlowTimer = FMath::Fmod(GlowTimer + DeltaTime, 2.0f * PI);
		GlowLight->SetIntensity(1600.0f + 600.0f * FMath::Sin(GlowTimer * 4.0f));
		GlowLight->SetAttenuationRadius(220.0f);
	}
}

void ABackroomsItemPickup::ThrowAt(const FVector& Impulse)
{
	if (State == EPickupState::Thrown || !MeshComp)
	{
		return;
	}
	bAwaitingPhysics = true;
	PhysicsDelay = 0.05f;
	PendingImpulse = Impulse;
	State = EPickupState::Thrown;
	RestTimeout = 0.0f;
	bNoiseSent = false;

	if (!bNoiseBound)
	{
		MeshComp->OnComponentHit.AddDynamic(this, &ABackroomsItemPickup::OnMeshHit);
		bNoiseBound = true;
	}
	SetGlowEnabled(false);
}

void ABackroomsItemPickup::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Шум летит от заметного удара, не от лёгкого касания пола при финише.
	if (State != EPickupState::Thrown || bNoiseSent || !MeshComp)
	{
		return;
	}
	const float Speed = MeshComp->GetPhysicsLinearVelocity().Size();
	if (Speed < 60.0f)
	{
		return;
	}
	bNoiseSent = true;
	OnItemNoise.Broadcast(GetActorLocation(), NoiseRadius);
}

bool ABackroomsItemPickup::TryPickupBy(ABackroomsPlayerCharacter* Player)
{
	// Подбор по кнопке: игрок сам решает взять предмет. Пикап сам ведёт счётчик:
	// принял всё — уничтожает себя; принял часть — остаток лежит в мире.
	if (!Player || IsInFlight())
	{
		return false;
	}
	UBackroomsItemSystem* Items = Player->GetItemSystem();
	if (!Items || Count <= 0)
	{
		return false;
	}

	int32 Leftover = Count;
	const bool bPicked = Items->PickUpItem(ItemId, Count, Leftover);
	const int32 Added = FMath::Clamp(Count - Leftover, 0, Count);
	Count = FMath::Max(0, Leftover);
	(void)Added;

	if (Count <= 0)
	{
		Destroy();
		return true;
	}
return false;
}

void ABackroomsItemPickup::SetGlowEnabled(bool bEnabled)
{
	if (bGlow == bEnabled)
	{
		return;
	}
	bGlow = bEnabled;
	if (GlowLight)
	{
		GlowLight->SetIntensity(bEnabled ? 1600.0f : 0.0f);
	}
	if (MeshComp)
	{
		// Стенсил — для пост-процессной золотой обводки, если она будет в контенте.
		MeshComp->SetRenderCustomDepth(bEnabled);
		MeshComp->SetCustomDepthStencilValue(bEnabled ? 4 : 0);
	}
}

void ABackroomsItemPickup::OnTriggerBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Автоподбор при касании отключён: предмет теперь берётся кнопкой «Взять».
	// Overlap оставлен, чтобы предмет подсвечивался как доступный (см. Tick).
	(void)OverlappedComp; (void)OtherActor; (void)OtherComp; (void)OtherBodyIndex; (void)bFromSweep; (void)SweepResult;
}

#include "BackroomsPhysProp.h"
#include "BackroomsChunkActor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

ABackroomsPhysProp::ABackroomsPhysProp()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MeshComp->SetMobility(EComponentMobility::Movable);
	// Всё, что включает QueryAndPhysics / читает физмат, падает на CDO:
	// GEngine ещё не инициализирован, и FBodyInstance не может получить
	// физический материал. На CDO это не нужно — состояние выставляется
	// в Initialize/BeginPlay; на placed-инстансах конструктор работает как надо.
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
		MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
		// Низкие демпфирования: даже после остановки лёгкие вещи держат инерцию
		// и катятся. Фрикцию, которая реально решает «катится/стоит», ведёт
		// ApplyFrictionByWeight (Task: материалы трения).
		MeshComp->SetLinearDamping(0.1f);
		MeshComp->SetAngularDamping(0.05f);
		// Контактные тени: «заземление» физического пропа (стык с полом/стеной)
		// рисуется короткой острой тенью рядом с источником (см. r.ContactShadows).
		MeshComp->SetCastShadow(true);
		MeshComp->bCastDynamicShadow = true;
		MeshComp->SetCastContactShadow(true);
		// Базовый физмат: динамический, переопределяется по весу в Initialize.
		// CreateDefaultSubobject (не NewObject с пустым именем) — иначе краш
		// «NewObject with empty name» в конструкторе актора.
		FrictionMaterial = NewObject<UPhysicalMaterial>(this, TEXT("FrictionMaterial"));
		FrictionMaterial->Friction = 0.6f;
		FrictionMaterial->Restitution = 0.35f;
		MeshComp->SetPhysMaterialOverride(FrictionMaterial);
	}
}

void ABackroomsPhysProp::BeginPlay()
{
	Super::BeginPlay();
	// Проп, размещённый в редакторе без Initialize, тоже получает трение по весу.
	ApplyFrictionByWeight();
	// Хук «сломанного» пропа: сильный удар -> OnPropBroken у чанка.
	MeshComp->OnComponentHit.AddDynamic(this, &ABackroomsPhysProp::OnMeshHit);
}

void ABackroomsPhysProp::SetOwnerChunk(ABackroomsChunkActor* InChunk)
{
	OwnerChunk = InChunk;
}

ABackroomsChunkActor* ABackroomsPhysProp::GetOwnerChunk() const
{
	return OwnerChunk.Get();
}

void ABackroomsPhysProp::NotifyPickedUp()
{
	if (ABackroomsChunkActor* Chunk = OwnerChunk.Get())
	{
		Chunk->NotifyStoryPropPickedUp(this);
	}
}

void ABackroomsPhysProp::OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (NormalImpulse.SizeSquared() > BrokenImpactThreshold * BrokenImpactThreshold)
	{
		if (ABackroomsChunkActor* Chunk = OwnerChunk.Get())
		{
			Chunk->OnPropBroken.Broadcast(this);
		}
	}
}

void ABackroomsPhysProp::ApplyFrictionByWeight()
{
	// Реалистичная связка: сила трения F = μ·N, N растёт с массой. Значит
	// тяжёлому нужен ВЫСОКИЙ μ (прижат к полу — не сдвинуть), лёгкому —
	// НИЗКИЙ μ (легко катить). Мягкая кривая по весу:
	//   лёгкий (~0.2–1 kg)  -> μ ≈ 0.10–0.25  (катится)
	//   средний (~1–10 kg)  -> μ ≈ 0.30–0.55
	//   тяжёлый (~10–60 kg) -> μ ≈ 0.65–0.95  (стоит на месте)
	const float Clamped = FMath::Clamp(WeightKg, 0.0f, 60.0f);
	const float Mu = FMath::Lerp(0.10f, 0.95f, FMath::Sqrt(Clamped / 60.0f));
	FrictionMaterial->Friction = Mu;
	// Лёгкий чуть отскакивает, тяжёлый гасится.
	FrictionMaterial->Restitution = FMath::Lerp(0.5f, 0.05f, FMath::Clamp(Clamped / 60.0f, 0.0f, 1.0f));
	// Применяем обновлённый материал (необходимо после изменения Kinematic флага).
	MeshComp->SetPhysMaterialOverride(FrictionMaterial);
}

void ABackroomsPhysProp::Initialize(UStaticMesh* Mesh, float Scale, float InWeightKg, const FVector& Pos, const FRotator& Rot)
{
	WeightKg = InWeightKg;
	if (Mesh)
	{
		for (const FStaticMaterial& SM : Mesh->GetStaticMaterials())
		{
			if (SM.MaterialInterface && SM.MaterialInterface->GetBlendMode() == BLEND_Translucent)
			{
				MeshComp->bDisallowNanite = true;
				break;
			}
		}
		MeshComp->SetStaticMesh(Mesh);
	}
	// Реалистичный масштаб: при необходимости чуть меньше, чтобы удобно держать.
	MeshComp->SetWorldScale3D(FVector(Scale));
	MeshComp->SetWorldLocation(Pos);
	MeshComp->SetWorldRotation(Rot);
	MeshComp->SetMassOverrideInKg(NAME_None, WeightKg);
	// Материалы трения: вес -> фрикция (лёгкое катится, тяжёлое стоит).
	ApplyFrictionByWeight();
	MeshComp->RecreatePhysicsState();
}

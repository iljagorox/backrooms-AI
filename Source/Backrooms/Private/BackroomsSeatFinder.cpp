#include "BackroomsSeatFinder.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"

namespace
{
	// Диапазон «сидячих» высот верхней грани от пола (см): стул/стол/коробка.
	constexpr float GSeatHeightMin = 45.0f;
	constexpr float GSeatHeightMax = 85.0f;

	// Оценить «верхнюю грань» инстанса по его мировому трансформу и габаритам меша.
	void EvaluateInstance(UStaticMesh* Mesh, const FTransform& InstWorld, float& OutSeatHeightCm, float& OutSeatTopZ, float& OutGroundZ)
	{
		const FBoxSphereBounds B = Mesh->GetBounds();
		const float Scale = InstWorld.GetScale3D().Z > 0.0f ? InstWorld.GetScale3D().Z : 1.0f;
		OutGroundZ = InstWorld.GetLocation().Z + (B.Origin.Z - B.BoxExtent.Z) * Scale;
		OutSeatTopZ = InstWorld.GetLocation().Z + (B.Origin.Z + B.BoxExtent.Z) * Scale;
		OutSeatHeightCm = OutSeatTopZ - OutGroundZ;
	}
}

namespace BackroomsSeatFinder
{
	FBackroomsSeat FindSeatNear(UWorld* World, const FVector& Origin, float SearchRadius)
	{
		FBackroomsSeat Result;
		if (!World)
		{
			return Result;
		}

// Бюджет поиска: даже на огромном стриминговом мире один проход не должен
	// «вешать» игру — сколько инстансов проверили, столько и хватит.
	constexpr int32 GMaxInstancesVisited = 4000;
	// «Достаточно близкое» сиденье — дальше мир не обходим, выходим сразу.
	constexpr float GoodEnoughDistSq = 450.0f * 450.0f;

	float BestDistSq = SearchRadius * SearchRadius;
	const FVector Origin2D(Origin.X, Origin.Y, 0.0f);
	int32 Visited = 0;

	// Обходим все чанки/акторы в мире: пропсы лежат инстанс-мешами.
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor->IsHidden())
		{
			continue;
		}

		TInlineComponentArray<UInstancedStaticMeshComponent*> Instancers;
		Actor->GetComponents(Instancers, /*bIncludeFromChildActors=*/true);
		for (UInstancedStaticMeshComponent* ISMC : Instancers)
		{
			UStaticMesh* Mesh = ISMC ? ISMC->GetStaticMesh() : nullptr;
			if (!Mesh || !ISMC->IsVisible())
			{
				continue;
			}

			const int32 Num = ISMC->GetInstanceCount();
			for (int32 i = 0; i < Num; ++i)
			{
				if (++Visited > GMaxInstancesVisited)
				{
					return Result; // бюджет исчерпан — что нашли, то и вернём
				}
				FTransform InstWorld;
				if (!ISMC->GetInstanceTransform(i, InstWorld, /*bWorldSpace=*/true))
				{
					continue;
				}
				const FVector Loc = InstWorld.GetLocation();
				const float DistSq = FVector::DistSquared2D(Loc, Origin2D);
				if (DistSq > BestDistSq)
				{
					continue;
				}

				float SeatHeightCm = 0.0f, SeatTopZ = 0.0f, GroundZ = 0.0f;
				EvaluateInstance(Mesh, InstWorld, SeatHeightCm, SeatTopZ, GroundZ);
				if (SeatHeightCm < GSeatHeightMin || SeatHeightCm > GSeatHeightMax)
				{
					continue; // не стул/скамейка/диван/стол/коробка сидячей высоты
				}
				BestDistSq = DistSq;

				// Глубина сиденья (половина меньшей стороны) — для «носа» предмета.
				const FBoxSphereBounds B = Mesh->GetBounds();
				const float Scale = InstWorld.GetScale3D().Z > 0.0f ? InstWorld.GetScale3D().Z : 1.0f;
				const float Depth = FMath::Max(20.0f, FMath::Min(B.BoxExtent.X, B.BoxExtent.Y) * Scale);

				// Подходная точка: смотрим на направление от сиденья в сторону Origin
				// (т.е. монстр подходит к «лицу» предмета оттуда, где стоит).
				FVector ToOrigin(Origin2D.X - Loc.X, Origin2D.Y - Loc.Y, 0.0f);
				if (ToOrigin.SizeSquared() < 1.0f)
				{
					ToOrigin = FVector(1.0f, 0.0f, 0.0f);
				}
				const FVector ApproachDir = ToOrigin.GetSafeNormal2D();
				FVector Approach = FVector(
					Loc.X + ApproachDir.X * (Depth + 55.0f),
					Loc.Y + ApproachDir.Y * (Depth + 55.0f), GroundZ);

				Result.bValid = true;
				Result.Center = FVector(Loc.X, Loc.Y, SeatTopZ);
				Result.ApproachPoint = Approach;
				Result.SeatHeightCm = SeatHeightCm;
				Result.SeatTopZ = SeatTopZ;
				Result.FacingYaw = (ApproachDir.Rotation().Yaw + 180.0f); // сесть, глядя на предмет

				if (DistSq <= GoodEnoughDistSq)
				{
					return Result; // нашли достаточно близкое место
				}
			}
		}
	}
	return Result;
}
}
#include "BackroomsItemIcons.h"
#include "BackroomsItemSystem.h"

#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "UObject/StrongObjectPtr.h"

#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"

namespace
{
	constexpr int32 GIconSize = 128;

	struct FIconState
	{
		// Актёр-капчер живой в мире (мир сам держит своих актёров). Держим слабо:
		// мир/тир умер — пересоздадим со свежим состоянием.
		TWeakObjectPtr<AActor> CaptureActor;
		TObjectPtr<USceneCaptureComponent2D> Capture;
		TObjectPtr<UStaticMeshComponent> MeshComp;
		TObjectPtr<UPointLightComponent> Light;

		// RT-иконки держим сильно: кисти ссылаются на них, а бесхозный транзиент
		// был бы собран GC. (Тот же класс проблемы, что и с data-asset'ами.)
		TMap<FName, TStrongObjectPtr<UTextureRenderTarget2D>> IconRTs;
		TArray<FName> Queue;
		FName Current;
		float CapturesLeft = 0.0f;

		TMap<FName, TSharedPtr<FSlateBrush>> Brushes;
	};

	FIconState& State()
	{
		static FIconState S;
		return S;
	}

	// Сбросить весь рендерер: убить актёра-капчера и все кэши. Вызывается при
	// смене мира и при полной очистке.
	void FreeRenderer()
	{
		FIconState& S = State();
		if (S.CaptureActor.IsValid())
		{
			S.CaptureActor->Destroy();
		}
		S.CaptureActor = nullptr;
		S.Capture = nullptr;
		S.MeshComp = nullptr;
		S.Light = nullptr;
		S.IconRTs.Reset();
		S.Queue.Reset();
		S.Current = NAME_None;
		S.CapturesLeft = 0.0f;
		S.Brushes.Reset();
	}

	// Убить старого капчера (если мир сменился) и поднять нового — в текущем мире.
	void EnsureRenderer(UWorld* World)
	{
		FIconState& S = State();
		if (S.CaptureActor.IsValid() && S.CaptureActor->GetWorld() == World && S.Capture)
		{
			return;
		}

		FreeRenderer();

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(),
			FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, -200000.0f)), Params);
		if (!Actor)
		{
			return;
		}
		S.CaptureActor = Actor;

		USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("IconRendererRoot"));
		Actor->SetRootComponent(Root);
		Root->RegisterComponent();

		S.MeshComp = NewObject<UStaticMeshComponent>(Actor, TEXT("IconMesh"));
		S.MeshComp->SetupAttachment(Root);
		S.MeshComp->SetMobility(EComponentMobility::Movable);
		S.MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		S.MeshComp->RegisterComponent();

		S.Light = NewObject<UPointLightComponent>(Actor, TEXT("IconLight"));
		S.Light->SetupAttachment(Root);
		S.Light->SetRelativeLocation(FVector(180.0f, -220.0f, 240.0f));
		S.Light->SetIntensity(5500.0f);
		S.Light->SetAttenuationRadius(2000.0f);
		S.Light->SetMobility(EComponentMobility::Movable);
		S.Light->RegisterComponent();

		S.Capture = NewObject<USceneCaptureComponent2D>(Actor, TEXT("IconCapture"));
		S.Capture->SetupAttachment(Root);
		S.Capture->SetRelativeLocation(FVector(0.0f, -300.0f, 55.0f));
		S.Capture->SetRelativeRotation(FRotator(-10.0f, 0.0f, 0.0f));
		S.Capture->FOVAngle = 30.0f;
		S.Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		S.Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		S.Capture->bCaptureEveryFrame = false;
		S.Capture->bAlwaysPersistRenderingState = true;
		S.Capture->ShowFlags.SetAtmosphere(false);
		S.Capture->ShowFlags.SetFog(false);
		S.Capture->ShowFlags.SetCloud(false);
		S.Capture->RegisterComponent();
	}

	// Поставить меш в кадр: нормализованный масштаб, геометрический центр в
	// начало, фиксированный «витринный» угол.
	void ConfigureMeshForIcon(UStaticMesh* Mesh)
	{
		FIconState& S = State();
		if (!S.MeshComp || !Mesh)
		{
			return;
		}
		S.MeshComp->SetStaticMesh(Mesh);

		const FBoxSphereBounds B = Mesh->GetBounds();
		const float MaxExtent = FMath::Max3(B.BoxExtent.X, B.BoxExtent.Y, B.BoxExtent.Z);
		const float Scale = (MaxExtent > 1.0f) ? (GIconSize * 0.42f / MaxExtent) : 0.5f;
		S.MeshComp->SetRelativeScale3D(FVector(Scale));
		S.MeshComp->SetRelativeLocation(-B.Origin * Scale);
		S.MeshComp->SetRelativeRotation(FRotator(-16.0f, 140.0f, 0.0f));

		S.Capture->ShowOnlyComponents.Reset();
		S.Capture->ShowOnlyComponents.Add(S.MeshComp);
	}
}

TSharedPtr<FSlateBrush> BackroomsItemIcons::GetItemIcon(UWorld* World, FName ItemId)
{
	if (!World || ItemId.IsNone())
	{
		return nullptr;
	}

	FIconState& S = State();
	EnsureRenderer(World);

	if (TSharedPtr<FSlateBrush>* Found = S.Brushes.Find(ItemId))
	{
		return *Found;
	}
	if (S.IconRTs.Contains(ItemId))
	{
		// Уже в очереди рендера — вернётся следующими кадрами.
		return nullptr;
	}

	UStaticMesh* Mesh = UBackroomsItemSystem::GetWorldMeshForItem(ItemId);
	if (!Mesh)
	{
		// Меша нет — иконка невозможна, слот остаётся текстовым.
		return nullptr;
	}

	UTextureRenderTarget2D* RT = NewObject<UTextureRenderTarget2D>(
		GetTransientPackage(), UTextureRenderTarget2D::StaticClass(),
		*FString::Printf(TEXT("ItemIconRT_%s"), *ItemId.ToString()), RF_Transient);
	RT->RenderTargetFormat = RTF_RGBA8;
	RT->ClearColor = FLinearColor(0.04f, 0.04f, 0.05f, 1.0f);
	RT->InitAutoFormat(GIconSize, GIconSize);
	RT->UpdateResourceImmediate(true);

	S.IconRTs.Emplace(ItemId, TStrongObjectPtr<UTextureRenderTarget2D>(RT));
	S.Queue.Add(ItemId);
	return nullptr;
}

void BackroomsItemIcons::TickRender(UWorld* World, float DeltaTime)
{
	FIconState& S = State();
	if (!World || !S.CaptureActor.IsValid() || S.CaptureActor->GetWorld() != World)
	{
		return;
	}
	if (S.Queue.Num() == 0)
	{
		return;
	}

	const FName ItemId = S.Queue[0];

	if (S.Current != ItemId)
	{
		const TStrongObjectPtr<UTextureRenderTarget2D>* RT = S.IconRTs.Find(ItemId);
		UStaticMesh* Mesh = UBackroomsItemSystem::GetWorldMeshForItem(ItemId);
		if (!RT || !Mesh)
		{
			S.Queue.RemoveAt(0);
			return;
		}
		S.Current = ItemId;
		S.Capture->TextureTarget = RT->Get();
		ConfigureMeshForIcon(Mesh);
		S.Capture->bCaptureEveryFrame = true;
		S.CapturesLeft = 0.12f;
		return;
	}

	if (S.CapturesLeft > 0.0f)
	{
		S.CapturesLeft -= DeltaTime;
		if (S.CapturesLeft <= 0.0f)
		{
			S.Capture->bCaptureEveryFrame = false;
			const TStrongObjectPtr<UTextureRenderTarget2D>* RT = S.IconRTs.Find(S.Current);
			if (RT && RT->Get())
			{
				TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
				Brush->SetResourceObject(RT->Get());
				Brush->ImageSize = FVector2D(GIconSize, GIconSize);
				S.Brushes.Add(S.Current, Brush);
			}
			S.Queue.RemoveAt(0);
			S.Current = NAME_None;
		}
	}
}

void BackroomsItemIcons::DestroyForWorld(UWorld* World)
{
	(void)World;
	FreeRenderer();
}
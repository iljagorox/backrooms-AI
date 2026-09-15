#include "BackroomsInventoryWidget.h"
#include "BackroomsPlayerCharacter.h"
#include "BackroomsItemSystem.h"
#include "BackroomsItemIcons.h"
#include "BackroomsLocalization.h"
#include "BackroomsInputSettings.h"

#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "Engine/Font.h"

namespace
{
	// Порядок предметов в бестиарии — как в каталоге (расходники).
	constexpr int32 GGridCols = 3;

	const FSlateBrush* GetInvPanelBrush()
	{
		static FSlateRoundedBoxBrush Brush(
			FLinearColor(0.06f, 0.06f, 0.09f, 0.90f), 16.0f,
			FLinearColor(0.40f, 0.38f, 0.30f, 0.45f), 1.5f);
		return &Brush;
	}

	const FSlateBrush* GetInvSlotBrush(bool bSelected)
	{
		static FSlateRoundedBoxBrush Normal(
			FLinearColor(0.09f, 0.09f, 0.12f, 0.85f), 10.0f,
			FLinearColor(0.35f, 0.33f, 0.26f, 0.45f), 1.5f);
		static FSlateRoundedBoxBrush Selected(
			FLinearColor(0.18f, 0.15f, 0.08f, 0.95f), 10.0f,
			FLinearColor(0.95f, 0.82f, 0.38f, 0.95f), 2.0f);
		return bSelected ? &Selected : &Normal;
	}

	// Кисть-заглушка на время, пока иконка рендерится (или её нет).
	const FSlateBrush* GetIconPlaceholderBrush()
	{
		static FSlateColorBrush Brush(FLinearColor(0.12f, 0.12f, 0.14f, 0.85f));
		return &Brush;
	}

	// Иконка предмета из runtime-рендера; заодно ставит её в очередь.
	const FSlateBrush* GetItemIconBrush(ABackroomsPlayerCharacter* P, const FBackroomsItemDef& Def)
	{
		UWorld* W = P ? P->GetWorld() : nullptr;
		const TSharedPtr<FSlateBrush> Brush = W
			? BackroomsItemIcons::GetItemIcon(W, Def.Id)
			: nullptr;
		return Brush.IsValid() ? Brush.Get() : GetIconPlaceholderBrush();
	}
}

ABackroomsPlayerCharacter* UBackroomsInventoryWidget::GetPlayer() const
{
	return Player.IsValid() ? Player.Get() : Cast<ABackroomsPlayerCharacter>(GetOwningPlayerPawn());
}

FString UBackroomsInventoryWidget::ItemName(const FBackroomsItemDef& Def) const
{
	const FString Key = FString::Printf(TEXT("Item.%s.Name"), *Def.Id.ToString());
	const FString Loc = BackroomsLoc::Get(*Key);
	// Если перевода нет, Get вернёт сам ключ — тогда берём имя из каталога.
	return (Loc == Key) ? Def.DisplayName : Loc;
}

FString UBackroomsInventoryWidget::ItemDesc(const FBackroomsItemDef& Def) const
{
	const FString Key = FString::Printf(TEXT("Item.%s.Desc"), *Def.Id.ToString());
	const FString Loc = BackroomsLoc::Get(*Key);
	return (Loc == Key) ? Def.Description : Loc;
}

TSharedRef<SWidget> UBackroomsInventoryWidget::RebuildWidget()
{
	// Каталог строим один раз: он статический и не меняется.
	if (Catalog.Num() == 0)
	{
		UBackroomsItemSystem::BuildDefaultItemDefs(Catalog);
	}

	MenuRoot = SNew(SOverlay);
	RebuildUI();
	return MenuRoot.ToSharedRef();
}

void UBackroomsInventoryWidget::RebuildUI()
{
	if (!MenuRoot.IsValid())
	{
		return;
	}
	MenuRoot->ClearChildren();

	const FLinearColor Gold(0.95f, 0.85f, 0.45f);
	const FLinearColor Value(0.96f, 0.96f, 0.96f);
	const FLinearColor Dim(0.62f, 0.62f, 0.62f);

	ABackroomsPlayerCharacter* P = GetPlayer();
	UBackroomsItemSystem* Items = P ? P->GetItemSystem() : nullptr;

	// --- Слоты предметов ---
	TSharedRef<SUniformGridPanel> Grid = SNew(SUniformGridPanel).SlotPadding(FMargin(8.0f));
	for (int32 i = 0; i < Catalog.Num(); ++i)
	{
		const FBackroomsItemDef& Def = Catalog[i];
		const int32 Count = Items ? Items->GetItemCount(Def.Id) : 0;
		const bool bOwned = Count > 0;
		const bool bSel = (i == SelectedIndex);

		TSharedRef<SBorder> SlotBorder =
			SNew(SBorder)
			.BorderImage(GetInvSlotBrush(bSel))
			.Padding(FMargin(10.0f, 8.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(72.0f).HeightOverride(72.0f)
						[
							SNew(SImage)
							.Image(GetItemIconBrush(P, Def))
							.DesiredSizeOverride(FVector2D(72.0f, 72.0f))
							.ColorAndOpacity(bOwned
								? FLinearColor::White
								: FLinearColor(1.0f, 1.0f, 1.0f, 0.35f))
						]
					]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(FMargin(0.0f, 5.0f, 0.0f, 0.0f))
					[ SNew(STextBlock).Text(FText::FromString(ItemName(Def)))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
						.ColorAndOpacity(bSel ? Gold : (bOwned ? Value : Dim)) ]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(FMargin(0.0f, 2.0f, 0.0f, 0.0f))
					[ SNew(STextBlock)
						.Text(FText::FromString(bOwned
							? FString::Printf(TEXT("x%d"), Count)
							: BackroomsLoc::Get(TEXT("HUD.Empty"))))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						.ColorAndOpacity(Dim) ]
			];

		Grid->AddSlot(i % GGridCols, i / GGridCols)
			[ SNew(SButton)
				.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.OnClicked_Lambda([this, i]() -> FReply { SelectItem(i); return FReply::Handled(); })
				[ SlotBorder ] ];
	}

	// --- 3D-превью выбранного предмета ---
	PreviewBrush = MakeShared<FSlateBrush>();
	PreviewImage = SNew(SImage).Image(PreviewBrush.Get());
	EnsurePreviewSetup();
	if (Catalog.IsValidIndex(SelectedIndex))
	{
		SetPreviewMesh(UBackroomsItemSystem::GetWorldMeshForItem(Catalog[SelectedIndex].Id));
	}

	TSharedRef<SBox> PreviewBox =
		SNew(SBox).WidthOverride(420.0f).HeightOverride(420.0f)
		[ SNew(SBorder).BorderImage(GetInvSlotBrush(false)).Padding(FMargin(6.0f))
			[ PreviewImage.ToSharedRef() ] ];

	// --- Левая колонка: имя, описание, эффект ---
	TSharedRef<SVerticalBox> Detail = SNew(SVerticalBox);
	if (Catalog.IsValidIndex(SelectedIndex))
	{
		const FBackroomsItemDef& Def = Catalog[SelectedIndex];
		Detail->AddSlot().AutoHeight().Padding(FMargin(0.0f, 0.0f, 0.0f, 10.0f))
			[ SAssignNew(DetailName, STextBlock).Text(FText::FromString(ItemName(Def)))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 26)).ColorAndOpacity(Gold) ];
		Detail->AddSlot().AutoHeight().Padding(FMargin(0.0f, 0.0f, 0.0f, 16.0f))
			[ SAssignNew(DetailDesc, STextBlock).Text(FText::FromString(ItemDesc(Def)))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 15)).ColorAndOpacity(Value)
				.AutoWrapText(true) ];

		// Эффект: тип восстановления и величина из каталога.
		const TCHAR* StatKey = TEXT("HUD.Health");
		switch (Def.RestoreType)
		{
		case 0: StatKey = TEXT("HUD.Hunger"); break;
		case 1: StatKey = TEXT("HUD.Thirst"); break;
		case 3: StatKey = TEXT("HUD.Sanity"); break;
		default: break;
		}
		Detail->AddSlot().AutoHeight()
			[ SAssignNew(DetailStats, STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("+%.0f  %s"),
					Def.RestoreAmount, *BackroomsLoc::Get(StatKey))))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
				.ColorAndOpacity(FLinearColor(0.55f, 0.85f, 0.60f)) ];
	}

	// Подсказка управления: закрыть панель.
	const FString CloseKey = BackroomsInput::KeyDisplayName(
		BackroomsInput::GetEffectiveKey(TEXT("Inventory"), TEXT("Tab")));
	Detail->AddSlot().FillHeight(1.0f)[ SNew(SSpacer) ];
	Detail->AddSlot().AutoHeight().Padding(FMargin(0.0f, 12.0f, 0.0f, 0.0f))
		[ SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s — %s"),
				*CloseKey, *BackroomsLoc::Get(TEXT("Menu.Back")))))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13)).ColorAndOpacity(Dim) ];

	// --- Компоновка: описание слева, 3D-модель справа, сетка снизу ---
	TSharedRef<SVerticalBox> Col = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(24.0f, 18.0f, 24.0f, 10.0f))
			[ SNew(STextBlock).Text(BackroomsLoc::Text(TEXT("HUD.Inventory")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 30)).ColorAndOpacity(Gold)
				.ShadowOffset(FVector2D(2, 2)).ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.8f)) ]
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(FMargin(24.0f, 0.0f, 24.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Top)
					[ SNew(SBorder).BorderImage(GetInvPanelBrush()).Padding(FMargin(20.0f))
						[ Detail ] ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(16.0f, 0.0f, 0.0f, 0.0f)).VAlign(VAlign_Top)
					[ SNew(SBorder).BorderImage(GetInvPanelBrush()).Padding(FMargin(12.0f))
						[ PreviewBox ] ]
			]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(24.0f, 0.0f, 24.0f, 18.0f))
			[ Grid ];

	TSharedRef<SBorder> BG = SNew(SBorder).BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.02f, 0.80f));
	BG->SetContent(Col);
	MenuRoot->AddSlot()[ BG ];
}

void UBackroomsInventoryWidget::SelectItem(int32 Index)
{
	if (!Catalog.IsValidIndex(Index))
	{
		return;
	}
	SelectedIndex = Index;
	RebuildUI();
}

void UBackroomsInventoryWidget::EnsurePreviewSetup()
{
	UWorld* World = GetWorld();
	if (!World || !PreviewImage.IsValid())
	{
		return;
	}

	if (!PreviewRT)
	{
		PreviewRT = NewObject<UTextureRenderTarget2D>(this, TEXT("BackroomsInvRT"));
		PreviewRT->RenderTargetFormat = RTF_RGBA8;
		PreviewRT->ClearColor = FLinearColor(0.03f, 0.03f, 0.04f, 1.0f);
		PreviewRT->InitAutoFormat(512, 512);
		PreviewRT->UpdateResourceImmediate(true);
	}

	if (!PreviewActor)
	{
		// Спавним далеко под уровнем, чтобы кадр не поймал геометрию мира.
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PreviewActor = World->SpawnActor<AActor>(AActor::StaticClass(),
			FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, -100000.0f)), Params);
	}

	if (PreviewActor && !PreviewMeshComp)
	{
		USceneComponent* Root = NewObject<USceneComponent>(PreviewActor, TEXT("InvPreviewRoot"));
		PreviewActor->SetRootComponent(Root);
		Root->RegisterComponent();

		PreviewMeshComp = NewObject<UStaticMeshComponent>(PreviewActor, TEXT("InvPreviewMesh"));
		PreviewMeshComp->SetupAttachment(Root);
		PreviewMeshComp->SetMobility(EComponentMobility::Movable);
		PreviewMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PreviewMeshComp->RegisterComponent();

		// Свет: без него модель была бы чёрной. ShowOnlyList ограничивает только
		// примитивы (меши), свет применяется всегда.
		UPointLightComponent* Light = NewObject<UPointLightComponent>(PreviewActor, TEXT("InvPreviewLight"));
		Light->SetupAttachment(Root);
		Light->SetRelativeLocation(FVector(200.0f, -250.0f, 260.0f));
		Light->SetIntensity(8000.0f);
		Light->SetAttenuationRadius(4000.0f);
		Light->SetMobility(EComponentMobility::Movable);
		Light->RegisterComponent();

		SceneCapture = NewObject<USceneCaptureComponent2D>(PreviewActor, TEXT("InvPreviewCapture"));
		SceneCapture->SetupAttachment(Root);
		SceneCapture->SetRelativeLocation(FVector(0.0f, -350.0f, 60.0f));
		SceneCapture->SetRelativeRotation(FRotator(-8.0f, 90.0f, 0.0f));
		SceneCapture->FOVAngle = 35.0f;
		SceneCapture->TextureTarget = PreviewRT;
		SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		SceneCapture->ShowOnlyComponents.Add(PreviewMeshComp);
		SceneCapture->bCaptureEveryFrame = true;
		SceneCapture->bAlwaysPersistRenderingState = true;
		SceneCapture->ShowFlags.SetAtmosphere(false);
		SceneCapture->ShowFlags.SetFog(false);
		SceneCapture->ShowFlags.SetCloud(false);
		SceneCapture->RegisterComponent();
	}

	if (PreviewBrush.IsValid() && PreviewRT)
	{
		PreviewBrush->SetResourceObject(PreviewRT);
		PreviewBrush->ImageSize = FVector2D(512.0f, 512.0f);
		if (PreviewImage.IsValid())
		{
			PreviewImage->SetImage(PreviewBrush.Get());
		}
	}
}

void UBackroomsInventoryWidget::SetPreviewMesh(UStaticMesh* Mesh)
{
	if (!PreviewMeshComp)
	{
		return;
	}
	PreviewMeshComp->SetStaticMesh(Mesh);
	if (!Mesh)
	{
		return;
	}
	// Нормализуем масштаб: модель любого размера вписывается в кадр, а её
	// геометрический центр ставим в начало координат превью.
	const FBoxSphereBounds B = Mesh->GetBounds();
	const float MaxExtent = FMath::Max3(B.BoxExtent.X, B.BoxExtent.Y, B.BoxExtent.Z);
	const float Scale = (MaxExtent > 1.0f) ? (120.0f / MaxExtent) : 1.0f;
	PreviewMeshComp->SetRelativeScale3D(FVector(Scale));
	PreviewMeshComp->SetRelativeLocation(-B.Origin * Scale);
	PreviewMeshComp->SetRelativeRotation(FRotator(0.0f, PreviewYaw, 0.0f));
}

void UBackroomsInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Движем очередь рендера иконок (запрошены в RebuildUI). Инвентарь обычно
	// закрыт — в эти кадры очередь двигает HUD своим TickRender.
	if (UWorld* W = GetWorld())
	{
		for (const FBackroomsItemDef& Def : Catalog)
		{
			BackroomsItemIcons::GetItemIcon(W, Def.Id);
		}
		BackroomsItemIcons::TickRender(W, InDeltaTime);
	}

	// Медленно вращаем модель, пока панель открыта — «потыкать и рассмотреть».
	if (!bIsOpen || !PreviewMeshComp || !PreviewMeshComp->GetStaticMesh())
	{
		return;
	}
	PreviewYaw += InDeltaTime * 35.0f;
	PreviewMeshComp->SetRelativeRotation(FRotator(0.0f, PreviewYaw, 0.0f));
}

void UBackroomsInventoryWidget::ShowInventory(bool bShow)
{
	bIsOpen = bShow;
	if (MenuRoot.IsValid())
	{
		MenuRoot->SetVisibility(bShow ? EVisibility::Visible : EVisibility::Collapsed);
	}
	SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (SceneCapture)
	{
		SceneCapture->bCaptureEveryFrame = bShow;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}
	if (bShow)
	{
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}

void UBackroomsInventoryWidget::ToggleInventory()
{
	ShowInventory(!bIsOpen);
}

void UBackroomsInventoryWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	MenuRoot.Reset();
	GridBox.Reset();
	DetailName.Reset();
	DetailDesc.Reset();
	DetailStats.Reset();
	PreviewImage.Reset();
	PreviewBrush.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

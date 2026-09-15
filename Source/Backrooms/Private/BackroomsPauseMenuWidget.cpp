#include "BackroomsPauseMenuWidget.h"
#include "BackroomsAudioSettings.h"
#include "BackroomsLocalization.h"
#include "BackroomsPlayerCharacter.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/PostProcessVolume.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "BackroomsInputSettings.h"
#include "BackroomsQualitySettings.h"
#include "Widgets/SOverlay.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"

static FString QualityToText(int32 Level)
{
	switch (Level)
	{
	case 0: return BackroomsLoc::Get(TEXT("Q.Low"));
	case 1: return BackroomsLoc::Get(TEXT("Q.Medium"));
	case 2: return BackroomsLoc::Get(TEXT("Q.High"));
	case 3: return BackroomsLoc::Get(TEXT("Q.Epic"));
	default: return BackroomsLoc::Get(TEXT("Q.Ultra"));
	}
}

// Стиль слайдера: гладкий тёмный жёлоб-пилюля + один золотой ползунок.
// Кисти задаются явно, иначе дефолтные 9-slice текстуры растягиваются и
// дают «рваный» вид.
static const FSliderStyle& GetBackroomsSliderStyle()
{
	static FSliderStyle Style = []()
	{
		static FSlateRoundedBoxBrush Track(FLinearColor(0.025f, 0.025f, 0.035f, 0.95f), 5.0f);
		static FSlateRoundedBoxBrush TrackHover(FLinearColor(0.06f, 0.06f, 0.08f, 0.95f), 5.0f);
		static FSlateRoundedBoxBrush TrackDisabled(FLinearColor(0.025f, 0.025f, 0.035f, 0.55f), 5.0f);
		static FSlateRoundedBoxBrush Thumb(FLinearColor(0.93f, 0.80f, 0.38f, 1.0f), 9.0f);
		static FSlateRoundedBoxBrush ThumbHover(FLinearColor(1.0f, 0.90f, 0.52f, 1.0f), 10.0f);
		static FSlateRoundedBoxBrush ThumbDisabled(FLinearColor(0.42f, 0.42f, 0.42f, 0.75f), 9.0f);

		Track.ImageSize = FVector2D(0.0f, 0.0f);
		TrackHover.ImageSize = FVector2D(0.0f, 0.0f);
		TrackDisabled.ImageSize = FVector2D(0.0f, 0.0f);
		Thumb.ImageSize = FVector2D(18.0f, 18.0f);
		ThumbHover.ImageSize = FVector2D(20.0f, 20.0f);
		ThumbDisabled.ImageSize = FVector2D(18.0f, 18.0f);

		FSliderStyle S;
		S.BarThickness = 10.0f;
		S.NormalBarImage = Track;
		S.HoveredBarImage = TrackHover;
		S.DisabledBarImage = TrackDisabled;
		S.NormalThumbImage = Thumb;
		S.HoveredThumbImage = ThumbHover;
		S.DisabledThumbImage = ThumbDisabled;
		return S;
	}();
	return Style;
}

// Другие кисти — сайдбар и карточка контента находятся в
// BackroomsMainMenuWidget.cpp. Здесь свои, потому что этот файл тоже статический.
// Тёмная панель категорий слева.
static const FSlateBrush* GetBackroomsSideBarBrush()
{
	static FSlateRoundedBoxBrush Brush(
		FLinearColor(0.045f, 0.045f, 0.07f, 0.86f),
		0.0f,
		FLinearColor(0.30f, 0.28f, 0.20f, 0.50f),
		1.0f);
	return &Brush;
}

// Карточка контента справа.
static const FSlateBrush* GetBackroomsContentCardBrush()
{
	static FSlateRoundedBoxBrush Brush(
		FLinearColor(0.085f, 0.085f, 0.11f, 0.72f),
		14.0f,
		FLinearColor(0.35f, 0.33f, 0.26f, 0.45f),
		1.5f);
	return &Brush;
}

// Карточка секции в правой панели: заголовок + контент, без внешней рамки.
static TSharedRef<SWidget> MakePauseSection(const FText& Title, TSharedRef<SWidget> Content)
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f, 0.0f, 2.0f, 8.0f)
		[
			SNew(STextBlock)
			.Text(Title)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
			.ColorAndOpacity(FLinearColor(0.96f, 0.90f, 0.62f))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			Content
		];
}

// Ряд настроек: подпись слева, ползунок посередине, значение (надпись) справа.
// OutLabel получает заполненный текст-указатель для обновления значения.
static TSharedRef<SHorizontalBox> MakePauseRow(const FText& Label, float SliderValue, TSharedPtr<STextBlock>& OutLabel, const FText& InitialValue, TFunction<void(float)> Handler)
{
	// Подпись — фиксированная доля, значение — фиксированная ширина справа,
	// слайдер забирает середину. Так слайдер не «съезжает» вбок из-за длинной
	// подписи и всегда стоит по центру строки.
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.34f).Padding(4.0f).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
			.ColorAndOpacity(FLinearColor(0.92f, 0.84f, 0.55f))
		]
		+ SHorizontalBox::Slot().FillWidth(0.66f).Padding(24.0f, 4.0f).VAlign(VAlign_Center)
		[
			SNew(SBox).HeightOverride(30.0f).VAlign(VAlign_Center)
			[
				SNew(SSlider).Style(&GetBackroomsSliderStyle())
				.Value(SliderValue)
				.OnValueChanged_Lambda(Handler)
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 4.0f, 4.0f, 4.0f).VAlign(VAlign_Center)
		[
			SNew(SBox).WidthOverride(150.0f).HAlign(HAlign_Right)
			[
				SAssignNew(OutLabel, STextBlock)
				.Text(InitialValue)
				.Justification(ETextJustify::Right)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
				.ColorAndOpacity(FLinearColor(0.90f, 0.90f, 0.92f))
			]
		];
}

void UBackroomsPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// Иначе "widget ... does not support focus" и SetWidgetToFocus не работает.
	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Collapsed);
}

TSharedRef<SWidget> UBackroomsPauseMenuWidget::RebuildWidget()
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;

	OverallQ   = Settings ? Settings->GetOverallScalabilityLevel() : 2;
	ViewDist   = Settings ? Settings->GetViewDistanceQuality() : 2;
	ShadowQ    = Settings ? Settings->GetShadowQuality() : 2;
	TexturesQ  = Settings ? Settings->GetTextureQuality() : 2;
	AAQuality  = Settings ? Settings->GetAntiAliasingQuality() : 2;
	PostProcess = Settings ? Settings->GetPostProcessingQuality() : 2;
	EffectsQuality = Settings ? Settings->GetVisualEffectQuality() : 2;

	if (OverallQ < 0) OverallQ = 2;

	FOV = 90.0f;
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ABackroomsPlayerCharacter* C = Cast<ABackroomsPlayerCharacter>(PC->GetPawn()))
		{
			if (UCameraComponent* Cam = C->FindComponentByClass<UCameraComponent>())
			{
				FOV = Cam->FieldOfView;
			}
		}
	}

	bVSync = Settings ? Settings->IsVSyncEnabled() : false;

	MasterVolume  = BackroomsAudio::GetVolume(TEXT("MasterVolume"), 1.0f);
	MenuVolume    = BackroomsAudio::GetVolume(TEXT("MenuVolume"), 1.0f);
	GameVolume    = BackroomsAudio::GetVolume(TEXT("GameVolume"), 1.0f);
	MonsterVolume = BackroomsAudio::GetVolume(TEXT("MonsterVolume"), 1.0f);
	OtherVolume   = BackroomsAudio::GetVolume(TEXT("OtherVolume"), 1.0f);

	MenuRoot = SNew(SOverlay);
	RebuildUI();
	return MenuRoot.ToSharedRef();
}

void UBackroomsPauseMenuWidget::RebuildUI()
{
	if (!MenuRoot.IsValid())
	{
		return;
	}
	MenuRoot->ClearChildren();

	const FLinearColor LabelColor(0.92f, 0.84f, 0.55f);
	const FLinearColor ValueColor(0.98f, 0.98f, 0.98f);
	const FLinearColor DimColor(0.62f, 0.62f, 0.62f);
	const FButtonStyle& NoBorder = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");

	// --- ГРАФИКА ---
	TSharedRef<SVerticalBox> GraphicsBox = SNew(SVerticalBox);
	GraphicsBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.Quality.Overall"))), (float)OverallQ / 3.0f, OverallQualityText,
			FText::FromString(QualityToText(OverallQ)),
			[this](float V) { OnOverallQualityChanged(V); })
	];
	GraphicsBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.Shadows"))), (float)ShadowQ / 3.0f, ShadowQualityText,
			FText::FromString(QualityToText(ShadowQ)),
			[this](float V) { OnShadowQualityChanged(V); })
	];
	GraphicsBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.Textures"))), (float)TexturesQ / 3.0f, TexturesQualityText,
			FText::FromString(QualityToText(TexturesQ)),
			[this](float V) { OnTexturesQualityChanged(V); })
	];
	GraphicsBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.ViewDistance"))), (float)ViewDist / 3.0f, ViewDistanceText,
			FText::FromString(QualityToText(ViewDist)),
			[this](float V) { OnViewDistanceChanged(V); })
	];
	GraphicsBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.AntiAliasing"))), (float)AAQuality / 3.0f, AAText,
			FText::FromString(QualityToText(AAQuality)),
			[this](float V) { OnAntiAliasChanged(V); })
	];
	GraphicsBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.PostProcess"))), (float)PostProcess / 3.0f, PostProcessText,
			FText::FromString(QualityToText(PostProcess)),
			[this](float V) { OnPostProcessChanged(V); })
	];
	GraphicsBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.Effects"))), (float)EffectsQuality / 3.0f, EffectsText,
			FText::FromString(QualityToText(EffectsQuality)),
			[this](float V) { OnEffectsChanged(V); })
	];
GraphicsBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.DepthOfField"))), (float)DofQuality / 3.0f, DofText,
			FText::FromString(QualityToText(DofQuality)),
			[this](float V) { OnDofChanged(V); })
	];
	GraphicsBox->AddSlot().AutoHeight().Padding(FMargin(0.0f, 18.0f, 0.0f, 0.0f))
	[
		SNew(SButton)
		.ContentPadding(FMargin(18, 8))
		.OnClicked_Lambda([this]() -> FReply { return OnPotatoModeClicked(); })
		[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Set.PotatoMode"))))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(BackroomsQuality::IsPotatoMode() ? FLinearColor(0.35f, 0.85f, 0.40f) : FLinearColor(0.9f, 0.9f, 0.9f)) ]
	];
	GraphicsBox->AddSlot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
	[
		SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Set.PotatoMode.Hint"))))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12)).ColorAndOpacity(DimColor)
			.AutoWrapText(true)
	];

	// --- КАМЕРА ---
	TSharedRef<SVerticalBox> CameraBox = SNew(SVerticalBox);
	CameraBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.FOV"))), (FOV - 60.0f) / 60.0f, FovText,
			FText::FromString(FString::Printf(TEXT("%d"), FMath::RoundToInt(FOV))),
			[this](float V) { OnFovChanged(V); })
	];
	CameraBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.Sensitivity"))), (Sensitivity - 0.2f) / 4.8f, SensitivityText,
			FText::FromString(FString::Printf(TEXT("%.1f"), Sensitivity)),
			[this](float V) { OnSensitivityChanged(V); })
	];
	CameraBox->AddSlot().AutoHeight()
	[
MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.RecordingEffect"))), (float)RecordingEffect / 3.0f, RecordingEffectText,
			FText::FromString(BackroomsLoc::Get(TEXT("Set.RecordingEffect.Off"))),
			[this](float V) { OnRecordingEffectChanged(V); })
	];
	CameraBox->AddSlot().AutoHeight()
	[
MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.VSync"))), bVSync ? 1.0f : 0.0f, VSyncText,
			FText::FromString(bVSync ? BackroomsLoc::Get(TEXT("Keys.On")) : BackroomsLoc::Get(TEXT("Keys.Off"))),
			[this](float V) { OnVSyncChanged(V); })
	];
	CameraBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.Gamma"))), (Gamma - 0.5f) / 1.5f, GammaText,
			FText::FromString(FString::Printf(TEXT("%.2f"), Gamma)),
			[this](float V) { OnGammaChanged(V); })
	];

	// --- ЗВУК ---
	TSharedRef<SVerticalBox> SoundBox = SNew(SVerticalBox);
	SoundBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.Volume.Master"))), MasterVolume, MasterVolText,
			FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MasterVolume * 100.0f))),
			[this](float V) { OnVolumeChanged(TEXT("MasterVolume"), V, MasterVolText); })
	];
	SoundBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.Volume.Menu"))), MenuVolume, MenuVolText,
			FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MenuVolume * 100.0f))),
			[this](float V) { OnVolumeChanged(TEXT("MenuVolume"), V, MenuVolText); })
	];
	SoundBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.Volume.Game"))), GameVolume, GameVolText,
			FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(GameVolume * 100.0f))),
			[this](float V) { OnVolumeChanged(TEXT("GameVolume"), V, GameVolText); })
	];
	SoundBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.Volume.Monster"))), MonsterVolume, MonsterVolText,
			FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MonsterVolume * 100.0f))),
			[this](float V) { OnVolumeChanged(TEXT("MonsterVolume"), V, MonsterVolText); })
	];
	SoundBox->AddSlot().AutoHeight()
	[
		MakePauseRow(FText::FromString(BackroomsLoc::Get(TEXT("Set.Volume.Other"))), OtherVolume, OtherVolText,
			FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(OtherVolume * 100.0f))),
			[this](float V) { OnVolumeChanged(TEXT("OtherVolume"), V, OtherVolText); })
	];

// --- УПРАВЛЕНИЕ: переназначение клавиш ---
	struct FBindInfo { const TCHAR* Id; const TCHAR* DefKey; };
	static const FBindInfo Binds[] = {
		{ TEXT("MoveForwardPlus"),  TEXT("W") },
		{ TEXT("MoveForwardMinus"), TEXT("S") },
		{ TEXT("MoveRightPlus"),    TEXT("D") },
		{ TEXT("MoveRightMinus"),   TEXT("A") },
		{ TEXT("Jump"),             TEXT("SpaceBar") },
		{ TEXT("Sprint"),           TEXT("LeftShift") },
		{ TEXT("Flashlight"),       TEXT("F") },
		{ TEXT("View"),             TEXT("V") },
		{ TEXT("Attack"),           TEXT("Q") },
		{ TEXT("Grab"),             TEXT("E") },
		{ TEXT("Push"),             TEXT("LeftMouseButton") },
		{ TEXT("Throw"),            TEXT("RightMouseButton") },
		{ TEXT("Inspect"),          TEXT("LeftAlt") },
		{ TEXT("Inventory"),        TEXT("Tab") },
		{ TEXT("Use"),              TEXT("R") },
		{ TEXT("Slot1"),            TEXT("One") },
		{ TEXT("Slot2"),            TEXT("Two") },
		{ TEXT("Slot3"),            TEXT("Three") },
		{ TEXT("Slot4"),            TEXT("Four") },
		{ TEXT("PauseMenu"),        TEXT("Escape") },
	};

	auto ReadonlyRow = [&](const FString& Key, const FString& Action) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.5f).VAlign(VAlign_Center).Padding(0, 5)
				[ SNew(STextBlock).Text(FText::FromString(Action)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 15)).ColorAndOpacity(ValueColor) ]
			+ SHorizontalBox::Slot().FillWidth(0.5f).VAlign(VAlign_Center)
				[ SNew(STextBlock).Text(FText::FromString(Key)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 15)).ColorAndOpacity(DimColor) ];
	};

auto BindRow = [&](const FString& BindId, const FString& DefaultKey) -> TSharedRef<SWidget>
	{
		const bool bWaitingThis = bWaitingForKey && PendingBindId == BindId;
		const FString KeyText = bWaitingThis ? TEXT("?") : BackroomsInput::KeyDisplayName(BackroomsInput::GetEffectiveKey(*BindId, *DefaultKey));
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.5f).VAlign(VAlign_Center).Padding(0, 5)
				[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(*(FString(TEXT("Bind.")) + BindId)))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 15)).ColorAndOpacity(ValueColor) ]
			+ SHorizontalBox::Slot().FillWidth(0.28f).VAlign(VAlign_Center)
				[
					SNew(SButton)
					.IsEnabled(!bWaitingForKey || bWaitingThis)
					.HAlign(HAlign_Fill)
					.ContentPadding(FMargin(10.0f, 6.0f))
					.OnClicked_Lambda([this, BindId]() -> FReply { return OnKeyBindClicked(BindId); })
					[ SNew(STextBlock).Text(FText::FromString(KeyText))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(bWaitingThis ? FLinearColor(0.35f, 0.85f, 0.40f) : FLinearColor(0.95f, 0.85f, 0.55f)) ]
				]
			+ SHorizontalBox::Slot().FillWidth(0.22f).VAlign(VAlign_Center).Padding(10, 0, 0, 0)
				[ SNew(STextBlock).Text(FText::FromString(bWaitingThis ? TEXT("") : BackroomsLoc::Get(TEXT("Bind.Change")))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)).ColorAndOpacity(DimColor) ];
	};

	auto BuildTabContent = [&](int32 Tab) -> TSharedRef<SWidget>
	{
		switch (Tab)
		{
case 0: return MakePauseSection(FText::FromString(BackroomsLoc::Get(TEXT("Set.Graphics"))), GraphicsBox);
		case 1: return MakePauseSection(FText::FromString(BackroomsLoc::Get(TEXT("Tab.Camera"))), CameraBox);
		case 2: return MakePauseSection(FText::FromString(BackroomsLoc::Get(TEXT("Tab.Sound"))), SoundBox);
		default: break;
		}

		TSharedRef<SVerticalBox> ControlsBox = SNew(SVerticalBox);
		ControlsBox->AddSlot().AutoHeight().Padding(2.0f, 0.0f, 2.0f, 12.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Tab.Controls"))))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22)).ColorAndOpacity(FLinearColor(0.96f, 0.90f, 0.62f)) ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ButtonStyle(&NoBorder)
					.ContentPadding(FMargin(12.0f, 4.0f))
					.Visibility(bWaitingForKey ? EVisibility::Visible : EVisibility::Collapsed)
					.OnClicked_Lambda([this]() -> FReply { return OnBindCancel(); })
[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Bind.Cancel"))))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15)).ColorAndOpacity(FLinearColor(0.95f, 0.42f, 0.38f)) ]
				]
		];

		if (bWaitingForKey)
		{
			ControlsBox->AddSlot().AutoHeight().Padding(2.0f, 0.0f, 2.0f, 10.0f)
[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Bind.PressKey"))))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)).ColorAndOpacity(FLinearColor(0.40f, 0.85f, 0.40f)) ];
		}

		ControlsBox->AddSlot().AutoHeight().Padding(2.0f, 0.0f, 2.0f, 6.0f)
[ ReadonlyRow(BackroomsLoc::Get(TEXT("Bind.Mouse")), BackroomsLoc::Get(TEXT("Bind.Look"))) ];

		for (const FBindInfo& B : Binds)
		{
			ControlsBox->AddSlot().AutoHeight()
				[ BindRow(B.Id, B.DefKey) ];
		}

		ControlsBox->AddSlot().AutoHeight().Padding(2.0f, 18.0f, 2.0f, 0.0f)
		[
			SNew(SButton)
			.ContentPadding(FMargin(18, 8))
			.OnClicked_Lambda([this]() -> FReply { return OnResetBindingsClicked(); })
[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Bind.Reset"))))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)).ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f)) ]
		];
		return ControlsBox;
	};

auto SideItem = [&](const FString& Label, int32 Tab, bool bSelected) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.ButtonStyle(&NoBorder)
			.HAlign(HAlign_Left)
			.ContentPadding(FMargin(26.0f, 10.0f, 26.0f, 10.0f))
			.OnClicked_Lambda([this, Tab]() -> FReply { return OnSettingsTabClicked(Tab); })
			[ SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", bSelected ? 20 : 16))
				.ColorAndOpacity(bSelected ? FLinearColor(0.98f, 0.88f, 0.45f) : FLinearColor(0.82f, 0.82f, 0.82f))
				.ShadowOffset(bSelected ? FVector2D(1.0f, 1.0f) : FVector2D::ZeroVector)
				.ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.7f)) ];
	};

	TSharedRef<SVerticalBox> SideBox = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(26.0f, 22.0f, 26.0f, 14.0f))
[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Settings.Categories")))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 13)).ColorAndOpacity(FLinearColor(0.55f, 0.55f, 0.55f)) ]
		+ SVerticalBox::Slot().AutoHeight()
			[ SideItem(BackroomsLoc::Get(TEXT("Set.Graphics")), 0, SettingsTab == 0) ]
		+ SVerticalBox::Slot().AutoHeight()
			[ SideItem(BackroomsLoc::Get(TEXT("Tab.Camera")), 1, SettingsTab == 1) ]
		+ SVerticalBox::Slot().AutoHeight()
			[ SideItem(BackroomsLoc::Get(TEXT("Tab.Sound")), 2, SettingsTab == 2) ]
		+ SVerticalBox::Slot().AutoHeight()
			[ SideItem(BackroomsLoc::Get(TEXT("Tab.Controls")), 3, SettingsTab == 3) ]
		+ SVerticalBox::Slot().FillHeight(1.0f) [ SNew(SSpacer) ]
		// Кнопки паузы — вертикально: в узком сайдбаре три кнопки в ряд
		// не помещались и наезжали друг на друга.
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(26.0f, 6.0f, 26.0f, 10.0f))
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FMargin(28, 10))
				.OnClicked_Lambda([this]() -> FReply { return OnResumeClicked(); })
				[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Pause.Resume")))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 20)).ColorAndOpacity(ValueColor) ]
			]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(26.0f, 0.0f, 26.0f, 10.0f))
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FMargin(28, 10))
				.OnClicked_Lambda([this]() -> FReply { return OnRestartClicked(); })
				[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Pause.Restart")))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 20)).ColorAndOpacity(ValueColor) ]
			]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(26.0f, 0.0f, 26.0f, 20.0f))
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FMargin(28, 10))
				.OnClicked_Lambda([this]() -> FReply { return OnQuitClicked(); })
				[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Pause.Quit")))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 20)).ColorAndOpacity(ValueColor) ]
			];

	TSharedRef<SBorder> SidePanel =
		SNew(SBorder)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.BorderImage(GetBackroomsSideBarBrush())
		[ SideBox ];

	const TSharedRef<SWidget> TabContent = BuildTabContent(SettingsTab);
	TSharedRef<SBorder> ContentCard =
		SNew(SBorder)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.BorderImage(GetBackroomsContentCardBrush())
		.Padding(FMargin(28.0f, 16.0f, 28.0f, 20.0f))
		[ TabContent ];

	// Панель настроек паузы во всю ширину экрана (как и в главном меню):
	// минимум отступов, широкий сайдбар, карточка забирает остаток.
	TSharedRef<SVerticalBox> SettingsCol =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(22.0f, 20.0f, 22.0f, 12.0f))
[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Settings.Title"))))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32)).ColorAndOpacity(FLinearColor(0.95f, 0.85f, 0.45f))
				.ShadowOffset(FVector2D(2, 2)).ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.8f)) ]
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(FMargin(22.0f, 0.0f, 22.0f, 20.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0.0f, 0.0f, 18.0f, 0.0f))
					[ SNew(SBox).WidthOverride(330.0f) [ SidePanel ] ]
				+ SHorizontalBox::Slot().FillWidth(1.0f)
					[ ContentCard ]
			];

	TSharedRef<SBorder> BG = SNew(SBorder).BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.02f, 0.62f));
	BG->SetContent(SettingsCol);
	MenuRoot->AddSlot() [ BG ];
}

void UBackroomsPauseMenuWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MenuRoot.Reset();
	OverallQualityText.Reset();
	ViewDistanceText.Reset();
	ShadowQualityText.Reset();
	TexturesQualityText.Reset();
	AAText.Reset();
	PostProcessText.Reset();
	EffectsText.Reset();
	DofText.Reset();
	RecordingEffectText.Reset();
	FovText.Reset();
	SensitivityText.Reset();
	VSyncText.Reset();
	GammaText.Reset();
	MasterVolText.Reset();
	MenuVolText.Reset();
	GameVolText.Reset();
	MonsterVolText.Reset();
	OtherVolText.Reset();
}

void UBackroomsPauseMenuWidget::ToggleMenu()
{
	ShowMenu(!bIsOpen);
}

void UBackroomsPauseMenuWidget::ShowMenu(bool bShow)
{
	bIsOpen = bShow;
	SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->SetPause(bShow);
		PC->bShowMouseCursor = bShow;
		if (bShow)
		{
			FInputModeGameAndUI Mode;
			Mode.SetWidgetToFocus(TakeWidget());
			PC->SetInputMode(Mode);
		}
		else
		{
			PC->SetInputMode(FInputModeGameOnly());
		}
	}

	if (bShow)
	{
		ApplyCameraEffect();
		ApplyAndSaveSettings();
	}
}

FReply UBackroomsPauseMenuWidget::OnResumeClicked()
{
	ShowMenu(false);
	return FReply::Handled();
}

FReply UBackroomsPauseMenuWidget::OnRestartClicked()
{
	ShowMenu(false);
	FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*CurrentLevel));
	return FReply::Handled();
}

FReply UBackroomsPauseMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
	return FReply::Handled();
}

FReply UBackroomsPauseMenuWidget::OnSettingsTabClicked(int32 Tab)
{
	if (SettingsTab != Tab)
	{
		SettingsTab = Tab;
		RebuildUI();
	}
	return FReply::Handled();
}

FReply UBackroomsPauseMenuWidget::OnKeyBindClicked(const FString& BindId)
{
	bWaitingForKey = true;
	PendingBindId = BindId;
	RebuildUI();
	SetFocus();
	return FReply::Handled();
}

FReply UBackroomsPauseMenuWidget::OnResetBindingsClicked()
{
	bWaitingForKey = false;
	PendingBindId.Empty();
	BackroomsInput::ClearAllKeys();
	BackroomsInput::ApplyBindings();
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsPauseMenuWidget::OnBindCancel()
{
	bWaitingForKey = false;
	PendingBindId.Empty();
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bWaitingForKey && !InKeyEvent.IsRepeat())
	{
		const FKey Key = InKeyEvent.GetKey();
		if (Key == EKeys::Escape)
		{
			return OnBindCancel();
		}
		if (Key.IsValid() && !PendingBindId.IsEmpty())
		{
			BackroomsInput::SetCustomKey(*PendingBindId, *Key.ToString());
			BackroomsInput::ApplyBindings();
			bWaitingForKey = false;
			PendingBindId.Empty();
			RebuildUI();
			return FReply::Handled();
		}
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UBackroomsPauseMenuWidget::OnOverallQualityChanged(float Value)
{
	int32 Level = FMath::Clamp(FMath::RoundToInt(Value * 3.0f), 0, 3);
	OverallQ = Level;
	if (OverallQualityText)
	{
		OverallQualityText->SetText(FText::FromString(QualityToText(Level)));
	}
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetOverallScalabilityLevel(Level);
		ApplyAndSaveSettings();
	}
}

void UBackroomsPauseMenuWidget::OnViewDistanceChanged(float Value)
{
	int32 Level = FMath::Clamp(FMath::RoundToInt(Value * 3.0f), 0, 3);
	ViewDist = Level;
	if (ViewDistanceText)
	{
		ViewDistanceText->SetText(FText::FromString(QualityToText(Level)));
	}
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetViewDistanceQuality(Level);
		ApplyAndSaveSettings();
	}
}

void UBackroomsPauseMenuWidget::OnShadowQualityChanged(float Value)
{
	int32 Level = FMath::Clamp(FMath::RoundToInt(Value * 3.0f), 0, 3);
	ShadowQ = Level;
	if (ShadowQualityText)
	{
		ShadowQualityText->SetText(FText::FromString(QualityToText(Level)));
	}
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetShadowQuality(Level);
		ApplyAndSaveSettings();
	}
}

void UBackroomsPauseMenuWidget::OnTexturesQualityChanged(float Value)
{
	int32 Level = FMath::Clamp(FMath::RoundToInt(Value * 3.0f), 0, 3);
	TexturesQ = Level;
	if (TexturesQualityText)
	{
		TexturesQualityText->SetText(FText::FromString(QualityToText(Level)));
	}
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetTextureQuality(Level);
		ApplyAndSaveSettings();
	}
}

void UBackroomsPauseMenuWidget::OnAntiAliasChanged(float Value)
{
	int32 Level = FMath::Clamp(FMath::RoundToInt(Value * 3.0f), 0, 3);
	AAQuality = Level;
	if (AAText)
	{
		AAText->SetText(FText::FromString(QualityToText(Level)));
	}
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetAntiAliasingQuality(Level);
		ApplyAndSaveSettings();
	}
}

void UBackroomsPauseMenuWidget::OnPostProcessChanged(float Value)
{
	int32 Level = FMath::Clamp(FMath::RoundToInt(Value * 3.0f), 0, 3);
	PostProcess = Level;
	if (PostProcessText)
	{
		PostProcessText->SetText(FText::FromString(QualityToText(Level)));
	}
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetPostProcessingQuality(Level);
		ApplyAndSaveSettings();
	}
}

void UBackroomsPauseMenuWidget::OnEffectsChanged(float Value)
{
	int32 Level = FMath::Clamp(FMath::RoundToInt(Value * 3.0f), 0, 3);
	EffectsQuality = Level;
	if (EffectsText)
	{
		EffectsText->SetText(FText::FromString(QualityToText(Level)));
	}
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetVisualEffectQuality(Level);
		ApplyAndSaveSettings();
	}
}

void UBackroomsPauseMenuWidget::OnDofChanged(float Value)
{
	int32 Level = FMath::Clamp(FMath::RoundToInt(Value * 3.0f), 0, 3);
	DofQuality = Level;
	if (DofText)
	{
		DofText->SetText(FText::FromString(QualityToText(Level)));
	}
	if (UWorld* World = GetWorld())
	{
		World->GetGameInstance()->Exec(World, *FString::Printf(TEXT("r.DepthOfFieldQuality %d"),
			DofQuality == 0 ? 0 : (DofQuality == 1 ? 2 : 4)));
	}
}

void UBackroomsPauseMenuWidget::OnRecordingEffectChanged(float Value)
{
	int32 Level = FMath::Clamp(FMath::RoundToInt(Value * 3.0f), 0, 3);
	RecordingEffect = Level;
if (RecordingEffectText)
	{
		FString Name;
		switch (Level) { case 0: Name = BackroomsLoc::Get(TEXT("Set.RecordingEffect.Off")); break; case 1: Name = BackroomsLoc::Get(TEXT("Set.RecordingEffect.Light")); break; case 2: Name = BackroomsLoc::Get(TEXT("Set.RecordingEffect.Medium")); break; default: Name = BackroomsLoc::Get(TEXT("Set.RecordingEffect.Strong")); break; }
		RecordingEffectText->SetText(FText::FromString(Name));
	}
	ApplyCameraEffect();
}

void UBackroomsPauseMenuWidget::OnFovChanged(float Value)
{
	float NewFov = 60.0f + Value * 60.0f;
	FOV = NewFov;
	if (FovText)
	{
		FovText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::RoundToInt(NewFov))));
	}
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ABackroomsPlayerCharacter* C = Cast<ABackroomsPlayerCharacter>(PC->GetPawn()))
		{
			if (UCameraComponent* Cam = C->FindComponentByClass<UCameraComponent>())
				Cam->SetFieldOfView(NewFov);
		}
	}
}

void UBackroomsPauseMenuWidget::OnSensitivityChanged(float Value)
{
	Sensitivity = FMath::Clamp(0.2f + Value * 4.8f, 0.2f, 5.0f);
	if (SensitivityText)
	{
		SensitivityText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Sensitivity)));
	}
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ABackroomsPlayerCharacter* C = Cast<ABackroomsPlayerCharacter>(PC->GetPawn()))
			C->SetMouseSensitivity(Sensitivity);
	}
}

void UBackroomsPauseMenuWidget::OnVSyncChanged(float Value)
{
	bVSync = Value >= 0.5f;
	if (VSyncText)
	{
		VSyncText->SetText(FText::FromString(bVSync ? BackroomsLoc::Get(TEXT("Keys.On")) : BackroomsLoc::Get(TEXT("Keys.Off"))));
	}
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetVSyncEnabled(bVSync);
		ApplyAndSaveSettings();
	}
}

FReply UBackroomsPauseMenuWidget::OnPotatoModeClicked()
{
	BackroomsQuality::SetPotatoMode(!BackroomsQuality::IsPotatoMode(), GetWorld());
	RebuildUI();
	return FReply::Handled();
}

void UBackroomsPauseMenuWidget::OnGammaChanged(float Value)
{
	Gamma = FMath::Clamp(0.5f + Value * 1.5f, 0.5f, 2.0f);
	if (GammaText)
	{
		GammaText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Gamma)));
	}
	if (UWorld* World = GetWorld())
	{
		World->GetGameInstance()->Exec(World, *FString::Printf(TEXT("r.Gamma %f"), Gamma));
	}
}

void UBackroomsPauseMenuWidget::OnVolumeChanged(const FString& Key, float Value, TSharedPtr<STextBlock> OutLabel)
{
	float Clamped = FMath::Clamp(Value, 0.0f, 1.0f);
	BackroomsAudio::SetVolume(*Key, Clamped);

	if (Key == TEXT("MasterVolume")) MasterVolume = Clamped;
	else if (Key == TEXT("MenuVolume")) MenuVolume = Clamped;
	else if (Key == TEXT("GameVolume")) GameVolume = Clamped;
	else if (Key == TEXT("MonsterVolume")) MonsterVolume = Clamped;
	else if (Key == TEXT("OtherVolume")) OtherVolume = Clamped;

	if (OutLabel.IsValid())
	{
		OutLabel->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Clamped * 100.0f))));
	}
}

APostProcessVolume* UBackroomsPauseMenuWidget::FindOrCreatePostProcessVolume()
{
	if (CameraVolume.IsValid())
	{
		return CameraVolume.Get();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.Name = MakeUniqueObjectName(World, APostProcessVolume::StaticClass(), TEXT("BackroomsCameraPP"));
	APostProcessVolume* Volume = World->SpawnActor<APostProcessVolume>(
		APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (!Volume)
	{
		return nullptr;
	}

	Volume->bUnbound = true;
	Volume->bEnabled = true;
	Volume->Priority = 100.0f;
	Volume->BlendWeight = 1.0f;
	CameraVolume = Volume;
	return Volume;
}

void UBackroomsPauseMenuWidget::ApplyCameraEffect()
{
	APostProcessVolume* Volume = FindOrCreatePostProcessVolume();
	if (!Volume)
	{
		return;
	}

	struct FGrainMap
	{
		float Grain;
		float Fringe;
	};
	static const FGrainMap Map[4] =
	{
		{ 0.00f, 0.00f }, // Выкл
		{ 0.15f, 0.30f }, // Лёгкая плёнка
		{ 0.35f, 0.60f }, // Средняя плёнка
		{ 0.60f, 1.20f }, // Сильная плёнка
	};
	const int32 Idx = FMath::Clamp(RecordingEffect, 0, 3);

	FPostProcessSettings& S = Volume->Settings;
	S.bOverride_FilmGrainIntensity = 1;
	S.FilmGrainIntensity = Map[Idx].Grain;
	S.bOverride_SceneFringeIntensity = 1;
	S.SceneFringeIntensity = Map[Idx].Fringe;
	S.bOverride_ChromaticAberrationStartOffset = 1;
	S.ChromaticAberrationStartOffset = 0.8f;
}

void UBackroomsPauseMenuWidget::ApplyAndSaveSettings()
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->ApplySettings(false);
		Settings->SaveSettings();
	}
}
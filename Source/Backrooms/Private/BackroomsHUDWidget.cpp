#include "BackroomsHUDWidget.h"
#include "BackroomsPlayerCharacter.h"
#include "BackroomsItemSystem.h"
#include "BackroomsProgression.h"
#include "BackroomsItemIcons.h"
#include "BackroomsLocalization.h"
#include "Modules/ModuleManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Rendering/SlateRenderTransform.h"

namespace
{
	struct FStatusIconDesc
	{
		EBackroomsStatus Status;
		const TCHAR* File;
	};

	// Порядок иконок совпадает с нумерацией 12..23 в Content/UI/StatusIcons.
	const FStatusIconDesc GStatusIcons[] =
	{
		{ EBackroomsStatus::Overheating,   TEXT("12_overheating.png") },
		{ EBackroomsStatus::Poison,        TEXT("13_poison_skull_green_bubbles.png") },
		{ EBackroomsStatus::Radiation,     TEXT("14_radiation_anomaly.png") },
		{ EBackroomsStatus::Wet,           TEXT("15_wet.png") },
		{ EBackroomsStatus::Sleepiness,    TEXT("16_sleepiness.png") },
		{ EBackroomsStatus::Noise,         TEXT("17_noise.png") },
		{ EBackroomsStatus::Odor,          TEXT("18_odor.png") },
		{ EBackroomsStatus::Adrenaline,    TEXT("19_adrenaline.png") },
		{ EBackroomsStatus::Slowing,       TEXT("20_slowing.png") },
		{ EBackroomsStatus::Acceleration,  TEXT("21_acceleration.png") },
		{ EBackroomsStatus::Protection,    TEXT("22_protection_shield.png") },
		{ EBackroomsStatus::Vulnerability, TEXT("23_vulnerability_cracked_shield.png") }
	};

	constexpr float GIconSize = 72.0f;

	// Раскладка иконок состояний: только края экрана (левая/правая колонки).
	// Центр остаётся чистым — там прицел и игровой мир, иконки его не закрывают.
	struct FIconAnchor
	{
		EHorizontalAlignment H;
		EVerticalAlignment V;
		FMargin Pad;
	};

	const FIconAnchor GIconAnchors[] =
	{
		{ HAlign_Right,  VAlign_Top,    FMargin(0.0f,  24.0f, 24.0f, 0.0f) }, // 0 перегрев
		{ HAlign_Right,  VAlign_Top,    FMargin(0.0f, 110.0f, 24.0f, 0.0f) }, // 1 отравление
		{ HAlign_Right,  VAlign_Top,    FMargin(0.0f, 196.0f, 24.0f, 0.0f) }, // 2 радиация
		{ HAlign_Left,   VAlign_Top,    FMargin(24.0f, 150.0f, 0.0f, 0.0f) }, // 3 влага
		{ HAlign_Left,   VAlign_Top,    FMargin(24.0f, 236.0f, 0.0f, 0.0f) }, // 4 сонливость
		{ HAlign_Left,   VAlign_Top,    FMargin(24.0f, 322.0f, 0.0f, 0.0f) }, // 5 шум
		{ HAlign_Left,   VAlign_Top,    FMargin(24.0f, 408.0f, 0.0f, 0.0f) }, // 6 запах
		{ HAlign_Right,  VAlign_Top,    FMargin(0.0f, 282.0f, 24.0f, 0.0f) }, // 7 адреналин
		{ HAlign_Right,  VAlign_Top,    FMargin(0.0f, 368.0f, 24.0f, 0.0f) }, // 8 замедление
		{ HAlign_Left,   VAlign_Top,    FMargin(24.0f, 494.0f, 0.0f, 0.0f) }, // 9 ускорение
		{ HAlign_Left,   VAlign_Top,    FMargin(24.0f, 580.0f, 0.0f, 0.0f) }, // 10 защита
		{ HAlign_Right,  VAlign_Top,    FMargin(0.0f, 454.0f, 24.0f, 0.0f) }  // 11 уязвимость
	};

	// Общая белая кисть одной ячейки приборной шкалы. Цвет (зажжён/погашен)
	// задаётся через ColorAndOpacity конкретного SImage.
	const FSlateBrush* GetHudCellBrush()
	{
		static FSlateRoundedBoxBrush Brush(FLinearColor::White, 2.0f);
		return &Brush;
	}

	// Погашенный сегмент: заметно темнее, но форма читается.
	const FLinearColor GCellOff(0.16f, 0.16f, 0.19f, 0.55f);

	// Приборная шкала: ряд дискретных ячеек. Заполнение идёт слева направо, как
	// у светодиодного индикатора. Возвращает контейнер, ячейки складывает в Meter.
	TSharedRef<SWidget> MakeSegmentedMeter(FHudMeter& Meter, float Width, float Height, int32 Cells)
	{
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
		Meter.Cells.Reset();
		for (int32 i = 0; i < Cells; ++i)
		{
			TSharedPtr<SImage> Cell;
			Row->AddSlot()
				.FillWidth(1.0f)
				.Padding(i == 0 ? FMargin(0.0f) : FMargin(3.0f, 0.0f, 0.0f, 0.0f))
				[
					SAssignNew(Cell, SImage)
					.Image(GetHudCellBrush())
					.ColorAndOpacity(GCellOff)
				];
			Meter.Cells.Add(Cell);
		}
		Meter.LastFrac = -1.0f;

		return SNew(SBox)
			.WidthOverride(Width)
			.HeightOverride(Height)
			[
				Row
			];
	}

	// Кисть подложки панелей HUD (скруглённая тёмная).
	const FSlateBrush* GetBackroomsHudPanelBrush()
	{
		static FSlateRoundedBoxBrush Brush(
			FLinearColor(0.02f, 0.02f, 0.03f, 0.55f),
			10.0f,
			FLinearColor(0.36f, 0.33f, 0.25f, 0.35f),
			1.0f);
		return &Brush;
	}

	// Стиль бара прогресса удержания (тёмный фон + золотая заливка).
	const FProgressBarStyle* GetBackroomsUseBarStyle()
	{
		static FProgressBarStyle Style;
		static FSlateColorBrush Bg(FLinearColor(0.05f, 0.05f, 0.07f, 0.95f));
		static FSlateColorBrush Fill(FLinearColor(0.95f, 0.70f, 0.20f, 1.0f));
		Style.SetBackgroundImage(Bg);
		Style.SetFillImage(Fill);
		return &Style;
	}

	// Кисть слота хотбара.
	const FSlateBrush* GetBackroomsHotbarSlotBrush()
	{
		static FSlateRoundedBoxBrush Brush(
			FLinearColor(0.05f, 0.05f, 0.06f, 0.55f),
			8.0f,
			FLinearColor(0.36f, 0.33f, 0.25f, 0.30f),
			1.0f);
		return &Brush;
	}

	// Кисть выбранного (активного) слота хотбара — золотая рамка.
	const FSlateBrush* GetBackroomsHotbarSelectedBrush()
	{
		static FSlateRoundedBoxBrush Brush(
			FLinearColor(0.18f, 0.15f, 0.06f, 0.75f),
			8.0f,
			FLinearColor(0.95f, 0.82f, 0.38f, 0.95f),
			2.0f);
		return &Brush;
	}

	// Метка строки шкалы (заголовок над прибором).
	TSharedRef<STextBlock> MakeMeterLabel(const FString& Text, int32 Size, const FLinearColor& Color)
	{
		return SNew(STextBlock)
			.Text(FText::FromString(Text))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", Size))
			.ColorAndOpacity(Color)
			.ShadowOffset(FVector2D(1.0f, 1.0f));
	}
}

TSharedPtr<FSlateDynamicImageBrush> UBackroomsHUDWidget::LoadIconBrush(const FString& FileName, const FName& ResourceName)
{
	const FString Path = FPaths::ProjectContentDir() / TEXT("UI/StatusIcons") / FileName;

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *Path))
	{
		UE_LOG(LogTemp, Warning, TEXT("BackroomsHUD: icon PNG not found: %s"), *Path);
		return nullptr;
	}

	IImageWrapperModule& Module = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	TSharedPtr<IImageWrapper> Wrapper = Module.CreateImageWrapper(EImageFormat::PNG);
	if (!Wrapper.IsValid() || !Wrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Warning, TEXT("BackroomsHUD: failed to decode PNG: %s"), *FileName);
		return nullptr;
	}

	const int32 Width = Wrapper->GetWidth();
	const int32 Height = Wrapper->GetHeight();
	TArray64<uint8> Raw;
	if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, Raw) || Width <= 0 || Height <= 0)
	{
		return nullptr;
	}

	TArray<uint8> Bgra;
	Bgra.Append(Raw.GetData(), (int32)Raw.Num());
	return FSlateDynamicImageBrush::CreateWithImageData(ResourceName, FVector2D(Width, Height), Bgra);
}

TSharedRef<SWidget> UBackroomsHUDWidget::RebuildWidget()
{
	PlaceholderBrush = MakeShared<FSlateColorBrush>(FLinearColor(0.15f, 0.15f, 0.15f, 0.25f));
	CellBrush = MakeShared<FSlateColorBrush>(FLinearColor::White);

	// --- Иконки состояний ---
	// Каждая иконка живёт в собственном слое Overlay и появляется только когда
	// соответствующее состояние активно (см. Refresh). Раскладка — по якорям
	// GIconAnchors, чтобы активные статусы всплывали в разных частях экрана.
	StatusIcons.Reset();
	IconBrushes.Reset();
	for (const FStatusIconDesc& Desc : GStatusIcons)
	{
		const FName ResourceName(*FString::Printf(TEXT("BackroomsStatus_%s"), Desc.File));
		TSharedPtr<FSlateDynamicImageBrush> Brush = LoadIconBrush(Desc.File, ResourceName);
		IconBrushes.Add(Brush);

		const FSlateBrush* BrushPtr = Brush.IsValid() ? static_cast<const FSlateBrush*>(Brush.Get()) : static_cast<const FSlateBrush*>(PlaceholderBrush.Get());

		TSharedPtr<SImage> Icon;
		SAssignNew(Icon, SImage)
			.Image(BrushPtr)
			.DesiredSizeOverride(FVector2D(GIconSize, GIconSize))
			.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
		StatusIcons.Add(Icon);
	}

	// --- Шкалы выживания (панель справа-снизу) ---
	// Центр низа отдан выносливости, поэтому рассудок/фонарик/голод/жажда
	// собраны в компактную колонку у правого края.
	SanityMeter.OnColor = FLinearColor(0.45f, 0.72f, 0.35f, 0.95f);
	FlashlightMeter.OnColor = FLinearColor(0.95f, 0.78f, 0.30f, 0.95f);
	HungerMeter.OnColor = FLinearColor(0.85f, 0.62f, 0.30f, 0.95f);
	ThirstMeter.OnColor = FLinearColor(0.40f, 0.68f, 0.88f, 0.95f);

	SurvivalBarsBox = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
		[
			SAssignNew(SanityText, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s 100%%"), *BackroomsLoc::Get(TEXT("HUD.Sanity")))))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			.ColorAndOpacity(FLinearColor(0.90f, 0.85f, 0.70f, 0.95f))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 3.0f, 0.0f, 10.0f)).HAlign(HAlign_Right)
		[
			MakeSegmentedMeter(SanityMeter, 300.0f, 14.0f, 14)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
		[
			SAssignNew(FlashlightText, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s 100%%"), *BackroomsLoc::Get(TEXT("HUD.Flashlight")))))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			.ColorAndOpacity(FLinearColor(0.80f, 0.78f, 0.62f, 0.85f))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 3.0f, 0.0f, 10.0f)).HAlign(HAlign_Right)
		[
			MakeSegmentedMeter(FlashlightMeter, 300.0f, 12.0f, 14)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 2.0f)).HAlign(HAlign_Right)
		[
			MakeMeterLabel(BackroomsLoc::Get(TEXT("HUD.Hunger")), 11, FLinearColor(0.85f, 0.65f, 0.35f, 0.90f))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
		[
			MakeSegmentedMeter(HungerMeter, 300.0f, 12.0f, 14)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 2.0f)).HAlign(HAlign_Right)
		[
			MakeMeterLabel(BackroomsLoc::Get(TEXT("HUD.Thirst")), 11, FLinearColor(0.45f, 0.70f, 0.85f, 0.90f))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
		[
			MakeSegmentedMeter(ThirstMeter, 300.0f, 12.0f, 14)
		];

	// --- Выносливость: крупная шкала внизу по центру ---
	StaminaMeter.OnColor = FLinearColor(0.28f, 0.80f, 0.88f, 0.95f);
	StaminaBox = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SAssignNew(StaminaText, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s 100%%"), *BackroomsLoc::Get(TEXT("HUD.Stamina")))))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			.ColorAndOpacity(FLinearColor(0.72f, 0.90f, 0.95f, 0.95f))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
		[
			MakeSegmentedMeter(StaminaMeter, 680.0f, 30.0f, 24)
		];

	// --- Хотбар: 4 квадратных слота (по центру, над шкалами). Каждый слот —
	// номер сверху, иконка предмета (runtime-рендер меша), количество снизу;
	// выбранный слот подсвечивается золотой рамкой.
	TSharedRef<SHorizontalBox> Hotbar = SNew(SHorizontalBox);
	HotbarSlots.Reset();
	for (int32 i = 0; i < 4; ++i)
	{
		FHotbarSlot NewSlot;
		TSharedRef<SBorder> Border = SNew(SBorder)
			.BorderImage(GetBackroomsHotbarSlotBrush())
			.Padding(FMargin(8.0f, 6.0f))
			[
				SNew(SOverlay)
				+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
					[ SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%d"), i + 1)))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						.ColorAndOpacity(FLinearColor(0.62f, 0.62f, 0.62f)) ]
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SAssignNew(NewSlot.Icon, SImage)
						.Image(PlaceholderBrush.Get())
						.DesiredSizeOverride(FVector2D(40.0f, 40.0f))
						.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f))
					]
				+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom)
					[ SAssignNew(NewSlot.CountText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
						.ColorAndOpacity(FLinearColor(0.95f, 0.85f, 0.45f)) ]
			];
		NewSlot.Text.Reset(); // имя предмета заменено иконкой (см. Refresh)
		NewSlot.Border = Border;
		Hotbar->AddSlot().AutoWidth().Padding(FMargin(5.0f, 0.0f))
			[ SNew(SBox).WidthOverride(96.0f).HeightOverride(72.0f)[ Border ] ];
		HotbarSlots.Add(NewSlot);
	}

	// --- Здоровье: отдельная панель слева-сверху, визуально отличается от
	//     нижних шкал (крупнее, толще, со скруглённой подложкой и цифрой). ---
	HealthMeter.OnColor = FLinearColor(0.78f, 0.16f, 0.14f, 0.95f);
	HealthPanel =
		SNew(SBorder)
		.BorderImage(GetBackroomsHudPanelBrush())
		.Padding(FMargin(14.0f, 10.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
			[
				SAssignNew(HealthText, STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s 100%%"), *BackroomsLoc::Get(TEXT("HUD.Health")))))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
				.ColorAndOpacity(FLinearColor(0.95f, 0.45f, 0.42f, 0.95f))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 5.0f, 0.0f, 0.0f))
			[
				MakeSegmentedMeter(HealthMeter, 300.0f, 18.0f, 16)
			]
		];

	// --- Подсказка-вызов: одна строка над центром низа. Изначально прозрачна,
	//     появляется только когда есть условие (см. UpdateHint). ---
	HintPanel =
		SNew(SBorder)
		.BorderImage(GetBackroomsHudPanelBrush())
		.Padding(FMargin(20.0f, 10.0f))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Visibility(EVisibility::HitTestInvisible)
		[
			SAssignNew(HintText, STextBlock)
			.Text(FText::GetEmpty())
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			.ColorAndOpacity(FLinearColor(0.98f, 0.90f, 0.60f, 0.0f))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.8f))
		];

	// Тост прогрессии: открытый перк, новый уровень. Висит недолго, не мешает.
	ToastPanel =
		SNew(SBorder)
		.BorderImage(GetBackroomsHudPanelBrush())
		.Padding(FMargin(18.0f, 8.0f))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.RenderOpacity(0.0f)
		.Visibility(EVisibility::HitTestInvisible)
		[
			SAssignNew(ToastText, STextBlock)
			.Text(FText::GetEmpty())
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			.ColorAndOpacity(FLinearColor(0.75f, 0.95f, 0.70f, 0.95f))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.8f))
		];

	// XP-строка прогрессии: скрыта, всплывает на пару секунд при начислении XP.
	XPBarPanel =
		SNew(SBorder)
		.BorderImage(GetBackroomsHudPanelBrush())
		.Padding(FMargin(16.0f, 6.0f))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.RenderOpacity(0.0f)
		.Visibility(EVisibility::HitTestInvisible)
		[
			SAssignNew(XPText, STextBlock)
			.Text(FText::GetEmpty())
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
			.ColorAndOpacity(FLinearColor(0.85f, 0.88f, 0.98f, 0.95f))
		];

	// --- Подсказка взаимодействия (по центру, чуть ниже прицела). Показывает,
	//     что можно сделать с предметом под прицелом и какой клавишей; клавиша
	//     берётся из текущих переназначений. ---
	InteractPromptPanel =
		SNew(SBorder)
		.BorderImage(GetBackroomsHudPanelBrush())
		.Padding(FMargin(14.0f, 6.0f))
		.HAlign(HAlign_Center)
		.Visibility(EVisibility::HitTestInvisible)
		[
			SAssignNew(InteractPromptText, STextBlock)
			.Text(FText::GetEmpty())
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 15))
			.ColorAndOpacity(FLinearColor(0.94f, 0.94f, 0.90f, 0.0f))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
		];

	// --- Прогресс удержания «использовать» (UseTime): ровно над прицелом.
	// Показывается, только пока игрок удерживает кнопку и предмет не доглотнулся. ---
	UseProgressPanel =
		SNew(SBorder)
		.BorderImage(GetBackroomsHudPanelBrush())
		.Padding(FMargin(18.0f, 8.0f))
		.HAlign(HAlign_Center)
		.Visibility(EVisibility::Collapsed)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SAssignNew(UseProgressName, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
					.ColorAndOpacity(FLinearColor(0.95f, 0.85f, 0.45f, 1.0f))
					.ShadowOffset(FVector2D(1.0f, 1.0f))
				]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
				[
					SNew(SBox).WidthOverride(220.0f).HeightOverride(10.0f)
					[
						SAssignNew(UseProgressBar, SProgressBar)
						.Style(GetBackroomsUseBarStyle())
						.Percent(0.0f)
						.BorderPadding(FVector2D(0.0f, 0.0f))
					]
				]
		];

	// --- Осмотр предмета (спека §3): имя + описание в центре, ниже прицела.
	//     Видно, только пока активен компонент осмотра. ---
	InspectInfoPanel =
		SNew(SBorder)
		.BorderImage(GetBackroomsHudPanelBrush())
		.Padding(FMargin(20.0f, 10.0f))
		.HAlign(HAlign_Center)
		.Visibility(EVisibility::Collapsed)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SAssignNew(InspectNameText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FLinearColor(1.0f, 0.95f, 0.75f, 1.0f))
					.ShadowOffset(FVector2D(1.0f, 1.0f))
				]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
				[
					SAssignNew(InspectDescText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.AutoWrapText(true)
					.ColorAndOpacity(FLinearColor(0.88f, 0.88f, 0.82f, 0.90f))
				]
		];

	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Left)
		.VAlign(EVerticalAlignment::VAlign_Top)
		.Padding(FMargin(24.0f, 24.0f, 0.0f, 0.0f))
		[
			HealthPanel.ToSharedRef()
		]
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Center)
		.Padding(FMargin(0.0f, 90.0f, 0.0f, 0.0f))
		[
			InteractPromptPanel.ToSharedRef()
		]
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Bottom)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 230.0f))
		[
			HintPanel.ToSharedRef()
		]
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Top)
		.Padding(FMargin(0.0f, 90.0f, 0.0f, 0.0f))
		[
			XPBarPanel.ToSharedRef()
		]
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Top)
		.Padding(FMargin(0.0f, 132.0f, 0.0f, 0.0f))
		[
			ToastPanel.ToSharedRef()
		]
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Center)
		.Padding(FMargin(0.0f, 140.0f, 0.0f, 0.0f))
		[
			UseProgressPanel.ToSharedRef()
		]
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Center)
		.Padding(FMargin(0.0f, 220.0f, 0.0f, 0.0f))
		[
			InspectInfoPanel.ToSharedRef()
		]
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Right)
		.VAlign(EVerticalAlignment::VAlign_Bottom)
		.Padding(FMargin(0.0f, 0.0f, 24.0f, 24.0f))
		[
			SurvivalBarsBox.ToSharedRef()
		]
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Right)
		.VAlign(EVerticalAlignment::VAlign_Top)
		.Padding(FMargin(0.0f, 24.0f, 24.0f, 0.0f))
		[
			SAssignNew(FpsText, STextBlock)
			.Text(FText::FromString(TEXT("FPS -- Avg -- 1% --")))
			.Font(FCoreStyle::GetDefaultFontStyle("Mono", 10))
			.ColorAndOpacity(FLinearColor(0.92f, 0.95f, 0.32f, 0.95f))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
		]
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Bottom)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 150.0f))
		[
			Hotbar
		]
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Bottom)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 36.0f))
		[
			StaminaBox.ToSharedRef()
		];

	// Иконки состояний: каждая — в собственном слое по своему якорю.
	for (int32 i = 0; i < StatusIcons.Num() && i < UE_ARRAY_COUNT(GIconAnchors); ++i)
	{
		Root->AddSlot()
			.HAlign(GIconAnchors[i].H)
			.VAlign(GIconAnchors[i].V)
			.Padding(GIconAnchors[i].Pad)
			[
				StatusIcons[i].ToSharedRef()
			];
	}

	return Root;
}

void UBackroomsHUDWidget::RefreshMeter(FHudMeter& Meter, float Frac)
{
	Frac = FMath::Clamp(Frac, 0.0f, 1.0f);
	if (Meter.LastFrac >= 0.0f && FMath::IsNearlyEqual(Frac, Meter.LastFrac, 0.002f))
	{
		return;
	}
	Meter.LastFrac = Frac;

	const int32 Num = Meter.Cells.Num();
	if (Num == 0)
	{
		return;
	}
	// Сколько ячеек горит: слева направо. Хотя бы одна горит при Frac > 0,
	// все горят только при Frac == 1.
	const int32 Lit = FMath::Clamp(FMath::CeilToInt(Frac * (float)Num), 0, Num);
	for (int32 i = 0; i < Num; ++i)
	{
		if (Meter.Cells[i].IsValid())
		{
			Meter.Cells[i]->SetColorAndOpacity((i < Lit) ? Meter.OnColor : GCellOff);
		}
	}
}

void UBackroomsHUDWidget::UpdateHint(ABackroomsPlayerCharacter* P)
{
	if (!P || !HintPanel.IsValid() || !HintText.IsValid())
	{
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	constexpr float ShowDuration = 4.5f;
	constexpr float FadeInTime = 0.35f;
	constexpr float FadeOutTime = 0.6f;
	constexpr float Cooldown = 30.0f;

	// Текущая подсказка истекла — снимаем.
	if (!HintActiveId.IsNone() && Now >= HintShownUntil)
	{
		HintActiveId = NAME_None;
		HintActivePriority = 0;
	}

	// --- Собираем кандидатов по фактическому состоянию игрока ---
	struct FHintCand { FName Id; FString Text; int32 Priority; };
	TArray<FHintCand> Candidates;

	// В городе угроз нет — подсказки неуместны.
	if (!P->IsInCity())
	{
		UBackroomsItemSystem* Items = P->GetItemSystem();
		if (Items)
		{
			const float Hp = (Items->MaxHealth > 0.0f) ? Items->Health / Items->MaxHealth : 1.0f;
			const float Hunger = (Items->MaxHunger > 0.0f) ? Items->Hunger / Items->MaxHunger : 1.0f;
			const float Thirst = (Items->MaxThirst > 0.0f) ? Items->Thirst / Items->MaxThirst : 1.0f;
			const float Sanity = (Items->MaxSanity > 0.0f) ? Items->Sanity / Items->MaxSanity : 1.0f;

			if (Hp < 0.25f)     { Candidates.Add({ TEXT("Hint.Critical"), TEXT("Critical condition — find medicine now"), 100 }); }
			if (Sanity < 0.25f) { Candidates.Add({ TEXT("Hint.Sanity"),   TEXT("Sanity is running low — hide and catch your breath"),        80 }); }
			if (Hunger < 0.25f) { Candidates.Add({ TEXT("Hint.Hunger"),   TEXT("Severe hunger — find food"),                       60 }); }
			if (Thirst < 0.25f) { Candidates.Add({ TEXT("Hint.Thirst"),   TEXT("Severe thirst — drink some water"),                      60 }); }
		}

		if (UBackroomsStatusComponent* Status = P->GetStatusComponent())
		{
			const float Adrenaline = Status->GetIntensity(EBackroomsStatus::Adrenaline);
			const float Sleepiness = Status->GetIntensity(EBackroomsStatus::Sleepiness);
			const float Poison = Status->GetIntensity(EBackroomsStatus::Poison);
			const float Radiation = Status->GetIntensity(EBackroomsStatus::Radiation);

			if (Adrenaline > 0.55f) { Candidates.Add({ TEXT("Hint.Monster"), TEXT("It's somewhere near — stay quiet"),                 95 }); }
			if (Poison > 0.35f)     { Candidates.Add({ TEXT("Hint.Poison"),  TEXT("You are poisoned — you need almond water or a medkit"), 90 }); }
			if (Radiation > 0.45f)  { Candidates.Add({ TEXT("Hint.Radiation"), TEXT("High radiation level — get out of here"),       85 }); }
			if (Sleepiness > 0.55f) { Candidates.Add({ TEXT("Hint.Sleep"),   TEXT("You are exhausted — stop and rest"),        45 }); }
		}

		if (P->IsFlashlightOn() && P->GetFlashlightBattery() < 15.0f)
		{
			Candidates.Add({ TEXT("Hint.Battery"), TEXT("Flashlight is almost out of charge"), 40 });
		}
	}

	// --- Выбор: самый приоритетный из неостывших; активную не прерываем, если
	//     новая не важнее. Это и делает подсказки «умными», а не спамом. ---
	const FHintCand* Best = nullptr;
	for (const FHintCand& C : Candidates)
	{
		if (const float* Until = HintCooldownUntil.Find(C.Id))
		{
			if (Now < *Until)
			{
				continue;
			}
		}
		if (!Best || C.Priority > Best->Priority)
		{
			Best = &C;
		}
	}

	const bool bActive = !HintActiveId.IsNone() && Now < HintShownUntil;
	if (Best)
	{
		const bool bSwitch = !bActive || (Best->Priority > HintActivePriority);
		if (bSwitch)
		{
			HintActiveId = Best->Id;
			HintActivePriority = Best->Priority;
			HintShownUntil = Now + ShowDuration;
			HintFadeInAt = Now;
			HintCooldownUntil.Add(Best->Id, Now + Cooldown);
			const FString HintKey = Best->Id.ToString();
			const FString HintLoc = BackroomsLoc::Get(*HintKey);
			HintText->SetText(FText::FromString((HintLoc == HintKey) ? Best->Text : HintLoc));
		}
	}

	// --- Прозрачность: плавный вход и затухание в конце. ---
	float Alpha = 0.0f;
	if (!HintActiveId.IsNone() && Now < HintShownUntil)
	{
		const float InT = FMath::Clamp((Now - HintFadeInAt) / FadeInTime, 0.0f, 1.0f);
		const float Remaining = HintShownUntil - Now;
		const float OutT = (Remaining < FadeOutTime) ? FMath::Clamp(Remaining / FadeOutTime, 0.0f, 1.0f) : 1.0f;
		Alpha = InT * OutT;
	}

	HintPanel->SetRenderOpacity(Alpha);
	HintText->SetColorAndOpacity(FLinearColor(0.98f, 0.90f, 0.60f, 0.95f * Alpha));
}

void UBackroomsHUDWidget::UpdateInteractPrompt(ABackroomsPlayerCharacter* P, float DeltaTime)
{
	// Панель появляется ТОЛЬКО когда есть промпт — мёртвый прямоугольник по
	// центру убран. Текст позиционируется под предметом, на который смотрим.
	if (!InteractPromptText.IsValid() || !InteractPromptPanel.IsValid())
	{
		return;
	}
	// Трассировку/поиск пикапа каждый кадр не делаем — раз в 0.12 с достаточно.
	InteractPromptTimer -= DeltaTime;
	const FString Prompt = P ? P->GetInteractPrompt() : FString();
	if (InteractPromptTimer > 0.0f && Prompt == LastInteractPrompt)
	{
		return;
	}
	InteractPromptTimer = 0.12f;

	if (Prompt.IsEmpty())
	{
		InteractPromptPanel->SetVisibility(EVisibility::Collapsed);
		InteractPromptText->SetText(FText::GetEmpty());
		LastInteractPrompt.Empty();
		return;
	}

	InteractPromptText->SetText(FText::FromString(Prompt));

	// «Текст ниже предмета»: берём точку под мешем предмета, проектируем её на
	// экран и сдвигаем панель относительно центра вьюпорта.
	APlayerController* PC = P ? Cast<APlayerController>(P->GetController()) : nullptr;
	FVector WorldPoint;
	const bool bHasPoint = P && P->GetInteractPromptWorldPoint(WorldPoint) && PC;
	FVector2D Offset(0.0f, 0.0f);
	if (bHasPoint)
	{
		FVector2D Screen;
		if (PC->ProjectWorldLocationToScreen(WorldPoint, Screen, true))
		{
			int32 VW = 0, VH = 0;
			PC->GetViewportSize(VW, VH);
			Offset = Screen - FVector2D(VW * 0.5f, VH * 0.5f);
		}
	}
	InteractPromptPanel->SetRenderTransform(FSlateRenderTransform(FVector2D(Offset.X, Offset.Y)));
	InteractPromptPanel->SetVisibility(EVisibility::HitTestInvisible);
	LastInteractPrompt = Prompt;
}

void UBackroomsHUDWidget::UpdateUseProgress(ABackroomsPlayerCharacter* P)
{
	if (!P || !UseProgressPanel.IsValid() || !UseProgressBar.IsValid() || !UseProgressName.IsValid())
	{
		return;
	}
	const bool bUsing = P && P->IsUseHeld();
	UseProgressPanel->SetVisibility(bUsing ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
	if (bUsing)
	{
		UseProgressBar->SetPercent(FMath::Clamp(P->GetUseHoldProgress(), 0.0f, 1.0f));
		UseProgressName->SetText(FText::FromString(P->GetUseHoldItemName()));
	}
}

void UBackroomsHUDWidget::UpdateInspectInfo(ABackroomsPlayerCharacter* P)
{
	if (!P || !InspectInfoPanel.IsValid() || !InspectNameText.IsValid() || !InspectDescText.IsValid())
	{
		return;
	}
	const bool bInspecting = P->IsInspecting();
	InspectInfoPanel->SetVisibility(bInspecting ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
	if (bInspecting)
	{
		InspectNameText->SetText(P->GetInspectItemName());
		InspectDescText->SetText(FText::FromString(P->GetInspectItemDescription()));
	}
}

void UBackroomsHUDWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	SurvivalBarsBox.Reset();
	StaminaBox.Reset();
	HealthPanel.Reset();
	HintPanel.Reset();
	HintText.Reset();
	ToastPanel.Reset();
	ToastText.Reset();
	XPBarPanel.Reset();
	XPText.Reset();
	InteractPromptPanel.Reset();
	InteractPromptText.Reset();
	UseProgressPanel.Reset();
	UseProgressBar.Reset();
	UseProgressName.Reset();
	InspectInfoPanel.Reset();
	InspectNameText.Reset();
	InspectDescText.Reset();
	SanityMeter.Cells.Reset();
	FlashlightMeter.Cells.Reset();
	HealthMeter.Cells.Reset();
	HungerMeter.Cells.Reset();
	ThirstMeter.Cells.Reset();
	StaminaMeter.Cells.Reset();
	SanityText.Reset();
	FlashlightText.Reset();
	StaminaText.Reset();
	HealthText.Reset();
	HotbarSlots.Reset();
	StatusIcons.Reset();
	IconBrushes.Reset();
	PlaceholderBrush.Reset();
	CellBrush.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UBackroomsHUDWidget::SetPlayer(ABackroomsPlayerCharacter* InPlayer)
{
	Player = InPlayer;
}

void UBackroomsHUDWidget::ShowToast(const FString& Message)
{
	if (!ToastText.IsValid())
	{
		return;
	}
	ToastText->SetText(FText::FromString(Message));
	ToastUntil = (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f) + 3.0f;
}

void UBackroomsHUDWidget::Refresh()
{
	ABackroomsPlayerCharacter* P = Player.Get();
	if (!P)
	{
		return;
	}
	UWorld* W = P->GetWorld();
	float DeltaTimeForPrompt = 0.015f;
	if (W)
	{
		// Движем очередь runtime-иконок предметов (хотбар + инвентарь).
		const float Now = W->GetTimeSeconds();
		const float Delta = (LastRefreshTime > 0.0f) ? FMath::Max(Now - LastRefreshTime, 0.0f) : 0.015f;
		LastRefreshTime = Now;
		DeltaTimeForPrompt = Delta;
		BackroomsItemIcons::TickRender(W, Delta);
	}

	// В городе выживальческие панели не нужны: голод/жажда/рассудок начинают
	// иметь смысл только под землёй. Прячем их, чтобы HUD не врал в мирной зоне.
	const bool bInCity = P->IsInCity();
	const EVisibility PanelVis = bInCity ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
	if (SurvivalBarsBox.IsValid()) { SurvivalBarsBox->SetVisibility(PanelVis); }
	if (StaminaBox.IsValid()) { StaminaBox->SetVisibility(PanelVis); }
	if (HealthPanel.IsValid()) { HealthPanel->SetVisibility(PanelVis); }

	UBackroomsItemSystem* Items = P->GetItemSystem();
	UBackroomsStatusComponent* Status = P->GetStatusComponent();

	// --- Рассудок ---
	if (Items)
	{
		const float Frac = (Items->MaxSanity > 0.0f) ? FMath::Clamp(Items->Sanity / Items->MaxSanity, 0.0f, 1.0f) : 0.0f;
		// Цвет прибора: зелёный -> жёлтый -> красный по мере падения.
		const FLinearColor Low(0.85f, 0.20f, 0.15f, 0.95f);
		const FLinearColor Mid(0.90f, 0.70f, 0.20f, 0.95f);
		const FLinearColor High(0.45f, 0.72f, 0.35f, 0.95f);
		SanityMeter.OnColor = (Frac < 0.5f)
			? FMath::Lerp(Low, Mid, Frac / 0.5f)
			: FMath::Lerp(Mid, High, (Frac - 0.5f) / 0.5f);
		RefreshMeter(SanityMeter, Frac);

		if (SanityText.IsValid())
		{
			SanityText->SetText(FText::FromString(FString::Printf(TEXT("%s %d%%"), *BackroomsLoc::Get(TEXT("HUD.Sanity")), FMath::RoundToInt(Frac * 100.0f))));
		}
	}

	// --- Фонарик ---
	{
		const float Battery = FMath::Clamp(P->GetFlashlightBattery() / 100.0f, 0.0f, 1.0f);
		FlashlightMeter.OnColor = (Battery < 0.2f)
			? FLinearColor(0.85f, 0.25f, 0.18f, 0.95f)
			: FLinearColor(0.95f, 0.78f, 0.30f, 0.95f);
		RefreshMeter(FlashlightMeter, Battery);
		if (FlashlightText.IsValid())
		{
			FlashlightText->SetText(FText::FromString(FString::Printf(TEXT("%s %d%%"), *BackroomsLoc::Get(TEXT("HUD.Flashlight")), FMath::RoundToInt(Battery * 100.0f))));
		}
	}

	// --- Выносливость (бег) ---
	{
		const float Max = P->GetMaxStamina();
		const float Frac = (Max > 0.0f) ? FMath::Clamp(P->GetStamina() / Max, 0.0f, 1.0f) : 0.0f;
		const FLinearColor Empty(0.82f, 0.34f, 0.20f, 0.95f);
		const FLinearColor Full(0.28f, 0.80f, 0.88f, 0.95f);
		StaminaMeter.OnColor = FMath::Lerp(Empty, Full, FMath::Clamp(Frac * 1.6f, 0.0f, 1.0f));
		RefreshMeter(StaminaMeter, Frac);

		if (StaminaText.IsValid())
		{
			StaminaText->SetText(FText::FromString(FString::Printf(TEXT("%s %d%%"), *BackroomsLoc::Get(TEXT("HUD.Stamina")), FMath::RoundToInt(Frac * 100.0f))));
		}
	}

	// --- Здоровье / голод / жажда ---
	if (Items)
	{
		const float HealthFrac = (Items->MaxHealth > 0.0f) ? FMath::Clamp(Items->Health / Items->MaxHealth, 0.0f, 1.0f) : 0.0f;
		const float HungerFrac = (Items->MaxHunger > 0.0f) ? FMath::Clamp(Items->Hunger / Items->MaxHunger, 0.0f, 1.0f) : 0.0f;
		const float ThirstFrac = (Items->MaxThirst > 0.0f) ? FMath::Clamp(Items->Thirst / Items->MaxThirst, 0.0f, 1.0f) : 0.0f;

		HealthMeter.OnColor = FMath::Lerp(
			FLinearColor(0.80f, 0.14f, 0.12f, 0.95f),
			FLinearColor(0.35f, 0.70f, 0.30f, 0.95f),
			HealthFrac);
		RefreshMeter(HealthMeter, HealthFrac);
		RefreshMeter(HungerMeter, HungerFrac);
		RefreshMeter(ThirstMeter, ThirstFrac);

		if (HealthText.IsValid())
		{
			HealthText->SetText(FText::FromString(FString::Printf(TEXT("%s %d%%"), *BackroomsLoc::Get(TEXT("HUD.Health")), FMath::RoundToInt(HealthFrac * 100.0f))));
		}
	}

	// --- Хотбар: массив слотов (иконка + количество + рамка выбора) ---
	for (int32 i = 0; i < HotbarSlots.Num(); ++i)
	{
		FHotbarSlot& HotSlot = HotbarSlots[i];
		if (!HotSlot.Border.IsValid() || !HotSlot.Icon.IsValid())
		{
			continue;
		}
		const FName Id = P->GetHotbarItemId(i);
		int32 Count = 0;
		if (!Id.IsNone() && Items)
		{
			Count = Items->GetItemCount(Id);
		}
		const bool bEmpty = (Id.IsNone() || Count <= 0);
		const bool bSelected = (P->GetSelectedHotbarSlot() == i);

		// Иконка предмета (runtime-рендер); пока нет — тёмная заглушка.
		if (!bEmpty)
		{
			const TSharedPtr<FSlateBrush> Brush = BackroomsItemIcons::GetItemIcon(P->GetWorld(), Id);
			if (Brush.IsValid())
			{
				HotSlot.Icon->SetImage(Brush.Get());
			}
			HotSlot.Icon->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.9f * (bSelected ? 1.0f : 0.75f)));
		}
		else
		{
			HotSlot.Icon->SetImage(PlaceholderBrush.Get());
			HotSlot.Icon->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.15f));
		}
		if (HotSlot.CountText.IsValid())
		{
			HotSlot.CountText->SetText(FText::FromString(bEmpty ? FString() : FString::Printf(TEXT("x%d"), Count)));
			HotSlot.CountText->SetColorAndOpacity(bEmpty
				? FLinearColor(1.0f, 1.0f, 1.0f, 0.0f)
				: FLinearColor(0.95f, 0.85f, 0.45f, bSelected ? 1.0f : 0.65f));
		}
		HotSlot.Border->SetBorderImage(bSelected ? GetBackroomsHotbarSelectedBrush() : GetBackroomsHotbarSlotBrush());
	}

	// --- Иконки состояний: неактивные полностью скрыты, активные разгораются.
	// В городе статусы среды не показываем вовсе (там нет ни радиации, ни монстра).
	if (Status && !bInCity)
	{
		for (int32 i = 0; i < StatusIcons.Num() && i < UE_ARRAY_COUNT(GStatusIcons); ++i)
		{
			if (!StatusIcons[i].IsValid())
			{
				continue;
			}
			const float Intensity = Status->GetIntensity(GStatusIcons[i].Status);
			const float Alpha = (Intensity > UBackroomsStatusComponent::ActiveThreshold)
				? FMath::Lerp(0.55f, 1.0f, FMath::Clamp(Intensity, 0.0f, 1.0f))
				: 0.0f;
			StatusIcons[i]->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, Alpha));
		}
	}
	else
	{
		for (int32 i = 0; i < StatusIcons.Num(); ++i)
		{
			if (StatusIcons[i].IsValid())
			{
				StatusIcons[i]->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
			}
		}
	}

	// --- Контекстная подсказка-вызов поверх всего ---
	UpdateHint(P);

	// --- Прогрессия: XP-строка и тост показываются коротко, только на изменении
	//     (нет постоянной полосы — Backrooms про погружение, а не про цифры). ---
	if (const UGameInstance* GI = P->GetGameInstance())
	{
		if (UBackroomsProgression* Prog = GI->GetSubsystem<UBackroomsProgression>())
		{
			const int32 Lvl = Prog->GetLevel();
			const int32 CurXP = Prog->GetXP();
			const float Now = W ? W->GetTimeSeconds() : 0.0f;
			if (LastSeenXP >= 0 && CurXP != LastSeenXP)
			{
				XPRevealUntil = Now + 2.5f;
				if (XPBarPanel.IsValid() && XPText.IsValid())
				{
					XPText->SetText(FText::FromString(FString::Printf(TEXT("%s %d  ·  XP %d/%d"),
						*BackroomsLoc::Get(TEXT("Prog.Level")), Lvl, CurXP, Prog->GetXPForNextLevel())));
				}
			}
			if (Lvl != LastSeenLevel && LastSeenLevel != 0)
			{
				ShowToast(FString::Printf(TEXT("%s %d"), *BackroomsLoc::Get(TEXT("Prog.LevelUp")), Lvl));
			}
			LastSeenLevel = Lvl;
			LastSeenXP = CurXP;

			if (XPBarPanel.IsValid())
			{
				XPBarPanel->SetRenderOpacity(Now < XPRevealUntil ? 1.0f : 0.0f);
			}
		}
	}
	if (ToastPanel.IsValid() && ToastText.IsValid())
	{
		const float Now = W ? W->GetTimeSeconds() : 0.0f;
		ToastPanel->SetRenderOpacity(Now < ToastUntil ? 1.0f : 0.0f);
	}

	// --- Подсказка взаимодействия (клавиши под предметом), прогресс удержания,
	//     информация об осмотре. Последние видны весь кадр, промпт — при наведении.
	UpdateInteractPrompt(P, DeltaTimeForPrompt);
	UpdateUseProgress(P);
	UpdateInspectInfo(P);
}

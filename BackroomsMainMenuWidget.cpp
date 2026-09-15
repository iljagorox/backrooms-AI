#include "BackroomsMainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "BackroomsPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Fonts/SlateFontInfo.h"
#include "Fonts/FontMeasure.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Brushes/SlateColorBrush.h"
#include "Engine/Font.h"
#include "Framework/Application/SlateApplication.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "MediaSource.h"
#include "FileMediaSource.h"
#include "MediaSoundComponent.h"
#include "Components/SceneComponent.h"
#include "BackroomsAudioSettings.h"
#include "BackroomsInputSettings.h"
#include "BackroomsDifficulty.h"
#include "BackroomsLevelBook.h"
#include "BackroomsWorldGenerator.h"
#include "BackroomsAchievements.h"
#include "BackroomsLocalization.h"
#include "BackroomsQualitySettings.h"
#include "Engine/PostProcessVolume.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "Modules/ModuleManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Input/Events.h"

// Подстановка %d в локализованную строку. FString::Printf требует
// компилируемый литерал формата, а у нас формат приходит из таблицы переводов,
// поэтому заменяем плейсхолдеры вручную.
static FString LocFormat(const FString& Fmt, const TArray<FString>& Args)
{
	FString Out = Fmt;
	for (const FString& A : Args)
	{
		Out = Out.Replace(TEXT("%d"), *A);
	}
	return Out;
}

static FSlateFontInfo MakeFont(UFont* FontObj, int32 Size)
{
	if (FontObj)
	{
		return FSlateFontInfo(FontObj, Size);
	}
	return FCoreStyle::GetDefaultFontStyle("Bold", Size);
}

// Стиль слайдера: гладкий тёмный жёлоб-пилюля + один золотой ползунок.
// Кисти задаются явно (а не наследуются от FCoreStyle), иначе дефолтные
// 9-slice текстуры растягиваются и дают «рваный» вид.
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

// Внешняя панель меню: один большой серый скруглённый полупрозрачный блок.
static const FSlateBrush* GetBackroomsPanelBrush()
{
	static FSlateRoundedBoxBrush Brush(
		FLinearColor(0.06f, 0.06f, 0.09f, 0.72f),
		22.0f,
		FLinearColor(0.40f, 0.38f, 0.30f, 0.45f),
		1.5f);
	return &Brush;
}

// Кисть левого сайдбара настроек (на всю высоту экрана).
static const FSlateBrush* GetBackroomsSideBarBrush()
{
	static FSlateRoundedBoxBrush Brush(
		FLinearColor(0.07f, 0.07f, 0.10f, 0.72f),
		16.0f,
		FLinearColor(0.40f, 0.38f, 0.30f, 0.40f),
		1.0f);
	return &Brush;
}

// Кисть карточки контента справа (занимает весь оставшийся экран).
static const FSlateBrush* GetBackroomsContentCardBrush()
{
	static FSlateRoundedBoxBrush Brush(
		FLinearColor(0.12f, 0.12f, 0.15f, 0.60f),
		16.0f,
		FLinearColor(0.40f, 0.38f, 0.30f, 0.35f),
		1.0f);
	return &Brush;
}

// Контент секции: заголовок + строки. Без собственной рамки — рамку даёт
// карточка контента, чтобы настройки занимали весь экран.
static TSharedRef<SWidget> MakeMainMenuSection(UFont* MenuFont, const FText& Title, TSharedRef<SWidget> Content)
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f, 0.0f, 2.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text(Title)
			.Font(MakeFont(MenuFont, 22))
			.ColorAndOpacity(FLinearColor(0.96f, 0.90f, 0.62f))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			Content
		];
}

// Загрузить PNG-превью с диска в runtime-кисть Slate. Возвращает nullptr,
// если файла нет или он не декодировался — тогда карточка рисует заглушку.
static TSharedPtr<FSlateDynamicImageBrush> LoadThumbBrush(const FString& FilePath, const FName& ResourceName)
{
	if (!FPaths::FileExists(FilePath))
	{
		return nullptr;
	}
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		return nullptr;
	}
	IImageWrapperModule& Module = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	TSharedPtr<IImageWrapper> Wrapper = Module.CreateImageWrapper(EImageFormat::PNG);
	if (!Wrapper.IsValid() || !Wrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		return nullptr;
	}
	const int32 W = Wrapper->GetWidth();
	const int32 H = Wrapper->GetHeight();
	TArray64<uint8> Raw;
	if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, Raw) || W <= 0 || H <= 0)
	{
		return nullptr;
	}
	TArray<uint8> Bgra;
	Bgra.Append(Raw.GetData(), (int32)Raw.Num());
	return FSlateDynamicImageBrush::CreateWithImageData(ResourceName, FVector2D(W, H), Bgra);
}

FString UBackroomsMainMenuWidget::ScalabilityName(int32 Level)
{
	switch (Level) { case 0: return TEXT("Low"); case 1: return TEXT("Medium"); case 2: return TEXT("High"); case 3: return TEXT("Epic"); default: return TEXT("High"); }
}
FString UBackroomsMainMenuWidget::DisplayModeName(int32 Mode)
{
	switch (Mode) { case 0: return BackroomsLoc::Get(TEXT("Set.DisplayMode.Fullscreen")); case 1: return BackroomsLoc::Get(TEXT("Set.DisplayMode.Borderless")); case 2: return BackroomsLoc::Get(TEXT("Set.DisplayMode.Windowed")); default: return BackroomsLoc::Get(TEXT("Set.DisplayMode.Fullscreen")); }
}
FString UBackroomsMainMenuWidget::DOFName(int32 Level)
{
	switch (Level) { case 0: return BackroomsLoc::Get(TEXT("Keys.Off")); case 1: return TEXT("Medium"); case 2: return TEXT("Cinematic"); default: return TEXT("Medium"); }
}
FString UBackroomsMainMenuWidget::LumenName(int32 Level)
{
	switch (Level) { case 0: return BackroomsLoc::Get(TEXT("Keys.Off")); case 1: return TEXT("Low"); case 2: return TEXT("High"); case 3: return TEXT("Epic"); default: return TEXT("High"); }
}
FString UBackroomsMainMenuWidget::DLSSModeName(int32 Mode)
{
	switch (Mode) { case 0: return BackroomsLoc::Get(TEXT("Keys.Off")); case 1: return TEXT("Quality"); case 2: return TEXT("Balanced"); case 3: return TEXT("Performance"); case 4: return TEXT("UltraPerf"); default: return TEXT("Balanced"); }
}

void UBackroomsMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Без этого движок пишет "widget ... does not support focus", а
	// SetWidgetToFocus/SetKeyboardFocus в ShowMenu() не ставят фокус — меню не
	// принимает клавиатуру/мышь для навигации по кнопкам.
	SetIsFocusable(true);

	// Громкости из GameUserSettings.ini (раздел BackroomsAudio).
	MasterVolume  = BackroomsAudio::GetVolume(TEXT("MasterVolume"), 1.0f);
	MenuVolume    = BackroomsAudio::GetVolume(TEXT("MenuVolume"), 1.0f);
	GameVolume    = BackroomsAudio::GetVolume(TEXT("GameVolume"), 1.0f);
	MonsterVolume = BackroomsAudio::GetVolume(TEXT("MonsterVolume"), 1.0f);
	OtherVolume   = BackroomsAudio::GetVolume(TEXT("OtherVolume"), 1.0f);

	MenuFont = LoadObject<UFont>(nullptr, TEXT("/Game/Fonts/ArialBold"));

	TArray<FIntPoint> Raw;
	if (UKismetSystemLibrary::GetSupportedFullscreenResolutions(Raw))
	{
		TSet<FIntPoint> Seen;
		for (const FIntPoint& P : Raw)
		{
			if (Seen.Contains(P)) continue;
			Seen.Add(P);
			Resolutions.Add({ FString::Printf(TEXT("%dx%d"), P.X, P.Y), P });
		}
		Resolutions.Sort([](const FResInfo& A, const FResInfo& B) { return A.Size.X < B.Size.X || (A.Size.X == B.Size.X && A.Size.Y < B.Size.Y); });
	}
	if (UGameUserSettings* S = GEngine->GetGameUserSettings())
	{
		S->LoadSettings();
		const FIntPoint Cur = S->GetScreenResolution();
		for (int32 i = 0; i < Resolutions.Num(); ++i)
		{
			if (Resolutions[i].Size == Cur) { SelectedRes = i; break; }
		}
		bVSync          = S->IsVSyncEnabled();

		// При самом первом запуске (маркер ещё не выставлен) включаем пресет
		// High: иначе движок начинает с Low, и картинка в тёмных коридорах
		// шумит/пикселит (Lumen-пробы на минимуме, объёмный туман выключен).
		const float bGraphicsApplied = BackroomsAudio::GetVolume(TEXT("GraphicsApplied"), 0.0f);
		const int32 SavedLevel = FMath::Clamp(S->GetOverallScalabilityLevel(), 0, 3);
		const int32 Level = (bGraphicsApplied > 0.5f) ? SavedLevel : 2;

		ShadowQuality  = Level;
		TextureQuality = Level;
		AAQuality      = Level;
		PostProcess    = Level;
		EffectsQuality = Level;
		ViewDistance    = Level;

		if (bGraphicsApplied < 0.5f)
		{
			S->SetOverallScalabilityLevel(Level);
			S->SaveSettings();
			BackroomsAudio::SetVolume(TEXT("GraphicsApplied"), 1.0f);
		}
	}
	if (CorridorMaterial)
	{
		CorridorRT = NewObject<UTextureRenderTarget2D>(this);
		CorridorRT->InitAutoFormat(1920, 1080);
		CorridorRT->UpdateResourceImmediate(true);
		DynamicMat = UMaterialInstanceDynamic::Create(CorridorMaterial, this);
		Brush = new FSlateBrush();
		Brush->SetResourceObject(CorridorRT);
		Brush->ImageSize = FVector2D(1920, 1080);
	}
	else
	{
		Brush = new FSlateBrush();
		Brush->TintColor = FSlateColor(FLinearColor(0.01f, 0.01f, 0.02f, 1.0f));
	}

	BackgroundPlayer = NewObject<UMediaPlayer>(this);
	BackgroundTexture = NewObject<UMediaTexture>(this);
	if (BackgroundPlayer && BackgroundTexture)
	{
		BackgroundTexture->SetMediaPlayer(BackgroundPlayer);
		BackgroundTexture->UpdateResource();
		BackgroundPlayer->OnEndReached.AddDynamic(this, &UBackroomsMainMenuWidget::OnBackgroundVideoEnded);

		// Звук меню: UMediaPlayer сам по себе молчит, пока к нему не привязан
		// UMediaSoundComponent. Спавним скрытый актёр-носитель (у виджета нет
		// сцены) и играем аудиодорожку фонового видео как музыку меню.
		if (UWorld* W = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = TEXT("BackroomsMenuAudio");
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			MenuAudioActor = W->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
			if (MenuAudioActor)
			{
				USceneComponent* Root = NewObject<USceneComponent>(MenuAudioActor, TEXT("Root"));
				MenuAudioActor->SetRootComponent(Root);
				Root->RegisterComponent();

				MenuAudio = NewObject<UMediaSoundComponent>(MenuAudioActor, TEXT("MenuAudio"));
				MenuAudio->SetupAttachment(Root);
				MenuAudio->SetMediaPlayer(BackgroundPlayer);
				MenuAudio->bIsUISound = true;        // слышно, пока игра на паузе
				MenuAudio->bAllowSpatialization = false;
				MenuAudio->RegisterComponent();
				ApplyMenuAudioVolume();
			}
		}

		StartBackgroundVideo(false);
		if (!Brush)
		{
			Brush = new FSlateBrush();
		}
		Brush->SetResourceObject(BackgroundTexture);
		Brush->ImageSize = FVector2D(1920.0f, 1080.0f);
		Brush->TintColor = FSlateColor(FLinearColor(0.72f, 0.72f, 0.72f, 1.0f));
	}

	// Создаём глобальный PP-объём сразу при открытии меню: стабильная
	// экспозиция должна действовать с первой секунды, а не ждать, пока
	// игрок дёрнет ползунок «Эффект камеры записи».
	ApplyCameraEffect();
}

void UBackroomsMainMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// Время, проведённое в меню: чем дольше, тем выше шанс скримера.
	MenuIdleTime += InDeltaTime;
	TickCorridor(InDeltaTime);
	if (TransitionAlpha > 0.0f)
	{
		TransitionAlpha = FMath::Max(0.0f, TransitionAlpha - InDeltaTime / 0.65f);
		if (TransitionOverlay.IsValid())
		{
			TransitionOverlay->SetBorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, TransitionAlpha));
		}
	}
}

void UBackroomsMainMenuWidget::TickCorridor(float DeltaTime)
{
	if (!DynamicMat || !CorridorRT) return;
	UVOffset += DeltaTime * 0.12f;
	DynamicMat->SetScalarParameterValue(FName("UVOffset"), UVOffset);
}

void UBackroomsMainMenuWidget::StartBackgroundVideo(bool bRareVideo)
{
	if (!BackgroundPlayer)
	{
		return;
	}

	bPlayingRareVideo = bRareVideo;
	const FString FileName = bRareVideo ? TEXT("skrimer background.mp4") : TEXT("loop background.mp4");
	FString FullPath = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Media"), FileName));

	if (!FPaths::FileExists(FullPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Backrooms: menu background video not found: %s"), *FullPath);
		return;
	}

	// UFileMediaSource, а не OpenUrl("file:///..."): WmfMedia не открывал путь
	// с пробелом в имени файла ("loop background.mp4") — LogWmfMedia: Failed to
	// open or read media file. FileMediaSource сам корректно экранирует путь.
	UFileMediaSource* Source = NewObject<UFileMediaSource>(this);
	Source->SetFilePath(FullPath);
	Source->PrecacheFile = false;
	BackgroundSource = Source;

	BackgroundPlayer->SetLooping(false);
	if (BackgroundPlayer->OpenSource(Source))
	{
		BackgroundPlayer->Play();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Backrooms: failed to open menu video %s"), *FullPath);
	}
}

void UBackroomsMainMenuWidget::OnBackgroundVideoEnded()
{
	// Шанс скримера растёт со временем в меню: базовые 15%, к 10 минутам
	// выходят на максимум (насыщение ~70%), дальше не растут — игрок не должен
	// получать скример гарантированно, но если он «завис» в меню, оно не молчит.
	float RareChance = 0.15f;
	const float IdleRamp = FMath::Clamp(MenuIdleTime / 600.0f, 0.0f, 1.0f);
	RareChance = FMath::Lerp(0.15f, 0.70f, IdleRamp);
	if (!bPlayingRareVideo && FMath::FRand() < RareChance)
	{
		// Редкий ролик запускается только после естественного окончания основного;
		// резкого переключения посреди кадра нет.
		UE_LOG(LogTemp, Log, TEXT("Backrooms: menu — screamer background triggered"));
		TransitionAlpha = 1.0f;
		StartBackgroundVideo(true);
	}
	else
	{
		TransitionAlpha = 1.0f;
		StartBackgroundVideo(false);
	}
}

bool UBackroomsMainMenuWidget::HasSavedGame() const
{
	const FString SavePath = FPaths::ProjectSavedDir() + TEXT("SaveGames/backrooms_save.sav");
	return FPaths::FileExists(SavePath);
}

TSharedRef<SWidget> UBackroomsMainMenuWidget::RebuildWidget()
{
	SAssignNew(MenuRoot, SOverlay);
	RebuildUI();
	return MenuRoot.ToSharedRef();
}

void UBackroomsMainMenuWidget::RebuildUI()
{
	if (!MenuRoot.IsValid()) return;
	MenuRoot->ClearChildren();

	if (Brush)
	{
		MenuRoot->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[ SNew(SImage).Image(Brush) ];
	}

	SAssignNew(TransitionOverlay, SBorder)
		.BorderBackgroundColor(FLinearColor::Transparent);
	MenuRoot->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[ TransitionOverlay.ToSharedRef() ];

	// Страница выбора уровней: сетка карточек с настоящими скриншотами из
	// уровней. Открытые — кликабельны, закрытые — под замком.
	if (bShowingLevelSelect)
	{
		RebuildLevelSelect();
		return;
	}

	// Страница достижений: список с прогрессом и очками.
	if (bShowingAchievements)
	{
		RebuildAchievements();
		return;
	}

	// Страница «Управление» из главного меню упразднена: переназначение клавиш
	// теперь в настройках (вкладка «УПРАВЛЕНИЕ»). Кнопка «Управление» просто
	// открывает настройки сразу на этой вкладке.
	if (!bShowingSettings)
	{
		// === MAIN MENU: заголовок слева вверху, кнопки — простой текст слева ===
		const bool bCanContinue = HasSavedGame();
		const float BtnAlpha = bCanContinue ? 1.0f : 0.35f;

		// Кнопка-текст без прямоугольника: встроенный NoBorder-стиль.
		const FButtonStyle& NoBorder = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		auto TextButton = [&](const FString& Label, float Size, const FLinearColor& Color,
			TFunction<FReply()> OnClick, bool bEnabled) -> TSharedRef<SWidget>
		{
			return SNew(SButton)
				.ButtonStyle(&NoBorder)
				.ContentPadding(FMargin(0.0f, 5.0f))
				.IsEnabled(bEnabled)
				.OnClicked_Lambda([OnClick]() -> FReply { return OnClick(); })
				[ SNew(STextBlock)
					.Text(FText::FromString(Label))
					.Font(MakeFont(MenuFont, (int32)Size))
					.ColorAndOpacity(Color) ];
		};

		TSharedRef<SWidget> MenuCol =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(70.0f, 55.0f, 0.0f, 0.0f)
				[ SNew(STextBlock).Text(FText::FromString(TEXT("B A C K R O O M S")))
					.Font(MakeFont(MenuFont, 52)).ColorAndOpacity(FLinearColor(0.95f, 0.85f, 0.45f))
					.ShadowOffset(FVector2D(3, 3)).ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.8f)) ]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(74.0f, 2.0f, 0.0f, 42.0f)
				[ SNew(STextBlock).Text(BackroomsLoc::Text(TEXT("Menu.Subtitle")))
					.Font(MakeFont(MenuFont, 13)).ColorAndOpacity(FLinearColor(0.55f, 0.55f, 0.55f, 0.85f)) ]

			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(74.0f, 4.0f, 0.0f, 4.0f)
				[ TextButton(BackroomsLoc::Get(TEXT("Menu.NewGame")), 26, FLinearColor(0.95f, 0.95f, 0.95f), [this]() { return OnPlayClicked(); }, true) ]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(74.0f, 4.0f, 0.0f, 4.0f)
				[ TextButton(BackroomsLoc::Get(TEXT("Menu.Continue")), 26, FLinearColor(BtnAlpha, BtnAlpha, BtnAlpha), [this]() { return OnContinueClicked(); }, bCanContinue) ]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(74.0f, 4.0f, 0.0f, 4.0f)
				[ TextButton(BackroomsLoc::Get(TEXT("Menu.Levels")), 20, FLinearColor(0.72f, 0.72f, 0.72f), [this]() { return OnLevelsClicked(); }, true) ]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(74.0f, 4.0f, 0.0f, 4.0f)
				[ TextButton(BackroomsLoc::Get(TEXT("Menu.Achievements")), 20, FLinearColor(0.72f, 0.72f, 0.72f), [this]() { return OnAchievementsClicked(); }, true) ]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(74.0f, 4.0f, 0.0f, 4.0f)
				[ TextButton(BackroomsLoc::Get(TEXT("Menu.Settings")), 20, FLinearColor(0.72f, 0.72f, 0.72f), [this]() { return OnSettingsClicked(); }, true) ]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(74.0f, 4.0f, 0.0f, 4.0f)
				[ TextButton(BackroomsLoc::Get(TEXT("Menu.Controls")), 20, FLinearColor(0.72f, 0.72f, 0.72f), [this]() { return OnControlsClicked(); }, true) ]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(74.0f, 4.0f, 0.0f, 4.0f)
				[ TextButton(BackroomsLoc::Get(TEXT("Menu.Quit")), 20, FLinearColor(0.72f, 0.72f, 0.72f), [this]() { return OnQuitClicked(); }, true) ]
			+ SVerticalBox::Slot().FillHeight(1.0f) [ SNew(SSpacer) ];

		MenuRoot->AddSlot().HAlign(HAlign_Fill).VAlign(VAlign_Fill) [ MenuCol ];
	}
	else
	{
		// === SETTINGS: весь экран. Слева сайдбар с категориями, справа карточка
		// выбранной секции. Скролла нет — каждый раздел помещается на экране.
		const FLinearColor LabelColor(0.92f, 0.84f, 0.55f);
		const FLinearColor ValueColor(0.98f, 0.98f, 0.98f);
		const FLinearColor DimColor(0.62f, 0.62f, 0.62f);
		const FButtonStyle& NoBorder = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");

		// Универсальная строка настройки: подпись слева, слайдер, значение.
		// Никакого RebuildUI при перетаскивании — иначе слайдер уничтожается
		// под курсором. Меняем только текст значения и применяем настройку.
		//
		// Ширины фиксированы (подпись фиксирована, значение фиксировано, слайдер
		// забирает остаток), иначе длинные подписи сжимают слайдер вбок — именно
		// из-за этого он раньше «съезжал» к краю.
		auto SliderRow = [&](const FString& Label, float NormValue,
			TFunction<void(float)> OnChanged,
			TSharedPtr<STextBlock>& OutLabel, const FString& InitText) -> TSharedRef<SWidget>
		{
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.34f).VAlign(VAlign_Center).Padding(0.0f, 7.0f)
					[ SNew(STextBlock).Text(FText::FromString(Label))
						.Font(MakeFont(MenuFont, 18)).ColorAndOpacity(LabelColor) ]
				+ SHorizontalBox::Slot().FillWidth(0.66f).VAlign(VAlign_Center).Padding(24.0f, 0.0f)
					[ SNew(SBox).HeightOverride(30.0f).VAlign(VAlign_Center)
						[ SNew(SSlider).Style(&GetBackroomsSliderStyle()).Value(NormValue).OnValueChanged_Lambda([OnChanged](float V) { OnChanged(V); }) ] ]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.0f, 0.0f, 4.0f, 0.0f)
					[ SNew(SBox).WidthOverride(150.0f).HAlign(HAlign_Right)
						[ SAssignNew(OutLabel, STextBlock).Text(FText::FromString(InitText))
							.Justification(ETextJustify::Right)
							.Font(MakeFont(MenuFont, 18)).ColorAndOpacity(ValueColor) ] ];
		};

		// Строка с выпадающим списком: подпись слева, комбобокс справа.
		// Строим комбобокс отдельными шагами (без вложенного SNew внутри SNew):
		// вложенные лямбды в DSL плохо перевариваются парсером макросов Slate.
		auto DropRow = [&](const FString& Label, const TArray<FString>& Options, int32 Current,
			TFunction<void(int32)> OnChanged) -> TSharedRef<SWidget>
		{
			// Items и текущий выбор держим через shared-ptr: делегаты комбобокса
			// хранят их, поэтому массив переживёт построение виджета.
			TSharedPtr<TArray<TSharedPtr<FString>>> Items = MakeShared<TArray<TSharedPtr<FString>>>();
			for (const FString& S : Options)
			{
				Items->Add(MakeShared<FString>(S));
			}
			TSharedPtr<FString> CurrentItem = Items->IsValidIndex(Current)
				? (*Items)[Current] : (Items->Num() ? (*Items)[0] : nullptr);

			TSharedRef<SWidget> Combo = SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(Items.Get())
				.InitiallySelectedItem(CurrentItem)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) -> TSharedRef<SWidget>
				{
					return SNew(STextBlock)
						.Text(FText::FromString(Item.IsValid() ? *Item : FString()))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16));
				})
				.OnSelectionChanged_Lambda([OnChanged, Items](TSharedPtr<FString> Item, ESelectInfo::Type) -> void
				{
					if (Item.IsValid())
					{
						const int32 Idx = Items->IndexOfByKey(Item);
						if (Idx != INDEX_NONE) { OnChanged(Idx); }
					}
				})
				[
					SNew(STextBlock)
					.Text(FText::FromString(CurrentItem.IsValid() ? *CurrentItem : FString()))
					.Font(MakeFont(MenuFont, 16)).ColorAndOpacity(ValueColor)
				];

			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.34f).VAlign(VAlign_Center).Padding(0.0f, 7.0f)
					[ SNew(STextBlock).Text(FText::FromString(Label))
						.Font(MakeFont(MenuFont, 18)).ColorAndOpacity(LabelColor) ]
				+ SHorizontalBox::Slot().FillWidth(0.66f).VAlign(VAlign_Center).Padding(24.0f, 0.0f)
					[ SNew(SBox).HeightOverride(32.0f)
						[ Combo ] ];
		};

		const FString ResInit = Resolutions.IsValidIndex(SelectedRes) ? Resolutions[SelectedRes].Label : TEXT("...");
		auto RecEffectName = [](int32 Level) -> FString
		{
			switch (Level) { case 0: return BackroomsLoc::Get(TEXT("Set.RecordingEffect.Off")); case 1: return BackroomsLoc::Get(TEXT("Set.RecordingEffect.Light")); case 2: return BackroomsLoc::Get(TEXT("Set.RecordingEffect.Medium")); default: return BackroomsLoc::Get(TEXT("Set.RecordingEffect.Strong")); }
		};

		// ---- ЭКРАН ----
		TSharedRef<SVerticalBox> ScreenBox = SNew(SVerticalBox);
		ScreenBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Resolution")), Resolutions.Num() > 1 ? (float)SelectedRes / (float)(Resolutions.Num() - 1) : 0.0f,
				[this](float V)
				{
					if (Resolutions.Num() < 2) return;
					SelectedRes = FMath::Clamp(FMath::RoundToInt(V * (Resolutions.Num() - 1)), 0, Resolutions.Num() - 1);
					if (ResLabel.IsValid()) ResLabel->SetText(FText::FromString(Resolutions[SelectedRes].Label));
					ApplySettings();
				}, ResLabel, ResInit) ];
		ScreenBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.DisplayMode")), (float)DisplayMode / 2.0f,
				[this](float V)
				{
					DisplayMode = FMath::Clamp(FMath::RoundToInt(V * 2.0f), 0, 2);
					if (DisplayModeLabel.IsValid()) DisplayModeLabel->SetText(FText::FromString(DisplayModeName(DisplayMode)));
					ApplySettings();
				}, DisplayModeLabel, DisplayModeName(DisplayMode)) ];
		ScreenBox->AddSlot().AutoHeight()
			[ SliderRow(TEXT("VSync"), bVSync ? 1.0f : 0.0f,
				[this](float V)
				{
					bVSync = V >= 0.5f;
					if (VSyncLabel.IsValid())
					{
						VSyncLabel->SetText(FText::FromString(bVSync ? BackroomsLoc::Get(TEXT("Keys.On")) : BackroomsLoc::Get(TEXT("Keys.Off"))));
						VSyncLabel->SetColorAndOpacity(bVSync ? FSlateColor(FLinearColor(0.35f, 0.9f, 0.35f)) : FSlateColor(FLinearColor(0.9f, 0.35f, 0.35f)));
					}
					ApplySettings();
				}, VSyncLabel, bVSync ? BackroomsLoc::Get(TEXT("Keys.On")) : BackroomsLoc::Get(TEXT("Keys.Off"))) ];
		ScreenBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.RenderScale")), (RenderScale - 0.25f) / 1.75f,
				[this](float V)
				{
					RenderScale = FMath::Clamp(0.25f + V * 1.75f, 0.25f, 2.0f);
					if (RenderScaleLabel.IsValid()) RenderScaleLabel->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), RenderScale * 100.0f)));
					ApplySettings();
				}, RenderScaleLabel, FString::Printf(TEXT("%.0f%%"), RenderScale * 100.0f)) ];
		ScreenBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Gamma")), (Gamma - 0.5f) / 2.5f,
				[this](float V)
				{
					Gamma = FMath::Clamp(0.5f + V * 2.5f, 0.5f, 3.0f);
					if (GammaLabel.IsValid()) GammaLabel->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Gamma)));
					ApplySettings();
				}, GammaLabel, FString::Printf(TEXT("%.1f"), Gamma)) ];

		// Язык интерфейса: выбор из списка стран/локалей (а не ползунок).
		// Смена применяется сразу: RebuildUI перечитает строки на новом языке.
		Language = (uint8)BackroomsLoc::GetLanguage();
		ScreenBox->AddSlot().AutoHeight()
			[ DropRow(BackroomsLoc::Get(TEXT("Set.Language")),
				BackroomsLoc::LanguageLabels(),
				(int32)Language,
				[this](int32 Index)
				{
					Language = (uint8)Index;
					BackroomsLoc::SetLanguage((EBackroomsLanguage)Language);
					RebuildUI();
				}) ];

		// ---- КАЧЕСТВО ----
		TSharedRef<SVerticalBox> QualityBox = SNew(SVerticalBox);
		QualityBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Shadows")), (float)ShadowQuality / 3.0f,
				[this](float V) { ShadowQuality = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3); if (ShadowLabel.IsValid()) ShadowLabel->SetText(FText::FromString(ScalabilityName(ShadowQuality))); ApplySettings(); }, ShadowLabel, ScalabilityName(ShadowQuality)) ];
		QualityBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Textures")), (float)TextureQuality / 3.0f,
				[this](float V) { TextureQuality = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3); if (TexLabel.IsValid()) TexLabel->SetText(FText::FromString(ScalabilityName(TextureQuality))); ApplySettings(); }, TexLabel, ScalabilityName(TextureQuality)) ];
		QualityBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.AntiAliasing")), (float)AAQuality / 3.0f,
				[this](float V) { AAQuality = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3); if (AALabel.IsValid()) AALabel->SetText(FText::FromString(ScalabilityName(AAQuality))); ApplySettings(); }, AALabel, ScalabilityName(AAQuality)) ];
		QualityBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.PostProcess")), (float)PostProcess / 3.0f,
				[this](float V) { PostProcess = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3); if (PostProcessLabel.IsValid()) PostProcessLabel->SetText(FText::FromString(ScalabilityName(PostProcess))); ApplySettings(); }, PostProcessLabel, ScalabilityName(PostProcess)) ];
		QualityBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Effects")), (float)EffectsQuality / 3.0f,
				[this](float V) { EffectsQuality = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3); if (EffectsLabel.IsValid()) EffectsLabel->SetText(FText::FromString(ScalabilityName(EffectsQuality))); ApplySettings(); }, EffectsLabel, ScalabilityName(EffectsQuality)) ];
		QualityBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.ViewDistance")), (float)ViewDistance / 3.0f,
				[this](float V) { ViewDistance = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3); if (ViewDistLabel.IsValid()) ViewDistLabel->SetText(FText::FromString(ScalabilityName(ViewDistance))); ApplySettings(); }, ViewDistLabel, ScalabilityName(ViewDistance)) ];
		QualityBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.DepthOfField")), (float)DOFQuality / 2.0f,
				[this](float V) { DOFQuality = FMath::Clamp(FMath::RoundToInt(V * 2.0f), 0, 2); if (DOFLabel.IsValid()) DOFLabel->SetText(FText::FromString(DOFName(DOFQuality))); ApplySettings(); }, DOFLabel, DOFName(DOFQuality)) ];
		QualityBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Bloom")), (float)BloomQuality / 3.0f,
				[this](float V) { BloomQuality = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3); if (BloomLabel.IsValid()) BloomLabel->SetText(FText::FromString(ScalabilityName(BloomQuality))); ApplySettings(); }, BloomLabel, ScalabilityName(BloomQuality)) ];
		QualityBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Lumen")), (float)LumenQuality / 3.0f,
				[this](float V) { LumenQuality = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3); if (LumenLabel.IsValid()) LumenLabel->SetText(FText::FromString(LumenName(LumenQuality))); ApplySettings(); }, LumenLabel, LumenName(LumenQuality)) ];
		QualityBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Upscale")), (float)DLSSMode / 4.0f,
				[this](float V) { DLSSMode = FMath::Clamp(FMath::RoundToInt(V * 4.0f), 0, 4); if (DLSSLabel.IsValid()) DLSSLabel->SetText(FText::FromString(DLSSModeName(DLSSMode))); ApplySettings(); }, DLSSLabel, DLSSModeName(DLSSMode)) ];
		QualityBox->AddSlot().AutoHeight().Padding(FMargin(0.0f, 18.0f, 0.0f, 0.0f))
			[
				SNew(SButton)
				.ContentPadding(FMargin(18, 8))
				.OnClicked_Lambda([this]() -> FReply
				{
					BackroomsQuality::SetPotatoMode(!BackroomsQuality::IsPotatoMode(), GetWorld());
					RebuildUI();
					return FReply::Handled();
				})
				[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Set.PotatoMode"))))
					.Font(MakeFont(MenuFont, 14))
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(BackroomsQuality::IsPotatoMode() ? FLinearColor(0.35f, 0.85f, 0.40f) : FLinearColor(0.9f, 0.9f, 0.9f)) ]
			];
		QualityBox->AddSlot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
			[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Set.PotatoMode.Hint"))))
				.Font(MakeFont(MenuFont, 12)).ColorAndOpacity(FLinearColor(0.62f, 0.62f, 0.62f))
				.AutoWrapText(true) ];

		// ---- ИГРА: сложность влияет на генерацию и угрозы ----
		Difficulty = (int32)BackroomsDifficulty::GetCurrent();
		TSharedRef<SVerticalBox> GameBox = SNew(SVerticalBox);
		GameBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Difficulty")), (float)Difficulty / (float)((int32)EBackroomsDifficulty::Count - 1),
				[this](float V)
				{
					Difficulty = FMath::Clamp(FMath::RoundToInt(V * (float)((int32)EBackroomsDifficulty::Count - 1)),
						0, (int32)EBackroomsDifficulty::Count - 1);
					BackroomsDifficulty::SetCurrent((EBackroomsDifficulty)Difficulty);
					if (DifficultyLabel.IsValid())
					{
						DifficultyLabel->SetText(BackroomsDifficulty::NameText((EBackroomsDifficulty)Difficulty));
					}
				}, DifficultyLabel, BackroomsDifficulty::NameText((EBackroomsDifficulty)Difficulty).ToString()) ];
		GameBox->AddSlot().AutoHeight().Padding(FMargin(0.0f, 2.0f, 0.0f, 10.0f))
			[ SNew(STextBlock)
				.Text(BackroomsDifficulty::DescriptionText((EBackroomsDifficulty)Difficulty))
				.Font(MakeFont(MenuFont, 14)).ColorAndOpacity(FLinearColor(0.62f, 0.62f, 0.62f))
				.AutoWrapText(true) ];
		GameBox->AddSlot().AutoHeight().Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
			[ SNew(STextBlock)
				.Text(FText::FromString(BackroomsLoc::Get(TEXT("Set.Difficulty.Hint"))))
				.Font(MakeFont(MenuFont, 13)).ColorAndOpacity(FLinearColor(0.55f, 0.55f, 0.55f))
				.AutoWrapText(true) ];

		// ---- КАМЕРА ----
		TSharedRef<SVerticalBox> CameraBox = SNew(SVerticalBox);
		CameraBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.FOV")), (FOV - 60.0f) / 60.0f,
				[this](float V)
				{
					FOV = FMath::Clamp(60.0f + V * 60.0f, 60.0f, 120.0f);
					if (FOVLabel.IsValid()) FOVLabel->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), FOV)));
					ApplySettings();
				}, FOVLabel, FString::Printf(TEXT("%.0f"), FOV)) ];
		CameraBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Sensitivity")), (Sensitivity - 0.2f) / 4.8f,
				[this](float V)
				{
					Sensitivity = FMath::Clamp(0.2f + V * 4.8f, 0.2f, 5.0f);
					if (SensitivityLabel.IsValid()) SensitivityLabel->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Sensitivity)));
					if (ABackroomsPlayerCharacter* P = Cast<ABackroomsPlayerCharacter>(GetOwningPlayerPawn())) P->SetMouseSensitivity(Sensitivity);
				}, SensitivityLabel, FString::Printf(TEXT("%.1f"), Sensitivity)) ];
		CameraBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.RecordingEffect")), (float)RecordingEffect / 3.0f,
				[this, RecEffectName](float V)
				{
					RecordingEffect = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3);
					if (RecordingEffectLabel.IsValid()) RecordingEffectLabel->SetText(FText::FromString(RecEffectName(RecordingEffect)));
					ApplyCameraEffect();
				}, RecordingEffectLabel, RecEffectName(RecordingEffect)) ];

		// ---- ЗВУК ----
		TSharedRef<SVerticalBox> SoundBox = SNew(SVerticalBox);
		SoundBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Volume.Master")), MasterVolume,
				[this](float V) { MasterVolume = FMath::Clamp(V, 0.0f, 1.0f); BackroomsAudio::SetVolume(TEXT("MasterVolume"), MasterVolume); ApplyMenuAudioVolume(); if (MasterVolLabel.IsValid()) MasterVolLabel->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MasterVolume * 100.0f)))); }, MasterVolLabel, FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MasterVolume * 100.0f))) ];
		SoundBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Volume.Menu")), MenuVolume,
				[this](float V) { MenuVolume = FMath::Clamp(V, 0.0f, 1.0f); BackroomsAudio::SetVolume(TEXT("MenuVolume"), MenuVolume); ApplyMenuAudioVolume(); if (MenuVolLabel.IsValid()) MenuVolLabel->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MenuVolume * 100.0f)))); }, MenuVolLabel, FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MenuVolume * 100.0f))) ];
		SoundBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Volume.Game")), GameVolume,
				[this](float V) { GameVolume = FMath::Clamp(V, 0.0f, 1.0f); BackroomsAudio::SetVolume(TEXT("GameVolume"), GameVolume); if (GameVolLabel.IsValid()) GameVolLabel->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(GameVolume * 100.0f)))); }, GameVolLabel, FString::Printf(TEXT("%d%%"), FMath::RoundToInt(GameVolume * 100.0f))) ];
		SoundBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Volume.Monster")), MonsterVolume,
				[this](float V) { MonsterVolume = FMath::Clamp(V, 0.0f, 1.0f); BackroomsAudio::SetVolume(TEXT("MonsterVolume"), MonsterVolume); if (MonsterVolLabel.IsValid()) MonsterVolLabel->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MonsterVolume * 100.0f)))); }, MonsterVolLabel, FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MonsterVolume * 100.0f))) ];
		SoundBox->AddSlot().AutoHeight()
			[ SliderRow(BackroomsLoc::Get(TEXT("Set.Volume.Other")), OtherVolume,
				[this](float V) { OtherVolume = FMath::Clamp(V, 0.0f, 1.0f); BackroomsAudio::SetVolume(TEXT("OtherVolume"), OtherVolume); if (OtherVolLabel.IsValid()) OtherVolLabel->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(OtherVolume * 100.0f)))); }, OtherVolLabel, FString::Printf(TEXT("%d%%"), FMath::RoundToInt(OtherVolume * 100.0f))) ];

		// ---- УПРАВЛЕНИЕ: переназначение кнопок ----
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

		// Не переназначаемые строки (мышь у камеры), просто справка.
		auto ReadonlyRow = [&](const FString& Key, const FString& Action) -> TSharedRef<SWidget>
		{
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.5f).VAlign(VAlign_Center).Padding(0, 5)
					[ SNew(STextBlock).Text(FText::FromString(Action)).Font(MakeFont(MenuFont, 15)).ColorAndOpacity(ValueColor) ]
				+ SHorizontalBox::Slot().FillWidth(0.5f).VAlign(VAlign_Center)
					[ SNew(STextBlock).Text(FText::FromString(Key)).Font(MakeFont(MenuFont, 15)).ColorAndOpacity(DimColor) ];
		};

		// Строка переназначения: действие — кнопка с текущей клавишей.
		auto BindRow = [&](const FString& BindId, const FString& DefaultKey) -> TSharedRef<SWidget>
		{
			const bool bWaitingThis = bWaitingForKey && PendingBindId == BindId;
			const FString KeyText = bWaitingThis ? TEXT("?") : BackroomsInput::KeyDisplayName(BackroomsInput::GetEffectiveKey(*BindId, *DefaultKey));
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.5f).VAlign(VAlign_Center).Padding(0, 5)
					[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(*(FString(TEXT("Bind.")) + BindId)))).Font(MakeFont(MenuFont, 15)).ColorAndOpacity(ValueColor) ]
				+ SHorizontalBox::Slot().FillWidth(0.28f).VAlign(VAlign_Center)
					[
						SNew(SButton)
						.IsEnabled(!bWaitingForKey || bWaitingThis)
						.HAlign(HAlign_Fill)
						.ContentPadding(FMargin(10.0f, 6.0f))
						.OnClicked_Lambda([this, BindId]() -> FReply { return OnKeyBindClicked(BindId); })
						[ SNew(STextBlock).Text(FText::FromString(KeyText))
							.Font(MakeFont(MenuFont, 15))
							.Justification(ETextJustify::Center)
							.ColorAndOpacity(bWaitingThis ? FLinearColor(0.35f, 0.85f, 0.40f) : FLinearColor(0.95f, 0.85f, 0.55f)) ]
					]
				+ SHorizontalBox::Slot().FillWidth(0.22f).VAlign(VAlign_Center).Padding(10, 0, 0, 0)
					[ SNew(STextBlock).Text(FText::FromString(bWaitingThis ? TEXT("") : BackroomsLoc::Get(TEXT("Bind.Change")))).Font(MakeFont(MenuFont, 12)).ColorAndOpacity(DimColor) ];
		};

		// Собирает контент выбранной категории.
		auto BuildTabContent = [&](int32 Tab) -> TSharedRef<SWidget>
		{
			switch (Tab)
			{
			case 0: return MakeMainMenuSection(MenuFont, BackroomsLoc::Text(TEXT("Tab.Screen")), ScreenBox);
			case 1: return MakeMainMenuSection(MenuFont, BackroomsLoc::Text(TEXT("Tab.Quality")), QualityBox);
			case 2: return MakeMainMenuSection(MenuFont, BackroomsLoc::Text(TEXT("Tab.Camera")), CameraBox);
			case 3: return MakeMainMenuSection(MenuFont, BackroomsLoc::Text(TEXT("Tab.Sound")), SoundBox);
			case 4: return MakeMainMenuSection(MenuFont, BackroomsLoc::Text(TEXT("Tab.Game")), GameBox);
			default: break;
			}

			TSharedRef<SVerticalBox> ControlsBox = SNew(SVerticalBox);
			ControlsBox->AddSlot().AutoHeight().Padding(2.0f, 0.0f, 2.0f, 12.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
					[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Tab.Controls"))))
						.Font(MakeFont(MenuFont, 22)).ColorAndOpacity(FLinearColor(0.96f, 0.90f, 0.62f)) ]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ButtonStyle(&NoBorder)
						.ContentPadding(FMargin(12.0f, 4.0f))
						.Visibility(bWaitingForKey ? EVisibility::Visible : EVisibility::Collapsed)
						.OnClicked_Lambda([this]() -> FReply { return OnBindCancel(); })
						[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Bind.Cancel"))))
							.Font(MakeFont(MenuFont, 15)).ColorAndOpacity(FLinearColor(0.95f, 0.42f, 0.38f)) ]
					]
			];

			if (bWaitingForKey)
			{
				ControlsBox->AddSlot().AutoHeight().Padding(2.0f, 0.0f, 2.0f, 10.0f)
					[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Bind.PressKey"))))
						.Font(MakeFont(MenuFont, 14)).ColorAndOpacity(FLinearColor(0.40f, 0.85f, 0.40f)) ];
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
					.Font(MakeFont(MenuFont, 14)).ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f)) ]
			];
			return ControlsBox;
		};

		// Пункт левого сайдбара: текстовая кнопка категории.
		auto SideItem = [&](const TCHAR* Label, int32 Tab, bool bSelected) -> TSharedRef<SWidget>
		{
			return SNew(SButton)
				.ButtonStyle(&NoBorder)
				.HAlign(HAlign_Left)
				.ContentPadding(FMargin(26.0f, 10.0f, 26.0f, 10.0f))
				.OnClicked_Lambda([this, Tab]() -> FReply { return OnSettingsTabClicked(Tab); })
				[ SNew(STextBlock)
					.Text(FText::FromString(Label))
					.Font(MakeFont(MenuFont, bSelected ? 20 : 16))
					.ColorAndOpacity(bSelected ? FLinearColor(0.98f, 0.88f, 0.45f) : FLinearColor(0.82f, 0.82f, 0.82f))
					.ShadowOffset(bSelected ? FVector2D(1.0f, 1.0f) : FVector2D::ZeroVector)
					.ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.7f)) ];
		};

		TSharedRef<SVerticalBox> SideBox = SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(26.0f, 22.0f, 26.0f, 14.0f))
			[ SNew(STextBlock).Text(BackroomsLoc::Text(TEXT("Settings.Categories"))).Font(MakeFont(MenuFont, 13)).ColorAndOpacity(FLinearColor(0.55f, 0.55f, 0.55f)) ]
		+ SVerticalBox::Slot().AutoHeight()
			[ SideItem(*BackroomsLoc::Get(TEXT("Tab.Screen")), 0, SettingsTab == 0) ]
		+ SVerticalBox::Slot().AutoHeight()
			[ SideItem(*BackroomsLoc::Get(TEXT("Tab.Quality")), 1, SettingsTab == 1) ]
		+ SVerticalBox::Slot().AutoHeight()
			[ SideItem(*BackroomsLoc::Get(TEXT("Tab.Camera")), 2, SettingsTab == 2) ]
		+ SVerticalBox::Slot().AutoHeight()
			[ SideItem(*BackroomsLoc::Get(TEXT("Tab.Sound")), 3, SettingsTab == 3) ]
		+ SVerticalBox::Slot().AutoHeight()
			[ SideItem(*BackroomsLoc::Get(TEXT("Tab.Game")), 4, SettingsTab == 4) ]
		+ SVerticalBox::Slot().AutoHeight()
			[ SideItem(*BackroomsLoc::Get(TEXT("Tab.Controls")), 5, SettingsTab == 5) ]
			+ SVerticalBox::Slot().FillHeight(1.0f) [ SNew(SSpacer) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(26.0f, 6.0f, 26.0f, 20.0f))
				[
					SNew(SButton)
					.ButtonStyle(&NoBorder)
					.HAlign(HAlign_Left)
					.ContentPadding(FMargin(0.0f, 6.0f))
					.OnClicked_Lambda([this]() -> FReply { return OnBackFromSettings(); })
					[ SNew(STextBlock).Text(BackroomsLoc::Text(TEXT("Menu.Back"))).Font(MakeFont(MenuFont, 18)).ColorAndOpacity(FLinearColor(0.78f, 0.78f, 0.78f)) ]
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

		// Настройки занимают весь экран: заголовок во всю ширину, ниже —
		// сайдбар категорий и карточка контента. Отступы сведены к минимуму,
		// чтобы это была широкая панель, а не узкая колонка по центру.
		TSharedRef<SVerticalBox> SettingsCol =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(22.0f, 20.0f, 22.0f, 12.0f))
				[ SNew(STextBlock).Text(BackroomsLoc::Text(TEXT("Settings.Title")))
					.Font(MakeFont(MenuFont, 32)).ColorAndOpacity(FLinearColor(0.95f, 0.85f, 0.45f))
					.ShadowOffset(FVector2D(2, 2)).ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.8f)) ]
			+ SVerticalBox::Slot().FillHeight(1.0f).Padding(FMargin(22.0f, 0.0f, 22.0f, 20.0f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0.0f, 0.0f, 18.0f, 0.0f))
						[ SNew(SBox).WidthOverride(330.0f) [ SidePanel ] ]
					+ SHorizontalBox::Slot().FillWidth(1.0f)
						[ ContentCard ]
				];

		// Полупрозрачная подложка во весь экран: видео просвечивает, но текст
		// остаётся читабельным.
		TSharedRef<SBorder> BG = SNew(SBorder).BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.02f, 0.62f));
		BG->SetContent(SettingsCol);
		MenuRoot->AddSlot() [ BG ];
	}
}

void UBackroomsMainMenuWidget::RebuildLevelSelect()
{
	UBackroomsLevelBook* Book = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		Book = GI->GetSubsystem<UBackroomsLevelBook>();
	}

	const FLinearColor LabelColor(0.92f, 0.84f, 0.55f);
	const FLinearColor ValueColor(0.98f, 0.98f, 0.98f);

	// Полупрозрачная подложка: видео меню остаётся видно по краям.
	TSharedRef<SBorder> BG = SNew(SBorder).BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.02f, 0.72f));

	TSharedRef<SVerticalBox> Col = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(22.0f, 20.0f, 22.0f, 10.0f))
			[ SNew(STextBlock).Text(BackroomsLoc::Text(TEXT("Levels.Title")))
				.Font(MakeFont(MenuFont, 32)).ColorAndOpacity(FLinearColor(0.95f, 0.85f, 0.45f))
				.ShadowOffset(FVector2D(2, 2)).ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.8f)) ];

	const int32 Furthest = Book ? Book->GetFurthestLevel() : -1;
	Col->AddSlot().AutoHeight().Padding(FMargin(24.0f, 0.0f, 22.0f, 14.0f))
		[ SNew(STextBlock)
			.Text(FText::FromString(LocFormat(
				BackroomsLoc::Get(TEXT("Levels.UnlockedFmt")),
				{ FString::FromInt(Furthest + 1) })))
			.Font(MakeFont(MenuFont, 14)).ColorAndOpacity(FLinearColor(0.60f, 0.60f, 0.60f)) ];

	// Сетка карточек 5x2. Каждая карточка — кнопка со скриншотом уровня.
	TSharedRef<SUniformGridPanel> Grid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(10.0f));

	for (int32 L = 0; L < 10; ++L)
	{
		const bool bUnlocked = Book ? Book->IsUnlocked(L) : (L == 0);
		const bool bVisited = Book ? Book->IsVisited(L) : false;

		// Превью грузим только для открытых уровней: закрытые — под замком.
		FSlateBrush* ThumbBrush = nullptr;
		if (bUnlocked && Book)
		{
			TSharedPtr<FSlateDynamicImageBrush>* Cached = LevelThumbBrushes.Find(L);
			if (!Cached)
			{
				const FName ResName(*FString::Printf(TEXT("BackroomsLevelThumb_%d"), L));
				Cached = &LevelThumbBrushes.Add(L, LoadThumbBrush(Book->GetThumbnailPath(L), ResName));
			}
			if (Cached->IsValid())
			{
				ThumbBrush = Cached->Get();
			}
		}

		const FLinearColor Accent = UBackroomsLevelBook::LevelAccent(L);

		TSharedRef<SOverlay> CardOverlay = SNew(SOverlay);

		// Фон карточки: превью либо акцентная заливка уровня.
		if (ThumbBrush)
		{
			CardOverlay->AddSlot()
				.HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[ SNew(SImage).Image(ThumbBrush)
					.ColorAndOpacity(bUnlocked ? FLinearColor::White : FLinearColor(0.25f, 0.25f, 0.25f, 1.0f)) ];
		}
		else
		{
			CardOverlay->AddSlot()
				.HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[ SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.ColorAndOpacity(bUnlocked ? Accent * 0.45f : FLinearColor(0.10f, 0.10f, 0.12f, 1.0f)) ];
		}

		// Затемнение снизу под текст.
		CardOverlay->AddSlot()
			.HAlign(HAlign_Fill).VAlign(VAlign_Bottom)
			[ SNew(SBorder).BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.62f))
				.Padding(FMargin(8.0f, 4.0f))
				[ SNew(STextBlock)
					.Text(UBackroomsLevelBook::LevelName(L))
					.Font(MakeFont(MenuFont, 15))
					.ColorAndOpacity(bUnlocked ? ValueColor : FLinearColor(0.55f, 0.55f, 0.55f)) ] ];

		// Метка «пройдено» / замок поверх превью.
		if (!bUnlocked)
		{
			CardOverlay->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Center)
				[ SNew(STextBlock).Text(FText::FromString(BackroomsLoc::Get(TEXT("Levels.Locked"))))
					.Font(MakeFont(MenuFont, 16)).ColorAndOpacity(FLinearColor(0.75f, 0.75f, 0.75f, 0.9f)) ];
		}
		else if (bVisited)
		{
			CardOverlay->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(6.0f)
				[ SNew(STextBlock).Text(FText::FromString(TEXT("✓")))
					.Font(MakeFont(MenuFont, 18)).ColorAndOpacity(FLinearColor(0.45f, 0.90f, 0.50f)) ];
		}

		const FButtonStyle& NoBorder = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		TSharedRef<SWidget> Card = SNew(SButton)
			.ButtonStyle(&NoBorder)
			.IsEnabled(bUnlocked)
			.OnClicked_Lambda([this, L]() -> FReply { return OnLevelClicked(L); })
			[ SNew(SBox).WidthOverride(300.0f).HeightOverride(180.0f)
				[ CardOverlay ] ];

		Grid->AddSlot(L % 5, L / 5)
			[ Card ];
	}

	TSharedRef<SScrollBox> Scroll = SNew(SScrollBox);
	Scroll->AddSlot()[ Grid ];
	Col->AddSlot().AutoHeight().Padding(FMargin(12.0f, 0.0f, 12.0f, 12.0f))
		[ Scroll ];

	Col->AddSlot().FillHeight(1.0f) [ SNew(SSpacer) ];
	Col->AddSlot().AutoHeight().Padding(FMargin(22.0f, 6.0f, 22.0f, 20.0f))
		[ SNew(SButton)
			.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.HAlign(HAlign_Left)
			.ContentPadding(FMargin(0.0f, 6.0f))
			.OnClicked_Lambda([this]() -> FReply { return OnBackFromLevels(); })
			[ SNew(STextBlock).Text(BackroomsLoc::Text(TEXT("Menu.Back")))
				.Font(MakeFont(MenuFont, 18)).ColorAndOpacity(FLinearColor(0.78f, 0.78f, 0.78f)) ] ];

	BG->SetContent(Col);
	MenuRoot->AddSlot()[ BG ];
}

void UBackroomsMainMenuWidget::RebuildAchievements()
{
	UBackroomsAchievements* Ach = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		Ach = GI->GetSubsystem<UBackroomsAchievements>();
	}

	const FLinearColor ValueColor(0.98f, 0.98f, 0.98f);
	const FLinearColor DimColor(0.60f, 0.60f, 0.60f);
	const FLinearColor GoldColor(0.95f, 0.85f, 0.45f);

	TSharedRef<SBorder> BG = SNew(SBorder).BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.02f, 0.75f));

	const int32 Unlocked = Ach ? Ach->GetUnlockedCount() : 0;
	const int32 Total = Ach ? Ach->GetTotalCount() : 0;
	const int32 Points = Ach ? Ach->GetTotalPoints() : 0;

	TSharedRef<SVerticalBox> Col = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(22.0f, 20.0f, 22.0f, 4.0f))
			[ SNew(STextBlock).Text(BackroomsLoc::Text(TEXT("Ach.Title")))
				.Font(MakeFont(MenuFont, 32)).ColorAndOpacity(GoldColor)
				.ShadowOffset(FVector2D(2, 2)).ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.8f)) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(24.0f, 0.0f, 22.0f, 14.0f))
			[ SNew(STextBlock)
				.Text(FText::FromString(LocFormat(
					BackroomsLoc::Get(TEXT("Ach.ProgressFmt")),
					{ FString::FromInt(Unlocked), FString::FromInt(Total), FString::FromInt(Points) })))
				.Font(MakeFont(MenuFont, 16)).ColorAndOpacity(ValueColor) ];

	// Список достижений. Каждая строка: флажок, название, прогресс, описание.
	TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
	TArray<FName> UnlockedIds;
	if (Ach)
	{
		// Достаём список правил через отчёт (публичный API уже отдаёт имена/порог).
		// Здесь проще показать по каждому правилу: имя, прогресс стата.
		// Правила недоступны снаружи — используем текстовый отчёт и парсим его
		// только для отображения? Нет: у нас есть конкретные геттеры, соберём сами.
	}

	// Публичный API достижений: IsUnlocked/GetStat по известным Id. Чтобы не
	// хардкодить список в UI дважды, берём его из отчёта Achievements.Report():
	// это единственная точка, где правила перечислены целиком.
	FString Report = Ach ? Ach->Report() : FString();
	TArray<FString> Lines;
	Report.ParseIntoArrayLines(Lines, false);
	for (const FString& Line : Lines)
	{
		// Формат строки: "  [X] Название (значение/порог)"
		if (!Line.StartsWith(TEXT("  [")))
		{
			continue;
		}
		const bool bDone = Line.Contains(TEXT("[X]"));
		FString Body = Line.Mid(2).TrimStartAndEnd();

		// После разделителя " | " идёт локализованное описание достижения
		// (отдаётся Report'ом; скрытых и неоткрытых — нет).
		FString RowText = Body;
		FString DescText;
		if (Body.Split(TEXT(" | "), &RowText, &DescText, ESearchCase::CaseSensitive))
		{
			DescText = DescText.TrimStartAndEnd();
		}

		TSharedRef<SVerticalBox> TextCol = SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(FText::FromString(RowText))
					.Font(MakeFont(MenuFont, 15))
					.ColorAndOpacity(bDone ? ValueColor : DimColor) ]
			+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(FText::FromString(DescText))
					.Font(MakeFont(MenuFont, 13))
					.ColorAndOpacity(FLinearColor(0.52f, 0.52f, 0.56f))
					.WrapTextAt(760.0f) ];

		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 4.0f, 10.0f, 4.0f)
				[ SNew(STextBlock).Text(FText::FromString(bDone ? TEXT("✓") : TEXT("•")))
					.Font(MakeFont(MenuFont, 18))
					.ColorAndOpacity(bDone ? FLinearColor(0.45f, 0.90f, 0.50f) : DimColor) ]
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[ TextCol ];

		TSharedRef<SBorder> Card = SNew(SBorder)
			.BorderImage(GetBackroomsContentCardBrush())
			.Padding(FMargin(12.0f, 6.0f))
			[ Row ];
		List->AddSlot().AutoHeight().Padding(FMargin(0.0f, 3.0f))[ Card ];
	}

	Col->AddSlot().FillHeight(1.0f).Padding(FMargin(22.0f, 0.0f, 22.0f, 8.0f))
		[ SNew(SScrollBox) + SScrollBox::Slot()[ List ] ];

	Col->AddSlot().AutoHeight().Padding(FMargin(22.0f, 6.0f, 22.0f, 20.0f))
		[ SNew(SButton)
			.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.HAlign(HAlign_Left)
			.ContentPadding(FMargin(0.0f, 6.0f))
			.OnClicked_Lambda([this]() -> FReply { return OnBackFromAchievements(); })
			[ SNew(STextBlock).Text(BackroomsLoc::Text(TEXT("Menu.Back")))
				.Font(MakeFont(MenuFont, 18)).ColorAndOpacity(FLinearColor(0.78f, 0.78f, 0.78f)) ] ];

	BG->SetContent(Col);
	MenuRoot->AddSlot()[ BG ];
}

void UBackroomsMainMenuWidget::SetMenuMediaActive(bool bActive)
{
	// Музыка меню — это аудиодорожка фонового видео. Без явной остановки она
	// продолжает играть в игре. Останавливаем плеер и глушим звуковой компонент.
	if (BackgroundPlayer)
	{
		if (bActive)
		{
			if (!BackgroundPlayer->IsPlaying())
			{
				BackgroundPlayer->Play();
			}
		}
		else
		{
			BackgroundPlayer->Pause();
		}
	}
	if (MenuAudio)
	{
		MenuAudio->SetVolumeMultiplier(bActive ? (MasterVolume * MenuVolume) : 0.0f);
	}
	if (MenuAudioActor)
	{
		MenuAudioActor->SetActorHiddenInGame(!bActive);
	}
}

void UBackroomsMainMenuWidget::ShowMenu(bool bShow)
{
	if (!MenuRoot.IsValid()) return;
	MenuRoot->SetVisibility(bShow ? EVisibility::Visible : EVisibility::Collapsed);
	SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (bShow)
	{
		SetMenuMediaActive(true);
		SetKeyboardFocus();
		if (APlayerController* PC = Cast<APlayerController>(GetOwningPlayer()))
		{
			FInputModeUIOnly UI;
			UI.SetWidgetToFocus(TakeWidget());
			UI.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(UI);
			PC->bShowMouseCursor = true;
			PC->SetPause(true);
		}
	}
	RebuildUI();
}

FReply UBackroomsMainMenuWidget::OnPlayClicked()
{
	ShowMenu(false);
	SetMenuMediaActive(false);
	if (APlayerController* PC = Cast<APlayerController>(GetOwningPlayer()))
	{
		PC->SetPause(false);
		FInputModeGameOnly Game;
		PC->SetInputMode(Game);
		PC->bShowMouseCursor = false;
		if (ABackroomsPlayerCharacter* C = Cast<ABackroomsPlayerCharacter>(PC->GetPawn()))
		{
			C->SetHUDVisible(true);
		}
	}
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnContinueClicked()
{
	ShowMenu(false);
	SetMenuMediaActive(false);
	if (APlayerController* PC = Cast<APlayerController>(GetOwningPlayer()))
	{
		PC->SetPause(false);
		FInputModeGameOnly Game;
		PC->SetInputMode(Game);
		PC->bShowMouseCursor = false;
		if (ABackroomsPlayerCharacter* C = Cast<ABackroomsPlayerCharacter>(PC->GetPawn()))
		{
			C->SetHUDVisible(true);
		}
	}
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnSettingsClicked()
{
	bShowingSettings = true;
	SettingsTab = 0;
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnControlsClicked()
{
	bShowingSettings = true;
	SettingsTab = 5; // УПРАВЛЕНИЕ (после добавления вкладки ИГРА)
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnLevelsClicked()
{
	bShowingSettings = false;
	bShowingLevelSelect = true;
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnBackFromLevels()
{
	bShowingLevelSelect = false;
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnAchievementsClicked()
{
	bShowingSettings = false;
	bShowingLevelSelect = false;
	bShowingAchievements = true;
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnBackFromAchievements()
{
	bShowingAchievements = false;
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnLevelClicked(int32 LevelIndex)
{
	UBackroomsLevelBook* Book = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		Book = GI->GetSubsystem<UBackroomsLevelBook>();
	}
	if (Book && !Book->IsUnlocked(LevelIndex))
	{
		return FReply::Handled(); // закрытый уровень не запускаем
	}

	// Переключаем генератор на выбранный уровень и запускаем игру.
	if (ABackroomsWorldGenerator* Gen = Cast<ABackroomsWorldGenerator>(
		UGameplayStatics::GetActorOfClass(this, ABackroomsWorldGenerator::StaticClass())))
	{
		Gen->SetLevel(LevelIndex);
	}
	return OnPlayClicked();
}

FReply UBackroomsMainMenuWidget::OnBackFromControls()
{
	bShowingControls = false;
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnBackFromSettings()
{
	bShowingSettings = false;
	bShowingControls = false;
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnResPrev()      { SelectedRes = FMath::Max(0, SelectedRes - 1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnResNext()      { SelectedRes = FMath::Min(Resolutions.Num()-1, SelectedRes+1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnDisplayModeNext() { DisplayMode = (DisplayMode+1)%3; ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnVSyncToggled() { bVSync = !bVSync; ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnShadowQualityPrev() { ShadowQuality = FMath::Max(0, ShadowQuality-1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnShadowQualityNext() { ShadowQuality = FMath::Min(3, ShadowQuality+1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnAAQualityPrev()      { AAQuality = FMath::Max(0, AAQuality-1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnAAQualityNext()      { AAQuality = FMath::Min(3, AAQuality+1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnPostProcessPrev()    { PostProcess = FMath::Max(0, PostProcess-1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnPostProcessNext()    { PostProcess = FMath::Min(3, PostProcess+1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnEffectsQualityPrev() { EffectsQuality = FMath::Max(0, EffectsQuality-1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnEffectsQualityNext() { EffectsQuality = FMath::Min(3, EffectsQuality+1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnViewDistancePrev()   { ViewDistance = FMath::Max(0, ViewDistance-1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnViewDistanceNext()   { ViewDistance = FMath::Min(3, ViewDistance+1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnTexQualityPrev()     { TextureQuality = FMath::Max(0, TextureQuality-1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnTexQualityNext()     { TextureQuality = FMath::Min(3, TextureQuality+1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnRenderScalePrev()    { RenderScale = FMath::Max(0.25f, RenderScale-0.1f); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnRenderScaleNext()    { RenderScale = FMath::Min(2.0f, RenderScale+0.1f); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnGammaPrev()          { Gamma = FMath::Max(0.5f, Gamma-0.1f); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnGammaNext()          { Gamma = FMath::Min(3.0f, Gamma+0.1f); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnFOVPrev()            { FOV = FMath::Max(60.0f, FOV-5.0f); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnFOVNext()            { FOV = FMath::Min(120.0f, FOV+5.0f); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnSensitivityPrev()    { Sensitivity = FMath::Max(0.2f, Sensitivity-0.1f); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnSensitivityNext()    { Sensitivity = FMath::Min(5.0f, Sensitivity+0.1f); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnDOFPrev()            { DOFQuality = FMath::Max(0, DOFQuality-1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnDOFNext()            { DOFQuality = FMath::Min(2, DOFQuality+1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnBloomPrev()          { BloomQuality = FMath::Max(0, BloomQuality-1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnBloomNext()          { BloomQuality = FMath::Min(3, BloomQuality+1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnLumenQualityPrev()   { LumenQuality = FMath::Max(0, LumenQuality-1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnLumenQualityNext()   { LumenQuality = FMath::Min(3, LumenQuality+1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnDLSSModePrev()       { DLSSMode = FMath::Max(0, DLSSMode-1); ApplySettings(); RebuildUI(); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnDLSSModeNext()       { DLSSMode = FMath::Min(4, DLSSMode+1); ApplySettings(); RebuildUI(); return FReply::Handled(); }

FReply UBackroomsMainMenuWidget::OnSettingsTabClicked(int32 Tab)
{
	if (SettingsTab != Tab)
	{
		SettingsTab = Tab;
		RebuildUI();
	}
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnKeyBindClicked(const FString& BindId)
{
	// Клик по кнопке «Изменить»: переводим UI в режим ожидания клавиши.
	bWaitingForKey = true;
	PendingBindId = BindId;
	RebuildUI();
	SetFocus();
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnResetBindingsClicked()
{
	bWaitingForKey = false;
	PendingBindId.Empty();
	BackroomsInput::ClearAllKeys();
	BackroomsInput::ApplyBindings();
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::OnBindCancel()
{
	bWaitingForKey = false;
	PendingBindId.Empty();
	RebuildUI();
	return FReply::Handled();
}

FReply UBackroomsMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
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

namespace
{
	float StepVolume(float Value, int32 Dir)
	{
		return FMath::Clamp(Value + 0.1f * (float)Dir, 0.0f, 1.0f);
	}
	void RefreshPct(const TSharedPtr<STextBlock>& Label, float Value)
	{
		if (Label.IsValid())
		{
			Label->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.0f))));
		}
	}
}

FReply UBackroomsMainMenuWidget::OnMasterVolPrev()  { MasterVolume  = StepVolume(MasterVolume, -1);  BackroomsAudio::SetVolume(TEXT("MasterVolume"), MasterVolume);  ApplyMenuAudioVolume(); RefreshPct(MasterVolLabel, MasterVolume);  return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnMasterVolNext()  { MasterVolume  = StepVolume(MasterVolume, +1);  BackroomsAudio::SetVolume(TEXT("MasterVolume"), MasterVolume);  ApplyMenuAudioVolume(); RefreshPct(MasterVolLabel, MasterVolume);  return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnMenuVolPrev()    { MenuVolume    = StepVolume(MenuVolume, -1);    BackroomsAudio::SetVolume(TEXT("MenuVolume"), MenuVolume);      ApplyMenuAudioVolume(); RefreshPct(MenuVolLabel, MenuVolume);      return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnMenuVolNext()    { MenuVolume    = StepVolume(MenuVolume, +1);    BackroomsAudio::SetVolume(TEXT("MenuVolume"), MenuVolume);      ApplyMenuAudioVolume(); RefreshPct(MenuVolLabel, MenuVolume);      return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnGameVolPrev()    { GameVolume    = StepVolume(GameVolume, -1);    BackroomsAudio::SetVolume(TEXT("GameVolume"), GameVolume);      RefreshPct(GameVolLabel, GameVolume);      return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnGameVolNext()    { GameVolume    = StepVolume(GameVolume, +1);    BackroomsAudio::SetVolume(TEXT("GameVolume"), GameVolume);      RefreshPct(GameVolLabel, GameVolume);      return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnMonsterVolPrev() { MonsterVolume = StepVolume(MonsterVolume, -1); BackroomsAudio::SetVolume(TEXT("MonsterVolume"), MonsterVolume); RefreshPct(MonsterVolLabel, MonsterVolume); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnMonsterVolNext() { MonsterVolume = StepVolume(MonsterVolume, +1); BackroomsAudio::SetVolume(TEXT("MonsterVolume"), MonsterVolume); RefreshPct(MonsterVolLabel, MonsterVolume); return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnOtherVolPrev()   { OtherVolume   = StepVolume(OtherVolume, -1);   BackroomsAudio::SetVolume(TEXT("OtherVolume"), OtherVolume);     RefreshPct(OtherVolLabel, OtherVolume);   return FReply::Handled(); }
FReply UBackroomsMainMenuWidget::OnOtherVolNext()   { OtherVolume   = StepVolume(OtherVolume, +1);   BackroomsAudio::SetVolume(TEXT("OtherVolume"), OtherVolume);     RefreshPct(OtherVolLabel, OtherVolume);   return FReply::Handled(); }

void UBackroomsMainMenuWidget::ApplyMenuAudioVolume()
{
	if (MenuAudio)
	{
		// Итоговая громкость меню = общая * музыка/меню.
		MenuAudio->SetVolumeMultiplier(MasterVolume * MenuVolume);
	}
}

void UBackroomsMainMenuWidget::ApplySettings()
{
	if (UGameUserSettings* S = GEngine->GetGameUserSettings())
	{
		// Разрешение и режим экрана применяем ТОЛЬКО когда они реально
		// изменились. Раньше эти строки выполнялись на движение любого
		// ползунка — окно пересоздавалось, экран мигал/гас и игра могла
		// вылететь. Теперь смена окна происходит лишь при правке «Экран».
		static FIntPoint LastRes(-1, -1);
		static int32 LastMode = -1;
		static bool bLastVSync = false;
		static bool bLastScaleSet = false;
		static float LastScale = -1.0f;

		const FIntPoint WantRes = Resolutions.IsValidIndex(SelectedRes) ? Resolutions[SelectedRes].Size : FIntPoint(-1, -1);
		const EWindowMode::Type WantMode =
			(DisplayMode == 0) ? EWindowMode::Fullscreen :
			(DisplayMode == 1) ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed;

		bool bDisplayChanged = false;
		if (WantRes != LastRes)
		{
			S->SetScreenResolution(WantRes);
			LastRes = WantRes;
			bDisplayChanged = true;
		}
		if (S->GetFullscreenMode() != WantMode)
		{
			S->SetFullscreenMode(WantMode);
			LastMode = (int32)DisplayMode;
			bDisplayChanged = true;
		}
		(void)bDisplayChanged;
		if (bVSync != bLastVSync)
		{
			S->SetVSyncEnabled(bVSync);
			bLastVSync = bVSync;
		}
		if (!bLastScaleSet || !FMath::IsNearlyEqual(RenderScale, LastScale, 0.001f))
		{
			S->SetResolutionScaleNormalized(RenderScale);
			LastScale = RenderScale;
			bLastScaleSet = true;
		}

		// Каждую категорию качества применяем отдельно. Раньше сюда писался
		// только ShadowQuality через SetOverallScalabilityLevel, из-за чего
		// ползунки текстур, сглаживания, постобработки, эффектов и дальности
		// меняли надпись, но не саму настройку.
		S->SetShadowQuality(ShadowQuality);
		S->SetTextureQuality(TextureQuality);
		S->SetAntiAliasingQuality(AAQuality);
		S->SetPostProcessingQuality(PostProcess);
		S->SetVisualEffectQuality(EffectsQuality);
		S->SetViewDistanceQuality(ViewDistance);
		// Lumen-качество связываем со скалируемыми группами глобального
		// освещения и отражений: без этого ползунок лишь включал Lumen, а
		// число проб/бюджеты трассировки оставались на стартовом пресете,
		// поэтому на свету возникали шум и пиксели.
		{
			const int32 LumenLevel = LumenQuality == 0 ? 0 : (LumenQuality >= 3 ? 3 : 2);
			S->SetGlobalIlluminationQuality(LumenLevel);
			S->SetReflectionQuality(LumenLevel);
		}
		// false: не перетирать настройки значениями из командной строки
		// (иначе -ResX/-ResY и подобное откатывали выбор игрока).
		S->ApplySettings(false);

		if (APlayerController* PC = Cast<APlayerController>(GetOwningPlayer()))
		{
			if (ABackroomsPlayerCharacter* C = Cast<ABackroomsPlayerCharacter>(PC->GetPawn()))
			{
				if (UCameraComponent* Cam = C->FindComponentByClass<UCameraComponent>())
					Cam->SetFieldOfView(FOV);
				C->SetMouseSensitivity(Sensitivity);
			}
		}

		if (UWorld* W = GetWorld())
		{
			W->GetGameInstance()->Exec(W, *FString::Printf(TEXT("r.Lumen.DiffuseIndirect.Allow %d"), LumenQuality > 0 ? 1 : 0));
			W->GetGameInstance()->Exec(W, *FString::Printf(TEXT("r.Lumen.Reflections.Allow %d"), LumenQuality > 0 ? 1 : 0));
			W->GetGameInstance()->Exec(W, *FString::Printf(TEXT("r.Lumen.ScreenProbeGather.ScreenTraces %d"), LumenQuality > 0 ? 1 : 0));
			W->GetGameInstance()->Exec(W, *FString::Printf(TEXT("r.DepthOfFieldQuality %d"), DOFQuality == 0 ? 0 : (DOFQuality == 1 ? 2 : 4)));
			W->GetGameInstance()->Exec(W, *FString::Printf(TEXT("r.BloomQuality %d"), BloomQuality));
			W->GetGameInstance()->Exec(W, *FString::Printf(TEXT("r.Gamma %f"), Gamma));

			// DLSS Super Resolution включается парой r.NGX.DLSS.Enable +
			// r.TemporalAA.Upscaler, а качество выбирается через долю
			// разрешения r.ScreenPercentage (cvar r.NGX.DLSS.Quality не
			// существует — из-за него ползунок ничего не делал).
			if (DLSSMode <= 0)
			{
				W->GetGameInstance()->Exec(W, TEXT("r.NGX.DLSS.Enable 0"));
				W->GetGameInstance()->Exec(W, TEXT("r.TemporalAA.Upscaler 0"));
				W->GetGameInstance()->Exec(W, *FString::Printf(TEXT("r.ScreenPercentage %.1f"), RenderScale * 100.0f));
			}
			else
			{
				static const float DLSSPercent[] = { 100.0f, 66.6f, 58.0f, 50.0f, 33.3f }; // DLAA/Quality/Balanced/Performance/UltraPerf
				const float Pct = DLSSPercent[FMath::Clamp(DLSSMode, 0, 4)];
				W->GetGameInstance()->Exec(W, TEXT("r.NGX.DLSS.Enable 1"));
				W->GetGameInstance()->Exec(W, TEXT("r.TemporalAA.Upscaler 1"));
				W->GetGameInstance()->Exec(W, *FString::Printf(TEXT("r.ScreenPercentage %.1f"), Pct));
			}
		}
	}
}

APostProcessVolume* UBackroomsMainMenuWidget::FindOrCreatePostProcessVolume()
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

	// Стабильная автоэкспозиция: histogram-адаптация с узкой рампой и
	// плавной сменой яркости убирает резкие «вспышки»/миг на переходах
	// из светлой комнаты в тёмную (главный раздражитель в коридорах).
	FPostProcessSettings& PS = Volume->Settings;
	PS.bOverride_AutoExposureMethod = 1;
	PS.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
	PS.bOverride_AutoExposureBias = 1;
	PS.AutoExposureBias = 0.0f;
	PS.bOverride_AutoExposureMinBrightness = 1;
	PS.AutoExposureMinBrightness = 0.03f;
	PS.bOverride_AutoExposureMaxBrightness = 1;
	PS.AutoExposureMaxBrightness = 8.0f;
	PS.bOverride_AutoExposureSpeedUp = 1;
	PS.AutoExposureSpeedUp = 1.5f;
	PS.bOverride_AutoExposureSpeedDown = 1;
	PS.AutoExposureSpeedDown = 1.5f;

	CameraVolume = Volume;
	return Volume;
}

void UBackroomsMainMenuWidget::ApplyCameraEffect()
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

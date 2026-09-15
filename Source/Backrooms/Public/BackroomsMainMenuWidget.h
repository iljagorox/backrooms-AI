#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "BackroomsMainMenuWidget.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;
class UFont;
class UMediaPlayer;
class UMediaTexture;
class UMediaSource;
class UMediaSoundComponent;
class AActor;
class SBorder;
struct FSlateDynamicImageBrush;
struct FSlateBrush;
class APostProcessVolume;

UCLASS()
class BACKROOMS_API UBackroomsMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void ShowMenu(bool bShow);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	UMaterialInterface* CorridorMaterial = nullptr;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	// --- Navigation ---
	FReply OnPlayClicked();
	FReply OnContinueClicked();
	FReply OnSettingsClicked();
	FReply OnControlsClicked();
	FReply OnQuitClicked();
	FReply OnBackFromSettings();
	FReply OnBackFromControls();

	// --- Меню выбора уровней ---
	FReply OnLevelsClicked();
	FReply OnBackFromLevels();
	FReply OnLevelClicked(int32 LevelIndex);
	void   RebuildLevelSelect();

	// --- Достижения ---
	FReply OnAchievementsClicked();
	FReply OnBackFromAchievements();
	void   RebuildAchievements();
	void   RebuildUI();
	void   ApplySettings();
	bool   HasSavedGame() const;

	// --- Display ---
	FReply OnResPrev();
	FReply OnResNext();
	FReply OnDisplayModeNext();

	// --- Graphics ---
	FReply OnVSyncToggled();
	FReply OnShadowQualityPrev();
	FReply OnShadowQualityNext();
	FReply OnAAQualityPrev();
	FReply OnAAQualityNext();
	FReply OnPostProcessPrev();
	FReply OnPostProcessNext();
	FReply OnEffectsQualityPrev();
	FReply OnEffectsQualityNext();
	FReply OnViewDistancePrev();
	FReply OnViewDistanceNext();
	FReply OnTexQualityPrev();
	FReply OnTexQualityNext();
	FReply OnRenderScalePrev();
	FReply OnRenderScaleNext();
	FReply OnGammaPrev();
	FReply OnGammaNext();
	FReply OnFOVPrev();
	FReply OnFOVNext();

	// --- NEW: Sensitivity / Blur / Bloom / Lumen / DLSS ---
	FReply OnSensitivityPrev();
	FReply OnSensitivityNext();
	FReply OnDOFPrev();
	FReply OnDOFNext();
	FReply OnBloomPrev();
	FReply OnBloomNext();
	FReply OnLumenQualityPrev();
	FReply OnLumenQualityNext();
	FReply OnDLSSModePrev();
	FReply OnDLSSModeNext();

	// --- Вкладки настроек ---
	FReply OnSettingsTabClicked(int32 Tab);
	FReply OnKeyBindClicked(const FString& BindId);
	FReply OnResetBindingsClicked();
	FReply OnBindCancel();
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// --- Звук ---
	FReply OnMasterVolPrev();
	FReply OnMasterVolNext();
	FReply OnMenuVolPrev();
	FReply OnMenuVolNext();
	FReply OnGameVolPrev();
	FReply OnGameVolNext();
	FReply OnMonsterVolPrev();
	FReply OnMonsterVolNext();
	FReply OnOtherVolPrev();
	FReply OnOtherVolNext();
	void ApplyMenuAudioVolume();

	void TickCorridor(float DeltaTime);
	void StartBackgroundVideo(bool bRareVideo);

	// Включить/выключить фоновое видео и его звук (музыку меню). При старте игры
	// обязательно выключаем: иначе аудиодорожка видео продолжает играть в игре.
	void SetMenuMediaActive(bool bActive);

	// Должен быть UFUNCTION: на него цепляется динамический делегат
	// BackgroundPlayer->OnEndReached.AddDynamic(...). Без UFUNCTION()
	// __Internal_BindDynamic падает на ensure и роняет игру (чёрный экран).
	UFUNCTION()
	void OnBackgroundVideoEnded();

	// --- Slate pointers ---
	TSharedPtr<SOverlay>   MenuRoot;
	TSharedPtr<STextBlock> ResLabel;
	TSharedPtr<STextBlock> DisplayModeLabel;
	TSharedPtr<STextBlock> VSyncLabel;
	TSharedPtr<STextBlock> ShadowLabel;
	TSharedPtr<STextBlock> AALabel;
	TSharedPtr<STextBlock> PostProcessLabel;
	TSharedPtr<STextBlock> EffectsLabel;
	TSharedPtr<STextBlock> ViewDistLabel;
	TSharedPtr<STextBlock> TexLabel;
	TSharedPtr<STextBlock> RenderScaleLabel;
	TSharedPtr<STextBlock> GammaLabel;
	TSharedPtr<STextBlock> FOVLabel;
	// NEW
	TSharedPtr<STextBlock> SensitivityLabel;
	TSharedPtr<STextBlock> DOFLabel;
	TSharedPtr<STextBlock> BloomLabel;
	TSharedPtr<STextBlock> LumenLabel;
	TSharedPtr<STextBlock> DLSSLabel;
	TSharedPtr<STextBlock> RecordingEffectLabel;

	// --- Звук ---
	TSharedPtr<STextBlock> MasterVolLabel;
	TSharedPtr<STextBlock> MenuVolLabel;
	TSharedPtr<STextBlock> GameVolLabel;
	TSharedPtr<STextBlock> MonsterVolLabel;
	TSharedPtr<STextBlock> OtherVolLabel;

	int32  SettingsTab = 0;      // 0=Экран 1=Качество 2=Камера 3=Звук 4=Игра 5=Управление
	int32  Difficulty = 2;       // индекс EBackroomsDifficulty (текущая сложность)
	TSharedPtr<STextBlock> DifficultyLabel;
	uint8  Language = 0;         // EBackroomsLanguage (0=RU, 1=EN)
	TSharedPtr<STextBlock> LanguageLabel;

	// --- Achievements state ---
	bool bShowingAchievements = false;
	bool   bWaitingForKey = false;
	FString PendingBindId;
	float  MasterVolume = 1.0f;
	float  MenuVolume = 1.0f;
	float  GameVolume = 1.0f;
	float  MonsterVolume = 1.0f;
	float  OtherVolume = 1.0f;

	UPROPERTY(Transient)
	TObjectPtr<UMediaSoundComponent> MenuAudio = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> MenuAudioActor = nullptr;

	// --- Corridor ---
	UMaterialInstanceDynamic* DynamicMat = nullptr;
	UTextureRenderTarget2D*   CorridorRT = nullptr;
	FSlateBrush*              Brush = nullptr;
	float                     UVOffset = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> BackgroundPlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMediaSource> BackgroundSource = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> BackgroundTexture = nullptr;
	TSharedPtr<SBorder> TransitionOverlay;

	bool bPlayingRareVideo = false;
	// Сколько секунд меню открыто: чем дольше игрок смотрит в меню, тем выше
	// шанс скример-ролика (насыщается, чтобы не стать гарантией).
	float MenuIdleTime = 0.0f;
	float TransitionAlpha = 0.0f;

	// --- Font ---
	UPROPERTY()
	TObjectPtr<UFont> MenuFont = nullptr;

	// --- Settings state ---
	bool bShowingSettings = false;
	bool bShowingControls = false;

	// --- Level select state ---
	bool bShowingLevelSelect = false;
	// Кисти-превью: SImage хранит сырой указатель, поэтому держим shared-ptr
	// живым. Индекс — номер уровня; невалидный значит «нет скриншота».
	TMap<int32, TSharedPtr<FSlateDynamicImageBrush>> LevelThumbBrushes;

	struct FResInfo { FString Label; FIntPoint Size; };
	TArray<FResInfo> Resolutions;
	int32  SelectedRes = 0;
	int32  DisplayMode = 1;
	bool   bVSync = true;
	int32  ShadowQuality = 2;
	int32  AAQuality = 2;
	int32  PostProcess = 2;
	int32  EffectsQuality = 2;
	int32  ViewDistance = 2;
	int32  TextureQuality = 2;
	float  RenderScale = 1.0f;
	float  Gamma = 1.0f;
	float  FOV = 90.0f;

	// NEW
	float  Sensitivity = 1.0f;   // 0.2 — 5.0 (шаг 0.1)
	int32  DOFQuality = 1;       // 0=Off 1=Medium 2=Cinematic
	int32  BloomQuality = 2;     // 0-3
	int32  LumenQuality = 2;     // 0=Off 1=Low 2=High 3=Epic
	int32  DLSSMode = 2;         // 0=Off 1=Quality 2=Balanced 3=Performance 4=UltraPerf
	int32  RecordingEffect = 0;  // 0=Выкл 1=Лёгкая плёнка 2=Средняя 3=Сильная

	TWeakObjectPtr<APostProcessVolume> CameraVolume;

	void ApplyCameraEffect();
	APostProcessVolume* FindOrCreatePostProcessVolume();

	static FString ScalabilityName(int32 Level);
	static FString DisplayModeName(int32 Mode);
	static FString DOFName(int32 Level);
	static FString LumenName(int32 Level);
	static FString DLSSModeName(int32 Mode);
};

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "BackroomsPauseMenuWidget.generated.h"

class SSlider;
class STextBlock;
class SScrollBox;
class SOverlay;
class APostProcessVolume;

UCLASS()
class BACKROOMS_API UBackroomsPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void ToggleMenu();
	void ShowMenu(bool bShow);
	bool IsMenuOpen() const { return bIsOpen; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void RebuildUI();
	FReply OnSettingsTabClicked(int32 Tab);
	FReply OnKeyBindClicked(const FString& BindId);
	FReply OnResetBindingsClicked();
	FReply OnBindCancel();
	FReply OnResumeClicked();
	FReply OnRestartClicked();
	FReply OnQuitClicked();

	// Графика
	void OnOverallQualityChanged(float Value);
	void OnViewDistanceChanged(float Value);
	void OnShadowQualityChanged(float Value);
	void OnTexturesQualityChanged(float Value);
	void OnAntiAliasChanged(float Value);
	void OnPostProcessChanged(float Value);
	void OnEffectsChanged(float Value);
	void OnDofChanged(float Value);
	void OnRecordingEffectChanged(float Value);
	void OnFovChanged(float Value);
	void OnSensitivityChanged(float Value);
	void OnVSyncChanged(float Value);
	void OnGammaChanged(float Value);
	FReply OnPotatoModeClicked();

	// Звук
	void OnVolumeChanged(const FString& Key, float Value, TSharedPtr<STextBlock> OutLabel);

	void ApplyAndSaveSettings();
	void ApplyCameraEffect();
	APostProcessVolume* FindOrCreatePostProcessVolume();

	bool bIsOpen = false;
	int32 SettingsTab = 0;
	bool bWaitingForKey = false;
	FString PendingBindId;
	TSharedPtr<SOverlay> MenuRoot;

	// Графика
	int32 OverallQ = 2;
	int32 ViewDist = 2;
	int32 ShadowQ = 2;
	int32 TexturesQ = 2;
	int32 AAQuality = 2;
	int32 PostProcess = 2;
	int32 EffectsQuality = 2;
	int32 DofQuality = 2;
	int32 RecordingEffect = 0;
	bool bVSync = false;
	float Gamma = 1.0f;
	float FOV = 90.0f;
	float Sensitivity = 1.0f;

	// Звук
	float MasterVolume = 1.0f;
	float MenuVolume = 1.0f;
	float GameVolume = 1.0f;
	float MonsterVolume = 1.0f;
	float OtherVolume = 1.0f;

	TWeakObjectPtr<APostProcessVolume> CameraVolume;

	TSharedPtr<STextBlock> OverallQualityText;
	TSharedPtr<STextBlock> ViewDistanceText;
	TSharedPtr<STextBlock> ShadowQualityText;
	TSharedPtr<STextBlock> TexturesQualityText;
	TSharedPtr<STextBlock> AAText;
	TSharedPtr<STextBlock> PostProcessText;
	TSharedPtr<STextBlock> EffectsText;
	TSharedPtr<STextBlock> DofText;
	TSharedPtr<STextBlock> RecordingEffectText;
	TSharedPtr<STextBlock> FovText;
	TSharedPtr<STextBlock> SensitivityText;
	TSharedPtr<STextBlock> VSyncText;
	TSharedPtr<STextBlock> GammaText;

	TSharedPtr<STextBlock> MasterVolText;
	TSharedPtr<STextBlock> MenuVolText;
	TSharedPtr<STextBlock> GameVolText;
	TSharedPtr<STextBlock> MonsterVolText;
	TSharedPtr<STextBlock> OtherVolText;
};
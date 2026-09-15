#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Sound/SoundBase.h"
#include "BackroomsLevelTheme.generated.h"

// Тема HUD/меню/аудио уровня. Один класс HUD на все уровни — читает эти поля.
USTRUCT(BlueprintType)
struct BACKROOMS_API FBackroomsLevelTheme
{
	GENERATED_BODY()

	// ---- HUD: акценты ----
	UPROPERTY(EditAnywhere, Category = "HUD")
	FLinearColor AccentPrimary = FLinearColor(0.95f, 0.82f, 0.38f, 1.0f); // use-bar, выбранный слот, подсказки
	UPROPERTY(EditAnywhere, Category = "HUD")
	FLinearColor AccentSecondary = FLinearColor(0.45f, 0.72f, 0.35f, 1.0f); // тост, XP, FPS

	UPROPERTY(EditAnywhere, Category = "HUD")
	FLinearColor PanelFill = FLinearColor(0.02f, 0.02f, 0.03f, 0.55f);
	UPROPERTY(EditAnywhere, Category = "HUD")
	FLinearColor PanelBorder = FLinearColor(0.36f, 0.33f, 0.25f, 0.35f);
	UPROPERTY(EditAnywhere, Category = "HUD")
	FLinearColor GCellOff = FLinearColor(0.16f, 0.16f, 0.19f, 0.55f);

	// ---- HUD: базовые On-цвета шкал ----
	UPROPERTY(EditAnywhere, Category = "HUD")
	FLinearColor MeterSanity = FLinearColor(0.45f, 0.72f, 0.35f, 0.95f);
	UPROPERTY(EditAnywhere, Category = "HUD")
	FLinearColor MeterFlashlight = FLinearColor(0.95f, 0.78f, 0.30f, 0.95f);
	UPROPERTY(EditAnywhere, Category = "HUD")
	FLinearColor MeterHunger = FLinearColor(0.85f, 0.62f, 0.30f, 0.95f);
	UPROPERTY(EditAnywhere, Category = "HUD")
	FLinearColor MeterThirst = FLinearColor(0.40f, 0.68f, 0.88f, 0.95f);
	UPROPERTY(EditAnywhere, Category = "HUD")
	FLinearColor MeterStamina = FLinearColor(0.28f, 0.80f, 0.88f, 0.95f);
	UPROPERTY(EditAnywhere, Category = "HUD")
	FLinearColor MeterHealth = FLinearColor(0.78f, 0.16f, 0.14f, 0.95f);

	// ---- HUD: видимость панелей ----
	UPROPERTY(EditAnywhere, Category = "HUD")
	bool bShowHealthPanel = true;
	UPROPERTY(EditAnywhere, Category = "HUD")
	bool bShowSurvivalBars = true;
	UPROPERTY(EditAnywhere, Category = "HUD")
	bool bShowStaminaBar = true;
	UPROPERTY(EditAnywhere, Category = "HUD")
	bool bShowHotbar = true;
	UPROPERTY(EditAnywhere, Category = "HUD")
	bool bShowFps = true;
	UPROPERTY(EditAnywhere, Category = "HUD")
	bool bShowStatusIcons = true;

	// ---- Аудио ----
	UPROPERTY(EditAnywhere, Category = "Audio")
	FString AmbientLayer = TEXT("lamp_hum");
	UPROPERTY(EditAnywhere, Category = "Audio")
	FString ReverbScene = TEXT("office");
	UPROPERTY(EditAnywhere, Category = "Audio")
	bool bEcho = false;
	UPROPERTY(EditAnywhere, Category = "Audio")
	TSoftObjectPtr<USoundBase> MusicTrack;   // пусто = тишина (ассетов ещё нет)
	UPROPERTY(EditAnywhere, Category = "Audio")
	TSoftObjectPtr<USoundBase> AmbientLoop;  // пусто = тишина
	UPROPERTY(EditAnywhere, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 0.7f;

	// Акцент темы PAUSE/GameOver меню (отдельный вектор, чтобы не менять HUD-палитру).
	UPROPERTY(EditAnywhere, Category = "Menu")
	FLinearColor MenuAccent = FLinearColor(0.93f, 0.80f, 0.38f, 1.0f);
};
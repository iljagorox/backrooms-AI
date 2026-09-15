#pragma once

#include "CoreMinimal.h"
#include "BackroomsLevelTheme.h"
#include "Engine/DataAsset.h"
#include "BackroomsLevelThemeConfig.generated.h"

// DataAsset конфигурации уровня: тема HUD/меню/аудио + имя карты.
// По ассету на уровень: /Game/Data/L{L}_Config. Если ассета нет — кодовая
// тема-фолбэк ULevelGeneratorProfile::BuildDefaultByLevel (см. GetActiveTheme).
UCLASS(BlueprintType)
class BACKROOMS_API UBackroomsLevelThemeConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	int32 LevelIndex = 0;

	// Имя карты уровня (путь пакета), куда переходит этот уровень.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	FString MapName = TEXT("/Game/Runtime/Lvl_L0");

	// Архитектура уровня строится ТОЛЬКО из FloorPlan (новый путь). При
	// включённом флаге чанк не имеет права молча падать в legacy-генерацию:
	// отсутствие плана — ошибка, генерация чанка прерывается.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	bool bUseFloorPlan = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme")
	FBackroomsLevelTheme Theme;
};
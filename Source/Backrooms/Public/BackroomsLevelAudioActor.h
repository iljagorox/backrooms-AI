#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BackroomsLevelTheme.h"
#include "BackroomsLevelAudioActor.generated.h"

class UAudioComponent;

// Фоновое аудио уровня: петля амбиента + музыкальная тема. Источник — тема
// уровня (FBackroomsLevelTheme): мягкие ссылки; ассетов пока нет — ApplyTheme
// молча даёт тишину. Строки слоя/реверба хранятся для будущих аудио-ассетов.
UCLASS()
class BACKROOMS_API ABackroomsLevelAudioActor : public AActor
{
	GENERATED_BODY()

public:
	ABackroomsLevelAudioActor();

	// Применить тему уровня: музыка и петля амбиента из мягких ссылок темы.
	void ApplyTheme(const FBackroomsLevelTheme& Theme);

	// Остановить все звуки (уход в меню / конец уровня).
	void StopAll();

	// Активный слой амбиента/реверба (строки темы, доступны для аудио-ассетов).
	FString GetActiveAmbientLayer() const { return ActiveTheme.AmbientLayer; }
	FString GetActiveReverbScene() const { return ActiveTheme.ReverbScene; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Audio")
	TObjectPtr<UAudioComponent> MusicComponent;

	UPROPERTY(VisibleAnywhere, Category = "Audio")
	TObjectPtr<UAudioComponent> AmbientComponent;

	FBackroomsLevelTheme ActiveTheme;
};
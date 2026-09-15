#include "BackroomsLevelAudioActor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

ABackroomsLevelAudioActor::ABackroomsLevelAudioActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Music"));
	MusicComponent->bAutoActivate = false;
	SetRootComponent(MusicComponent);

	AmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Ambient"));
	AmbientComponent->bAutoActivate = false;
	AmbientComponent->SetupAttachment(MusicComponent);
}

void ABackroomsLevelAudioActor::ApplyTheme(const FBackroomsLevelTheme& Theme)
{
	ActiveTheme = Theme;

	// Музыка: мягкая ссылка темы; ассета нет (пустая ссылка) — тишина.
	USoundBase* Music = ActiveTheme.MusicTrack.LoadSynchronous();
	if (Music)
	{
		MusicComponent->SetSound(Music);
		MusicComponent->SetVolumeMultiplier(FMath::Clamp(ActiveTheme.MusicVolume, 0.0f, 1.0f));
		if (!MusicComponent->IsPlaying())
		{
			MusicComponent->Play();
		}
	}
	else
	{
		MusicComponent->Stop();
	}

	// Петля амбиента: ассета нет — тишина.
	USoundBase* Loop = ActiveTheme.AmbientLoop.LoadSynchronous();
	if (Loop)
	{
		AmbientComponent->SetSound(Loop);
		AmbientComponent->SetPitchMultiplier(1.0f);
		if (!AmbientComponent->IsPlaying())
		{
			AmbientComponent->Play();
		}
	}
	else
	{
		AmbientComponent->Stop();
	}

	UE_LOG(LogTemp, Display, TEXT("BR Audio: level theme applied (ambient=%s reverb=%s music=%s loop=%s)"),
		*ActiveTheme.AmbientLayer, *ActiveTheme.ReverbScene,
		Music ? *Music->GetName() : TEXT("silence"),
		Loop ? *Loop->GetName() : TEXT("silence"));
}

void ABackroomsLevelAudioActor::StopAll()
{
	MusicComponent->Stop();
	AmbientComponent->Stop();
}
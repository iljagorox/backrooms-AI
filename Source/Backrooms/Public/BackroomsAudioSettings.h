#pragma once

#include "CoreMinimal.h"
#include "Misc/ConfigCacheIni.h"

// Громкости звука, сохраняются в GameUserSettings.ini. Разделы: меню/музыка,
// игра, монстры, прочее. Значения 0..1. Читаются одним местом, чтобы UI и
// игровые системы (музыка, шаги, монстры) брали одно и то же.
namespace BackroomsAudio
{
	inline const TCHAR* Section()
	{
		return TEXT("/Script/Backrooms.BackroomsAudio");
	}

	inline float GetVolume(const TCHAR* Key, float Default = 1.0f)
	{
		float V = Default;
		if (GConfig)
		{
			GConfig->GetFloat(Section(), Key, V, GGameUserSettingsIni);
		}
		return FMath::Clamp(V, 0.0f, 1.0f);
	}

	inline void SetVolume(const TCHAR* Key, float Value)
	{
		if (GConfig)
		{
			GConfig->SetFloat(Section(), Key, FMath::Clamp(Value, 0.0f, 1.0f), GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		}
	}
}

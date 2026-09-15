#pragma once

#include "CoreMinimal.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

// «Картофельный режим»: пресет максимального FPS в ущерб графике. Применяет
// набор r.* CVar'ов (низкий ScreenPercentage/резкость текстур, Lumen выключен,
// тени/AA/Bloom на минимуме), как качественные слайдеры он не затрагивается.
// Флаг переживает перезапуск игры (GameUserSettings.ini) и снова применяется
// в ABackroomsPlayerCharacter::ApplyRendererDefaults.
namespace BackroomsQuality
{
	inline const TCHAR* Section()
	{
		return TEXT("/Script/Backrooms.BackroomsQuality");
	}

	inline bool IsPotatoMode()
	{
		bool b = false;
		if (GConfig)
		{
			GConfig->GetBool(Section(), TEXT("bPotatoMode"), b, GGameUserSettingsIni);
		}
		return b;
	}

	inline void ApplyPotato(bool bOn, UWorld* World)
	{
		if (!World || !GEngine)
		{
			return;
		}

		const static TArray<TPair<const TCHAR*, const TCHAR*>> Potato =
		{
			{ TEXT("r.ScreenPercentage"),            TEXT("15") },
			{ TEXT("r.MipMapLODBias"),               TEXT("10") },
			{ TEXT("r.Lumen.GlobalIllumination.Method"), TEXT("0") },
			{ TEXT("r.ShadowQuality"),               TEXT("0") },
			{ TEXT("r.ContactShadows"),              TEXT("0") },
			{ TEXT("r.Nanite.MaxPixelsPerEdge"),     TEXT("64") },
			{ TEXT("r.PostProcessAAQuality"),        TEXT("0") },
			{ TEXT("r.BloomQuality"),                TEXT("0") },
		};
		const static TArray<TPair<const TCHAR*, const TCHAR*>> Restore =
		{
			{ TEXT("r.ScreenPercentage"),            TEXT("100") },
			{ TEXT("r.MipMapLODBias"),               TEXT("0") },
			{ TEXT("r.Lumen.GlobalIllumination.Method"), TEXT("1") },
			{ TEXT("r.ShadowQuality"),               TEXT("5") },
			{ TEXT("r.ContactShadows"),              TEXT("1") },
			{ TEXT("r.Nanite.MaxPixelsPerEdge"),     TEXT("1") },
			{ TEXT("r.PostProcessAAQuality"),        TEXT("4") },
			{ TEXT("r.BloomQuality"),                TEXT("5") },
		};

		for (const auto& Pair : (bOn ? Potato : Restore))
		{
			GEngine->Exec(World, *FString::Printf(TEXT("%s %s"), Pair.Key, Pair.Value));
		}
	}

	inline void SetPotatoMode(bool bOn, UWorld* World)
	{
		if (GConfig)
		{
			GConfig->SetBool(Section(), TEXT("bPotatoMode"), bOn, GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		}
		ApplyPotato(bOn, World);
	}
}
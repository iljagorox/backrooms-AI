#pragma once

#include "CoreMinimal.h"
#include "Misc/ConfigCacheIni.h"

// Переназначение клавиш. Привязки сохраняются в GameUserSettings.ini (раздел
// BackroomsInput), чтобы не трогать DefaultInput.ini. ApplyBindings() правит
// глобальный UInputSettings, поэтому legacy BindAxis/BindAction (по именам из
// DefaultInput.ini) подхватывают новые клавиши мгновенно, без пересоздания
// InputComponent.
namespace BackroomsInput
{
	inline const TCHAR* Section()
	{
		return TEXT("/Script/Backrooms.BackroomsInput");
	}

	// Кастомная клавиша для BindId ("" если не переназначалась).
	FString GetCustomKey(const TCHAR* BindId);

	// Переназначаемая клавиша, либо дефолт.
	FString GetEffectiveKey(const TCHAR* BindId, const TCHAR* DefaultKey);

	// Сохранить переназначение.
	void SetCustomKey(const TCHAR* BindId, const TCHAR* KeyName);

	// Сбросить все переназначения к дефолтам.
	void ClearAllKeys();

	// Применить текущие привязки к UInputSettings (вступает в силу сразу).
	void ApplyBindings();

	// Человекочитаемое имя клавиши для отображения в меню.
	FString KeyDisplayName(const FString& KeyName);
}
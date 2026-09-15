#include "BackroomsInputSettings.h"
#include "BackroomsLocalization.h"
#include "GameFramework/InputSettings.h"
#include "InputCoreTypes.h"

namespace BackroomsInput
{
	namespace
	{
		// Все настраиваемые BindId — для полного сброса из конфига.
		const TArray<FString>& AllBindIds()
		{
			static const TArray<FString> Ids = {
				TEXT("MoveForwardPlus"), TEXT("MoveForwardMinus"),
				TEXT("MoveRightPlus"),  TEXT("MoveRightMinus"),
				TEXT("Jump"),      TEXT("Sprint"),   TEXT("Flashlight"),
				TEXT("Crouch"),    TEXT("View"),     TEXT("Attack"),
				TEXT("Grab"),      TEXT("Push"),     TEXT("Throw"),
				TEXT("Use"),       TEXT("Inspect"),  TEXT("Slot1"),
				TEXT("Slot2"),     TEXT("Slot3"),    TEXT("Slot4"),
				TEXT("PauseMenu"),
			};
			return Ids;
		}
	}

	FString GetCustomKey(const TCHAR* BindId)
	{
		if (!GConfig)
		{
			return TEXT("");
		}
		FString Value;
		GConfig->GetString(Section(), BindId, Value, GGameUserSettingsIni);
		return Value;
	}

	FString GetEffectiveKey(const TCHAR* BindId, const TCHAR* DefaultKey)
	{
		const FString Custom = GetCustomKey(BindId);
		return Custom.IsEmpty() ? FString(DefaultKey) : Custom;
	}

	void SetCustomKey(const TCHAR* BindId, const TCHAR* KeyName)
	{
		if (!GConfig)
		{
			return;
		}
		GConfig->SetString(Section(), BindId, KeyName, GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}

	void ClearAllKeys()
	{
		if (!GConfig)
		{
			return;
		}
		for (const FString& Id : AllBindIds())
		{
			GConfig->RemoveKey(Section(), *Id, GGameUserSettingsIni);
		}
		GConfig->Flush(false, GGameUserSettingsIni);
	}

	void ApplyBindings()
	{
		UInputSettings* Settings = UInputSettings::GetInputSettings();
		if (!Settings)
		{
			return;
		}

		// Действия: одна клавиша на действие. Стираем заводские привязки и
		// ставим одну — выбранную игроком либо дефолтную.
		struct FActionBind { const TCHAR* BindId; FName ActionName; FName DefaultKey; };
		static const FActionBind Actions[] = {
			{ TEXT("Jump"),       TEXT("Jump"),       TEXT("SpaceBar") },
			{ TEXT("Sprint"),     TEXT("Sprint"),     TEXT("LeftShift") },
			{ TEXT("Crouch"),     TEXT("Crouch"),     TEXT("LeftControl") },
			{ TEXT("Flashlight"), TEXT("Flashlight"), TEXT("F") },
			{ TEXT("View"),       TEXT("View"),       TEXT("V") },
			{ TEXT("Attack"),     TEXT("Attack"),     TEXT("Q") },
			{ TEXT("Grab"),       TEXT("Grab"),       TEXT("E") },
			{ TEXT("Push"),       TEXT("Push"),       TEXT("LeftMouseButton") },
			{ TEXT("Throw"),      TEXT("Throw"),      TEXT("RightMouseButton") },
			{ TEXT("Use"),        TEXT("Use"),        TEXT("R") },
			{ TEXT("Inspect"),    TEXT("Inspect"),    TEXT("LeftAlt") },
			{ TEXT("Slot1"),      TEXT("Slot1"),      TEXT("One") },
			{ TEXT("Slot2"),      TEXT("Slot2"),      TEXT("Two") },
			{ TEXT("Slot3"),      TEXT("Slot3"),      TEXT("Three") },
			{ TEXT("Slot4"),      TEXT("Slot4"),      TEXT("Four") },
			{ TEXT("PauseMenu"),  TEXT("PauseMenu"),  TEXT("Escape") },
		};

		for (const FActionBind& B : Actions)
		{
			TArray<FInputActionKeyMapping> Old;
			Settings->GetActionMappingByName(B.ActionName, Old);
			for (const FInputActionKeyMapping& M : Old)
			{
				Settings->RemoveActionMapping(M);
			}

			const FKey EffectiveKey(*GetEffectiveKey(B.BindId, *B.DefaultKey.ToString()));
			Settings->AddActionMapping(FInputActionKeyMapping(B.ActionName, EffectiveKey));
		}

		// Оси движения: две клавиши на ось (вперёд/назад, вправо/влево).
		struct FAxisBind { FName AxisName; const TCHAR* PlusId; const TCHAR* MinusId; FName PlusKey; FName MinusKey; float PlusScale; float MinusScale; };
		static const FAxisBind Axes[] = {
			{ TEXT("MoveForward"), TEXT("MoveForwardPlus"), TEXT("MoveForwardMinus"), TEXT("W"), TEXT("S"), 1.0f, -1.0f },
			{ TEXT("MoveRight"),   TEXT("MoveRightPlus"),   TEXT("MoveRightMinus"),   TEXT("D"), TEXT("A"), 1.0f, -1.0f },
		};

		for (const FAxisBind& B : Axes)
		{
			TArray<FInputAxisKeyMapping> Old;
			Settings->GetAxisMappingByName(B.AxisName, Old);
			for (const FInputAxisKeyMapping& M : Old)
			{
				Settings->RemoveAxisMapping(M);
			}

			const FKey PlusKey(*GetEffectiveKey(B.PlusId, *B.PlusKey.ToString()));
			const FKey MinusKey(*GetEffectiveKey(B.MinusId, *B.MinusKey.ToString()));
			Settings->AddAxisMapping(FInputAxisKeyMapping(B.AxisName, PlusKey, B.PlusScale));
			Settings->AddAxisMapping(FInputAxisKeyMapping(B.AxisName, MinusKey, B.MinusScale));
		}
	}

	FString KeyDisplayName(const FString& KeyName)
	{
		// Локализованные имена клавиш (ключи Keys.*). Стрелки и буквы/цифры
		// переводить не нужно — рисуются как есть.
		static const TMap<FString, FString> LocKeys = {
			{ TEXT("SpaceBar"),          TEXT("Keys.SpaceBar") },
			{ TEXT("LeftShift"),         TEXT("Keys.LeftShift") },
			{ TEXT("RightShift"),        TEXT("Keys.RightShift") },
			{ TEXT("LeftCtrl"),          TEXT("Keys.LeftCtrl") },
			{ TEXT("RightCtrl"),         TEXT("Keys.RightCtrl") },
			{ TEXT("LeftAlt"),           TEXT("Keys.LeftAlt") },
			{ TEXT("RightAlt"),          TEXT("Keys.RightAlt") },
			{ TEXT("Escape"),            TEXT("Keys.Escape") },
			{ TEXT("LeftMouseButton"),   TEXT("Keys.LeftMouse") },
			{ TEXT("RightMouseButton"),  TEXT("Keys.RightMouse") },
			{ TEXT("MiddleMouseButton"), TEXT("Keys.MiddleMouse") },
			{ TEXT("ThumbMouseButton"),  TEXT("Keys.ThumbMouse1") },
			{ TEXT("ThumbMouseButton2"), TEXT("Keys.ThumbMouse2") },
			{ TEXT("MouseX"),            TEXT("Keys.MouseX") },
			{ TEXT("MouseY"),            TEXT("Keys.MouseY") },
			{ TEXT("MouseWheelAxis"),    TEXT("Keys.MouseWheel") },
			{ TEXT("BackSpace"),         TEXT("Keys.Backspace") },
			{ TEXT("Tab"),               TEXT("Keys.Tab") },
			{ TEXT("Enter"),             TEXT("Keys.Enter") },
		};
		if (const FString* LocKey = LocKeys.Find(KeyName))
		{
			const FString LocName = BackroomsLoc::Get(**LocKey);
			if (LocName != *LocKey)
			{
				return LocName;
			}
		}
		if (KeyName == TEXT("Up"))    return TEXT("\x2191");
		if (KeyName == TEXT("Down"))  return TEXT("\x2193");
		if (KeyName == TEXT("Left"))  return TEXT("\x2190");
		if (KeyName == TEXT("Right")) return TEXT("\x2192");
		return KeyName;
	}
}
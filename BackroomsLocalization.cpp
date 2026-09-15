#include "BackroomsLocalization.h"
#include "BackroomsLocRegistry.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
	const TCHAR* GSection = TEXT("/Script/Backrooms.BackroomsLocalization");
	const TCHAR* GKey = TEXT("Language");
	bool GRegistered = false;

	int32 LangIndex(EBackroomsLanguage Lang)
	{
		const int32 I = (int32)Lang;
		return (I < 0 || I >= (int32)EBackroomsLanguage::Count) ? 0 : I;
	}
}

namespace BackroomsLoc
{
	FLanguageTable& Table(EBackroomsLanguage Lang)
	{
		static TArray<FLanguageTable> Tables;
		if (Tables.Num() == 0)
		{
			Tables.SetNum((int32)EBackroomsLanguage::Count);
		}
		return Tables[LangIndex(Lang)];
	}

	void EnsureRegistered()
	{
		if (GRegistered)
		{
			return;
		}
		GRegistered = true;
		// Порядок не важен: языки независимы.
		Register_RU(Table(EBackroomsLanguage::Russian));
		Register_EN(Table(EBackroomsLanguage::English));
		Register_ES(Table(EBackroomsLanguage::Spanish));
		Register_FR(Table(EBackroomsLanguage::French));
		Register_DE(Table(EBackroomsLanguage::German));
		Register_IT(Table(EBackroomsLanguage::Italian));
		Register_PT(Table(EBackroomsLanguage::Portuguese));
		Register_PL(Table(EBackroomsLanguage::Polish));
		Register_TR(Table(EBackroomsLanguage::Turkish));
		Register_ZH(Table(EBackroomsLanguage::Chinese));
		Register_JA(Table(EBackroomsLanguage::Japanese));
		Register_KO(Table(EBackroomsLanguage::Korean));
		Register_AR(Table(EBackroomsLanguage::Arabic));
	}
}

EBackroomsLanguage BackroomsLoc::GetLanguage()
{
	int32 V = (int32)EBackroomsLanguage::Russian;
	if (GConfig)
	{
		GConfig->GetInt(GSection, GKey, V, GGameUserSettingsIni);
	}
	if (V < 0 || V >= (int32)EBackroomsLanguage::Count)
	{
		V = (int32)EBackroomsLanguage::Russian;
	}
	return (EBackroomsLanguage)V;
}

void BackroomsLoc::SetLanguage(EBackroomsLanguage Lang)
{
	if (GConfig)
	{
		GConfig->SetInt(GSection, GKey, (int32)Lang, GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}
}

FString BackroomsLoc::LanguageCode()
{
	switch (GetLanguage())
	{
	case EBackroomsLanguage::English:    return TEXT("en");
	case EBackroomsLanguage::Spanish:    return TEXT("es");
	case EBackroomsLanguage::French:     return TEXT("fr");
	case EBackroomsLanguage::German:     return TEXT("de");
	case EBackroomsLanguage::Italian:    return TEXT("it");
	case EBackroomsLanguage::Portuguese: return TEXT("pt");
	case EBackroomsLanguage::Polish:     return TEXT("pl");
	case EBackroomsLanguage::Turkish:    return TEXT("tr");
	case EBackroomsLanguage::Chinese:    return TEXT("zh");
	case EBackroomsLanguage::Japanese:   return TEXT("ja");
	case EBackroomsLanguage::Korean:     return TEXT("ko");
	case EBackroomsLanguage::Arabic:     return TEXT("ar");
	default:                             return TEXT("ru");
	}
}

FString BackroomsLoc::Get(const TCHAR* Key, EBackroomsLanguage Lang)
{
	EnsureRegistered();
	const FString* Found = Table(Lang).Find(Key);
	if (Found)
	{
		return *Found;
	}
	// Фолбэк: сначала английский (де-факто интернациональный базовый), потом
	// русский — базовый для проекта и самый полный. Так ни один из 13 языков
	// не показывает «голый ключ», если какая-то строка ещё не переведена.
	const FString* Fallback = Table(EBackroomsLanguage::English).Find(Key);
	if (Fallback) { return *Fallback; }
	Fallback = Table(EBackroomsLanguage::Russian).Find(Key);
	return Fallback ? *Fallback : FString(Key);
}

FString BackroomsLoc::Get(const TCHAR* Key)
{
	return Get(Key, GetLanguage());
}

FText BackroomsLoc::Text(const TCHAR* Key)
{
	return FText::FromString(Get(Key));
}

const TArray<FString>& BackroomsLoc::LanguageLabels()
{
	// Страна/локаль, а не абстрактное «RU/EN»: игроку понятнее, что выбирает.
	// Порядок совпадает с EBackroomsLanguage.
	static const TArray<FString> Labels =
	{
		TEXT("Русский (Россия)"),
		TEXT("English (United States)"),
		TEXT("Español (España)"),
		TEXT("Français (France)"),
		TEXT("Deutsch (Deutschland)"),
		TEXT("Italiano (Italia)"),
		TEXT("Português (Brasil)"),
		TEXT("Polski (Polska)"),
		TEXT("Türkçe (Türkiye)"),
		TEXT("中文 (简体)"),
		TEXT("日本語"),
		TEXT("한국어"),
		TEXT("العربية (فلسطين)")
	};
	return Labels;
}

FString BackroomsLoc::LanguageShort(EBackroomsLanguage Lang)
{
	switch (Lang)
	{
	case EBackroomsLanguage::English:    return TEXT("EN");
	case EBackroomsLanguage::Spanish:    return TEXT("ES");
	case EBackroomsLanguage::French:     return TEXT("FR");
	case EBackroomsLanguage::German:     return TEXT("DE");
	case EBackroomsLanguage::Italian:    return TEXT("IT");
	case EBackroomsLanguage::Portuguese: return TEXT("PT");
	case EBackroomsLanguage::Polish:     return TEXT("PL");
	case EBackroomsLanguage::Turkish:    return TEXT("TR");
	case EBackroomsLanguage::Chinese:    return TEXT("ZH");
	case EBackroomsLanguage::Japanese:   return TEXT("JA");
	case EBackroomsLanguage::Korean:     return TEXT("KO");
	case EBackroomsLanguage::Arabic:     return TEXT("AR");
	default:                             return TEXT("RU");
	}
}

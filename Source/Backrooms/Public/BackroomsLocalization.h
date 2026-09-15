#pragma once

#include "CoreMinimal.h"
#include "BackroomsLocalization.generated.h"

// Язык интерфейса. Хранится в GameUserSettings.ini, переживает перезапуск.
UENUM(BlueprintType)
enum class EBackroomsLanguage : uint8
{
	Russian    UMETA(DisplayName = "Русский"),
	English    UMETA(DisplayName = "English"),
	Spanish    UMETA(DisplayName = "Español"),
	French     UMETA(DisplayName = "Français"),
	German     UMETA(DisplayName = "Deutsch"),
	Italian    UMETA(DisplayName = "Italiano"),
	Portuguese UMETA(DisplayName = "Português"),
	Polish     UMETA(DisplayName = "Polski"),
	Turkish    UMETA(DisplayName = "Türkçe"),
	Chinese    UMETA(DisplayName = "中文"),
	Japanese   UMETA(DisplayName = "日本語"),
	Korean     UMETA(DisplayName = "한국어"),
	Arabic     UMETA(DisplayName = "العربية"),
	Count      UMETA(Hidden)
};

// Локализация меню и HUD. Устроена как реестр «ключ -> текст» на каждый язык:
// у каждого языка СВОЙ файл (BackroomsLoc_<Lang>.cpp), поэтому переводы не
// свалены в одну простыню, а добавление языка/строки не трогает UI и другие
// языки. Имена собственные (Backrooms, названия локаций L0..L9) не переводятся.
namespace BackroomsLoc
{
	// Таблица одного языка: ключ -> строка.
	using FLanguageTable = TMap<FString, FString>;

	// Доступ к таблице языка (создаётся лениво). Регистрация строк — Add.
	BACKROOMS_API FLanguageTable& Table(EBackroomsLanguage Lang);

	BACKROOMS_API EBackroomsLanguage GetLanguage();
	BACKROOMS_API void SetLanguage(EBackroomsLanguage Lang);
	BACKROOMS_API FString LanguageCode();

	// Строка по ключу на текущем языке. Неизвестный ключ возвращается как есть
	// (сразу видно, что забыли перевести, а не пустота).
	BACKROOMS_API FString Get(const TCHAR* Key);
	BACKROOMS_API FString Get(const TCHAR* Key, EBackroomsLanguage Lang);

	// Удобные обёртки.
	BACKROOMS_API FText Text(const TCHAR* Key);

	// Подписи для списка выбора языка (страна/локаль). Порядок = enum.
	BACKROOMS_API const TArray<FString>& LanguageLabels();
	BACKROOMS_API FString LanguageShort(EBackroomsLanguage Lang);

	// Вызвать один раз при старте: заполняет все таблицы. Вызывается лениво из
	// Table()/Get(), так что вручную звать не обязательно.
	BACKROOMS_API void EnsureRegistered();
}

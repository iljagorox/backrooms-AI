#pragma once

#include "CoreMinimal.h"
#include "BackroomsDifficulty.generated.h"

class ULevelGeneratorProfile;

// Уровень сложности мира. Выбор влияет и на генерацию (плотность стен, двери,
// количество пропсов), и на «давление среды» (скорость появления угрозы), и на
// состав стартовых ресурсов. Хранится в GameUserSettings.ini, переживает
// перезапуск и читается одним местом — UI и генератор берут одно значение.
UENUM(BlueprintType)
enum class EBackroomsDifficulty : uint8
{
	Peaceful  UMETA(DisplayName = "Мирный"),
	Easy      UMETA(DisplayName = "Лёгкий"),
	Normal    UMETA(DisplayName = "Обычный"),
	Hard      UMETA(DisplayName = "Сложный"),
	Nightmare UMETA(DisplayName = "Кошмар"),
	Count     UMETA(Hidden)
};

// Числовое описание сложности. Все множители — вокруг «1.0 = как задумано».
USTRUCT(BlueprintType)
struct FBackroomsDifficultyDef
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Difficulty")
	FName Id;

	UPROPERTY(BlueprintReadOnly, Category = "Difficulty")
	FText Name;

	UPROPERTY(BlueprintReadOnly, Category = "Difficulty")
	FText Description;

	// --- Генерация ---
	// WallBias множит порог стен: >1 = больше стен (теснее), <1 = просторнее.
	float WallBias = 1.0f;
	// DoorBias множит порог дверей: выше = больше проёмов (легче связанность).
	float DoorBias = 1.0f;
	// PropBias множит плотность пропсов (лут): выше = щедрее.
	float PropBias = 1.0f;
	// ScaleScatter влияет на разброс пропсов (хаос в расстановке).
	float ScatterBias = 1.0f;

	// --- Угрозы ---
	// Сколько монстров может быть одновременно.
	int32 MaxMonsters = 1;
	// Множитель задержки перед монстром (>1 = монстр позже, спокойнее).
	float MonsterDelayScale = 1.0f;
	// Множитель скорости роста давления среды.
	float PressureScale = 1.0f;
	// Базовое количество расходников на старте.
	int32 StartingSupplies = 0;
	// Рейтинг: множитель скорости убывания голода/жажды (>1 = быстрее слабеешь).
	float SurvivalDrainScale = 1.0f;

	// --- Дроп ---
	// Множитель шанса ПОЛЕЗНОГО лута (расходники): выше = щедрее.
	// Держим близко к 1.0 даже на «Кошмаре» — иначе игра превращается в пытку.
	float UtilityDropBias = 1.0f;
	// Абсолютный шанс ДЕКОРА на клетку. На высокой сложности полезного меньше,
	// но мир не пустеет — декора становится больше.
	float DecorChance = 0.08f;
	// Минимум полезных предметов на чанк: пол скупости. Даже на «Кошмаре»
	// игрок не остаётся совсем без снабжения (анти-софтлок).
	int32 MinUtilityPerChunk = 0;
	// Не открывать достижения об исследовании (для «коймара» — выживание важнее).
	bool bPermadeath = false;

	// Цвет для UI.
	FLinearColor Accent = FLinearColor(0.95f, 0.85f, 0.45f);
};

// Доступ к таблице сложностей и настройке генерации.
namespace BackroomsDifficulty
{
	BACKROOMS_API const TArray<FBackroomsDifficultyDef>& GetAll();
	BACKROOMS_API const FBackroomsDifficultyDef& Get(EBackroomsDifficulty D);
	BACKROOMS_API FString Name(EBackroomsDifficulty D);
	BACKROOMS_API FText NameText(EBackroomsDifficulty D);
	BACKROOMS_API FText DescriptionText(EBackroomsDifficulty D);

	// Текущая выбранная сложность (из GameUserSettings.ini).
	BACKROOMS_API EBackroomsDifficulty GetCurrent();
	BACKROOMS_API void SetCurrent(EBackroomsDifficulty D);

	// Применить сложность к профилю генерации: сдвигает пороги плотности,
	// щедрость пропсов и т.п. Вызывается генератором ПОСЛЕ построения профиля
	// уровня, поэтому влияет на любой уровень одинаково.
	BACKROOMS_API void ApplyToProfile(ULevelGeneratorProfile* Profile, EBackroomsDifficulty D);
}

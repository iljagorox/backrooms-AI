#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BackroomsAchievements.generated.h"

// Описание одного достижения. Условие разблокировки — накопленное значение
// статистики (Stat) не меньше порога (Threshold). Так одна таблица правил
// покрывает все достижения, а добавить новое = добавить строку.
USTRUCT(BlueprintType)
struct FBackroomsAchievementDef
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	FName Id;

	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	FText Name;

	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	FText Description;

	// Скрытое достижение: имя/условие не показываются до разблокировки.
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	bool bHidden = false;

	// Сколько «очков» даёт (для суммарного прогресса игрока).
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	int32 Points = 10;

	// Идентификатор статистики и порог срабатывания.
	FName Stat;
	float Threshold = 1.0f;
};

// Событие разблокировки — для всплывающего уведомления в HUD.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementUnlocked, FName, Id, const FText&, Name);

// Система достижений (GameInstance). Работает поверх простых счётчиков-статов:
// игра сообщает факты (подобрал предмет, спустился в Бэкрумс, встретил монстра),
// а система сама решает, какие достижения открылись. Прогресс сохраняется вместе
// с Seed/уровнем и переживает перезапуск — важное свойство для реиграбельности.
UCLASS()
class BACKROOMS_API UBackroomsAchievements : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Событие для HUD/меню.
	UPROPERTY(BlueprintAssignable, Category = "Achievement")
	FOnAchievementUnlocked OnAchievementUnlocked;

	// --- Отчётность об игровых фактах (вызывается из геймплея) ---
	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void AddStat(FName Stat, float Amount = 1.0f);

	// Установить абсолютное значение (напр., максимум давления среды).
	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void SetStatMax(FName Stat, float Value);

	// Уникальные локации: запоминаем множество посещённых уровней.
	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void NotifyLevelVisited(int32 LevelIndex);

	// Предметы и их категории (для «фармацевта»/«водохлёба»).
	void NotifyItemPickedUp();
	void NotifyItemUsed(int32 Category);

	// Игровые вехи.
	void NotifyBackroomsEntered();
	void NotifyMonsterEncountered();
	void NotifyExitUsed();
	void NotifyDeath();

	// Время в Бэкрумсе и пройденный путь (см) — вызывается из Tick игрока.
	void NotifyTimeInBackrooms(float DeltaSeconds);
	void NotifyDistance(float Centimeters);

	// --- Запросы ---
	UFUNCTION(BlueprintPure, Category = "Achievement")
	bool IsUnlocked(FName Id) const;

	UFUNCTION(BlueprintPure, Category = "Achievement")
	float GetStat(FName Stat) const;

	UFUNCTION(BlueprintPure, Category = "Achievement")
	int32 GetUnlockedCount() const { return Unlocked.Num(); }

	UFUNCTION(BlueprintPure, Category = "Achievement")
	int32 GetTotalCount() const;

	UFUNCTION(BlueprintPure, Category = "Achievement")
	int32 GetTotalPoints() const;

	// Принудительно открыть (для отладки/консоли).
	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void Unlock(FName Id);

	// Текстовый отчёт (для DevTool/логов).
	FString Report() const;

	// Сброс прогресса достижений.
	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void ResetAll();

	// Статистика, которую напрямую читают те или иные системы.
	static const FName StatLevelsVisited;
	static const FName StatTimeInBackrooms;
	static const FName StatDistanceCm;
	static const FName StatItemsPickedUp;
	static const FName StatItemsUsed;
	static const FName StatMedicinesUsed;
	static const FName StatAlmondUsed;
	static const FName StatMonsterEncounters;
	static const FName StatExitsUsed;
	static const FName StatDeaths;
	static const FName StatMaxPressure;
	static const FName StatFoodUsed;
	static const FName StatBatteriesUsed;
	static const FName StatSprints;
	static const FName StatRoomsEntered;

private:
	const TArray<FBackroomsAchievementDef>& Rules() const;

	// Пересчитать условия всех достижений; открыть подходящие.
	void Evaluate();

	// Найти определение по Id.
	const FBackroomsAchievementDef* FindDef(FName Id) const;

	// --- Сохранение (AppData/Local/.../Backrooms/Achievements.dat) ---
	static FString GetSavePath();
	void Save() const;
	void Load();

	// Разблокированные достижения.
	TSet<FName> Unlocked;
	// Накопленные статистики.
	TMap<FName, float> Stats;
	// Посещённые локации (для уникальности) — держим как стат-счётчик множества.
	TSet<int32> VisitedLevels;
};

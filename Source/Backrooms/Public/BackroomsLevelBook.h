#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BackroomsLevelBook.generated.h"

class UBackroomsLevelThemeConfig;
struct FURL;

// Книга уровней: какие локации уже открыты и как выглядит превью каждой.
// Открытие — по прохождению: локация считается доступной, если игрок уже
// побывал в ней или в любой более ранней. Превью — настоящий скриншот из
// уровня, снятый в момент, когда игрок впервые его увидел; готовые картинки
// хранятся в Saved/Screenshots и переживают перезапуск.
//
// Меню уровней читает отсюда имена, замок и путь к картинке — сама подсистема
// ничего не знает про UI, поэтому её же можно использовать для чего угодно.
UCLASS()
class BACKROOMS_API UBackroomsLevelBook : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Отметить уровень как посещённый (открывает его и все предыдущие).
	UFUNCTION(BlueprintCallable, Category = "Levels")
	void MarkVisited(int32 LevelIndex);

	// Открыт ли уровень для выбора.
	UFUNCTION(BlueprintPure, Category = "Levels")
	bool IsUnlocked(int32 LevelIndex) const;

	// Посещён ли уровень лично игроком (для бейджа «пройден»).
	UFUNCTION(BlueprintPure, Category = "Levels")
	bool IsVisited(int32 LevelIndex) const { return Visited.Contains(LevelIndex); }

	// Номер уровня, с которого стоит продолжить (самый дальний открытый).
	UFUNCTION(BlueprintPure, Category = "Levels")
	int32 GetFurthestLevel() const { return FurthestLevel; }

	// Путь к PNG-превью уровня (может не существовать — тогда меню рисует
	// стилизованную заглушку с цветом уровня).
	UFUNCTION(BlueprintPure, Category = "Levels")
	FString GetThumbnailPath(int32 LevelIndex) const;

	// Запросить снятие скриншота текущего вида и привязать его к уровню.
	// Вызывается генератором, когда мир уровня впервые достроен.
	UFUNCTION(BlueprintCallable, Category = "Levels")
	void RequestThumbnail(int32 LevelIndex);

	// Есть ли уже превью для уровня.
	UFUNCTION(BlueprintPure, Category = "Levels")
	bool HasThumbnail(int32 LevelIndex) const;

	// Название и акцентный цвет уровня (для карточки без скриншота).
	static FText LevelName(int32 LevelIndex);
	static FLinearColor LevelAccent(int32 LevelIndex);

	// Имя карты уровня (путь пакета). Lvl_L0..Lvl_L9.
	static FString GetMapName(int32 LevelIndex);
	// Обратный разбор: имя карты -> LevelIndex, -1 если не наша карта.
	static int32 GetLevelIndexFromMapName(const FString& MapName);
	// Карта, где показывается главное меню (L0).
	static FString GetMenuMapName();
	// Прочитать seed из URL-опции карты (?seed=N); false если нет.
	static bool GetSeedFromUrl(const FURL& Url, int32& OutSeed);

	// DataAsset конфига уровня (/Game/Data/L{L}_Config); nullptr если ассет
	// не создан — тогда используется кодовая тема-фолбэк профиля.
	UFUNCTION(BlueprintPure, Category = "Levels")
	const UBackroomsLevelThemeConfig* GetConfig(int32 LevelIndex) const;

	// Сбросить прогресс открытия.
	UFUNCTION(BlueprintCallable, Category = "Levels")
	void ResetAll();

private:
	static FString SavePath();
	void Save() const;
	void Load();

	// Прячем скриншот под именем уровня; движок кладёт файл в ScreenShotDir.
	// Ждём кадр и переносим/запоминаем готовый путь.
	void FinalizeThumbnail(int32 LevelIndex);

	TSet<int32> Visited;
	int32 FurthestLevel = 0;

	// Кэш загруженных конфигов уровней (слабо: ассеты напрямую не держим).
	mutable TMap<int32, TWeakObjectPtr<UBackroomsLevelThemeConfig>> ConfigCache;

	// Уровни, для которых запрошен скриншот, но файл ещё не готов.
	TSet<int32> PendingThumbnail;

	FTimerHandle ThumbnailTimer;
};

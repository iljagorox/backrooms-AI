#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BackroomsProgression.generated.h"

// Ветка прокачки. Игрок разных стилей получает своё:
//  - Выживальщик: HP/голод/жажда (для тех, кто идёт «попугаться»);
//  - Лазутчик: слоты/фонарь/скорость (для лутеров и исследователей);
//  - Разум: рассудок/страх (для хардкорщиков, кто лезет к монстрам).
UENUM(BlueprintType)
enum class EBackroomsPerkTree : uint8
{
	Survival  UMETA(DisplayName = "Выживание"),
	Scavenger UMETA(DisplayName = "Лазутчик"),
	Mind      UMETA(DisplayName = "Разум"),
	Count     UMETA(Hidden)
};

// Описание перка. Бонусы — множители/прибавки к базовым статам; система
// собирает их в FBackroomsPerkBonuses, а компоненты игрока читают итог.
USTRUCT(BlueprintType)
struct FBackroomsPerkDef
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	FName Id;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	FText Name;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	FText Description;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	EBackroomsPerkTree Tree = EBackroomsPerkTree::Survival;

	// Требуемый уровень игрока и цена в очках навыков.
	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	int32 RequiredLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	int32 Cost = 1;

	// Перк-предусловие (должен быть взят раньше). NAME_None — можно с начала.
	FName Requires;

	// --- Эффекты (аддитивные; 0 = нет) ---
	float BonusMaxHealth = 0.0f;      // + к максимуму HP
	float BonusMaxStamina = 0.0f;
	float StaminaDrainScale = 0.0f;   // − доля расхода бега (0.15 = −15%)
	float BonusInventorySlots = 0.0f;
	float BonusSanity = 0.0f;         // + к максимуму рассудка
	float SanityDrainScale = 0.0f;    // − доля убывания рассудка
	float FearResistScale = 0.0f;     // − доля роста страха
	float HungerDrainScale = 0.0f;    // − доля расхода голода/жажды
	float FlashlightDrainScale = 0.0f;// − доля расхода фонаря
	float SpeedBonus = 0.0f;          // + доля скорости ходьбы/бега
	float XPBoost = 0.0f;             // + доля получаемого XP (чекпоинт-перк)
};

// Сводка бонусов по всем взятым перкам. 1.0 = без изменений.
USTRUCT(BlueprintType)
struct FBackroomsPerkBonuses
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	float MaxHealthAdd = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	float MaxStaminaAdd = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	float MaxSanityAdd = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	float InventorySlotsAdd = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	float StaminaDrainMult = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	float SanityDrainMult = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	float FearGrowthMult = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	float SurvivalDrainMult = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	float FlashlightDrainMult = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	float SpeedMult = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	float XPBoost = 0.0f;
};

// Итог забега для локального лидерборда и достижений.
USTRUCT(BlueprintType)
struct FBackroomsRunRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Run")
	int32 Seed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Run")
	int32 Difficulty = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Run")
	int32 LevelIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Run")
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Run")
	float TimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Run")
	int32 RoomsDiscovered = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Run")
	bool bSurvived = false;

	UPROPERTY(BlueprintReadOnly, Category = "Run")
	FDateTime Date;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProgressionChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPerkUnlocked, FName, PerkId, const FText&, Name, int32, PointsLeft);

// Мета-прогрессия игрока: XP, уровни, очки навыков, перки и лидерборд.
// Живёт в GameInstance, сохраняется в Saved/Backrooms/Progression.dat и
// переживает перезапуск — за счёт этого реиграбельность и «100 часов».
UCLASS()
class BACKROOMS_API UBackroomsProgression : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnProgressionChanged OnProgressionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnPerkUnlocked OnPerkUnlocked;

	// --- XP и уровни ---
	// Начислить XP (с учётом перка-буста). Возвращает фактически начисленное.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	int32 AwardXP(int32 BaseAmount, FName Reason);

	// События мира: исследование, спуск, выживание. XP логичен процессу, а не
	// «собери шарики»: за первое посещение чанка/комнаты и за переход вглубь.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void NotifyRoomDiscovered(int32 RoomId, int32 LevelIndex);

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void NotifyChunkDiscovered(int32 ChunkX, int32 ChunkY, int32 LevelIndex);

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void NotifyDescended(int32 NewLevelIndex);

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void NotifyRunSurvived(float SecondsAtLevel);

	// Накопить время текущего забега (зовётся из Tick игрока, пока он в Бэкрумсе).
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void TickRun(float DeltaSeconds);

	// Начать забег: запомнить seed и уровень для записи в лидерборд.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void BeginRun(int32 Seed, int32 LevelIndex);

	// Завершить забег и отправить запись в лидерборд. Вызывается на смерти
	// (bSurvived=false) и на выходе с уровня (bSurvived=true).
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void EndRun(bool bSurvived);

	// --- Перки ---
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool UnlockPerk(FName PerkId);

	UFUNCTION(BlueprintPure, Category = "Progression")
	bool IsPerkUnlocked(FName PerkId) const { return UnlockedPerks.Contains(PerkId); }

	UFUNCTION(BlueprintPure, Category = "Progression")
	FBackroomsPerkBonuses GetBonuses() const { return Bonuses; }

	UFUNCTION(BlueprintPure, Category = "Progression")
	const TArray<FBackroomsPerkDef>& AllPerks() const;

	// --- Лидерборд ---
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void SubmitRun(const FBackroomsRunRecord& Record);

	UFUNCTION(BlueprintPure, Category = "Progression")
	const TArray<FBackroomsRunRecord>& GetLeaderboard() const { return Leaderboard; }

	// --- Запросы ---
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetLevel() const { return Level; }

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetXP() const { return XP; }

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetXPForNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetSkillPoints() const { return SkillPoints; }

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetPerksUnlocked() const { return UnlockedPerks.Num(); }

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetTotalPerks() const;

	// Текстовый отчёт (DevTool/логи).
	FString Report() const;

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void ResetAll();

private:
	// Собрать бонусы по всем взятым перкам.
	void RecomputeBonuses();

	static FString GetSavePath();
	void Save() const;
	void Load();

	// Название перка/описания — ключи локализации Perk.<Id>.Name/.Desc.
	static FText PerkNameText(FName Id);
	static FText PerkDescText(FName Id);

	// Правила перков (статические данные).
	static const TArray<FBackroomsPerkDef>& Rules();

	int32 XP = 0;
	int32 Level = 1;
	int32 SkillPoints = 0;
	TSet<FName> UnlockedPerks;
	FBackroomsPerkBonuses Bonuses;

	// Текущий забег: копится, пока игрок в Бэкрумсе.
	int32 RunStartLevel = 0;
	int32 RunSeed = 0;
	int32 RunLevelIndex = 0;
	TSet<int32> RunRooms;
	float RunTime = 0.0f;

	// Уникально посещённые клетки/комнаты (за них XP выдаётся один раз).
	TSet<int32> DiscoveredRooms;
	TSet<int32> DiscoveredChunks;

	TArray<FBackroomsRunRecord> Leaderboard;
};
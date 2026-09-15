#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShuffleBag.h"
#include "Engine/TimerHandle.h"
#include "BackroomsEventSystem.generated.h"

class ABackroomsWorldGenerator;
class ABackroomsChunkActor;
class USoundBase;

// Типы случайных событий.
UENUM(BlueprintType)
enum class EBackroomsEventType : uint8
{
	LightFlicker     UMETA(DisplayName = "Мерцание света"),
	LightOutage      UMETA(DisplayName = "Погас свет"),
	BoxDisappear     UMETA(DisplayName = "Ящик исчез"),
	BoxAppear        UMETA(DisplayName = "Ящик появился"),
	EntityGrowl      UMETA(DisplayName = "Рык сущности"),
	EntityFootsteps  UMETA(DisplayName = "Шаги в темноте"),
	Whisper          UMETA(DisplayName = "Шёпот"),
	DistantBang     UMETA(DisplayName = "Грохот издалека"),
	WallDrawing      UMETA(DisplayName = "Рисунок на стене"),
	PipeCreak        UMETA(DisplayName = "Скрип трубы"),
	DoorSlam         UMETA(DisplayName = "Хлопнула дверь"),
	EmergencyLight   UMETA(DisplayName = "Аварийный свет"),
	FogIncrease      UMETA(DisplayName = "Увеличение тумана"),
	StaticNoise      UMETA(DisplayName = "Помехи на экране"),
	FootprintAppear  UMETA(DisplayName = "Следы на полу")
};

// Конфигурация события.
USTRUCT(BlueprintType)
struct FBackroomsEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBackroomsEventType Type = EBackroomsEventType::LightFlicker;

	// Минимальный интервал между срабатываниями (сек).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinInterval = 10.0f;

	// Максимальный интервал между срабатываниями (сек).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxInterval = 60.0f;

	// Длительность эффекта (сек).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 2.0f;

	// Радиус действия (0 = весь уровень).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 0.0f;

	// Вероятность срабатывания за интервал (0.0 - 1.0).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Probability = 0.3f;

	// «Страшность» события (0..1): ниже 0.5 — успокаивающее, выше — страшное.
	// Используется для ПЕРЕУКЛЮЧЕНИЯ ВЕСОВ, когда состояние игрока низкое:
	// при упадке рассудка/здоровья (distress высок) страшные события получают
	// больший вес, успокаивающие — меньший. Так локация реагирует на игрока,
	// а не идёт по расписанию.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FearWeight = 0.5f;

	// Громкость звука (для звуковых событий).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Volume = 1.0f;

	// ---- Пуассоновский процесс (события не «по расписанию», а кластерами) ----
	// Вероятность, что сработавшее событие запускает «залп» из ещё нескольких
	// событий подряд (ужастик держит в тонусе: шёл, шёл — и вдруг 3 события).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BurstChance = 0.35f;

	// Сколько ещё событий выстрелит в этом «залпе» после первого (0 = без залпа).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxBurstFollowups = 2;
};

// Последовательность (цепочка) событий. Ужастик не должен срабатывать
// одним случайным событием — вместо этого игрок ощущает «сцену»: несколько
// событий подряд по порядку (например: мерцание -> шёпот -> хлопок двери ->
// помехи). Цепочка выбирается из пула (с учётом веса и состояния игрока) и
// проигрывает свои шаги по очереди с задержкой между ними.
USTRUCT(BlueprintType)
struct FBackroomsEventSequence
{
	GENERATED_BODY()

	// Ид цепочки (для логов/следования).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Id;

	// Вес цепочки в пуле (чем выше, тем чаще выбирается).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 1.0f;

	// «Страшность» цепочки: при низком состоянии игрока страшные цепочки
	// получают больший вес — веса в локации «меняются местами».
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FearWeight = 0.5f;

	// Упорядоченные шаги цепочки (проигрываются по порядку).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EBackroomsEventType> Steps;

	// Задержка между шагами цепочки (сек).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StepIntervalMin = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StepIntervalMax = 1.8f;
};

// Результат выполнения события (для UI/логов).
USTRUCT(BlueprintType)
struct FBackroomsEventResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (IgnoreForMemberInitializationTest))
	EBackroomsEventType Type = EBackroomsEventType::LightFlicker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (IgnoreForMemberInitializationTest))
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (IgnoreForMemberInitializationTest))
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBackroomsEvent, const FBackroomsEventResult&, Event);

UCLASS(BlueprintType, Blueprintable)
class BACKROOMS_API UBackroomsEventSystem : public UObject
{
	GENERATED_BODY()

public:
	// Список возможных событий (настраивается в Blueprint).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TArray<FBackroomsEvent> Events;

	// Пул последовательностей (цепочек) событий.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TArray<FBackroomsEventSequence> Sequences;

	// Событие при срабатывании (для подписки в Blueprint).
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBackroomsEvent OnEventTriggered;

	// Инициализировать систему с событиями по умолчанию.
	UFUNCTION(BlueprintCallable, Category = "Events")
	void InitializeDefaultEvents();

	// Тик проверки событий (вызывается из генератора).
	UFUNCTION(BlueprintCallable, Category = "Events")
	void TickEvents(float DeltaSeconds, ABackroomsWorldGenerator* Generator);

	// Принудительно запустить событие (для тестов/скриптов).
	UFUNCTION(BlueprintCallable, Category = "Events")
	void TriggerEvent(EBackroomsEventType Type, FVector Location);

	// Принудительно запустить сразу всю цепочку-последовательность.
	UFUNCTION(BlueprintCallable, Category = "Events")
	void TriggerSequence(FName SequenceId);

	// Подать текущее состояние игрока (рассудок/здоровье). Низкие значения
	// повышают вес страшных событий/цепочек — локация «реагирует» на игрока,
	// а не идёт по расписанию.
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetPlayerStats(float InSanity, float InMaxSanity);

	// ---- Связь АТМОСФЕРЫ с приходом МОНСТРА (реализовано в C++, живая) ----
	// Монстр сообщает системе своё присутствие и состояние. Система отвечает
	// атмосферой:
	//   * ИССЛЕДОВАНИЕ (bInvestigating) -> в локации начинают звучать ШЁПОТЫ.
	//   * МОНСТР РЯДОМ  -> тихие, ненавязчивые звуки (шёпот/дыхание/гул),
	//     громкость зависит от близости.
	// Это не «один громкий пугающий звук по триггеру», а постоянная, живая
	// смена давления, которую игрок ощущает (СИСТЕМА ЧУВСТВ возвращается
	// атмосферой в игру).
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetMonsterPresence(bool bInvestigating, const FVector& MonsterLocation, float DistToPlayer);

	// Текущий «уровень тревоги» состоянии игрока в [0,1] (1 = полный упадок).
	UFUNCTION(BlueprintPure, Category = "State")
	float GetDistress() const;

	// Включить/выключить систему.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	bool bEnabled = true;

	// Зерно для временного шума (плавные изменения атмосферы/весов).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	int32 Seed = 1337;

private:
	// Таймеры до следующего срабатывания для каждого типа.
	TMap<EBackroomsEventType, float> Timers;

	// Сэмплирование интервала ожидания по экспоненциальному закону
	// (пуассоновский процесс): -ln(1-u)/lambda. Даёт естественные
	// «пачки» событий и долгое затишье вместо равномерного ритма.
	float SamplePoissonInterval(const FBackroomsEvent& Event) const;

	// Состояние «залпа»: когда событие сработало, иногда за ним должны быстрым
	// темпом идти ещё события (кластер).
	int32 BurstRemaining = 0;
	float BurstTimer = 0.0f;
	EBackroomsEventType BurstType = EBackroomsEventType::LightFlicker;

	// ---- Цепочки (последовательности) событий ----
	// Мешок-колода, из которой раздаются цепочки (без немедленного повтора
	// одной и той же цепочки подряд; колода перетасовывается в конце).
	FShuffleBag SequenceBag;

	// Очередь шагов текущей цепочки и таймер до следующего шага.
	TArray<EBackroomsEventType> ChainQueue;
	float ChainStepTimer = 0.0f;
	float ChainCooldown = 4.0f;   // пауза между разными цепочками

	// Выбрать следующую цепочку из пула с учётом весов и состояния игрока.
	const FBackroomsEventSequence* PickNextSequence(float WorldTime);

	// Проиграть все шаги выбранной цепочки по порядку.
	void QueueSequence(const FBackroomsEventSequence& Seq, ABackroomsWorldGenerator* Generator);
	void AdvanceChain(float DeltaSeconds, ABackroomsWorldGenerator* Generator);

	// ---- Состояние игрока -> переключение весов ----
	// Уровень тревоги (0..1), плавно сглаженный TemporalNoise, чтобы не было
	// жёсткого скачка-порога (нескриптованность).
	float TargetDistress = 0.0f;
	float CurrentDistress = 0.0f;

	// ---- Состояние монстра -> атмосфера ----
	bool bMonsterInvestigating = false;
	FVector MonsterLocation = FVector::ZeroVector;
	float MonsterProximity = 0.0f; // 0 = далеко, 1 = вплотную
	float MonsterSfxTimer = 0.0f;  // таймер «ненавязчивых» звуков рядом
	float WhisperTimer = 0.0f;     // таймер шёпотов при исследовании

	// Реакция атмосферы на монстра в TickEvents.
	void TickMonsterAtmosphere(float DeltaSeconds, ABackroomsWorldGenerator* Generator);

	// Найти конфиг события типа.
	const FBackroomsEvent* FindEventConfig(EBackroomsEventType Type) const;

	// Создать пул цепочек по умолчанию (для прототипа без Blueprint-ассетов).
	void BuildDefaultSequences();

	// Активные эффекты (для отслеживания длительности).
	struct FActiveEvent
	{
		EBackroomsEventType Type;
		float TimeRemaining;
		FVector Location;
	};
	TArray<FActiveEvent> ActiveEffects;

	// ---- «Мешок случайностей» для звуковых вариантов ----
	// Ужастик не должен повторять один и тот же звук подряд (это выдаёт
	// скриптованность). Поэтому варианты звуков (шаги, дёрганье двери)
	// раздаются из тасованной колоды; колода перетасовывается, когда
	// опустела. Variant пара: набор soft-ссылок на звуки + сам мешок.

	// Варианты звуков шагов (ожидается 10-15 штук).
	UPROPERTY(EditAnywhere, Category = "AudioVariants")
	TArray<TSoftObjectPtr<USoundBase>> FootstepVariants;

	// Варианты звуков дёрганья/хлопка двери (ожидается 10-15 штук).
	UPROPERTY(EditAnywhere, Category = "AudioVariants")
	TArray<TSoftObjectPtr<USoundBase>> DoorRattleVariants;

	// Конкретные звуки для одиночных событий (загружаются по UPROPERTY,
	// а не nullptr-заглушки в коде).
	UPROPERTY(EditAnywhere, Category = "AudioVariants")
	TSoftObjectPtr<USoundBase> GrowlSound;

	UPROPERTY(EditAnywhere, Category = "AudioVariants")
	TSoftObjectPtr<USoundBase> WhisperSound;

	UPROPERTY(EditAnywhere, Category = "AudioVariants")
	TSoftObjectPtr<USoundBase> CreakSound;

	UPROPERTY(EditAnywhere, Category = "AudioVariants")
	TSoftObjectPtr<USoundBase> BangSound;

	// Мешки (колоды) для соответствующих наборов.
	FShuffleBag FootstepBag;
	FShuffleBag DoorRattleBag;

	// Кеш загруженных звуков: полный путь ассета -> USoundBase. UPROPERTY,
	// чтобы GC видел ссылки и не выгружал звуки, пока они в кеше.
	UPROPERTY()
	TMap<FName, TObjectPtr<USoundBase>> SoundCache;

	// Серия шагов: активный таймер + его состояние. Идёт на член-хендл,
	// чтобы повторный вызов сбрасывал серию, а сам таймер гасился внутри.
	FTimerHandle ActiveFootstepHandle;
	TObjectPtr<ABackroomsWorldGenerator> FootstepGenerator;
	FVector FootstepLocation = FVector::ZeroVector;
	float FootstepVolume = 1.0f;
	int32 FootstepStepCount = 0;
	bool bFootstepsActive = false;

	// Один шаг серии «шаги в темноте» (вызывается таймером).
	void TickFootstep();

	// Взять звук из «мешка» — следующий карты без немедленного повтора.
	USoundBase* DrawFromBag(FShuffleBag& Bag, const TArray<TSoftObjectPtr<USoundBase>>& Variants);

	// Синхронно загрузить звук из soft-ссылки (с кешем по имени ассета).
	USoundBase* GetLoadedSound(const TSoftObjectPtr<USoundBase>& Soft);

	void ExecuteEvent(const FBackroomsEvent& Event, ABackroomsWorldGenerator* Generator);
	void ApplyLightFlicker(ABackroomsWorldGenerator* Generator, float Duration, const FVector& Location, float Radius);
	void ApplyLightOutage(ABackroomsWorldGenerator* Generator, float Duration, const FVector& Location, float Radius);
	void ApplyEntityGrowl(ABackroomsWorldGenerator* Generator, const FVector& Location, float Volume);
	void ApplyEntityFootsteps(ABackroomsWorldGenerator* Generator, const FVector& Location, float Volume);
	void ApplyWhisper(ABackroomsWorldGenerator* Generator, const FVector& Location, float Volume);
	void ApplyWallDrawing(ABackroomsWorldGenerator* Generator, const FVector& Location);
	void ApplyPipeCreak(ABackroomsWorldGenerator* Generator, const FVector& Location, float Volume);
	void ApplyDoorSlam(ABackroomsWorldGenerator* Generator, const FVector& Location, float Volume);
	void ApplyEmergencyLight(ABackroomsWorldGenerator* Generator, float Duration, const FVector& Location);
	void ApplyFogIncrease(ABackroomsWorldGenerator* Generator, float Duration);
	void ApplyStaticNoise(ABackroomsWorldGenerator* Generator, float Duration);
};

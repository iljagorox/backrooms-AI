#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BackroomsSeatFinder.h"
#include "BackroomsMonsterAIController.generated.h"

class UBackroomsSenseComponent;
class ACharacter;
class UBackroomsEventSystem;
class USoundBase;

// Состояния монстра (минимум — 3).
UENUM(BlueprintType)
enum class EBackroomsMonsterState : uint8
{
	Patroling      UMETA(DisplayName = "Патрулирование"),
	Investigating  UMETA(DisplayName = "Исследование (звук/зрение)"),
	Sitting        UMETA(DisplayName = "Сидит"),
	Watching       UMETA(DisplayName = "Наблюдение издалека"),
	Stalking       UMETA(DisplayName = "Преследование на дистанции"),
	Alerted        UMETA(DisplayName = "Тревога (заметил игрока)"),
	Disappearing   UMETA(DisplayName = "Исчезновение")
};

// Мозг монстра: связывает СИСТЕМУ ЧУВСТВ (слух + конус обзора как у человека)
// с поведением. Монстр не нападает по таймеру — он сначала:
//   1) ПАТРУЛИРУЕТ (бродит по точкам, пока не услышит/не заметит);
//   2) ИССЛЕДУЕТ — когда конус его зрения пересекся с игроком или он услышал
//      звук: подходит к источнику, осматривается по сторонам и ходит вокруг
//      точки; и только
//   3) ТРЕВОГА — когда реально уверен в игроке, начинает преследование.
UCLASS()
class BACKROOMS_API ABackroomsMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABackroomsMonsterAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

	// Куда смотрел/что знает монстр (для отладки и закупки на цель).
	UPROPERTY(BlueprintReadOnly, Category = "Monster")
	EBackroomsMonsterState CurrentState = EBackroomsMonsterState::Patroling;

	// Ссылка на сенсор монстра (создаётся в OnPossess на пешке).
	UPROPERTY(BlueprintReadOnly, Category = "Monster")
	TObjectPtr<UBackroomsSenseComponent> Sense;

	// Цель (игрок).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	TObjectPtr<ACharacter> Target;

	// Система событий, которой монстр сообщает о себе (для связи атмосферы
	// с приходом монстра: при расследовании — шёпоты, рядом — тихие звуки).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	TObjectPtr<UBackroomsEventSystem> EventSystem;

	// Звуки «монстр заметил игрока» (Content/Audio/MonsterNoticed1/2): при
	// входе в состояние Alerted играет случайный из двух — в мире у монстра,
	// чтобы игрок слышал, откуда он.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	TSoftObjectPtr<USoundBase> NoticedSoundA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	TSoftObjectPtr<USoundBase> NoticedSoundB;

protected:
	// Точка, которую сейчас исследует монстр (источник звука/точка пересечения).
	UPROPERTY(BlueprintReadOnly, Category = "Monster")
	FVector InvestigatePoint = FVector::ZeroVector;

private:
	// Скорости/тайминги.
	UPROPERTY(EditAnywhere, Category = "Monster|Patrol")
	float PatrolRadius = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Monster|Patrol")
	float PatrolRePickInterval = 6.0f;

	// Исследование: сколько точек по кругу обойдёт и на сколько направлений
	// посмотрит вокруг источника.
	UPROPERTY(EditAnywhere, Category = "Monster|Investigate")
	int32 InvestigateCirclePoints = 4;

	UPROPERTY(EditAnywhere, Category = "Monster|Investigate")
	float InvestigateCircleRadius = 260.0f;

	UPROPERTY(EditAnywhere, Category = "Monster|Investigate")
	float InvestigateIdlePerView = 0.8f;

	// Время, после которого «успокоился» и вернулся в патруль (если не видит цель).
	UPROPERTY(EditAnywhere, Category = "Monster|Investigate")
	float AbandonAfter = 8.0f;

	// --- Сидение на мебели ---
	UPROPERTY(EditAnywhere, Category = "Monster|Sitting")
	float SitSearchRadius = 900.0f;    // искать мебель в этом радиусе (см)
	UPROPERTY(EditAnywhere, Category = "Monster|Sitting")
	float SitMinDuration = 6.0f;       // мин время сидения (с)
	UPROPERTY(EditAnywhere, Category = "Monster|Sitting")
	float SitMaxDuration = 14.0f;      // макс время сидения (с)
	UPROPERTY(EditAnywhere, Category = "Monster|Sitting")
	float SitMinCooldown = 25.0f;      // мин пауза между посадками (с)
	UPROPERTY(EditAnywhere, Category = "Monster|Sitting")
	float SitMaxCooldown = 45.0f;      // макс пауза между посадками (с)

	FBackroomsSeat CurrentSeat;        // выбранное место
	bool bSitApproaching = false;      // идёт к месту (ещё не села)
	bool bSeated = false;              // уже сидит
	float SitTimer = 0.0f;             // время в состоянии
	float SitTargetDuration = 10.0f;   // сколько просидит в этот раз
	float SitProbeTimer = 8.0f;        // как часто пробуем найти место
	float NextSitSearchTime = 0.0f;    // абсолютное время следующего поиска
	// Монстр НЕ ищет стул, пока мир «собирается» и не пришло его «настроение»
	// (индивидуальное окно-задержка, задаётся в OnPossess, чтобы вся стая не
	// сканировала мир одновременно и сразу после спавна).
	float SitSearchEarliestTime = 0.0f;

	// --- Решимость ---
	// Пока игрок СПРЯТАЛСЯ, решимость [0..1] медленно падает, но монстр из-за
	// 3D-шума Перлина может РЕЗКО решить обернуться/развернуться — даже если
	// не получил ни звука, ни взгляда. Это держит в напряжении: прятаться
	// полностью «безопасно» нельзя.
	UPROPERTY(EditAnywhere, Category = "Monster|Resolve")
	float Resolve = 1.0f;

	// Скорость падения решимости в сек (когда источник исчез).
	UPROPERTY(EditAnywhere, Category = "Monster|Resolve")
	float ResolveDecay = 0.12f;

	// Как часто 3D-шум может вызывать резкий разворот (рад/сек шума).
	UPROPERTY(EditAnywhere, Category = "Monster|Resolve")
	float SnapTurnNoiseScale = 0.35f;

	// Порог шума, при котором монстр резко разворачивается (0..1, меньше — чаще).
	UPROPERTY(EditAnywhere, Category = "Monster|Resolve")
	float SnapTurnThreshold = 0.85f;

	// Сколько решимости возвращается за резкий разворот.
	UPROPERTY(EditAnywhere, Category = "Monster|Resolve")
	float ResolveRefreshOnSnap = 0.25f;

	// --- Наблюдение / преследование на дистанции / исчезновение ---
	// Диапазон, в котором монстр предпочитает «наблюдать» из тени.
	UPROPERTY(EditAnywhere, Category = "Monster|Stalk")
	float WatchMinDistance = 1400.0f;

	UPROPERTY(EditAnywhere, Category = "Monster|Stalk")
	float WatchMaxDistance = 2600.0f;

	// На какой дистанции монстр держится, преследуя на отдалении.
	UPROPERTY(EditAnywhere, Category = "Monster|Stalk")
	float StalkDistance = 1200.0f;

	// Сколько секунд игрок должен смотреть на монстра в упор взглядом, чтобы
	// тот «растворился» (страх + невозможность рассмотреть).
	UPROPERTY(EditAnywhere, Category = "Monster|Stalk")
	float GazeBreakSeconds = 0.7f;

	// Длительность исчезновения, после которого монстр появляется в другом месте.
	UPROPERTY(EditAnywhere, Category = "Monster|Stalk")
	float DisappearDuration = 4.0f;

	float PatrolTimer = 0.0f;
	int32 CircleStep = 0;
	int32 LookDir = 0;
	float InvestigateTimer = 0.0f;
	float StateTime = 0.0f;
	FVector CircleCenter = FVector::ZeroVector;
	float NextSnapTime = 0.0f;

	// Накопленное время, пока игрок смотрит прямо на монстра.
	float GazeTimer = 0.0f;
	float DisappearTimer = 0.0f;

	// Очередь исследования: места из ПАМЯТИ монстра (последние 3-5, где видел/
	// слышал игрока). Монстр обходит их по очереди — поэтому бегать по одним
	// и тем же местам опасно: монстр к ним возвращается.
	TArray<FVector> InvestigateQueue;
	int32 QueueIdx = 0;

	void RunPatrol(float DeltaSeconds);
	void RunInvestigate(float DeltaSeconds);
	void RunAlerted(float DeltaSeconds);
	void RunWatching(float DeltaSeconds);
	void RunStalking(float DeltaSeconds);
	void RunDisappearing(float DeltaSeconds);
	void SetInvestigateFromSense();
	FVector GetRandomPatrolPoint() const;
	FVector ApplyMovementError(const FVector& Goal) const;
	// Смотрит ли игрок прямо на монстра (для Watching/Stalking/Disappearing).
	bool IsPlayerLookingAtMonster(float MaxDistance) const;
	// Точка для повторного появления после исчезновения (подальше от игрока).
	FVector GetDisappearPoint() const;
	// Резкий разворот под действием 3D-шума (возвращает true, если сработал).
	bool TrySnapTurn();
	// Играет случайный из двух звуков «монстр заметил игрока» в мире у монстра.
	void PlayNoticeSound();
	void RunSitting(float DeltaSeconds);
	bool TryStartSit();
	// Поведенческие условия посадки: мир создан, монстр спокойно патрулирует,
	// про игрока ничего не известно, игрок далеко.
	bool ShouldSeekSit() const;
	void StandUpFromSitting();
	const TCHAR* GetStateDisplayName() const;
};

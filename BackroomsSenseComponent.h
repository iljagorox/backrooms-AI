#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BackroomsSenseComponent.generated.h"

// Уровень осведомлённости монстра.
UENUM(BlueprintType)
enum class EBackroomsAwareness : uint8
{
	Unaware    UMETA(DisplayName = "Не замечает"),
	Suspecting UMETA(DisplayName = "Что-то слышал"),
	Alerted    UMETA(DisplayName = "Насторожен / заметил")
};

// Тип раздражителя (стимула), который чувствует монстр.
UENUM(BlueprintType)
enum class EBackroomsStimulusType : uint8
{
	Sound   UMETA(DisplayName = "Звук (шаги/падающие предметы)"),
	Sight   UMETA(DisplayName = "Зрение (конус обзора)"),
	Contact UMETA(DisplayName = "Контакт (очень близко)")
};

// Воспоминание монстра: место, где он видел/слышал игрока.
// Монстр ПОМНИТ последние 3-5 таких мест (память), чтобы не терять след,
// даже когда игрок прячется.
USTRUCT(BlueprintType)
struct BACKROOMS_API FBackroomsSenseMemory
{
	GENERATED_BODY()

	// Где видели/слышали.
	UPROPERTY(BlueprintReadOnly, Category = "Sense")
	FVector Location = FVector::ZeroVector;

	// Чем запомнили: звук или зрение.
	UPROPERTY(BlueprintReadOnly, Category = "Sense")
	EBackroomsStimulusType Type = EBackroomsStimulusType::Sound;

	// Когда запомнили (мировое время).
	UPROPERTY(BlueprintReadOnly, Category = "Sense")
	float Time = 0.0f;
};

// Монстр должен не просто нападать по позиции, а ИМЕТЬ СИСТЕМУ ЧУВСТВ:
//   - слух: слышит звуки (шаги, падающие предметы) с учётом громкости,
//     расстояния и затухания (в т.ч. глушатся плотными стенами);
//   - зрение: конус обзора КАК У ЧЕЛОВЕКА (FOV + дальность + заслонённость
//     стенами через line trace — за стену не видит);
//   - периферическое зрение: слабее в уголках глаза;
//   - осведомлённость нарастает от стимула и постепенно спадает (память).
// Всё это убирает «скриптованное» мгновенное нападение по таймеру.
UCLASS(ClassGroup = (Backrooms), meta = (BlueprintSpawnableComponent))
class BACKROOMS_API UBackroomsSenseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBackroomsSenseComponent();

	// ---- Настройка (естественные параметры) ----
	// Дальность зрения (см).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense|Vision")
	float VisionRange = 2400.0f;

	// Центральный конус обзора (градусы) — «как у человека» (обычно ~100-110).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense|Vision")
	float VisionFOVDegrees = 100.0f;

	// Периферийный угол (градусы) — замечает движение «краем глаза».
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense|Vision")
	float PeripheralFOVDegrees = 180.0f;

	// Дальность слуха (см).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense|Hearing")
	float HearingRange = 2000.0f;

	// Порог громкости, с которого звук вообще начинает восприниматься.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense|Hearing")
	float MinHeardLoudness = 0.15f;

	// Скорость спада осведомлённости (в секунду) — «забывает» стимул.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense|Memory")
	float AwarenessDecay = 1.5f;

	// Сколько последних мест (видел/слышал игрока) монстр ПОМНИТ (3-5).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense|Memory")
	int32 MaxMemory = 5;

	// Память: места, где видели/слышали игрока (последние <= MaxMemory).
	UPROPERTY(BlueprintReadOnly, Category = "Sense|Memory")
	TArray<FBackroomsSenseMemory> Memory;

	// Как быстро нарастает осведомлённость от звука.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense|Hearing")
	float SoundGain = 1.0f;

	// ---- Результат ----
	// Текущая осведомлённость [0..1].
	UPROPERTY(BlueprintReadOnly, Category = "Sense")
	float Awareness = 0.0f;

	// Последнее известное место игрока (куда монстр идёт, если насторожен).
	UPROPERTY(BlueprintReadOnly, Category = "Sense")
	FVector LastKnownPlayerLocation = FVector::ZeroVector;

	// Последний раз, когда что-то видел/слышал.
	UPROPERTY(BlueprintReadOnly, Category = "Sense")
	float LastStimulusTime = -1e9f;

	// Монстр услышал звук (шаги, падающий предмет). Звук затухает с
	// расстоянием и гасится толстыми стенами.
	UFUNCTION(BlueprintCallable, Category = "Sense|Hearing")
	void ReportNoise(const FVector& SourceLocation, float Loudness);

	// Монстр видит цель своим конусом обзора (line of sight).
	UFUNCTION(BlueprintCallable, Category = "Sense|Vision")
	void UpdateVision(const FVector& PlayerLocation);

	// Тик: распад осведомлённости и т.д.
	UFUNCTION(BlueprintCallable, Category = "Sense")
	void UpdateSenses(float DeltaSeconds);

	// Уровень осведомлённости (для логики ИИ).
	UFUNCTION(BlueprintPure, Category = "Sense")
	EBackroomsAwareness GetAwarenessLevel() const;

	// ИГРОК УЧИТСЯ, ЧТО НЕЛЬЗЯ БЕГАТЬ В ОДНИХ И ТЕХ ЖЕ МЕСТАХ:
	// монстр ПОМНИТ последние места (память) и при возвращении ИГРОКА в
	// уже побыванное место «вспоминает» его и с подозрением осматривает.
	// Метод возвращает, была ли точка уже в памяти НЕДАВНО (т.е. игрок
	// вернулся на знакомое монстру место) — тогда монстр привязчивее.
	UFUNCTION(BlueprintPure, Category = "Sense|Memory")
	bool WasRecentlyRemembered(const FVector& Location, float WithinSeconds) const;

private:
	// Внутренний помощник: добавить/обновить воспоминание (память 3-5 мест).
	void Remember(const FVector& Location, EBackroomsStimulusType Type);
};

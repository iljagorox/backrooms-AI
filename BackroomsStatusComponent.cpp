#include "BackroomsStatusComponent.h"
#include "BackroomsPlayerCharacter.h"
#include "BackroomsItemSystem.h"
#include "BackroomsWorldGenerator.h"
#include "BackroomsRiggedMonster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"

namespace
{
	// Интенсивность ниже этого порога считаем «нет состояния».
	constexpr float KStatusEpsilon = 0.001f;
}

UBackroomsStatusComponent::UBackroomsStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

ABackroomsPlayerCharacter* UBackroomsStatusComponent::GetPlayer() const
{
	return Cast<ABackroomsPlayerCharacter>(GetOwner());
}

ABackroomsWorldGenerator* UBackroomsStatusComponent::FindGenerator() const
{
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ABackroomsWorldGenerator> It(World); It; ++It)
		{
			return *It;
		}
	}
	return nullptr;
}

float UBackroomsStatusComponent::Approach(float Current, float Target, float DeltaTime, float SpeedPerSec)
{
	const float MaxDelta = FMath::Max(0.0f, SpeedPerSec) * DeltaTime;
	if (Target > Current)
	{
		return FMath::Min(Target, Current + MaxDelta);
	}
	return FMath::Max(Target, Current - MaxDelta);
}

void UBackroomsStatusComponent::AddStatus(EBackroomsStatus Status, float Intensity, float Duration)
{
	if (Status == EBackroomsStatus::None)
	{
		return;
	}
	const float Clamped = FMath::Clamp(Intensity, 0.0f, 1.0f);
	if (Duration > 0.0f)
	{
		FBackroomsStatusState State;
		State.Intensity = Clamped;
		State.TimeRemaining = Duration;
		State.bDriven = false;
		Statuses.Add(Status, State);
	}
	else
	{
		FBackroomsStatusState State;
		State.Intensity = Clamped;
		State.bDriven = true;
		Statuses.Add(Status, State);
	}
}

void UBackroomsStatusComponent::RemoveStatus(EBackroomsStatus Status)
{
	Statuses.Remove(Status);
}

float UBackroomsStatusComponent::GetIntensity(EBackroomsStatus Status) const
{
	const FBackroomsStatusState* State = Statuses.Find(Status);
	return State ? State->Intensity : 0.0f;
}

bool UBackroomsStatusComponent::IsStatusActive(EBackroomsStatus Status) const
{
	return GetIntensity(Status) > ActiveThreshold;
}

float UBackroomsStatusComponent::GetStatusTimeRemaining(EBackroomsStatus Status) const
{
	const FBackroomsStatusState* State = Statuses.Find(Status);
	return State ? State->TimeRemaining : 0.0f;
}

void UBackroomsStatusComponent::SetMonsterProximity(float InProximity01)
{
	MonsterProximity = FMath::Clamp(InProximity01, 0.0f, 1.0f);
}

void UBackroomsStatusComponent::NotifyItemUsed(const FBackroomsItemDef& Item)
{
	// Условия восстановления идут по категории предмета, а не по одной кнопке.
	// Лекарства: снимают отравление/облучение и дают защиту от среды на время.
	if (Item.Category == EBackroomsItemCategory::Medicine)
	{
		AddStatus(EBackroomsStatus::Protection, 1.0f, 15.0f);
		RemoveStatus(EBackroomsStatus::Poison);
		RemoveStatus(EBackroomsStatus::Radiation);
		// Медикаменты частично бодрят, но не заменяют отдых.
		Fatigue = FMath::Max(0.0f, Fatigue - 0.35f);
	}
	// Миндальная вода: снимает отравление и заметную долю усталости.
	else if (Item.Category == EBackroomsItemCategory::AlmondWater)
	{
		RemoveStatus(EBackroomsStatus::Poison);
		Fatigue = FMath::Max(0.0f, Fatigue - 0.25f);
	}
	// Энергетик/чай: гонит сонливость (условное «поспать»), но не лечит отраву.
	else if (Item.Category == EBackroomsItemCategory::Energy)
	{
		Fatigue = FMath::Max(0.0f, Fatigue - 0.45f);
	}
	// Восстановление рассудка: снимает уязвимость, даёт защиту, чуть бодрит.
	if (Item.RestoreType == 3)
	{
		RemoveStatus(EBackroomsStatus::Vulnerability);
		AddStatus(EBackroomsStatus::Protection, 1.0f, 8.0f);
		Fatigue = FMath::Max(0.0f, Fatigue - 0.10f);
	}
}

void UBackroomsStatusComponent::UpdateEnvironment(ABackroomsPlayerCharacter* Player, float DeltaTime)
{
	const float Speed = Player ? Player->GetVelocity().Size() : 0.0f;
	const bool bMoving = Speed > 20.0f;
	const bool bSprint = Player ? Player->IsSprinting() : false;

	UBackroomsItemSystem* Items = Player ? Player->GetItemSystem() : nullptr;
	const float Sanity = Items ? Items->Sanity : 100.0f;
	const float MaxSanity = Items ? FMath::Max(1.0f, Items->MaxSanity) : 100.0f;
	const float Health = Items ? Items->Health : 100.0f;

	ABackroomsWorldGenerator* Generator = FindGenerator();
	const int32 Level = Generator ? Generator->LevelIndex : 0;

	// ---- Близость монстра ----
	float Proximity = 0.0f;
	if (Player)
	{
		const FVector PlayerLoc = Player->GetActorLocation();
		float Nearest = TNumericLimits<float>::Max();
		for (TActorIterator<ABackroomsRiggedMonster> It(GetWorld()); It; ++It)
		{
			Nearest = FMath::Min(Nearest, FVector::Dist(PlayerLoc, It->GetActorLocation()));
		}
		if (Nearest < TNumericLimits<float>::Max())
		{
			Proximity = FMath::Clamp(1.0f - Nearest / FMath::Max(1.0f, MonsterPanicDistance), 0.0f, 1.0f);
		}
	}
	MonsterProximity = FMath::Max(MonsterProximity - DeltaTime * 0.5f, Proximity);

	auto Drive = [this](EBackroomsStatus Status, float Target)
	{
		const float Clamped = FMath::Clamp(Target, 0.0f, 1.0f);
		if (Clamped <= KStatusEpsilon)
		{
			Statuses.Remove(Status);
			return;
		}
		FBackroomsStatusState& State = Statuses.FindOrAdd(Status);
		State.Intensity = Clamped;
		State.bDriven = true;
	};

	// ---- Перегрев: бег + электрические/силовые уровни (L3 Power) ----
	SprintHeat = Approach(SprintHeat, (bSprint && bMoving) ? 1.0f : 0.0f,
		DeltaTime, (bSprint && bMoving) ? 0.14f : 0.28f);
	const float HeatAmbient = (Level == 3) ? 0.35f : 0.0f;
	Drive(EBackroomsStatus::Overheating, SprintHeat * 0.9f + HeatAmbient);

	// ---- Влажность: водопровод (L2 Pipes) и океан (L7 Ocean) ----
	const bool bWetLevel = (Level == 2 || Level == 7);
	Drive(EBackroomsStatus::Wet, bWetLevel ? 0.45f : 0.0f);

	// ---- Радиация: силовой (L3) и больница (L9) — пульсирующий фон ----
	const bool bRadLevel = (Level == 3 || Level == 9);
	if (bRadLevel)
	{
		const float T = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		RadiationPulse = 0.55f + 0.45f * FMath::Sin(T * 0.35f);
	}
	else
	{
		RadiationPulse = 0.0f;
	}
	Drive(EBackroomsStatus::Radiation, RadiationPulse);

	// ---- Отравление: больница (L9) — споры в воздухе ----
	if (Level == 9)
	{
		const float T = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		PoisonPulse = 0.5f + 0.5f * FMath::Sin(T * 0.22f + 1.3f);
	}
	else
	{
		PoisonPulse = 0.0f;
	}
	Drive(EBackroomsStatus::Poison, PoisonPulse * 0.8f);

	// ---- Усталость/сонливость ----
	// Копится в основном от БЕГА (ты устаёшь, потому что бегаешь), а не просто
	// от времени. Режима сна в игре нет, поэтому «поспать» заменяет отдых на
	// месте: стоишь спокойно — усталость медленно уходит. Низкий рассудок
	// ускоряет упадок сил.
	const float LowSanity01 = (Sanity < LowSanityThreshold)
		? (LowSanityThreshold - Sanity) / FMath::Max(1.0f, LowSanityThreshold)
		: 0.0f;

	// Скорость накопления: бег даёт основной вклад, ходьба — слабый, стояние 0.
	const bool bRunning = bSprint && bMoving;
	const float AccumRate = (bRunning ? 0.055f : (bMoving ? 0.012f : 0.004f))
		+ LowSanity01 * 0.030f;
	Fatigue = FMath::Clamp(Fatigue + AccumRate * DeltaTime, 0.0f, 1.0f);

	// Восстановление: только когда игрок реально стоит. Чем дольше стоит, тем
	// быстрее «отходит» (до 3x) — но полностью выспаться на ходу нельзя.
	if (!bMoving)
	{
		RestTimer += DeltaTime;
		const float RestBoost = 1.0f + FMath::Min(RestTimer, 4.0f) * 0.5f;
		Fatigue = FMath::Max(0.0f, Fatigue - 0.020f * RestBoost * DeltaTime);
	}
	else
	{
		RestTimer = 0.0f;
	}
	Drive(EBackroomsStatus::Sleepiness, Fatigue);

	// ---- Запах: кровь при низком здоровье + грязь/испарения ----
	float OdorTarget = (Health < 60.0f) ? (60.0f - Health) / 60.0f * 0.85f : 0.0f;
	OdorTarget += GetIntensity(EBackroomsStatus::Wet) * 0.20f;
	OdorTarget += GetIntensity(EBackroomsStatus::Poison) * 0.35f;
	Drive(EBackroomsStatus::Odor, OdorTarget);

	// ---- Адреналин: монстр рядом / мало здоровья / паника рассудка ----
	float AdrenalineTarget = MonsterProximity;
	if (Health < 30.0f)
	{
		AdrenalineTarget = FMath::Max(AdrenalineTarget, (30.0f - Health) / 30.0f);
	}
	if (Sanity < LowSanityThreshold)
	{
		AdrenalineTarget = FMath::Max(AdrenalineTarget, 0.4f);
	}
	Drive(EBackroomsStatus::Adrenaline, AdrenalineTarget);

	// ---- Защита: выдаётся предметами как timed-состояние (NotifyItemUsed),
	// поэтому среда её НЕ трогает — иначе перезаписывала бы таймер каждый тик.

	// ---- Уязвимость: рассудок ниже порога ----
	Drive(EBackroomsStatus::Vulnerability, LowSanity01);

	// ---- Замедление: суммарный «груз» среды ----
	const float SlowTarget =
		GetIntensity(EBackroomsStatus::Wet) * 0.5f +
		GetIntensity(EBackroomsStatus::Sleepiness) * 0.6f +
		GetIntensity(EBackroomsStatus::Overheating) * 0.4f +
		GetIntensity(EBackroomsStatus::Poison) * 0.5f +
		GetIntensity(EBackroomsStatus::Radiation) * 0.3f;
	Drive(EBackroomsStatus::Slowing, SlowTarget);

	// ---- Ускорение: адреналиновый подъём ----
	const float Adrenaline = GetIntensity(EBackroomsStatus::Adrenaline);
	Drive(EBackroomsStatus::Acceleration, Adrenaline > 0.35f ? Adrenaline : 0.0f);

	// Плавное «остывание» пульсов на уровнях без угрозы уже задано Drive(...,0).

	// ---- Пассивное восстановление рассудка в безопасной зоне ----
	// «Безопасно» = рядом нет монстра и нет активных угроз (радиация/отравление/
	// перегрев). Стоять спокойно восстанавливает быстрее, чем на ходу.
	if (Items)
	{
		const bool bThreat = MonsterProximity > 0.1f
			|| GetIntensity(EBackroomsStatus::Radiation) > ActiveThreshold
			|| GetIntensity(EBackroomsStatus::Poison) > ActiveThreshold
			|| GetIntensity(EBackroomsStatus::Overheating) > ActiveThreshold;
		const float BaseRegen = bMoving ? 0.5f : 1.2f;
		Items->SanityRegenPerSecond = bThreat ? 0.0f : BaseRegen;
	}
}

void UBackroomsStatusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ABackroomsPlayerCharacter* Player = GetPlayer();

	if (bAutoEnvironment && Player)
	{
		UpdateEnvironment(Player, DeltaTime);
	}
	else
	{
		// Без автосбора все «ведомые» состояния плавно угасают.
		for (auto It = Statuses.CreateIterator(); It; ++It)
		{
			if (It->Value.bDriven)
			{
				It->Value.Intensity = Approach(It->Value.Intensity, 0.0f, DeltaTime, 0.5f);
				if (It->Value.Intensity <= KStatusEpsilon)
				{
					It.RemoveCurrent();
				}
			}
		}
	}

	// Состояния с явным сроком (лекарства и т.п.) тикают и снимаются по времени.
	for (auto It = Statuses.CreateIterator(); It; ++It)
	{
		if (!It->Value.bDriven)
		{
			It->Value.TimeRemaining -= DeltaTime;
			if (It->Value.TimeRemaining <= 0.0f)
			{
				It.RemoveCurrent();
			}
		}
	}
}

float UBackroomsStatusComponent::GetSpeedMultiplier() const
{
	float M = 1.0f;
	M *= 1.0f - 0.35f * GetIntensity(EBackroomsStatus::Slowing);
	M *= 1.0f - 0.25f * GetIntensity(EBackroomsStatus::Wet);
	// Усталость лишь слегка замедляет (сон не реализован как механика — нельзя
	// «выспаться», можно только отдышаться), поэтому вклад небольшой.
	M *= 1.0f - 0.12f * GetIntensity(EBackroomsStatus::Sleepiness);
	M *= 1.0f - 0.20f * GetIntensity(EBackroomsStatus::Overheating);
	M *= 1.0f - 0.25f * GetIntensity(EBackroomsStatus::Poison);
	M *= 1.0f + 0.22f * GetIntensity(EBackroomsStatus::Acceleration);
	M *= 1.0f + 0.15f * GetIntensity(EBackroomsStatus::Adrenaline);
	return FMath::Clamp(M, 0.35f, 1.6f);
}

float UBackroomsStatusComponent::GetSanityDrainMultiplier() const
{
	float M = 1.0f;
	M += 0.80f * GetIntensity(EBackroomsStatus::Vulnerability);
	M += 0.50f * GetIntensity(EBackroomsStatus::Overheating);
	M += 0.50f * GetIntensity(EBackroomsStatus::Radiation);
	M += 0.30f * GetIntensity(EBackroomsStatus::Sleepiness);
	M *= 1.0f - 0.60f * GetIntensity(EBackroomsStatus::Protection);
	return FMath::Clamp(M, 0.10f, 4.0f);
}

float UBackroomsStatusComponent::GetExtraSanityDrainPerSecond() const
{
	return GetIntensity(EBackroomsStatus::Radiation) * 0.40f
		+ GetIntensity(EBackroomsStatus::Poison) * 0.30f;
}

float UBackroomsStatusComponent::GetHealthDrainPerSecond() const
{
	return GetIntensity(EBackroomsStatus::Poison) * 0.80f
		+ GetIntensity(EBackroomsStatus::Radiation) * 0.40f
		+ GetIntensity(EBackroomsStatus::Overheating) * 0.20f;
}

float UBackroomsStatusComponent::GetNoiseLoudness() const
{
	float Noise = GetIntensity(EBackroomsStatus::Noise);
	Noise = FMath::Max(Noise, GetIntensity(EBackroomsStatus::Odor) * 0.3f);
	return FMath::Clamp(Noise, 0.0f, 1.0f);
}

float UBackroomsStatusComponent::GetCameraDrift() const
{
	const float Drift =
		GetIntensity(EBackroomsStatus::Sleepiness) * 0.6f +
		GetIntensity(EBackroomsStatus::Poison) * 0.4f +
		GetIntensity(EBackroomsStatus::Radiation) * 0.4f +
		GetIntensity(EBackroomsStatus::Vulnerability) * 0.2f;
	return FMath::Clamp(Drift, 0.0f, 1.0f);
}

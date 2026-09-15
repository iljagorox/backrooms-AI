#include "BackroomsSenseComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

namespace BackroomsSenseLOS
{
	// Р•СЃС‚СЊ Р»Рё РїСЂСЏРјР°СЏ РІРёРґРёРјРѕСЃС‚СЊ/СЃР»С‹С€РёРјРѕСЃС‚СЊ РґРѕ С‚РѕС‡РєРё (РЅРµ РїРµСЂРµРєСЂС‹С‚Рѕ РїР»РѕС‚РЅС‹РјРё СЃС‚РµРЅР°РјРё).
	bool To(const UBackroomsSenseComponent* Self, const FVector& From, const FVector& To)
	{
		UWorld* World = Self->GetWorld();
		if (!World)
		{
			return true; // Р±РµР· РјРёСЂР° вЂ” СЃС‡РёС‚Р°РµРј С‡С‚Рѕ СЃР»С‹С€РЅРѕ/РІРёРґРЅРѕ
		}
		FCollisionQueryParams Params(SCENE_QUERY_STAT(BackroomsSenseLOS), false, Self->GetOwner());
		FHitResult Hit;
		// Р‘Р»РѕРєРёСЂСѓСЋС‚ ECC_WorldStatic / PhysicsBody (РїРѕР»С‹, СЃС‚РµРЅС‹, РјРµР±РµР»СЊ).
		const bool bBlocked = World->LineTraceSingleByChannel(Hit, From, To, ECC_WorldStatic, Params);
		return !bBlocked;
	}
}

UBackroomsSenseComponent::UBackroomsSenseComponent()
{
	// РЎРµРЅСЃРѕСЂ РЅРµ С‚СЂРµР±СѓРµС‚ РѕС‚РґРµР»СЊРЅРѕРіРѕ С‚РёРєР° вЂ” UpdateSenses РІС‹Р·С‹РІР°РµС‚СЃСЏ РёР· РІР»Р°РґРµР»СЊС†Р°
	// (РјРѕРЅСЃС‚СЂР°), С‡С‚РѕР±С‹ СѓРїСЂР°РІР»СЏС‚СЊ РїРѕСЂСЏРґРєРѕРј РѕР±РЅРѕРІР»РµРЅРёСЏ.
	PrimaryComponentTick.bCanEverTick = false;
}

void UBackroomsSenseComponent::ReportNoise(const FVector& SourceLocation, float Loudness)
{
	if (Loudness < MinHeardLoudness)
	{
		return; // С‚РёС€Рµ РїРѕСЂРѕРіР° вЂ” РЅРµ СЃР»С‹С€РёРј
	}

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector SensorLoc = Owner->GetActorLocation();
	const float Dist = FVector::Dist(SourceLocation, SensorLoc);
	if (Dist > HearingRange)
	{
		return; // СЃР»РёС€РєРѕРј РґР°Р»РµРєРѕ
	}

	// Р—Р°С‚СѓС…Р°РЅРёРµ РїРѕ СЂР°СЃСЃС‚РѕСЏРЅРёСЋ (Р»РёРЅРµР№РЅРѕ), СѓРјРЅРѕР¶РµРЅРЅРѕРµ РЅР° РіСЂРѕРјРєРѕСЃС‚СЊ.
	const float DistanceFactor = 1.0f - Dist / HearingRange;
	float Gain = Loudness * DistanceFactor * SoundGain;

	// Р“Р»СѓС€РёРј С‚РѕР»СЃС‚С‹РјРё СЃС‚РµРЅР°РјРё: РµСЃР»Рё РјРµР¶РґСѓ РёСЃС‚РѕС‡РЅРёРєРѕРј Рё РјРѕРЅСЃС‚СЂРѕРј РµСЃС‚СЊ РїРµСЂРµРіРѕСЂРѕРґРєР°
	// (РїРѕР»/СЃС‚РµРЅР°/РјРµР±РµР»СЊ), Р·РІСѓРє СЃР»С‹С€РµРЅ Р·Р°РјРµС‚РЅРѕ С‚РёС€Рµ.
	if (!BackroomsSenseLOS::To(this, SensorLoc, SourceLocation))
	{
		Gain *= 0.25f;
	}

	Awareness = FMath::Clamp(Awareness + Gain, 0.0f, 1.0f);
	LastKnownPlayerLocation = SourceLocation;
	LastStimulusTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Remember(SourceLocation, EBackroomsStimulusType::Sound);
}

void UBackroomsSenseComponent::UpdateVision(const FVector& PlayerLocation)
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector SensorLoc = Owner->GetActorLocation();
	const FVector ToPlayer = PlayerLocation - SensorLoc;
	const float Dist = ToPlayer.Size();
	if (Dist > VisionRange)
	{
		return; // РґР°Р»СЊС€Рµ Р·СЂРµРЅРёСЏ
	}

	// РЈРіРѕР» РІР·РіР»СЏРґР° РѕС‚РЅРѕСЃРёС‚РµР»СЊРЅРѕ РЅР°РїСЂР°РІР»РµРЅРёСЏ, РєСѓРґР° СЃРјРѕС‚СЂРёС‚ РјРѕРЅСЃС‚СЂ.
	const FVector Forward = Owner->GetActorForwardVector();
	const FVector Dir = ToPlayer.GetSafeNormal();
	const float Dot = FVector::DotProduct(Forward, Dir);
	const float CosFOV = FMath::Cos(FMath::DegreesToRadians(VisionFOVDegrees * 0.5f));
	const float CosPeriph = FMath::Cos(FMath::DegreesToRadians(PeripheralFOVDegrees * 0.5f));

	// РџСЂСЏРјРѕРµ Р·СЂРµРЅРёРµ (С†РµРЅС‚СЂ РєРѕРЅСѓСЃР°) вЂ” СЃРёР»СЊРЅРµРµ.
	const bool bInFOV = Dot >= CosFOV;
	// РџРµСЂРёС„РµСЂРёР№РЅРѕРµ (РєСЂР°РµРј РіР»Р°Р·Р°): РјРµРЅСЊС€Рµ, РЅРѕ Р±Р»РёР·РєРѕ СЃСЂР°Р±Р°С‚С‹РІР°РµС‚.
	const bool bInPeriph = Dot >= CosPeriph;

	float VisualGain = 0.0f;
	if (bInFOV)
	{
		VisualGain = FMath::Clamp(0.6f + (1.0f - Dist / VisionRange) * 0.4f, 0.0f, 1.0f);
	}
	else if (bInPeriph && Dist < VisionRange * 0.6f)
	{
		VisualGain = 0.15f * (1.0f - Dist / (VisionRange * 0.6f));
	}
	else
	{
		return; // РІРЅРµ СѓРіР»Р° РѕР±Р·РѕСЂР° вЂ” РЅРµ РІРёРґРёС‚
	}

	// Р—Р° СЃС‚РµРЅРѕР№ РјРѕРЅСЃС‚СЂ РќР• РІРёРґРёС‚ (С‡РµР»РѕРІРµС‡РµСЃРєРѕРµ Р·СЂРµРЅРёРµ РЅРµ СЃРєРІРѕР·СЊ СЃС‚РµРЅС‹).
	if (!BackroomsSenseLOS::To(this, SensorLoc, PlayerLocation))
	{
		return;
	}

	Awareness = FMath::Clamp(Awareness + VisualGain, 0.0f, 1.0f);
	LastKnownPlayerLocation = PlayerLocation;
	LastStimulusTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Remember(PlayerLocation, EBackroomsStimulusType::Sight);
}

void UBackroomsSenseComponent::UpdateSenses(float DeltaSeconds)
{
	// Осведомлённость постепенно спадает (монстр «забывает» — не стоит вечно
	// в режиме тревоги, что тоже убирает скриптованность).
	Awareness = FMath::Max(0.0f, Awareness - AwarenessDecay * DeltaSeconds);
}

EBackroomsAwareness UBackroomsSenseComponent::GetAwarenessLevel() const
{
	if (Awareness >= 0.6f)
	{
		return EBackroomsAwareness::Alerted;
	}
	if (Awareness >= 0.2f)
	{
		return EBackroomsAwareness::Suspecting;
	}
	return EBackroomsAwareness::Unaware;
}

void UBackroomsSenseComponent::Remember(const FVector& Location, EBackroomsStimulusType Type)
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Если это место уже знаем — просто обновляем время (переоткрываем), чтобы
	// монстр понял: игрок ВЕРНУЛСЯ на старое место. Игрок учится не бегать
	// по одним и тем же точкам.
	const float MergeDistance = 250.0f;
	for (FBackroomsSenseMemory& M : Memory)
	{
		if (FVector::DistSquared(M.Location, Location) <= MergeDistance * MergeDistance)
		{
			M.Location = Location;
			M.Type = Type;
			M.Time = Now;
			return;
		}
	}

	// Новое место — добавляем, но держим память короткой (3-5 мест).
	FBackroomsSenseMemory NewM;
	NewM.Location = Location;
	NewM.Type = Type;
	NewM.Time = Now;
	Memory.Add(NewM);

	while (Memory.Num() > MaxMemory)
	{
		// Удаляем самое старое воспоминание (память как у живого существа).
		int32 OldestIdx = 0;
		for (int32 i = 1; i < Memory.Num(); ++i)
		{
			if (Memory[i].Time < Memory[OldestIdx].Time)
			{
				OldestIdx = i;
			}
		}
		Memory.RemoveAt(OldestIdx);
	}
}

bool UBackroomsSenseComponent::WasRecentlyRemembered(const FVector& Location, float WithinSeconds) const
{
	if (!GetWorld())
	{
		return false;
	}
	const float Now = GetWorld()->GetTimeSeconds();
	const float MergeDistance = 250.0f;
	for (const FBackroomsSenseMemory& M : Memory)
	{
		const bool bNear = FVector::DistSquared(M.Location, Location) <= MergeDistance * MergeDistance;
		const bool bRecent = (Now - M.Time) <= WithinSeconds;
		if (bNear && bRecent)
		{
			return true;
		}
	}
	return false;
}

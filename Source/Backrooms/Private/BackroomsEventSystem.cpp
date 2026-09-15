#include "BackroomsEventSystem.h"
#include "BackroomsLocalization.h"
#include "BackroomsWorldGenerator.h"
#include "BackroomsChunkActor.h"
#include "BackroomsNoise.h"
#include "BackroomsNoiseMarker.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

void UBackroomsEventSystem::InitializeDefaultEvents()
{
	Events.Empty();

	// Мерцание света — РЕДКОЕ (саспенс-событие, а не «мигалка 24/7»):
	// большой интервал и низкая вероятность — только для короткого удара.
	FBackroomsEvent Flicker;
	Flicker.Type = EBackroomsEventType::LightFlicker;
	Flicker.MinInterval = 240.0f;
	Flicker.MaxInterval = 900.0f;
	Flicker.Duration = 0.8f;
	Flicker.Radius = 1200.0f;
	Flicker.Probability = 0.06f;
	Events.Add(Flicker);

	// Погас свет — очень редко.
	FBackroomsEvent Outage;
	Outage.Type = EBackroomsEventType::LightOutage;
	Outage.MinInterval = 60.0f;
	Outage.MaxInterval = 180.0f;
	Outage.Duration = 3.0f;
	Outage.Radius = 2000.0f;
	Outage.Probability = 0.08f;
	Events.Add(Outage);

	// Ящик исчез — только при тьме.
	FBackroomsEvent BoxGone;
	BoxGone.Type = EBackroomsEventType::BoxDisappear;
	BoxGone.MinInterval = 40.0f;
	BoxGone.MaxInterval = 120.0f;
	BoxGone.Duration = 0.0f;
	BoxGone.Radius = 1500.0f;
	BoxGone.Probability = 0.05f;
	Events.Add(BoxGone);

	// Ящик появился — очень редко.
	FBackroomsEvent BoxNew;
	BoxNew.Type = EBackroomsEventType::BoxAppear;
	BoxNew.MinInterval = 60.0f;
	BoxNew.MaxInterval = 200.0f;
	BoxNew.Duration = 0.0f;
	BoxNew.Radius = 1500.0f;
	BoxNew.Probability = 0.03f;
	Events.Add(BoxNew);

	// Рык сущности — еле слышно, далеко.
	FBackroomsEvent Growl;
	Growl.Type = EBackroomsEventType::EntityGrowl;
	Growl.MinInterval = 90.0f;
	Growl.MaxInterval = 300.0f;
	Growl.Duration = 1.5f;
	Growl.Radius = 3000.0f;
	Growl.Probability = 0.06f;
	Growl.Volume = 0.15f;
	Events.Add(Growl);

	// Шаги — редко, тихо.
	FBackroomsEvent Steps;
	Steps.Type = EBackroomsEventType::EntityFootsteps;
	Steps.MinInterval = 120.0f;
	Steps.MaxInterval = 360.0f;
	Steps.Duration = 2.0f;
	Steps.Radius = 2500.0f;
	Steps.Probability = 0.04f;
	Steps.Volume = 0.1f;
	Events.Add(Steps);

	// Шёпот — очень редко.
	FBackroomsEvent Whisper;
	Whisper.Type = EBackroomsEventType::Whisper;
	Whisper.MinInterval = 150.0f;
	Whisper.MaxInterval = 400.0f;
	Whisper.Duration = 1.5f;
	Whisper.Radius = 800.0f;
	Whisper.Probability = 0.02f;
	Whisper.Volume = 0.08f;
	Events.Add(Whisper);

	// Грохот издалека.
	FBackroomsEvent Bang;
	Bang.Type = EBackroomsEventType::DistantBang;
	Bang.MinInterval = 100.0f;
	Bang.MaxInterval = 300.0f;
	Bang.Duration = 0.8f;
	Bang.Radius = 4000.0f;
	Bang.Probability = 0.05f;
	Bang.Volume = 0.25f;
	Events.Add(Bang);

	// Рисунок на стене — еле заметный.
	FBackroomsEvent Drawing;
	Drawing.Type = EBackroomsEventType::WallDrawing;
	Drawing.MinInterval = 180.0f;
	Drawing.MaxInterval = 600.0f;
	Drawing.Duration = 3.0f;
	Drawing.Radius = 600.0f;
	Drawing.Probability = 0.02f;
	Events.Add(Drawing);

	// Скрип трубы — иногда.
	FBackroomsEvent Creak;
	Creak.Type = EBackroomsEventType::PipeCreak;
	Creak.MinInterval = 30.0f;
	Creak.MaxInterval = 90.0f;
	Creak.Duration = 0.6f;
	Creak.Radius = 1500.0f;
	Creak.Probability = 0.1f;
	Creak.Volume = 0.2f;
	Events.Add(Creak);

	// Хлопок двери — редко.
	FBackroomsEvent Slam;
	Slam.Type = EBackroomsEventType::DoorSlam;
	Slam.MinInterval = 60.0f;
	Slam.MaxInterval = 180.0f;
	Slam.Duration = 0.3f;
	Slam.Radius = 2000.0f;
	Slam.Probability = 0.06f;
	Slam.Volume = 0.3f;
	Events.Add(Slam);

	// Аварийный свет — редко.
	FBackroomsEvent Emergency;
	Emergency.Type = EBackroomsEventType::EmergencyLight;
	Emergency.MinInterval = 90.0f;
	Emergency.MaxInterval = 240.0f;
	Emergency.Duration = 4.0f;
	Emergency.Radius = 0.0f;
	Emergency.Probability = 0.04f;
	Events.Add(Emergency);

	// Туман — редко.
	FBackroomsEvent Fog;
	Fog.Type = EBackroomsEventType::FogIncrease;
	Fog.MinInterval = 120.0f;
	Fog.MaxInterval = 400.0f;
	Fog.Duration = 8.0f;
	Fog.Radius = 0.0f;
	Fog.Probability = 0.03f;
	Events.Add(Fog);

	// Помехи — редко.
	FBackroomsEvent Static;
	Static.Type = EBackroomsEventType::StaticNoise;
	Static.MinInterval = 60.0f;
	Static.MaxInterval = 180.0f;
	Static.Duration = 1.5f;
	Static.Radius = 0.0f;
	Static.Probability = 0.04f;
	Events.Add(Static);

	// Следы — очень редко.
	FBackroomsEvent Footprints;
	Footprints.Type = EBackroomsEventType::FootprintAppear;
	Footprints.MinInterval = 200.0f;
	Footprints.MaxInterval = 500.0f;
	Footprints.Duration = 10.0f;
	Footprints.Radius = 800.0f;
	Footprints.Probability = 0.02f;
	Events.Add(Footprints);

	// Инициализация таймеров пуассоновским интервалом ожидания.
	for (const FBackroomsEvent& E : Events)
	{
		Timers.Add(E.Type, SamplePoissonInterval(E));
	}

	// Пул цепочек (последовательностей) событий по умолчанию.
	BuildDefaultSequences();

	// Колода для раздачи цепочек — инициализируется по числу последовательностей.
	SequenceBag.Init(Sequences.Num());
	ChainQueue.Reset();
	// Долгая стартовая пауза: пока идёт создание уровня (LoadingBar) и игрок
	// привыкает к миру — никаких скачков. Первый саспенс-удар не раньше 2 минут.
	ChainCooldown = 120.0f;
}

// Экспоненциальный интервал между событиями (пуассоновский процесс):
//   P(>t) = e^(-lambda*t) — память-свободное распределение.
// Это рождает естественное «кластерное» срабатывание: то несколько событий
// подряд, то долгое затишье. В отличие от равномерного FMath::RandRange,
// которое даёт скучный ритм «по расписанию».
float UBackroomsEventSystem::SamplePoissonInterval(const FBackroomsEvent& Event) const
{
	// Средний интервал = середина диапазона; lambda = 1/mean.
	const float Mean = FMath::Max(0.5f, 0.5f * (Event.MinInterval + Event.MaxInterval));
	const float Lambda = 1.0f / Mean;
	// Обратное преобразование: t = -ln(1-u)/lambda. ln(0) не бывает, т.к. FRand < 1.
	const float U = FMath::Clamp(FMath::FRand(), 0.00001f, 0.99999f);
	const float Interval = -FMath::Loge(1.0f - U) / Lambda;
	// Не даём уйти в слишком короткие/длинные хвосты относительно конфигурации.
	return FMath::Clamp(Interval, Event.MinInterval * 0.5f, Event.MaxInterval * 2.0f);
}

void UBackroomsEventSystem::TickEvents(float DeltaSeconds, ABackroomsWorldGenerator* Generator)
{
	if (!bEnabled || !Generator)
	{
		return;
	}

	const float WorldTime = Generator->GetWorld()->GetTimeSeconds();

	// Плавное подтягивание «тревоги» к целевому значению. Направление и темп
	// модулируются TemporalNoise (шум во времени), чтобы веса не щёлкали
	// на пороге, а «дышали» — убирает скриптованность.
	const float Noise = BackroomsNoise::TemporalNoise(WorldTime, 0.08f, 1.3f, Seed);
	CurrentDistress = FMath::FInterpTo(CurrentDistress, TargetDistress,
		DeltaSeconds, 0.4f + Noise * 0.8f);

	// Обновление активных эффектов.
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		ActiveEffects[i].TimeRemaining -= DeltaSeconds;
		if (ActiveEffects[i].TimeRemaining <= 0.0f)
		{
			ActiveEffects.RemoveAt(i);
		}
	}

	// Продвигаем текущую цепочку (последовательность) событий.
	AdvanceChain(DeltaSeconds, Generator);

	// Атмосфера, связанная с приходом монстра (ИССЛЕДОВАНИЕ -> шёпоты,
	// МОНСТР РЯДОМ -> тихие ненавязчивые звуки).
	TickMonsterAtmosphere(DeltaSeconds, Generator);

	// Обновление «залпа» (кластера) одиночных событий.
	if (BurstRemaining > 0)
	{
		for (const FBackroomsEvent& E : Events)
		{
			if (E.Type == BurstType)
			{
				BurstTimer -= DeltaSeconds;
				if (BurstTimer <= 0.0f)
				{
					ExecuteEvent(E, Generator);
					--BurstRemaining;
					// Короткий интервал — чтобы игрок ощутил «пачку».
					BurstTimer = FMath::FRandRange(0.4f, 1.5f);
				}
				break;
			}
		}
	}

	// Фоновые одиночные события (пуассоновский интервал ожидания).
	for (FBackroomsEvent& Event : Events)
	{
		float* Timer = Timers.Find(Event.Type);
		if (!Timer)
		{
			continue;
		}

		*Timer -= DeltaSeconds;
		if (*Timer > 0.0f)
		{
			continue;
		}

		// Событие созрело: сэмплируем следующий пуассоновский интервал.
		*Timer = SamplePoissonInterval(Event);

		// Вероятность появления, откорректированная состоянием игрока:
		// низкий рассудок повышает шанс страшных событий (веса меняются).
		const float AdjProb = FMath::Clamp(
			Event.Probability * FMath::Lerp(1.0f, Event.FearWeight, CurrentDistress),
			0.0f, 1.0f);
		if (FMath::FRand() >= AdjProb)
		{
			continue;
		}

		ExecuteEvent(Event, Generator);

		// Кластер: c вероятностью BurstChance — серия событий подряд.
		if (Event.MaxBurstFollowups > 0 && FMath::FRand() < Event.BurstChance)
		{
			BurstRemaining = Event.MaxBurstFollowups;
			BurstTimer = FMath::FRandRange(0.5f, 1.1f);
			BurstType = Event.Type;
		}
	}

	// Запуск цепочек: если текущая цепочка кончилась и прошла пауза,
	// раздаём следующую из «мешка» последовательностей (с учётом состояния).
	if (ChainQueue.Num() == 0)
	{
		ChainCooldown -= DeltaSeconds;
		if (ChainCooldown <= 0.0f)
		{
			if (const FBackroomsEventSequence* Seq = PickNextSequence(WorldTime))
			{
				QueueSequence(*Seq, Generator);
				// Редко: цепочки не должны «мигать светом 24/7». Между цепочками
				// держим длинную паузу — саспенс дозируется.
				ChainCooldown = FMath::FRandRange(120.0f, 240.0f);
			}
		}
	}
}

void UBackroomsEventSystem::SetPlayerStats(float InSanity, float InMaxSanity)
{
	const float MaxS = FMath::Max(1.0f, InMaxSanity);
	// Тревога растёт при падении рассудка (и ужастик «включается»).
	TargetDistress = FMath::Clamp(1.0f - InSanity / MaxS, 0.0f, 1.0f);
}

float UBackroomsEventSystem::GetDistress() const
{
	return CurrentDistress;
}

void UBackroomsEventSystem::SetMonsterPresence(bool bInvestigating, const FVector& InMonsterLocation, float DistToPlayer)
{
	bMonsterInvestigating = bInvestigating;
	MonsterLocation = InMonsterLocation;
	// Близость: 0 = далеко, 1 = вплотную (линейно по дистанции ~1200 см).
	MonsterProximity = FMath::Clamp(1.0f - DistToPlayer / 1200.0f, 0.0f, 1.0f);
}

// Атмосфера, привязанная к приходу монстра. Реализация полностью в C++,
// чтобы логика была «живой»:
//   * если монстр ИССЛЕДУЕТ (ищет игрока) — в локации звучат ШЁПОТЫ,
//     интервал «дышит» шумом, а сам звук смещается случайным Перлином;
//   * если монстр РЯДОМ — тихие, ненавязчивые звуки (почти на грани слышимости)
//     с тактом, зависящим от близости. Игрок чувствует давление, но это
//     не откровенный скрим.
void UBackroomsEventSystem::TickMonsterAtmosphere(float DeltaSeconds, ABackroomsWorldGenerator* Generator)
{
	if (!Generator)
	{
		return;
	}
	UWorld* World = Generator->GetWorld();
	const float WorldTime = World ? World->GetTimeSeconds() : 0.0f;

	// 1) РЕЖИМ РАССЛЕДОВАНИЯ: шёпоты.
	if (bMonsterInvestigating)
	{
		WhisperTimer -= DeltaSeconds;
		// Интервал шёпотов «дышит» временным шумом — не по расписанию.
		const float Baseline = 1.5f + BackroomsNoise::TemporalNoise(WorldTime, 4.0f, 0.0f, Seed ^ 0x11) * 3.0f;
		if (WhisperTimer <= 0.0f)
		{
			// Звук слегка «блуждает» 3D-Перлином вокруг монстра (страшнее).
			const FVector SoundLoc = MonsterLocation + FVector(
				BackroomsNoise::Perlin3D(WorldTime * 0.3f, 1.7f, 0.0f, Seed) * 320.0f,
				BackroomsNoise::Perlin3D(0.5f, WorldTime * 0.3f, 2.2f, Seed) * 320.0f,
				0.0f);
			const float Vol = FMath::Clamp(0.25f + MonsterProximity * 0.35f, 0.0f, 0.6f);
			ApplyWhisper(Generator, SoundLoc, Vol);
			WhisperTimer = Baseline;
		}
	}

	// 2) МОНСТР РЯДОМ: тихие ненавязчивые звуки.
	if (MonsterProximity > 0.2f)
	{
		MonsterSfxTimer -= DeltaSeconds;
		const float Takt = FMath::Lerp(6.0f, 2.5f, MonsterProximity)
			+ BackroomsNoise::TemporalNoise(WorldTime, 0.5f, 9.0f, Seed ^ 0x77) * 2.0f;
		if (MonsterSfxTimer <= 0.0f)
		{
			// Тихий шёпот/дыхание: почти на грани слышимости, но давит.
			const float Vol = FMath::Clamp(0.04f + MonsterProximity * 0.2f, 0.0f, 0.28f);
			ApplyWhisper(Generator, MonsterLocation, Vol);
			MonsterSfxTimer = Takt;
		}
	}
}

const FBackroomsEvent* UBackroomsEventSystem::FindEventConfig(EBackroomsEventType Type) const
{
	for (const FBackroomsEvent& E : Events)
	{
		if (E.Type == Type)
		{
			return &E;
		}
	}
	return nullptr;
}

void UBackroomsEventSystem::BuildDefaultSequences()
{
	Sequences.Empty();

	// 1) Тихая сцена: скрип -> шёпот. Без мерцания — фон не должен мигать.
	{
		FBackroomsEventSequence Seq;
		Seq.Id = TEXT("AmbientCreep");
		Seq.Weight = 1.0f;
		Seq.FearWeight = 0.45f;
		Seq.Steps = { EBackroomsEventType::PipeCreak, EBackroomsEventType::Whisper };
		Seq.StepIntervalMin = 0.8f;
		Seq.StepIntervalMax = 2.0f;
		Sequences.Add(Seq);
	}

	// 2) Запертый: хлопок двери -> мерцание -> помехи.
	{
		FBackroomsEventSequence Seq;
		Seq.Id = TEXT("LockedIn");
		Seq.Weight = 0.8f;
		Seq.FearWeight = 0.7f;
		Seq.Steps = { EBackroomsEventType::DoorSlam, EBackroomsEventType::LightFlicker, EBackroomsEventType::StaticNoise };
		Seq.StepIntervalMin = 0.5f;
		Seq.StepIntervalMax = 1.4f;
		Sequences.Add(Seq);
	}

	// 3) Погоня-нарастание: грохот -> шаги -> рык -> погас свет.
	{
		FBackroomsEventSequence Seq;
		Seq.Id = TEXT("Crescendo");
		Seq.Weight = 0.5f;
		Seq.FearWeight = 1.0f;
		Seq.Steps = { EBackroomsEventType::DistantBang, EBackroomsEventType::EntityFootsteps,
			EBackroomsEventType::EntityGrowl, EBackroomsEventType::LightOutage };
		Seq.StepIntervalMin = 0.6f;
		Seq.StepIntervalMax = 1.2f;
		Sequences.Add(Seq);
	}
}

const FBackroomsEventSequence* UBackroomsEventSystem::PickNextSequence(float WorldTime)
{
	if (Sequences.Num() == 0)
	{
		return nullptr;
	}
	if (SequenceBag.GetNumVariants() != Sequences.Num())
	{
		SequenceBag.Init(Sequences.Num());
	}

	// Пробуем выдать цепочку из мешка. Мешок гарантирует отсутствие немедленных
	// повторов и перетасовывается в конце. But состояние (distress) меняет ВЕС
	// каждой цепочки — поэтому перебираем до 3 попыток, выбирая взвешенно.
	const int32 NumSequences = Sequences.Num();
	for (int32 Attempt = 0; Attempt < 4; ++Attempt)
	{
		const int32 Idx = SequenceBag.Next();
		if (Idx < 0 || Idx >= NumSequences)
		{
			return nullptr;
		}
		const FBackroomsEventSequence& Seq = Sequences[Idx];
		// Взвешенный порог: страшные цепочки поднимаются при высоком distress,
		// успокаивающие — опускаются. «Веса меняются местами».
		const float W = Seq.Weight * FMath::Lerp(1.0f, Seq.FearWeight, CurrentDistress);
		// Если distressed — почти всегда берём страшное; спокойное принимаем
		// с меньшей вероятностью.
		const float Accept = FMath::Clamp(W, 0.0f, 2.0f);
		if (FMath::FRand() < Accept * 0.6f)
		{
			return &Seq;
		}
	}

	// Фолбэк: первая из пула (редко; гарантирует, что цепочки вообще идут).
	return &Sequences[0];
}

void UBackroomsEventSystem::QueueSequence(const FBackroomsEventSequence& Seq, ABackroomsWorldGenerator* Generator)
{
	UE_LOG(LogTemp, Display, TEXT("BR Sequence: %s (%d steps)"), *Seq.Id.ToString(), Seq.Steps.Num());

	// Ставим шаги цепочки в очередь (уже в нужном порядке).
	ChainQueue = Seq.Steps;
	ChainStepTimer = FMath::FRandRange(Seq.StepIntervalMin, Seq.StepIntervalMax);
	(void)Generator;
}

void UBackroomsEventSystem::AdvanceChain(float DeltaSeconds, ABackroomsWorldGenerator* Generator)
{
	if (ChainQueue.Num() == 0)
	{
		return;
	}

	ChainStepTimer -= DeltaSeconds;
	if (ChainStepTimer > 0.0f)
	{
		return;
	}

	// Выстреливаем текущий шаг цепочки.
	const EBackroomsEventType Step = ChainQueue[0];
	ChainQueue.RemoveAt(0);

	if (const FBackroomsEvent* Cfg = FindEventConfig(Step))
	{
		ExecuteEvent(*Cfg, Generator);
	}

	// Ставим интервал до следующего шага (если остались).
	if (ChainQueue.Num() > 0)
	{
		ChainStepTimer = FMath::FRandRange(0.6f, 1.8f);
	}
}

void UBackroomsEventSystem::TriggerSequence(FName SequenceId)
{
	for (const FBackroomsEventSequence& Seq : Sequences)
	{
		if (Seq.Id == SequenceId)
		{
			// Запускаем без генератора на месте — реальный плеер возьмёт
			// генератор из контекста; здесь просто логируем и ставим очередь.
			QueueSequence(Seq, nullptr);
			return;
		}
	}
}


void UBackroomsEventSystem::TriggerEvent(EBackroomsEventType Type, FVector Location)
{
	for (const FBackroomsEvent& Event : Events)
	{
		if (Event.Type == Type)
		{
			// Создаём фиктивный генератор дляEXECUTE.
			// В реальном вызове Location берётся от игрока.
			FBackroomsEventResult Result;
			Result.Type = Type;
			Result.Location = Location;
			Result.Duration = Event.Duration;
			const FString DescKey = FString::Printf(TEXT("Event.%s"), *UEnum::GetValueAsName(Type).ToString());
			Result.Description = BackroomsLoc::Get(*DescKey);
			OnEventTriggered.Broadcast(Result);
			break;
		}
	}
}

void UBackroomsEventSystem::ExecuteEvent(const FBackroomsEvent& Event, ABackroomsWorldGenerator* Generator)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(Generator, 0);
	FVector PlayerLoc = FVector::ZeroVector;
	if (PC && PC->GetPawn())
	{
		PlayerLoc = PC->GetPawn()->GetActorLocation();
	}

	// Случайная позиция рядом с игроком.
	const float Angle = FMath::FRand() * 360.0f;
	const float Dist = FMath::FRandRange(500.0f, Event.Radius > 0 ? Event.Radius : 1500.0f);
	const FVector EventLoc(
		PlayerLoc.X + FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
		PlayerLoc.Y + FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
		PlayerLoc.Z);

	FBackroomsEventResult Result;
	Result.Type = Event.Type;
	Result.Location = EventLoc;
	Result.Duration = Event.Duration;
	{
		const FString DescKey = FString::Printf(TEXT("Event.%s"), *UEnum::GetValueAsName(Event.Type).ToString());
		Result.Description = BackroomsLoc::Get(*DescKey);
		if (Result.Description == DescKey)
		{
			Result.Description = BackroomsLoc::Get(TEXT("Event.Unknown"));
		}
	}

	switch (Event.Type)
	{
	case EBackroomsEventType::LightFlicker:
		ApplyLightFlicker(Generator, Event.Duration, EventLoc, Event.Radius);
		break;
	case EBackroomsEventType::LightOutage:
		ApplyLightOutage(Generator, Event.Duration, EventLoc, Event.Radius);
		break;
	case EBackroomsEventType::BoxDisappear:
	case EBackroomsEventType::BoxAppear:
		break;
	case EBackroomsEventType::EntityGrowl:
		ApplyEntityGrowl(Generator, EventLoc, Event.Volume);
		break;
	case EBackroomsEventType::EntityFootsteps:
		ApplyEntityFootsteps(Generator, EventLoc, Event.Volume);
		break;
	case EBackroomsEventType::Whisper:
		ApplyWhisper(Generator, EventLoc, Event.Volume);
		break;
	case EBackroomsEventType::DistantBang:
		if (USoundBase* Bang = GetLoadedSound(BangSound))
		{
			UAudioComponent* AC = UGameplayStatics::SpawnSoundAtLocation(
				Generator, Bang, EventLoc, FRotator::ZeroRotator, Event.Volume, 1.0f);
			ABackroomsNoiseMarker::SpawnAt(Generator ? Generator->GetWorld() : GetWorld(), EventLoc)->BindSound(AC);
		}
		else
		{
			ABackroomsNoiseMarker::SpawnAt(Generator ? Generator->GetWorld() : GetWorld(), EventLoc);
		}
		break;
	case EBackroomsEventType::WallDrawing:
		ApplyWallDrawing(Generator, EventLoc);
		break;
	case EBackroomsEventType::PipeCreak:
		ApplyPipeCreak(Generator, EventLoc, Event.Volume);
		break;
	case EBackroomsEventType::DoorSlam:
		ApplyDoorSlam(Generator, EventLoc, Event.Volume);
		break;
	case EBackroomsEventType::EmergencyLight:
		ApplyEmergencyLight(Generator, Event.Duration, EventLoc);
		break;
	case EBackroomsEventType::FogIncrease:
		ApplyFogIncrease(Generator, Event.Duration);
		break;
	case EBackroomsEventType::StaticNoise:
		ApplyStaticNoise(Generator, Event.Duration);
		break;
	case EBackroomsEventType::FootprintAppear:
		break;
	default:
		break;
	}

	ActiveEffects.Add({ Event.Type, Event.Duration, EventLoc });
	OnEventTriggered.Broadcast(Result);
	UE_LOG(LogTemp, Display, TEXT("BR Event: %s"), *Result.Description);
}

void UBackroomsEventSystem::ApplyLightFlicker(ABackroomsWorldGenerator* Generator, float Duration, const FVector& Location, float Radius)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(Generator, 0);
	if (PC && PC->PlayerCameraManager)
	{
		// Мерцание через лёгкий fade-in/out: едва заметное, короткое, не
		// «выбивает» зрение. Глубина ~5%, суммарно ~0.2 с — саспенс-укол,
		// а не стробоскоп.
		PC->PlayerCameraManager->StartCameraFade(0.0f, 0.05f, 0.06f, FLinearColor::Black, false, true);
		FTimerHandle Handle;
		Generator->GetWorldTimerManager().SetTimer(Handle, [PC]()
		{
			if (PC && PC->PlayerCameraManager)
			{
				PC->PlayerCameraManager->StartCameraFade(0.05f, 0.0f, 0.14f, FLinearColor::Black, false, true);
			}
		}, 0.18f, false);
	}
}

void UBackroomsEventSystem::ApplyLightOutage(ABackroomsWorldGenerator* Generator, float Duration, const FVector& Location, float Radius)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(Generator, 0);
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(0.0f, 0.7f, 0.5f, FLinearColor::Black, false, true);
		FTimerHandle Handle;
		Generator->GetWorldTimerManager().SetTimer(Handle, [PC]()
		{
			if (PC && PC->PlayerCameraManager)
			{
				PC->PlayerCameraManager->StartCameraFade(0.7f, 0.0f, 1.0f, FLinearColor::Black, false, true);
			}
		}, Duration, false);
	}
}

void UBackroomsEventSystem::ApplyEntityGrowl(ABackroomsWorldGenerator* Generator, const FVector& Location, float Volume)
{
	// Звук рыка берём из GrowlSound (загружается по soft-ссылке). Если ассет
	// не назначен — тихо, без бесполезного PlaySoundAtLocation(nullptr).
	if (USoundBase* Sound = GetLoadedSound(GrowlSound))
	{
		UAudioComponent* AC = UGameplayStatics::SpawnSoundAtLocation(
			Generator, Sound, Location, FRotator::ZeroRotator, Volume);
		// Маркер «откуда шум», живущий ровно пока звук играет.
		ABackroomsNoiseMarker::SpawnAt(Generator ? Generator->GetWorld() : GetWorld(), Location)->BindSound(AC);
		return;
	}
	// Без звука — маркер просто доживает свой страховочный Lifetime.
	ABackroomsNoiseMarker::SpawnAt(Generator ? Generator->GetWorld() : GetWorld(), Location);
}

void UBackroomsEventSystem::ApplyEntityFootsteps(ABackroomsWorldGenerator* Generator, const FVector& Location, float Volume)
{
	// Серия тихих ударов с интервалом. Каждый шаг берёт ПРИВЯЗАННЫЙ к
	// конкретной позиции звук из «мешка» шагов (перетасовка колоды).
	if (FootstepBag.GetNumVariants() != FootstepVariants.Num())
	{
		FootstepBag.Init(FootstepVariants.Num());
	}

	// Новый вызов перезапускает серию: сбрасываем старый таймер, если он был.
	Generator->GetWorldTimerManager().ClearTimer(ActiveFootstepHandle);
	FootstepGenerator = Generator;
	FootstepLocation = Location;
	FootstepVolume = Volume;
	FootstepStepCount = 0;
	bFootstepsActive = true;
	Generator->GetWorldTimerManager().SetTimer(ActiveFootstepHandle, this, &UBackroomsEventSystem::TickFootstep, 0.4f, true);

	// Маркер «здесь шаги»: живёт пока идёт серия (5 шагов по 0.4 с ~ 2.1 с).
	ABackroomsNoiseMarker::SpawnAt(Generator->GetWorld(), Location, 2.6f);
}

void UBackroomsEventSystem::TickFootstep()
{
	// Метод-делегат таймера: refcount по TObjectPtr, дескриптор — член, поэтому
	// ClearTimer по нему гарантированно останавливает именно этот таймер.
	if (!bFootstepsActive)
	{
		return;
	}
	ABackroomsWorldGenerator* Generator = FootstepGenerator.Get();
	if (!Generator)
	{
		bFootstepsActive = false;
		return;
	}
	if (FootstepStepCount >= 5)
	{
		bFootstepsActive = false;
		Generator->GetWorldTimerManager().ClearTimer(ActiveFootstepHandle);
		return;
	}
	USoundBase* Sound = DrawFromBag(FootstepBag, FootstepVariants);
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(Generator, Sound, FootstepLocation, FootstepVolume * 0.5f, 1.0f, 0.0f);
	}
	++FootstepStepCount;
}

void UBackroomsEventSystem::ApplyWhisper(ABackroomsWorldGenerator* Generator, const FVector& Location, float Volume)
{
	if (USoundBase* Sound = GetLoadedSound(WhisperSound))
	{
		UAudioComponent* AC = UGameplayStatics::SpawnSoundAtLocation(
			Generator, Sound, Location, FRotator::ZeroRotator, Volume, 1.2f);
		ABackroomsNoiseMarker::SpawnAt(Generator ? Generator->GetWorld() : GetWorld(), Location)->BindSound(AC);
		return;
	}
	ABackroomsNoiseMarker::SpawnAt(Generator ? Generator->GetWorld() : GetWorld(), Location);
}

void UBackroomsEventSystem::ApplyWallDrawing(ABackroomsWorldGenerator* Generator, const FVector& Location)
{
	// Визуальный эффект — «рисунок» появляется и исчезает.
	// Реализуется через декаль-компонент в Blueprint.
}

void UBackroomsEventSystem::ApplyPipeCreak(ABackroomsWorldGenerator* Generator, const FVector& Location, float Volume)
{
	if (USoundBase* Sound = GetLoadedSound(CreakSound))
	{
		UAudioComponent* AC = UGameplayStatics::SpawnSoundAtLocation(
			Generator, Sound, Location, FRotator::ZeroRotator, Volume, 0.8f);
		ABackroomsNoiseMarker::SpawnAt(Generator ? Generator->GetWorld() : GetWorld(), Location)->BindSound(AC);
		return;
	}
	ABackroomsNoiseMarker::SpawnAt(Generator ? Generator->GetWorld() : GetWorld(), Location);
}

void UBackroomsEventSystem::ApplyDoorSlam(ABackroomsWorldGenerator* Generator, const FVector& Location, float Volume)
{
	// Хлопок/дёрганье двери — берём вариант из «мешка» дверей (без повторов).
	if (DoorRattleBag.GetNumVariants() != DoorRattleVariants.Num())
	{
		DoorRattleBag.Init(DoorRattleVariants.Num());
	}
	USoundBase* Sound = DrawFromBag(DoorRattleBag, DoorRattleVariants);
	if (Sound)
	{
		UAudioComponent* AC = UGameplayStatics::SpawnSoundAtLocation(Generator, Sound, Location, FRotator::ZeroRotator, Volume);
		// Маркер «хлопнула дверь»: живёт пока звучит хлопок.
		ABackroomsNoiseMarker::SpawnAt(Generator ? Generator->GetWorld() : GetWorld(), Location)->BindSound(AC);
	}
	else
	{
		ABackroomsNoiseMarker::SpawnAt(Generator ? Generator->GetWorld() : GetWorld(), Location);
	}
	// Встряска камеры.
	APlayerController* PC = UGameplayStatics::GetPlayerController(Generator, 0);
	if (PC && PC->GetPawn())
	{
		PC->GetPawn()->SetActorLocation(PC->GetPawn()->GetActorLocation() + FVector(0, 0, 5.0f));
	}
}

void UBackroomsEventSystem::ApplyEmergencyLight(ABackroomsWorldGenerator* Generator, float Duration, const FVector& Location)
{
	// Красный аварийный свет — через Blueprint-компонент.
}

void UBackroomsEventSystem::ApplyFogIncrease(ABackroomsWorldGenerator* Generator, float Duration)
{
	// Увеличение плотности тумана.
	// В реальном проекте — через AExponentialHeightFogComponent.
}

void UBackroomsEventSystem::ApplyStaticNoise(ABackroomsWorldGenerator* Generator, float Duration)
{
	// CRT-шум на экране — через post-process или UI-виджет.
	APlayerController* PC = UGameplayStatics::GetPlayerController(Generator, 0);
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(0.0f, 0.07f, 0.05f, FLinearColor(0.1f, 0.1f, 0.1f), false, true);
		FTimerHandle Handle;
		Generator->GetWorldTimerManager().SetTimer(Handle, [PC]()
		{
			if (PC && PC->PlayerCameraManager)
			{
				PC->PlayerCameraManager->StartCameraFade(0.07f, 0.0f, 0.3f, FLinearColor(0.1f, 0.1f, 0.1f), false, true);
			}
		}, Duration, false);
	}
}

// Взять звук из «мешка»: следующий вариант из тасованной колоды. Если колода
// опустела — построить и перетасовать новую (без немедленного повтора первой
// карты). Пустой набор/незагруженный звук возвращают null и геймкод просто
// не играет звука.
USoundBase* UBackroomsEventSystem::DrawFromBag(FShuffleBag& Bag, const TArray<TSoftObjectPtr<USoundBase>>& Variants)
{
	if (Variants.Num() == 0)
	{
		return nullptr;
	}
	if (Bag.GetNumVariants() != Variants.Num())
	{
		Bag.Init(Variants.Num());
	}
	const int32 Idx = Bag.Next();
	if (Idx == INDEX_NONE || Idx >= Variants.Num())
	{
		return nullptr;
	}
	return Variants[Idx].Get();
}

// Загрузка звука из soft-ссылки с кешем по имени ассета. Пустая ссылка
// возвращает nullptr (событие просто "молчит", без бесполезного вызова
// PlaySoundAtLocation(nullptr)).
USoundBase* UBackroomsEventSystem::GetLoadedSound(const TSoftObjectPtr<USoundBase>& Soft)
{
	if (!Soft.IsValid())
	{
		return nullptr;
	}
	const FName Key = FName(*Soft.ToSoftObjectPath().ToString());
	if (TObjectPtr<USoundBase>* Found = SoundCache.Find(Key))
	{
		return Found->Get();
	}
	USoundBase* Sound = Soft.LoadSynchronous();
	if (Sound)
	{
		SoundCache.Add(Key, Sound);
	}
	return Sound;
}


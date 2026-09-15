#include "BackroomsMonsterAIController.h"
#include "BackroomsSenseComponent.h"
#include "BackroomsNoise.h"
#include "BackroomsEventSystem.h"
#include "BackroomsRiggedMonster.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "NavigationSystem.h"
#include "BackroomsSeatFinder.h"
#include "DrawDebugHelpers.h"

ABackroomsMonsterAIController::ABackroomsMonsterAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	// Ассеты «монстр заметил игрока» импортированы в Content/Audio (скрипт
	// import_audio.py); при входе в Alerted играет случайный из двух.
	NoticedSoundA = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Audio/MonsterNoticed1")));
	NoticedSoundB = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Audio/MonsterNoticed2")));
}

void ABackroomsMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Сенсор монстра — на пешке (слух + конус обзора как у человека).
	if (InPawn)
	{
		Sense = NewObject<UBackroomsSenseComponent>(InPawn);
		if (Sense)
		{
			Sense->RegisterComponent();
		}
	}

	AActor* WorldTarget = UGameplayStatics::GetPlayerPawn(this, 0);
	Target = Cast<ACharacter>(WorldTarget);

	// Каждый монстр «заводится» с собственным окном до первого поиска стула:
	// мир успевает доспавнить чанки, а монстры не сканируют его всем скопом.
	if (UWorld* World = GetWorld())
	{
		SitSearchEarliestTime = World->GetTimeSeconds() + FMath::FRandRange(SitMinCooldown, SitMaxCooldown);
	}
}

void ABackroomsMonsterAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	StateTime += DeltaSeconds;

	if (!GetPawn())
	{
		return;
	}

	// Заметка на РАНЕЕ: спросить до того, как состояние переключится ниже.
	const bool bWasAlerted = (CurrentState == EBackroomsMonsterState::Alerted);

	// Ставим цель = игрока, если ещё не найден.
	if (!Target)
	{
		Target = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	}

	// Ощущаем мир каждый тик сенсором.
	if (Sense)
	{
		if (Target && GetPawn())
		{
			Sense->UpdateVision(Target->GetActorLocation());

			// ИГРОК УЧИТСЯ, ЧТО НЕЛЬЗЯ БЕГАТЬ В ОДНИХ И ТЕХ ЖЕ МЕСТАХ: если
			// сейчас он находится в месте, которое монстр НЕДАВНО запомнил,
			// монстр «вспоминает» его и становится привязчивее (решимость
			// не падает — он ждёт жертву здесь).
			const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
			const float LastHeardAge = Sense->LastStimulusTime < 0.0f ? 1e9f : Now - Sense->LastStimulusTime;
			if (Sense->WasRecentlyRemembered(Target->GetActorLocation(), 20.0f) && LastHeardAge > 3.0f)
			{
				// Вернуться на старое место — небезопасно.
				Resolve = FMath::Clamp(Resolve + 0.02f, 0.0f, 1.0f);
			}
		}
		Sense->UpdateSenses(DeltaSeconds);
	}

	// Выбор состояния по уровню осведомлённости сенсора.
	const bool bTargetInSight = Target && GetPawn() &&
		(Sense ? Sense->GetAwarenessLevel() == EBackroomsAwareness::Alerted : FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation()) < 800.0f);

	const float DistToTarget = (Target && GetPawn())
		? FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation()) : 1e9f;
	const bool bPlayerGazing = IsPlayerLookingAtMonster(WatchMaxDistance * 1.4f);

	// Сидит: машину состояний не трогаем — выход решит RunSitting.
	if (CurrentState == EBackroomsMonsterState::Sitting)
	{
	}
	else if (CurrentState == EBackroomsMonsterState::Disappearing)
	{
		RunDisappearing(DeltaSeconds);
	}
	else if (bTargetInSight)
	{
		CurrentState = EBackroomsMonsterState::Alerted;
	}
	else if (Sense && Sense->GetAwarenessLevel() == EBackroomsAwareness::Suspecting)
	{
		// Переход в исследование: взяли точку интереса у сенсора.
		if (CurrentState != EBackroomsMonsterState::Investigating)
		{
			SetInvestigateFromSense();
		}
		CurrentState = EBackroomsMonsterState::Investigating;
	}
	else
	{
		// Ничего не слышал/не видел, но игрок рядом: тихое наблюдение из тени
		// и преследование на дистанции — «мягкая» угроза до тревоги.
		if (DistToTarget >= WatchMinDistance && DistToTarget <= WatchMaxDistance)
		{
			CurrentState = EBackroomsMonsterState::Watching;
		}
		else if (DistToTarget < WatchMinDistance && DistToTarget > StalkDistance * 0.6f)
		{
			CurrentState = EBackroomsMonsterState::Stalking;
		}
		else
		{
			CurrentState = EBackroomsMonsterState::Patroling;
		}
	}

	// Выполняем текущее состояние.
	switch (CurrentState)
	{
	case EBackroomsMonsterState::Patroling:     RunPatrol(DeltaSeconds); break;
	case EBackroomsMonsterState::Investigating: RunInvestigate(DeltaSeconds); break;
	case EBackroomsMonsterState::Sitting:       RunSitting(DeltaSeconds); break;
	case EBackroomsMonsterState::Watching:      RunWatching(DeltaSeconds); break;
	case EBackroomsMonsterState::Stalking:      RunStalking(DeltaSeconds); break;
	case EBackroomsMonsterState::Disappearing:  break;
	default:                                    RunAlerted(DeltaSeconds); break;
	}

	// Мягкое поведение: скорость монстра зависит от режима. Тревога —
	// быстрый подход, во всём остальном — медленное патрулирование.
	if (ABackroomsRiggedMonster* Monster = Cast<ABackroomsRiggedMonster>(GetPawn()))
	{
		const bool bSitNow = CurrentState == EBackroomsMonsterState::Sitting && bSeated;
		Monster->SetSitting(bSitNow, CurrentSeat.bValid ? CurrentSeat.SeatHeightCm : 0.0f);
		Monster->SetAggressiveMode(
			CurrentState == EBackroomsMonsterState::Alerted ||
			CurrentState == EBackroomsMonsterState::Stalking);
		Monster->SetInspectMode(CurrentState == EBackroomsMonsterState::Investigating);
		Monster->SetStateDisplayName(GetStateDisplayName());
	}

	// Только что перешёл в тревогу — «монстр заметил игрока» (случайный из двух).
	if (!bWasAlerted && CurrentState == EBackroomsMonsterState::Alerted)
	{
		PlayNoticeSound();
	}

	// Сообщаем системе событий о состоянии монстра -> атмосфера реагирует:
	// при расследовании — шёпоты, при приближении — тихие ненавязчивые звуки.
	if (EventSystem)
	{
		const bool bInv = CurrentState == EBackroomsMonsterState::Investigating ||
			CurrentState == EBackroomsMonsterState::Watching ||
			CurrentState == EBackroomsMonsterState::Stalking;
		const float DistToPlayer = Target ? FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation()) : 1e9f;
		EventSystem->SetMonsterPresence(bInv, GetPawn()->GetActorLocation(), DistToPlayer);
	}
}

FVector ABackroomsMonsterAIController::GetRandomPatrolPoint() const
{
	const AActor* OwnedPawn = GetPawn();
	if (!OwnedPawn)
	{
		return FVector::ZeroVector;
	}

	// «Умная ошибка»: точка патруля смещена Perlin-шумом от точного случайного
	// места, а не тыкается по карте идеально — движение выглядит живым.
	const FVector Base = OwnedPawn->GetActorLocation();

	// Сначала пробуем найти достижимую точку через навигационную сеть.
	// В стриминговом процедурном мире NavMesh может не быть готов
	// (чанки перестраиваются при движении) — тогда полагаемся на верх.
	if (UWorld* World = GetWorld())
	{
		if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World))
		{
			FVector OutPoint;
			if (UNavigationSystemV1::K2_GetRandomReachablePointInRadius(GetWorld(), Base, OutPoint, PatrolRadius))
			{
				return OutPoint;
			}
		}
	}

	// Фолбэк: случайная точка вокруг (если NavMesh не готов).
	const float Angle = FMath::FRandRange(0.0f, 360.0f);
	const float NoiseR = BackroomsNoise::TemporalNoise(GetWorld()->GetTimeSeconds(), 0.1f, Angle, FMath::RandRange(1, 9999));
	const float R = PatrolRadius * (0.5f + NoiseR);
	const float DX = FMath::Cos(FMath::DegreesToRadians(Angle)) * R;
	const float DY = FMath::Sin(FMath::DegreesToRadians(Angle)) * R;
	return Base + FVector(DX, DY, 0.0f);
}

// Плавная «умная ошибка» в направлении движения: добавляем время-шумовой
// сдвиг, что чтобы чудовище немного заплеталось/колебалось (не роботово прямо).
FVector ABackroomsMonsterAIController::ApplyMovementError(const FVector& Goal) const
{
	const AActor* OwnedPawn = GetPawn();
	if (!OwnedPawn)
	{
		return Goal;
	}
	const float T = GetWorld()->GetTimeSeconds();
	// Маленькие, но заметные временные отклонения:
	const float ErrX = (BackroomsNoise::TemporalNoise(T, 2.2f, 0.0f, 777) - 0.5f) * 220.0f;
	const float ErrY = (BackroomsNoise::TemporalNoise(T, 1.8f, 5.0f, 778) - 0.5f) * 220.0f;
	return Goal + FVector(ErrX, ErrY, 0.0f);
}

void ABackroomsMonsterAIController::RunPatrol(float DeltaSeconds)
{
	// Бросаем исследование (если успокоился).
	InvestigateTimer = 0.0f;

	PatrolTimer -= DeltaSeconds;
	if (PatrolTimer <= 0.0f)
	{
		PatrolTimer = PatrolRePickInterval;
		const FVector Point = ApplyMovementError(GetRandomPatrolPoint());
		MoveToLocation(Point, 80.0f, false, true, true, false, nullptr, false);
	}

	// Иногда вместо новой точки патруля монстр садится на найденную мебель.
	SitProbeTimer -= DeltaSeconds;
	if (SitProbeTimer <= 0.0f)
	{
		SitProbeTimer = FMath::FRandRange(2.0f, 4.0f);
		if (TryStartSit())
		{
			return;
		}
	}

	// На ходу плавно «промахиваемся» взглядом по сторонам (scatter look).
	if (APawn* P = GetPawn())
	{
		const float Yaw = 360.0f * BackroomsNoise::TemporalNoise(GetWorld()->GetTimeSeconds(), 0.25f, 2.0f, 1234);
		P->SetActorRotation(FRotator(0.0f, Yaw, 0.0f));
	}
}

void ABackroomsMonsterAIController::SetInvestigateFromSense()
{
	if (Sense)
	{
		InvestigatePoint = Sense->LastKnownPlayerLocation;
	}

	// Строим очередь исследования из ПАМЯТИ сенсора (последние 3-5 мест,
	// где видели/слышали игрока), начиная с самого свежего. За счёт этого
	// игрок УЧИТСЯ, что нельзя бегать по одним и тем же местам — монстр
	// каждый раз проверяет все запомненные точки.
	InvestigateQueue.Reset();
	if (Sense)
	{
		// Самые свежие воспоминания — вперёд.
		TArray<FBackroomsSenseMemory> Sorted = Sense->Memory;
		Sorted.Sort([](const FBackroomsSenseMemory& A, const FBackroomsSenseMemory& B)
		{
			return A.Time > B.Time;
		});
		for (const FBackroomsSenseMemory& M : Sorted)
		{
			InvestigateQueue.Add(M.Location);
		}
	}
	if (InvestigateQueue.Num() == 0)
	{
		InvestigateQueue.Add(InvestigatePoint);
	}
	QueueIdx = 0;
	CircleCenter = InvestigateQueue[FMath::Min(QueueIdx, InvestigateQueue.Num() - 1)];
	CircleStep = 0;
	LookDir = 0;
	InvestigateTimer = 0.0f;
}

void ABackroomsMonsterAIController::RunInvestigate(float DeltaSeconds)
{
	InvestigateTimer += DeltaSeconds;

	// РЕШИМОСТЬ: пока игрок спрятался, она медленно падает...
	Resolve = FMath::Clamp(Resolve - ResolveDecay * DeltaSeconds, 0.0f, 1.0f);
	// ...но 3D-шум Перлина может РЕЗКО заставить монстра развернуться, даже не
	// получив ни звука, ни взгляда — он «чувствует» и возвращает себе решимость.
	if (TrySnapTurn())
	{
		// Резкий разворот: меняем контур исследования на противоположную сторону.
		CircleStep += InvestigateCirclePoints / 2; // половина круга назад
		LookDir = (LookDir + 2) % 4;               // и смотрим в другую сторону
	}

	if (Resolve <= 0.0f)
	{
		// Источник потерян, решимость на нуле — возвращаемся в патруль.
		PatrolTimer = 0.0f;
		CurrentState = EBackroomsMonsterState::Patroling;
		return;
	}
	// Жёсткий предохранитель: даже если решимость всё время подпитывается
	// разворотами, слишком долго у точки не сидим.
	if (InvestigateTimer > AbandonAfter)
	{
		PatrolTimer = 0.0f;
		CurrentState = EBackroomsMonsterState::Patroling;
		return;
	}

	// Обновляем точку источника живым временным шумом -> «дрожит», монстр
	// не целится идеально в то, где слышал/видел раньше.
	FVector LivePoint = ApplyMovementError(InvestigateQueue[FMath::Min(QueueIdx, InvestigateQueue.Num() - 1)]);

	// Фаза 1: ПОДОЙТИ к очередной запомненной точке.
	const float DistToSource = FVector::Dist(GetPawn()->GetActorLocation(), LivePoint);
	if (DistToSource > InvestigateCircleRadius * 1.6f)
	{
		MoveToLocation(LivePoint, 40.0f, false, true, true, false, nullptr, false);
		return;
	}

	// Подошли к точке — переходим к следующему воспоминанию (обход всей памяти).
	if (QueueIdx < InvestigateQueue.Num() - 1)
	{
		QueueIdx++;
		CircleCenter = InvestigateQueue[QueueIdx];
		return;
	}

	// Фаза 2: у источника — ОСМОТРЕТЬСЯ ПО СТОРОНАМ (повернуть голову сначала).
	const int32 NumLooks = 4;
	LookDir = (LookDir + 1) % NumLooks;
	const float LookYaw = 360.0f * (float)LookDir / (float)NumLooks
		+ (BackroomsNoise::TemporalNoise(GetWorld()->GetTimeSeconds(), 1.0f, LookDir, 321) - 0.5f) * 30.0f;
	if (APawn* P = GetPawn())
	{
		P->SetActorRotation(FRotator(0.0f, LookYaw, 0.0f));
	}

	// Фаза 3: ПРОХОДИТЬ ВОКРУГ точки (по кругу).
	CircleStep %= InvestigateCirclePoints;
	const float A = 360.0f * (float)CircleStep / (float)InvestigateCirclePoints;
	const float CA = FMath::DegreesToRadians(A);
	const FVector CircleTarget = CircleCenter + FVector(FMath::Cos(CA), FMath::Sin(CA), 0.0f) * InvestigateCircleRadius;
	StopMovement();
	MoveToLocation(CircleTarget, 40.0f, false, true, true, false, nullptr, false);
	CircleStep++;
}

// Резкий разворот под действием 3D-шума Перлина.
// Идея: в 3D-поле шума подаём ОТНОСИТЕЛЬНУЮ позицию игрока (X/Y) и время (Z).
// Когда шум «выстреливает» выше порога — монстр внезапно решает развернуться и
// «почувствовать» источник: даже если игрок спрятался, он возвращает себе
// решимость, и прятаться «насовсем» нельзя. Возвращает true, если сработал.
bool ABackroomsMonsterAIController::TrySnapTurn()
{
	const AActor* OwnedPawn = GetPawn();
	const UWorld* World = GetWorld();
	if (!OwnedPawn || !World)
	{
		return false;
	}

	// Вход 3D-шума = позиция игрока (пространственно-когерентно) + время.
	FVector Coord = FVector::ZeroVector;
	if (Target)
	{
		Coord = (Target->GetActorLocation() - OwnedPawn->GetActorLocation()) * 0.001f;
	}

	// Контроль частоты срабатывания по АБСОЛЮТНОМУ времени мира (а не decrement
	// на DeltaSeconds-тике): разворот возможен не чаще, чем раз в 1.5 сек.
	const float Now = World->GetTimeSeconds();
	if (Now < NextSnapTime)
	{
		return false;
	}
	NextSnapTime = Now + 1.5f;

	// 3D-шум: ось Z = время -> «ползущий во времени» порог разворота.
	const float WorldTime = World->GetTimeSeconds();
	const float Noise = BackroomsNoise::Noise3D01(
		Coord.X, Coord.Y, Coord.Z,
		/*bUseTime*/ true, WorldTime, /*ZScale*/ SnapTurnNoiseScale, /*Seed*/ 40401);

	if (Noise > SnapTurnThreshold)
	{
		// Резко решил развернуться и «прощупать» — решимость поднялась.
		Resolve = FMath::Clamp(Resolve + ResolveRefreshOnSnap, 0.0f, 1.0f);
		return true;
	}
	return false;
}

void ABackroomsMonsterAIController::RunAlerted(float DeltaSeconds)
{
	if (!Target || !GetPawn())
	{
		CurrentState = EBackroomsMonsterState::Patroling;
		return;
	}
	// Умная ошибка в преследовании: цель слегка «дрожит», монстр не целится
	// идеально в игрока — его атака/заход более живой и пугающий.
	// Мягкое поведение: монстр НЕ бросается на игрока и не атакует по
	// скрипту. Он подходит до дистанции «нависания» и останавливается,
	// просто находясь рядом (угроза + шанс сбежать). Жёсткости нет.
	float MonsterStandoff = 170.0f;
	if (const ABackroomsRiggedMonster* Monster = Cast<const ABackroomsRiggedMonster>(GetPawn()))
	{
		MonsterStandoff = Monster->StandoffDistance;
	}
	const float DistToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());

	// Если игрок в упор «поймал» монстра взглядом с дистанции — тот растворяется.
	if (DistToPlayer > MonsterStandoff * 3.0f && IsPlayerLookingAtMonster(WatchMaxDistance))
	{
		GazeTimer += DeltaSeconds;
		if (GazeTimer >= GazeBreakSeconds)
		{
			GazeTimer = 0.0f;
			DisappearTimer = 0.0f;
			CurrentState = EBackroomsMonsterState::Disappearing;
			return;
		}
	}
	else
	{
		GazeTimer = 0.0f;
	}

	if (DistToPlayer <= MonsterStandoff)
	{
		// Уже «навис» над игроком — стоим (без агрессии, без тарана).
		StopMovement();
		return;
	}

	const FVector LiveTarget = ApplyMovementError(Target->GetActorLocation());
	MoveToActor(Target, MonsterStandoff * 0.5f, true, true, true, nullptr, false);
}

// «Монстр заметил игрока»: случайный из двух звуков (от импортированных
// ассетов Content/Audio/MonsterNoticed1/2) — в мире у монстра, чтобы игрок
// слышал направление. Тихий промах, если ассеты ещё не импортированы.
void ABackroomsMonsterAIController::PlayNoticeSound()
{
	APawn* MonsterPawn = GetPawn();
	if (!MonsterPawn)
	{
		return;
	}

	USoundBase* SoundA = NoticedSoundA.LoadSynchronous();
	USoundBase* SoundB = NoticedSoundB.LoadSynchronous();
	USoundBase* Pick = FMath::RandBool() ? SoundA : SoundB;
	if (!Pick)
	{
		Pick = SoundA ? SoundA : SoundB;
	}

	if (Pick)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this, Pick, MonsterPawn->GetActorLocation(), FRotator::ZeroRotator, 1.0f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BackroomsMonster: noticed-sounds not imported (/Game/Audio/MonsterNoticed1/2)"));
	}
}

// Смотрит ли игрок прямо на монстра (взгляд ~35 градусов + прямая видимость).
bool ABackroomsMonsterAIController::IsPlayerLookingAtMonster(float MaxDistance) const
{
	const APawn* OwnedPawn = GetPawn();
	if (!OwnedPawn || !Target)
	{
		return false;
	}
	const FVector PlayerLoc = Target->GetActorLocation();
	const FVector MonsterLoc = OwnedPawn->GetActorLocation();
	const FVector ToMonster = MonsterLoc - PlayerLoc;
	const float Dist = ToMonster.Size();
	if (Dist > MaxDistance || Dist < 1.0f)
	{
		return false;
	}

	const FVector ViewDir = Target->GetControlRotation().Vector();
	const float Dot = FVector::DotProduct(ViewDir, ToMonster / Dist);
	if (Dot < FMath::Cos(FMath::DegreesToRadians(35.0f)))
	{
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterGazeLOS), false, Target);
		if (World->LineTraceSingleByChannel(Hit, PlayerLoc, MonsterLoc, ECC_WorldStatic, Params))
		{
			return false; // за стеной — не «поймал взглядом»
		}
	}
	return true;
}

FVector ABackroomsMonsterAIController::GetDisappearPoint() const
{
	const AActor* OwnedPawn = GetPawn();
	if (!OwnedPawn)
	{
		return FVector::ZeroVector;
	}
	const FVector PlayerLoc = Target ? Target->GetActorLocation() : FVector::ZeroVector;

	for (int32 i = 0; i < 12; ++i)
	{
		const FVector P = GetRandomPatrolPoint();
		if (!Target || FVector::Dist(P, PlayerLoc) > 1800.0f)
		{
			return P;
		}
	}
	// Фолбэк: уйти подальше в случайную сторону.
	return OwnedPawn->GetActorLocation() +
		FVector(FMath::FRandRange(-3000.0f, 3000.0f), FMath::FRandRange(-3000.0f, 3000.0f), 0.0f);
}

void ABackroomsMonsterAIController::RunWatching(float DeltaSeconds)
{
	StopMovement();

	if (Target && GetPawn())
	{
		const FVector Dir = Target->GetActorLocation() - GetPawn()->GetActorLocation();
		const float Yaw = Dir.Rotation().Yaw +
			(BackroomsNoise::TemporalNoise(GetWorld()->GetTimeSeconds(), 0.7f, 3.0f, 555) - 0.5f) * 20.0f;
		GetPawn()->SetActorRotation(FRotator(0.0f, Yaw, 0.0f));
	}

	if (IsPlayerLookingAtMonster(WatchMaxDistance * 1.4f))
	{
		GazeTimer += DeltaSeconds;
	}
	else
	{
		GazeTimer = 0.0f;
	}
	if (GazeTimer >= GazeBreakSeconds)
	{
		GazeTimer = 0.0f;
		DisappearTimer = 0.0f;
		CurrentState = EBackroomsMonsterState::Disappearing;
	}
}

void ABackroomsMonsterAIController::RunStalking(float DeltaSeconds)
{
	if (!Target || !GetPawn())
	{
		CurrentState = EBackroomsMonsterState::Patroling;
		return;
	}

	const FVector PlayerLoc = Target->GetActorLocation();
	const FVector MonsterLoc = GetPawn()->GetActorLocation();
	const float Dist = FVector::Dist(MonsterLoc, PlayerLoc);

	if (Dist > StalkDistance * 1.15f)
	{
		MoveToActor(Target, StalkDistance, true, true, true, nullptr, false);
	}
	else if (Dist < StalkDistance * 0.8f)
	{
		const FVector Away = MonsterLoc + (MonsterLoc - PlayerLoc).GetSafeNormal() * 500.0f;
		MoveToLocation(Away, 60.0f, false, true, true, false, nullptr, false);
	}
	else
	{
		StopMovement();
	}

	// Смотрит на игрока, но чуть «плывёт».
	const FVector Dir = PlayerLoc - MonsterLoc;
	const float Yaw = Dir.Rotation().Yaw +
		(BackroomsNoise::TemporalNoise(GetWorld()->GetTimeSeconds(), 0.9f, 7.0f, 556) - 0.5f) * 25.0f;
	GetPawn()->SetActorRotation(FRotator(0.0f, Yaw, 0.0f));

	if (IsPlayerLookingAtMonster(WatchMaxDistance * 1.2f))
	{
		GazeTimer += DeltaSeconds;
	}
	else
	{
		GazeTimer = 0.0f;
	}
	if (GazeTimer >= GazeBreakSeconds)
	{
		GazeTimer = 0.0f;
		DisappearTimer = 0.0f;
		CurrentState = EBackroomsMonsterState::Disappearing;
	}
	else if (Dist < 700.0f)
	{
		// Подошёл слишком близко — выходит на прямой контакт.
		CurrentState = EBackroomsMonsterState::Alerted;
	}
}

void ABackroomsMonsterAIController::RunDisappearing(float DeltaSeconds)
{
	StopMovement();
	if (ABackroomsRiggedMonster* Monster = Cast<ABackroomsRiggedMonster>(GetPawn()))
	{
		if (!Monster->IsVanished())
		{
			Monster->Vanish();
		}
	}

	DisappearTimer += DeltaSeconds;
	if (DisappearTimer >= DisappearDuration)
	{
		DisappearTimer = 0.0f;
		PatrolTimer = 0.0f;
		if (ABackroomsRiggedMonster* Monster = Cast<ABackroomsRiggedMonster>(GetPawn()))
		{
			Monster->ReappearAt(GetDisappearPoint());
		}
		CurrentState = EBackroomsMonsterState::Patroling;
	}
}

// ------------------------------------------------------------ сидение

void ABackroomsMonsterAIController::RunSitting(float DeltaSeconds)
{
	ABackroomsRiggedMonster* Monster = Cast<ABackroomsRiggedMonster>(GetPawn());
	if (!Monster || !GetWorld())
	{
		return;
	}

	// Заметила игрока взглядом/в пределах досягаемости — встаёт и реагирует.
	const bool bSeeTarget = Target && GetPawn() &&
		(Sense ? Sense->GetAwarenessLevel() == EBackroomsAwareness::Alerted
			: FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation()) < 800.0f);
	if (bSeeTarget)
	{
		StandUpFromSitting();
		CurrentState = EBackroomsMonsterState::Alerted;
		return;
	}
	if (Sense && Sense->GetAwarenessLevel() == EBackroomsAwareness::Suspecting)
	{
		StandUpFromSitting();
		SetInvestigateFromSense();
		CurrentState = EBackroomsMonsterState::Investigating;
		return;
	}

	// Идёт к месту: шлём MoveTo каждый тик, пока не дошла.
	if (bSitApproaching)
	{
		if (FVector::Dist2D(GetPawn()->GetActorLocation(), CurrentSeat.ApproachPoint) > 45.0f)
		{
			MoveToLocation(CurrentSeat.ApproachPoint, 25.0f, false, true, true, false, nullptr, false);
			return;
		}
		// Дошла: разворот к предмету, отключение движения, посадка.
		const FVector From = GetPawn()->GetActorLocation();
		const FVector To = CurrentSeat.Center;
		GetPawn()->SetActorRotation(FRotator(0.0f, (To - From).Rotation().Yaw, 0.0f));
		if (UCharacterMovementComponent* Move = Monster->GetCharacterMovement())
		{
			Move->DisableMovement();
		}
		bSitApproaching = false;
		bSeated = true;
		SitTimer = 0.0f;
		return;
	}

	// Сидит: копит время, потом спокойно встаёт.
	if (bSeated)
	{
		SitTimer += DeltaSeconds;
		if (SitTimer >= SitTargetDuration)
		{
			StandUpFromSitting();
			PatrolTimer = 0.0f;
			CurrentState = EBackroomsMonsterState::Patroling;
		}
	}
}

bool ABackroomsMonsterAIController::TryStartSit()
{
	APawn* P = GetPawn();
	if (!P || !GetWorld())
	{
		return false;
	}

	// Поведенческое решение: мир создан, спокойный патруль, игрок далеко.
	if (!ShouldSeekSit())
	{
		return false;
	}

	// Кулдаун между посадками (абсолютное время мира).
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now < NextSitSearchTime)
	{
		return false;
	}
	NextSitSearchTime = Now + FMath::FRandRange(SitMinCooldown, SitMaxCooldown);

	FBackroomsSeat Seat = BackroomsSeatFinder::FindSeatNear(GetWorld(), P->GetActorLocation(), SitSearchRadius);
	if (!Seat.bValid)
	{
		return false;
	}
	// Не садиться прямо у игрока (спойлеры) и не вблизи точки интереса.
	if (Target && FVector::Dist2D(Seat.ApproachPoint, Target->GetActorLocation()) < 1200.0f)
	{
		return false;
	}

	CurrentSeat = Seat;
	bSitApproaching = true;
	bSeated = false;
	SitTimer = 0.0f;
	SitTargetDuration = FMath::FRandRange(SitMinDuration, SitMaxDuration);
	CurrentState = EBackroomsMonsterState::Sitting;
	StopMovement();
	MoveToLocation(Seat.ApproachPoint, 25.0f, false, true, true, false, nullptr, false);

	UE_LOG(LogTemp, Log, TEXT("KareliaSit: иду сесть, сиденье %.0f см (h=%.0f см)"),
		Seat.Center.Z, Seat.SeatHeightCm);
	DrawDebugPoint(GetWorld(), Seat.ApproachPoint, 12.0f, FColor::Green, false, 2.5f);
	DrawDebugPoint(GetWorld(), Seat.Center, 12.0f, FColor::Yellow, false, 2.5f);
	return true;
}

void ABackroomsMonsterAIController::StandUpFromSitting()
{
	if (ABackroomsRiggedMonster* Monster = Cast<ABackroomsRiggedMonster>(GetPawn()))
	{
		Monster->SetSitting(false, 0.0f);
		if (UCharacterMovementComponent* Move = Monster->GetCharacterMovement())
		{
			Move->SetMovementMode(MOVE_Walking);
		}
	}
	bSitApproaching = false;
	bSeated = false;
	UE_LOG(LogTemp, Log, TEXT("KareliaSit: встала"));
}

bool ABackroomsMonsterAIController::ShouldSeekSit() const
{
	const UWorld* World = GetWorld();
	const APawn* P = GetPawn();
	if (!World || !P)
	{
		return false;
	}

	// Мир ещё «собирается» (спавн уровня/стриминг чанков): стул не ищем,
	// пока не прошло индивидуальное «настроение» этого монстра.
	if (World->GetTimeSeconds() < SitSearchEarliestTime)
	{
		return false;
	}

	// Решение принимается только в спокойном патруле (не наблюдение/сталк/
	// тревога/уже сидит).
	if (CurrentState != EBackroomsMonsterState::Patroling)
	{
		return false;
	}

	// Не садиться, если о игроке что-то известно (одна тревога/подозрение
	// ещё в памяти сенсора).
	if (Sense && Sense->GetAwarenessLevel() != EBackroomsAwareness::Unaware)
	{
		return false;
	}

	// Игрок должен быть далеко — не садиться у него на виду.
	if (Target && FVector::Dist2D(P->GetActorLocation(), Target->GetActorLocation()) < 1600.0f)
	{
		return false;
	}

	return true;
}

const TCHAR* ABackroomsMonsterAIController::GetStateDisplayName() const
{
	switch (CurrentState)
	{
	case EBackroomsMonsterState::Investigating: return TEXT("Осматривает");
	case EBackroomsMonsterState::Sitting:       return TEXT("Сидит");
	case EBackroomsMonsterState::Watching:      return TEXT("Наблюдает");
	case EBackroomsMonsterState::Stalking:      return TEXT("Крадётся");
	case EBackroomsMonsterState::Alerted:       return TEXT("Заметила!");
	default:                                    return TEXT("Патрулирует");
	}
}

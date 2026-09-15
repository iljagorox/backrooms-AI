#include "BackroomsProgression.h"

#include "BackroomsDifficulty.h"
#include "BackroomsLocalization.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogBackroomsProgression, Log, All);

namespace
{
constexpr int32 XPPerRoom = 4;
	constexpr int32 XPPerChunk = 2;
	constexpr int32 XPPerDescent = 40;
	constexpr int32 XPPerSurvival = 15;
	constexpr int32 MaxLeaderboardEntries = 50;

	// 1 очко за уровень + 1 бонусное каждые 3 уровня (см. AwardXP).
	constexpr int32 XPPerLevelBase = 120;
	constexpr int32 XPPerLevelGrowth = 60;
}

void UBackroomsProgression::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Load();
	RecomputeBonuses();
}

void UBackroomsProgression::Deinitialize()
{
	Save();
	Super::Deinitialize();
}

FString UBackroomsProgression::GetSavePath()
{
	return FPaths::ProjectSavedDir() / TEXT("Backrooms/Progression.dat");
}

const TArray<FBackroomsPerkDef>& UBackroomsProgression::Rules()
{
	static const TArray<FBackroomsPerkDef> Table = []()
	{
		TArray<FBackroomsPerkDef> T;
		auto Add = [&T](FName Id, EBackroomsPerkTree Tree, int32 Req, int32 Cost, FName Requires)
		{
			FBackroomsPerkDef D;
			D.Id = Id;
			D.Name = UBackroomsProgression::PerkNameText(Id);
			D.Description = UBackroomsProgression::PerkDescText(Id);
			D.Tree = Tree;
			D.RequiredLevel = Req;
			D.Cost = Cost;
			D.Requires = Requires;
			T.Add(D);
			return &T.Last();
		};

		// --- Выживание (танк/снабжение) ---
		Add(TEXT("Surv.ToughBody"), EBackroomsPerkTree::Survival, 1, 1, NAME_None)->BonusMaxHealth = 25.0f;
		Add(TEXT("Surv.IronStomach"), EBackroomsPerkTree::Survival, 2, 1, TEXT("Surv.ToughBody"))->HungerDrainScale = 0.20f;
		Add(TEXT("Surv.Endurance"), EBackroomsPerkTree::Survival, 2, 1, NAME_None)->BonusMaxStamina = 30.0f;
		Add(TEXT("Surv.Vitality"), EBackroomsPerkTree::Survival, 4, 2, TEXT("Surv.IronStomach"))->BonusMaxHealth = 20.0f;
		Add(TEXT("Surv.Adaptation"), EBackroomsPerkTree::Survival, 6, 2, TEXT("Surv.IronStomach"))->HungerDrainScale = 0.15f;
		Add(TEXT("Surv.Bulwark"), EBackroomsPerkTree::Survival, 8, 2, TEXT("Surv.Vitality"))->BonusMaxHealth = 40.0f;

		// --- Лазутчик (лут/мобильность) ---
		Add(TEXT("Scav.Pockets"), EBackroomsPerkTree::Scavenger, 1, 1, NAME_None)->BonusInventorySlots = 6.0f;
		Add(TEXT("Scav.Marathon"), EBackroomsPerkTree::Scavenger, 2, 1, NAME_None)->StaminaDrainScale = 0.15f;
		Add(TEXT("Scav.LightFeet"), EBackroomsPerkTree::Scavenger, 2, 1, NAME_None)->SpeedBonus = 0.08f;
		Add(TEXT("Scav.Battery"), EBackroomsPerkTree::Scavenger, 3, 1, NAME_None)->FlashlightDrainScale = 0.25f;
		Add(TEXT("Scav.DeepPockets"), EBackroomsPerkTree::Scavenger, 4, 2, TEXT("Scav.Pockets"))->BonusInventorySlots = 8.0f;
		Add(TEXT("Scav.Nomad"), EBackroomsPerkTree::Scavenger, 6, 2, TEXT("Scav.DeepPockets"))->BonusInventorySlots = 10.0f;
		Add(TEXT("Scav.Swift"), EBackroomsPerkTree::Scavenger, 8, 2, TEXT("Scav.LightFeet"))->SpeedBonus = 0.12f;

		// --- Разум (рассудок/страх) ---
		Add(TEXT("Mind.Focus"), EBackroomsPerkTree::Mind, 1, 1, NAME_None)->BonusSanity = 20.0f;
		Add(TEXT("Mind.Calm"), EBackroomsPerkTree::Mind, 2, 1, NAME_None)->SanityDrainScale = 0.20f;
		Add(TEXT("Mind.Nerve"), EBackroomsPerkTree::Mind, 2, 1, NAME_None)->FearResistScale = 0.25f;
		Add(TEXT("Mind.Insight"), EBackroomsPerkTree::Mind, 3, 1, NAME_None)->XPBoost = 0.15f;
		Add(TEXT("Mind.Meditation"), EBackroomsPerkTree::Mind, 4, 2, TEXT("Mind.Calm"))->SanityDrainScale = 0.20f;
		Add(TEXT("Mind.Clarity"), EBackroomsPerkTree::Mind, 6, 2, TEXT("Mind.Focus"))->BonusSanity = 30.0f;
		Add(TEXT("Mind.Unbroken"), EBackroomsPerkTree::Mind, 8, 2, TEXT("Mind.Meditation"))->FearResistScale = 0.30f;

		return T;
	}();
	return Table;
}

const TArray<FBackroomsPerkDef>& UBackroomsProgression::AllPerks() const
{
	return Rules();
}

int32 UBackroomsProgression::GetTotalPerks() const
{
	return Rules().Num();
}

FText UBackroomsProgression::PerkNameText(FName Id)
{
	return BackroomsLoc::Text(*FString::Printf(TEXT("Perk.%s.Name"), *Id.ToString()));
}

FText UBackroomsProgression::PerkDescText(FName Id)
{
	return BackroomsLoc::Text(*FString::Printf(TEXT("Perk.%s.Desc"), *Id.ToString()));
}

int32 UBackroomsProgression::GetXPForNextLevel() const
{
	// Квадратичная кривая: 120, 180, 240… — прокачка требует реального забега,
	// а не пары минут в игре (никакого «pay to win»).
	return XPPerLevelBase + XPPerLevelGrowth * (Level - 1);
}

int32 UBackroomsProgression::AwardXP(int32 BaseAmount, FName Reason)
{
	if (BaseAmount <= 0)
	{
		return 0;
	}
	const int32 Amount = FMath::RoundToInt(BaseAmount * (1.0f + Bonuses.XPBoost));
	const int32 OldLevel = Level;
	XP += Amount;
	while (XP >= GetXPForNextLevel())
	{
		XP -= GetXPForNextLevel();
		Level++;
		SkillPoints++;
		if (Level % 3 == 0)
		{
			SkillPoints++;
		}
	}
	if (Level != OldLevel)
	{
		OnProgressionChanged.Broadcast(Level);
		UE_LOG(LogBackroomsProgression, Log, TEXT("Уровень повышен: %d (XP +%d, причина %s)"), Level, Amount, *Reason.ToString());
	}
	else if (!Reason.IsNone())
	{
		UE_LOG(LogBackroomsProgression, Verbose, TEXT("XP +%d (%s)"), Amount, *Reason.ToString());
	}
	Save();
	return Amount;
}

void UBackroomsProgression::NotifyRoomDiscovered(int32 RoomId, int32 LevelIndex)
{
	const int32 Key = (LevelIndex * 1000003) ^ (RoomId * 2654435761u);
	if (DiscoveredRooms.Contains(Key))
	{
		return;
	}
	DiscoveredRooms.Add(Key);
	RunRooms.Add(Key);
	AwardXP(XPPerRoom, TEXT("RoomDiscovered"));
}

void UBackroomsProgression::NotifyChunkDiscovered(int32 ChunkX, int32 ChunkY, int32 LevelIndex)
{
	const int32 Key = (LevelIndex * 1000003) ^ (ChunkX * 31337) ^ (ChunkY * 6151);
	if (DiscoveredChunks.Contains(Key))
	{
		return;
	}
	DiscoveredChunks.Add(Key);
	AwardXP(XPPerChunk, TEXT("ChunkDiscovered"));
}

void UBackroomsProgression::NotifyDescended(int32 NewLevelIndex)
{
	if (NewLevelIndex <= 0)
	{
		return;
	}
	AwardXP(XPPerDescent * FMath::Max(1, NewLevelIndex), TEXT("Descended"));
}

void UBackroomsProgression::NotifyRunSurvived(float SecondsAtLevel)
{
	const int32 Minutes = FMath::FloorToInt(SecondsAtLevel / 60.0f);
	AwardXP(XPPerSurvival * FMath::Max(1, Minutes), TEXT("Survived"));
}

void UBackroomsProgression::TickRun(float DeltaSeconds)
{
	RunTime += DeltaSeconds;
}

void UBackroomsProgression::BeginRun(int32 Seed, int32 LevelIndex)
{
	RunSeed = Seed;
	RunLevelIndex = LevelIndex;
	RunStartLevel = LevelIndex;
	RunTime = 0.0f;
	RunRooms.Empty();
}

void UBackroomsProgression::EndRun(bool bSurvived)
{
	// Запись в лидерборд: очки складываются из времени, изученных комнат и
	// текущего уровня. Живёт локально, данные агрегируются со всех версий игры.
	FBackroomsRunRecord R;
	R.Seed = RunSeed;
	R.Difficulty = (int32)BackroomsDifficulty::GetCurrent();
	R.LevelIndex = RunLevelIndex;
	R.TimeSeconds = RunTime;
	R.RoomsDiscovered = RunRooms.Num();
	R.bSurvived = bSurvived;
	R.Date = FDateTime::Now();
	R.Score = FMath::RoundToInt(RunTime * 0.5f)
		+ R.RoomsDiscovered * 20
		+ RunLevelIndex * 100
		+ (bSurvived ? 500 : 0);

	if (bSurvived)
	{
		NotifyRunSurvived(RunTime);
	}
	SubmitRun(R);

	// Сброс забега под следующий.
	RunTime = 0.0f;
	RunRooms.Empty();
}

bool UBackroomsProgression::UnlockPerk(FName PerkId)
{
	if (UnlockedPerks.Contains(PerkId))
	{
		return false;
	}
	const FBackroomsPerkDef* Def = Rules().FindByPredicate([PerkId](const FBackroomsPerkDef& X) { return X.Id == PerkId; });
	if (!Def)
	{
		return false;
	}
	if (Level < Def->RequiredLevel || SkillPoints < Def->Cost)
	{
		return false;
	}
	if (!Def->Requires.IsNone() && !UnlockedPerks.Contains(Def->Requires))
	{
		return false;
	}
	UnlockedPerks.Add(PerkId);
	SkillPoints -= Def->Cost;
	RecomputeBonuses();
	OnPerkUnlocked.Broadcast(PerkId, Def->Name, SkillPoints);
	OnProgressionChanged.Broadcast(Level);
	Save();
	return true;
}

void UBackroomsProgression::RecomputeBonuses()
{
	FBackroomsPerkBonuses B;
	float StaminaCut = 0.0f;
	float SanityCut = 0.0f;
	float FearCut = 0.0f;
	float HungerCut = 0.0f;
	float FlashCut = 0.0f;
	float SpeedAdd = 0.0f;

	for (const FName& Id : UnlockedPerks)
	{
		const FBackroomsPerkDef* D = Rules().FindByPredicate([Id](const FBackroomsPerkDef& X) { return X.Id == Id; });
		if (!D)
		{
			continue;
		}
		B.MaxHealthAdd += D->BonusMaxHealth;
		B.MaxStaminaAdd += D->BonusMaxStamina;
		B.MaxSanityAdd += D->BonusSanity;
		B.InventorySlotsAdd += D->BonusInventorySlots;
		StaminaCut += D->StaminaDrainScale;
		SanityCut += D->SanityDrainScale;
		FearCut += D->FearResistScale;
		HungerCut += D->HungerDrainScale;
		FlashCut += D->FlashlightDrainScale;
		SpeedAdd += D->SpeedBonus;
		B.XPBoost += D->XPBoost;
	}

	B.StaminaDrainMult = FMath::Max(0.3f, 1.0f - StaminaCut);
	B.SanityDrainMult = FMath::Max(0.3f, 1.0f - SanityCut);
	B.FearGrowthMult = FMath::Max(0.3f, 1.0f - FearCut);
	B.SurvivalDrainMult = FMath::Max(0.3f, 1.0f - HungerCut);
	B.FlashlightDrainMult = FMath::Max(0.3f, 1.0f - FlashCut);
	B.SpeedMult = 1.0f + SpeedAdd;
	Bonuses = B;
}

void UBackroomsProgression::SubmitRun(const FBackroomsRunRecord& Record)
{
	Leaderboard.Add(Record);
	Leaderboard.Sort([](const FBackroomsRunRecord& A, const FBackroomsRunRecord& B) { return A.Score > B.Score; });
	if (Leaderboard.Num() > MaxLeaderboardEntries)
	{
		Leaderboard.SetNum(MaxLeaderboardEntries);
	}
	Save();
}

FString UBackroomsProgression::Report() const
{
	FString Out = FString::Printf(TEXT("Уровень %d (XP %d/%d), очки %d, перки %d/%d\n"),
		Level, XP, GetXPForNextLevel(), SkillPoints, UnlockedPerks.Num(), GetTotalPerks());
	for (const FBackroomsRunRecord& R : Leaderboard)
	{
		Out += FString::Printf(TEXT("  Seed %d | сложн. %d | ур. %d | очки %d | %.0f с | комнат %d\n"),
			R.Seed, R.Difficulty, R.LevelIndex, R.Score, R.TimeSeconds, R.RoomsDiscovered);
	}
	return Out;
}

void UBackroomsProgression::ResetAll()
{
	XP = 0;
	Level = 1;
	SkillPoints = 0;
	UnlockedPerks.Empty();
	DiscoveredRooms.Empty();
	DiscoveredChunks.Empty();
	Leaderboard.Empty();
	RecomputeBonuses();
	Save();
}

void UBackroomsProgression::Save() const
{
	FString Out;
	Out += FString::Printf(TEXT("XP %d\n"), XP);
	Out += FString::Printf(TEXT("LEVEL %d\n"), Level);
	Out += FString::Printf(TEXT("POINTS %d\n"), SkillPoints);
	for (const FName& Id : UnlockedPerks)
	{
		Out += FString::Printf(TEXT("PERK %s\n"), *Id.ToString());
	}
	for (const int32 Key : DiscoveredRooms)
	{
		Out += FString::Printf(TEXT("ROOM %d\n"), Key);
	}
	for (const int32 Key : DiscoveredChunks)
	{
		Out += FString::Printf(TEXT("CHUNK %d\n"), Key);
	}
	for (const FBackroomsRunRecord& R : Leaderboard)
	{
		Out += FString::Printf(TEXT("RUN %d %d %d %d %.2f %d %d %lld\n"),
			R.Seed, R.Difficulty, R.LevelIndex, R.Score, R.TimeSeconds, R.RoomsDiscovered,
			R.bSurvived ? 1 : 0, R.Date.GetTicks());
	}
	FFileHelper::SaveStringToFile(Out, *GetSavePath(), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

void UBackroomsProgression::Load()
{
	FString Raw;
	if (!FFileHelper::LoadFileToString(Raw, *GetSavePath()))
	{
		return;
	}
	TArray<FString> Lines;
	Raw.ParseIntoArrayLines(Lines, true);
	for (const FString& Line : Lines)
	{
		TArray<FString> Parts;
		Line.ParseIntoArrayWS(Parts);
		if (Parts.Num() == 0)
		{
			continue;
		}
		const FString& Tag = Parts[0];
		if (Tag == TEXT("XP") && Parts.Num() >= 2)
		{
			XP = FCString::Atoi(*Parts[1]);
		}
		else if (Tag == TEXT("LEVEL") && Parts.Num() >= 2)
		{
			Level = FCString::Atoi(*Parts[1]);
		}
		else if (Tag == TEXT("POINTS") && Parts.Num() >= 2)
		{
			SkillPoints = FCString::Atoi(*Parts[1]);
		}
		else if (Tag == TEXT("PERK") && Parts.Num() >= 2)
		{
			UnlockedPerks.Add(FName(*Parts[1]));
		}
		else if (Tag == TEXT("ROOM") && Parts.Num() >= 2)
		{
			DiscoveredRooms.Add(FCString::Atoi(*Parts[1]));
		}
		else if (Tag == TEXT("CHUNK") && Parts.Num() >= 2)
		{
			DiscoveredChunks.Add(FCString::Atoi(*Parts[1]));
		}
		else if (Tag == TEXT("RUN") && Parts.Num() >= 8)
		{
			FBackroomsRunRecord R;
			R.Seed = FCString::Atoi(*Parts[1]);
			R.Difficulty = FCString::Atoi(*Parts[2]);
			R.LevelIndex = FCString::Atoi(*Parts[3]);
			R.Score = FCString::Atoi(*Parts[4]);
			R.TimeSeconds = FCString::Atof(*Parts[5]);
			R.RoomsDiscovered = FCString::Atoi(*Parts[6]);
			R.bSurvived = FCString::Atoi(*Parts[7]) != 0;
			if (Parts.Num() >= 9)
			{
				R.Date = FDateTime(FCString::Atoi64(*Parts[8]));
			}
			Leaderboard.Add(R);
		}
	}
	Leaderboard.Sort([](const FBackroomsRunRecord& A, const FBackroomsRunRecord& B) { return A.Score > B.Score; });
}
#include "BackroomsAchievements.h"
#include "BackroomsLocalization.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Internationalization/Text.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

// Идентификаторы статистик. Вынесены в статики, чтобы геймплей писал по имени
// без магических строк и чтобы опечатку ловил линкер.
const FName UBackroomsAchievements::StatLevelsVisited(TEXT("LevelsVisited"));
const FName UBackroomsAchievements::StatTimeInBackrooms(TEXT("TimeInBackrooms"));
const FName UBackroomsAchievements::StatDistanceCm(TEXT("DistanceCm"));
const FName UBackroomsAchievements::StatItemsPickedUp(TEXT("ItemsPickedUp"));
const FName UBackroomsAchievements::StatItemsUsed(TEXT("ItemsUsed"));
const FName UBackroomsAchievements::StatMedicinesUsed(TEXT("MedicinesUsed"));
const FName UBackroomsAchievements::StatAlmondUsed(TEXT("AlmondUsed"));
const FName UBackroomsAchievements::StatMonsterEncounters(TEXT("MonsterEncounters"));
const FName UBackroomsAchievements::StatExitsUsed(TEXT("ExitsUsed"));
const FName UBackroomsAchievements::StatDeaths(TEXT("Deaths"));
const FName UBackroomsAchievements::StatMaxPressure(TEXT("MaxPressure"));
const FName UBackroomsAchievements::StatFoodUsed(TEXT("FoodUsed"));
const FName UBackroomsAchievements::StatBatteriesUsed(TEXT("BatteriesUsed"));
const FName UBackroomsAchievements::StatSprints(TEXT("Sprints"));
const FName UBackroomsAchievements::StatRoomsEntered(TEXT("RoomsEntered"));

namespace
{
	// Одна таблица: достижение = (стат, порог). Порядок — как в меню.
	TArray<FBackroomsAchievementDef> BuildRules()
	{
		auto Make = [](const TCHAR* Id, const TCHAR* Name, const TCHAR* Desc,
			FName Stat, float Threshold, int32 Points, bool bHidden = false)
		{
			FBackroomsAchievementDef D;
			D.Id = FName(Id);
			D.Name = FText::FromString(Name);
			D.Description = FText::FromString(Desc);
			D.Stat = Stat;
			D.Threshold = Threshold;
			D.Points = Points;
			D.bHidden = bHidden;
			return D;
		};

		TArray<FBackroomsAchievementDef> R;
		R.Add(Make(TEXT("FirstSteps"), TEXT("Первые шаги"),
			TEXT("Спуститься в Бэкрумс."), UBackroomsAchievements::StatTimeInBackrooms, 0.1f, 10));
		R.Add(Make(TEXT("Tourist"), TEXT("Турист"),
			TEXT("Побывать в 3 разных локациях."), UBackroomsAchievements::StatLevelsVisited, 3.0f, 15));
		R.Add(Make(TEXT("Explorer"), TEXT("Исследователь"),
			TEXT("Побывать в 5 разных локациях."), UBackroomsAchievements::StatLevelsVisited, 5.0f, 25));
		R.Add(Make(TEXT("DeepDiver"), TEXT("На глубине"),
			TEXT("Побывать в 7 разных локациях."), UBackroomsAchievements::StatLevelsVisited, 7.0f, 40));
		R.Add(Make(TEXT("AllLevels"), TEXT("Весь этаж"),
			TEXT("Побывать во всех 10 локациях."), UBackroomsAchievements::StatLevelsVisited, 10.0f, 80));
		R.Add(Make(TEXT("Survivor"), TEXT("Выживший"),
			TEXT("Продержаться в Бэкрумсе 15 минут."), UBackroomsAchievements::StatTimeInBackrooms, 900.0f, 30));
		R.Add(Make(TEXT("Marathon"), TEXT("Марафонец"),
			TEXT("Пройти 3 километра под землёй."), UBackroomsAchievements::StatDistanceCm, 300000.0f, 25));
		R.Add(Make(TEXT("Scavenger"), TEXT("Собиратель"),
			TEXT("Подобрать 10 предметов."), UBackroomsAchievements::StatItemsPickedUp, 10.0f, 15));
		R.Add(Make(TEXT("Pharmacist"), TEXT("Фармацевт"),
			TEXT("Использовать 5 медикаментов."), UBackroomsAchievements::StatMedicinesUsed, 5.0f, 20));
		R.Add(Make(TEXT("Hydrated"), TEXT("Миндальный вкус"),
			TEXT("Выпить 5 порций миндальной воды."), UBackroomsAchievements::StatAlmondUsed, 5.0f, 20));
		R.Add(Make(TEXT("MonsterAware"), TEXT("Оно рядом"),
			TEXT("Встретить сущность и выжить."), UBackroomsAchievements::StatMonsterEncounters, 1.0f, 30));
		R.Add(Make(TEXT("EscapeArtist"), TEXT("Первый выход"),
			TEXT("Найти выход из локации."), UBackroomsAchievements::StatExitsUsed, 1.0f, 20));
		R.Add(Make(TEXT("FrequentFlyer"), TEXT("Частый гость"),
			TEXT("Пройти через 5 выходов."), UBackroomsAchievements::StatExitsUsed, 5.0f, 45));
		R.Add(Make(TEXT("DeepFear"), TEXT("Предел страха"),
			TEXT("Довести давление среды до максимума."), UBackroomsAchievements::StatMaxPressure, 6.0f, 40));
		R.Add(Make(TEXT("Dead"), TEXT("Один из них"),
			TEXT("Погибнуть в Бэкрумсе."), UBackroomsAchievements::StatDeaths, 1.0f, 5, true));
		R.Add(Make(TEXT("WellFed"), TEXT("Сытый"),
			TEXT("Съесть 10 порций еды."), UBackroomsAchievements::StatFoodUsed, 10.0f, 15));
		R.Add(Make(TEXT("Electrician"), TEXT("Электрик"),
			TEXT("Вставить 10 батареек в фонарик."), UBackroomsAchievements::StatBatteriesUsed, 10.0f, 20));
		R.Add(Make(TEXT("Sprinter"), TEXT("Бегун"),
			TEXT("Начать бег 50 раз."), UBackroomsAchievements::StatSprints, 50.0f, 20));
		R.Add(Make(TEXT("Cartographer"), TEXT("Картограф"),
			TEXT("Пройти 200 комнат."), UBackroomsAchievements::StatRoomsEntered, 200.0f, 35));
		R.Add(Make(TEXT("Speedrun"), TEXT("Скороход"),
			TEXT("Пройти 5 километров под землёй."), UBackroomsAchievements::StatDistanceCm, 500000.0f, 30));
		R.Add(Make(TEXT("Gourmand"), TEXT("Гурман"),
			TEXT("Продержаться 30 минут в Бэкрумсе."), UBackroomsAchievements::StatTimeInBackrooms, 1800.0f, 45));
		return R;
	}
}

FString UBackroomsAchievements::GetSavePath()
{
	return FPaths::ProjectSavedDir() / TEXT("Backrooms/Achievements.dat");
}

void UBackroomsAchievements::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Load();
	Evaluate();
}

void UBackroomsAchievements::Deinitialize()
{
	Save();
	Super::Deinitialize();
}

const TArray<FBackroomsAchievementDef>& UBackroomsAchievements::Rules() const
{
	static const TArray<FBackroomsAchievementDef> R = BuildRules();
	return R;
}

const FBackroomsAchievementDef* UBackroomsAchievements::FindDef(FName Id) const
{
	for (const FBackroomsAchievementDef& D : Rules())
	{
		if (D.Id == Id)
		{
			return &D;
		}
	}
	return nullptr;
}

void UBackroomsAchievements::AddStat(FName Stat, float Amount)
{
	if (Stat.IsNone() || Amount == 0.0f)
	{
		return;
	}
	float& V = Stats.FindOrAdd(Stat);
	V += Amount;
	Evaluate();
}

void UBackroomsAchievements::SetStatMax(FName Stat, float Value)
{
	float& V = Stats.FindOrAdd(Stat);
	if (Value > V)
	{
		V = Value;
		Evaluate();
	}
}

void UBackroomsAchievements::NotifyLevelVisited(int32 LevelIndex)
{
	if (VisitedLevels.Contains(LevelIndex))
	{
		return;
	}
	VisitedLevels.Add(LevelIndex);
	Stats.FindOrAdd(StatLevelsVisited) = (float)VisitedLevels.Num();
	Evaluate();
}

void UBackroomsAchievements::NotifyItemPickedUp()
{
	AddStat(StatItemsPickedUp, 1.0f);
}

void UBackroomsAchievements::NotifyItemUsed(int32 Category)
{
	AddStat(StatItemsUsed, 1.0f);
	// Категории совпадают с EBackroomsItemCategory: 0=Food, 2=Medicine,
	// 3=Battery, 9=AlmondWater.
	if (Category == 2)
	{
		AddStat(StatMedicinesUsed, 1.0f);
	}
	else if (Category == 9)
	{
		AddStat(StatAlmondUsed, 1.0f);
	}
	else if (Category == 0)
	{
		AddStat(StatFoodUsed, 1.0f);
	}
	else if (Category == 3)
	{
		AddStat(StatBatteriesUsed, 1.0f);
	}
}

void UBackroomsAchievements::NotifyBackroomsEntered()
{
	// Крошечный ненулевой сдвиг открывает «Первые шаги» (порог 0.1).
	AddStat(StatTimeInBackrooms, 0.11f);
}

void UBackroomsAchievements::NotifyMonsterEncountered()
{
	AddStat(StatMonsterEncounters, 1.0f);
}

void UBackroomsAchievements::NotifyExitUsed()
{
	AddStat(StatExitsUsed, 1.0f);
}

void UBackroomsAchievements::NotifyDeath()
{
	AddStat(StatDeaths, 1.0f);
}

void UBackroomsAchievements::NotifyTimeInBackrooms(float DeltaSeconds)
{
	if (DeltaSeconds > 0.0f)
	{
		AddStat(StatTimeInBackrooms, DeltaSeconds);
	}
}

void UBackroomsAchievements::NotifyDistance(float Centimeters)
{
	if (Centimeters > 0.0f)
	{
		AddStat(StatDistanceCm, Centimeters);
	}
}

bool UBackroomsAchievements::IsUnlocked(FName Id) const
{
	return Unlocked.Contains(Id);
}

float UBackroomsAchievements::GetStat(FName Stat) const
{
	const float* V = Stats.Find(Stat);
	return V ? *V : 0.0f;
}

int32 UBackroomsAchievements::GetTotalCount() const
{
	return Rules().Num();
}

int32 UBackroomsAchievements::GetTotalPoints() const
{
	int32 P = 0;
	for (const FBackroomsAchievementDef& D : Rules())
	{
		if (Unlocked.Contains(D.Id))
		{
			P += D.Points;
		}
	}
	return P;
}

void UBackroomsAchievements::Unlock(FName Id)
{
	if (Unlocked.Contains(Id))
	{
		return;
	}
	const FBackroomsAchievementDef* D = FindDef(Id);
	if (!D)
	{
		UE_LOG(LogTemp, Warning, TEXT("BackroomsAchievements: unknown achievement %s"), *Id.ToString());
		return;
	}
Unlocked.Add(Id);
	// Локализованное имя достижения; если перевода нет — падаем на определение.
	const FString NameKey = FString::Printf(TEXT("Ach.%s.Name"), *Id.ToString());
	const FString LocName = BackroomsLoc::Get(*NameKey);
	const FText Title = (LocName == NameKey) ? D->Name : FText::FromString(LocName);
	OnAchievementUnlocked.Broadcast(Id, Title);
	UE_LOG(LogTemp, Display, TEXT("BackroomsAchievements: UNLOCKED '%s'"), *Title.ToString());
	Save();
}

void UBackroomsAchievements::Evaluate()
{
	for (const FBackroomsAchievementDef& D : Rules())
	{
		if (Unlocked.Contains(D.Id))
		{
			continue;
		}
		const float* V = Stats.Find(D.Stat);
		if (V && *V >= D.Threshold)
		{
			Unlock(D.Id);
		}
	}
}

FString UBackroomsAchievements::Report() const
{
FString S;
	S += FString::Printf(TEXT("=== ACHIEVEMENTS: %d/%d, points %d ===\n"),
		GetUnlockedCount(), GetTotalCount(), GetTotalPoints());
	for (const FBackroomsAchievementDef& D : Rules())
	{
		const bool bUnlocked = Unlocked.Contains(D.Id);
		const float Value = GetStat(D.Stat);
		// Локализованное имя (ключ Ach.<Id>.Name); если перевода нет — вернётся
		// ключ, тогда падаем на имя из определения.
		const FString NameKey = FString::Printf(TEXT("Ach.%s.Name"), *D.Id.ToString());
		const FString LocName = BackroomsLoc::Get(*NameKey);
		const FString Title = (D.bHidden && !bUnlocked)
			? TEXT("???")
			: ((LocName == NameKey) ? D.Name.ToString() : LocName);
		// Локализованное описание — идёт в карточку меню после разделителя.
		const FString DescKey = FString::Printf(TEXT("Ach.%s.Desc"), *D.Id.ToString());
		const FString LocDesc = BackroomsLoc::Get(*DescKey);
		const FString Desc = ((D.bHidden && !bUnlocked) || LocDesc == DescKey)
			? FString()
			: LocDesc;
		S += FString::Printf(TEXT("  [%s] %s (%.1f/%.1f)%s\n"),
			bUnlocked ? TEXT("X") : TEXT(" "), *Title, Value, D.Threshold,
			Desc.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" | %s"), *Desc));
	}
	return S;
}

void UBackroomsAchievements::ResetAll()
{
	Unlocked.Reset();
	Stats.Reset();
	VisitedLevels.Reset();
	Save();
	UE_LOG(LogTemp, Display, TEXT("BackroomsAchievements: progress reset"));
}

void UBackroomsAchievements::Save() const
{
	const FString Path = GetSavePath();
	const FString Dir = FPaths::GetPath(Path);
	IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
	if (!PF.DirectoryExists(*Dir))
	{
		PF.CreateDirectoryTree(*Dir);
	}

	FString Out;
	Out += TEXT("# Backrooms achievements save v1\n");
	for (const FName& Id : Unlocked)
	{
		Out += FString::Printf(TEXT("ACH %s\n"), *Id.ToString());
	}
	for (const TPair<FName, float>& KV : Stats)
	{
		Out += FString::Printf(TEXT("STAT %s %.6f\n"), *KV.Key.ToString(), KV.Value);
	}
	for (int32 L : VisitedLevels)
	{
		Out += FString::Printf(TEXT("LEVEL %d\n"), L);
	}

	FFileHelper::SaveStringToFile(Out, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

void UBackroomsAchievements::Load()
{
	const FString Path = GetSavePath();
	FString Content;
	if (!FFileHelper::LoadFileToString(Content, *Path))
	{
		return; // нет сохранения — стартуем с чистого листа
	}

	TArray<FString> Lines;
	Content.ParseIntoArrayLines(Lines, false);
	for (const FString& LineRaw : Lines)
	{
		const FString Line = LineRaw.TrimStartAndEnd();
		if (Line.IsEmpty() || Line.StartsWith(TEXT("#")))
		{
			continue;
		}
		FString Key, A, B;
		Line.Split(TEXT(" "), &Key, &A);
		if (Key == TEXT("ACH"))
		{
			Unlocked.Add(FName(*A.TrimStartAndEnd()));
		}
		else if (Key == TEXT("STAT"))
		{
			A.Split(TEXT(" "), &A, &B);
			if (!A.IsEmpty() && !B.IsEmpty())
			{
				Stats.Add(FName(*A.TrimStartAndEnd()), FCString::Atof(*B.TrimStartAndEnd()));
			}
		}
		else if (Key == TEXT("LEVEL"))
		{
			VisitedLevels.Add(FCString::Atoi(*A.TrimStartAndEnd()));
		}
	}
}

// ---- Консольные команды (отладка достижений) ----
namespace
{
	UBackroomsAchievements* FindAchievements()
	{
		if (!GEngine)
		{
			return nullptr;
		}
		UWorld* World = GEngine->GetCurrentPlayWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UBackroomsAchievements>() : nullptr;
	}

	static FAutoConsoleCommand CmdList(
		TEXT("BR.Ach.List"),
		TEXT("Show achievements list and progress."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			if (UBackroomsAchievements* A = FindAchievements())
			{
				UE_LOG(LogTemp, Display, TEXT("\n%s"), *A->Report());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("BR.Ach.List: achievements subsystem unavailable."));
			}
		})
	);

	static FAutoConsoleCommand CmdUnlock(
		TEXT("BR.Ach.Unlock"),
		TEXT("Unlock an achievement: BR.Ach.Unlock FirstSteps"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 1) { return; }
			if (UBackroomsAchievements* A = FindAchievements())
			{
				A->Unlock(FName(*Args[0]));
			}
		})
	);

	static FAutoConsoleCommand CmdReset(
		TEXT("BR.Ach.Reset"),
		TEXT("Reset all achievements."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			if (UBackroomsAchievements* A = FindAchievements())
			{
				A->ResetAll();
			}
		})
	);
}

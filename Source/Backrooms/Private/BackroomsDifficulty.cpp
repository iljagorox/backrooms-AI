#include "BackroomsDifficulty.h"
#include "LevelGeneratorProfile.h"
#include "BackroomsLocalization.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
	const TCHAR* GSection = TEXT("/Script/Backrooms.BackroomsDifficulty");
	const TCHAR* GKey = TEXT("Difficulty");
}

const TArray<FBackroomsDifficultyDef>& BackroomsDifficulty::GetAll()
{
	static const TArray<FBackroomsDifficultyDef> Table = []()
	{
		TArray<FBackroomsDifficultyDef> T;

		auto Add = [&T](EBackroomsDifficulty D, const TCHAR* Name, const TCHAR* Desc,
			float Wall, float Door, float Prop, float Scatter,
			int32 MaxMon, float Delay, float Pressure, int32 Supplies,
			float Drain, bool bPerma, FLinearColor Accent,
			float Utility, float Decor, int32 MinUtility)
		{
			FBackroomsDifficultyDef Def;
			Def.Id = FName(UEnum::GetValueAsName(D).ToString());
			Def.Name = FText::FromString(Name);
			Def.Description = FText::FromString(Desc);
			Def.WallBias = Wall;
			Def.DoorBias = Door;
			Def.PropBias = Prop;
			Def.ScatterBias = Scatter;
			Def.MaxMonsters = MaxMon;
			Def.MonsterDelayScale = Delay;
			Def.PressureScale = Pressure;
			Def.StartingSupplies = Supplies;
			Def.SurvivalDrainScale = Drain;
			Def.bPermadeath = bPerma;
			Def.Accent = Accent;
			Def.UtilityDropBias = Utility;
			Def.DecorChance = Decor;
			Def.MinUtilityPerChunk = MinUtility;
			T.Add(Def);
		};

		// Utility — множитель полезного дропа; держим его близко к 1.0 даже на
		// «Кошмаре»: играть должно быть тяжело, но не безнадёжно. Полезного
		// становится меньше, зато декора (DecorChance) — больше, мир не пустеет.
		Add(EBackroomsDifficulty::Peaceful, TEXT("Мирный"),
			TEXT("Без монстров и давления. Только исследование и вид мира."),
			0.80f, 1.35f, 1.40f, 1.0f, 0, 99.0f, 0.0f, 4, 0.6f, false,
			FLinearColor(0.55f, 0.85f, 0.60f), 1.50f, 0.06f, 4);

		Add(EBackroomsDifficulty::Easy, TEXT("Лёгкий"),
			TEXT("Свободные, щедрые локации. Один монстр появляется поздно."),
			0.90f, 1.20f, 1.25f, 1.0f, 1, 1.6f, 0.7f, 2, 0.8f, false,
			FLinearColor(0.60f, 0.82f, 0.55f), 1.25f, 0.07f, 2);

		Add(EBackroomsDifficulty::Normal, TEXT("Обычный"),
			TEXT("Задуманный баланс: тесные коридоры, голод, одиночная сущность."),
			1.00f, 1.00f, 1.00f, 1.0f, 1, 1.0f, 1.0f, 1, 1.0f, false,
			FLinearColor(0.95f, 0.85f, 0.45f), 1.00f, 0.08f, 1);

		Add(EBackroomsDifficulty::Hard, TEXT("Сложный"),
			TEXT("Больше стен и меньше дверей, скудный лут, две сущности."),
			1.20f, 0.92f, 0.70f, 1.1f, 2, 0.7f, 1.4f, 0, 1.25f, false,
			FLinearColor(0.92f, 0.55f, 0.30f), 0.75f, 0.12f, 1);

		Add(EBackroomsDifficulty::Nightmare, TEXT("Кошмар"),
			TEXT("Лабиринт-ловушка, минимум припасов, три сущности, смерть окончательна."),
			1.45f, 0.82f, 0.45f, 1.25f, 3, 0.5f, 2.0f, 0, 1.6f, true,
			FLinearColor(0.85f, 0.20f, 0.20f), 0.55f, 0.16f, 1);

		return T;
	}();
	return Table;
}

const FBackroomsDifficultyDef& BackroomsDifficulty::Get(EBackroomsDifficulty D)
{
	const TArray<FBackroomsDifficultyDef>& All = GetAll();
	const int32 I = (int32)D;
	if (I < 0 || I >= All.Num())
	{
		return All[(int32)EBackroomsDifficulty::Normal];
	}
	return All[I];
}

FString BackroomsDifficulty::Name(EBackroomsDifficulty D)
{
	// Имена/описания берём из таблицы локализации (ключи Diff.<Id>.Name/.Desc),
	// чтобы сложность была переведена на все языки. Если строки нет — вернётся
	// ключ, и это сразу видно.
	return BackroomsLoc::Get(*FString::Printf(TEXT("Diff.%s.Name"), *Get(D).Id.ToString()));
}

FText BackroomsDifficulty::NameText(EBackroomsDifficulty D)
{
	return FText::FromString(Name(D));
}

FText BackroomsDifficulty::DescriptionText(EBackroomsDifficulty D)
{
	return FText::FromString(BackroomsLoc::Get(*FString::Printf(TEXT("Diff.%s.Desc"), *Get(D).Id.ToString())));
}

EBackroomsDifficulty BackroomsDifficulty::GetCurrent()
{
	int32 V = (int32)EBackroomsDifficulty::Normal;
	if (GConfig)
	{
		GConfig->GetInt(GSection, GKey, V, GGameUserSettingsIni);
	}
	if (V < 0 || V >= (int32)EBackroomsDifficulty::Count)
	{
		V = (int32)EBackroomsDifficulty::Normal;
	}
	return (EBackroomsDifficulty)V;
}

void BackroomsDifficulty::SetCurrent(EBackroomsDifficulty D)
{
	if (GConfig)
	{
		GConfig->SetInt(GSection, GKey, (int32)D, GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}
}

void BackroomsDifficulty::ApplyToProfile(ULevelGeneratorProfile* Profile, EBackroomsDifficulty D)
{
	if (!Profile)
	{
		return;
	}
	const FBackroomsDifficultyDef& Def = Get(D);

	// Пороги — это «границы» плотности, поэтому множители применяем аккуратно
	// и зажимаем в разумный диапазон: сложность не должна ломать генератор.
	Profile->WallThreshold = FMath::Clamp(Profile->WallThreshold * Def.WallBias, 0.05f, 0.75f);
	Profile->DoorThreshold = FMath::Clamp(Profile->DoorThreshold * Def.DoorBias, 0.02f, 0.45f);
	Profile->ScatterThreshold = FMath::Clamp(Profile->ScatterThreshold * Def.ScatterBias, 0.05f, 0.95f);

	// Плотность пропсов («щедрость» лута).
	Profile->MaxPropsPerRoom = FMath::Clamp(Profile->MaxPropsPerRoom * Def.PropBias, 0.2f, 12.0f);

	// Шансы дропа: полезного меньше, декора — больше. Ставим нижние пороги,
	// чтобы «Кошмар» не превращался в безнадёгу.
	Profile->ItemPickupChance = FMath::Clamp(0.10f * Def.UtilityDropBias, 0.04f, 0.20f);
	Profile->DecorPropChance = FMath::Clamp(Def.DecorChance, 0.02f, 0.25f);
	Profile->MinUtilityPerChunk = FMath::Max(1, Def.MinUtilityPerChunk);
}

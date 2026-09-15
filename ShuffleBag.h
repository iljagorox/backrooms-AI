#pragma once

#include "CoreMinimal.h"

// «Мешок случайностей» (shuffle bag / deck).
// Идея: имеем набор вариантов (например 10-15 звуков шагов или дёрганья двери).
// Кладём их все в «колоду», тасуем и раздаём по одному. Когда колода кончается —
// тасуем новую. Гарантирует, что каждый вариант выпадает примерно одинаково
// часто, при этом не повторяется дважды подряд (в отличие от чистого FRand,
// который может зациклиться на одном звуке) — ощущается естественно, не
// «скриптованно» и не надоедает.
//
// Использование:
//   ShuffleBag Bag;
//   Bag.Init(15);          // 15 вариантов
//   int32 Idx = Bag.Next(); // 0..14, без немедленного повтора
class BACKROOMS_API FShuffleBag
{
public:
	// Задать число вариантов (от 1 до N) и перемешать первоначальную колоду.
	void Init(int32 InNumVariants)
	{
		NumVariants = FMath::Max(1, InNumVariants);
		LastDrawn = INDEX_NONE;
		BuildDeck();
	}

	// Взять следующий вариант. Когда колода опустела — построить и тасовать новую,
	// причём так, чтобы первый элемент новой колоды не совпал с последним
	// выданным (убирает повтор на стыке колод).
	int32 Next()
	{
		if (NumVariants <= 1)
		{
			return NumVariants == 1 ? 0 : INDEX_NONE;
		}

		if (Deck.Num() == 0)
		{
			BuildDeck();
			// Не даём первой карте новой колоды совпасть с последней выданной.
			if (LastDrawn != INDEX_NONE && Deck.Num() > 1 && Deck[0] == LastDrawn)
			{
				const int32 SwapWith = 1 + Rng.RandRange(0, Deck.Num() - 2);
				Deck.Swap(0, SwapWith);
			}
		}

		LastDrawn = Deck.Pop(EAllowShrinking::No);
		return LastDrawn;
	}

	// Сколько вариантов сейчас в колоде (до конца).
	int32 Remaining() const { return Deck.Num(); }

	int32 GetNumVariants() const { return NumVariants; }

private:
	void BuildDeck()
	{
		Deck.Reset();
		Deck.SetNumUninitialized(NumVariants);
		for (int32 i = 0; i < NumVariants; ++i)
		{
			Deck[i] = i;
		}
		// Тасование Фишера-Йетса (каждая перестановка равновероятна).
		for (int32 i = NumVariants - 1; i > 0; --i)
		{
			const int32 j = Rng.RandRange(0, i);
			Deck.Swap(i, j);
		}
	}

	// Локальный генератор: не зависит от глобального FMath::Rand (потокобезопаснее).
	FRandomStream Rng;
	int32 NumVariants = 0;
	int32 LastDrawn = INDEX_NONE;
	TArray<int32> Deck;
};

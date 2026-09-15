#include "BackroomsItemSystem.h"
#include "BackroomsInventoryComponent.h"
#include "BackroomsInventoryData.h"
#include "BackroomsItemPickup.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/StrongObjectPtr.h"

void UBackroomsItemSystem::InitializeDefaultItems()
{
	BuildDefaultItemDefs(ItemDefs);
}

void UBackroomsItemSystem::SetInventoryComponent(UInventoryComponent* InComponent)
{
	InventoryComponent = InComponent;
}

UItemDataAsset* UBackroomsItemSystem::GetItemDataAsset(FName ItemId)
{
	// Runtime-ассеты для встроенных предметов каталога: создаются один раз и
	// держатся в статической карте (каталог фиксирован, предметы меняются).
	// TObjectPtr в статике не корневит ассеты: без сильной ссылки GC собирал
	// transient-ассеты, и меши/иконки «поехали рандомно». TStrongObjectPtr
	// держит каждый ассет живым до выхода из приложения.
	static TMap<FName, TStrongObjectPtr<UItemDataAsset>> DataAssets;

	if (const TStrongObjectPtr<UItemDataAsset>* Found = DataAssets.Find(ItemId))
	{
		return Found->Get();
	}

	TArray<FBackroomsItemDef> Defs;
	BuildDefaultItemDefs(Defs);
	const FBackroomsItemDef* Def = nullptr;
	for (const FBackroomsItemDef& D : Defs)
	{
		if (D.Id == ItemId)
		{
			Def = &D;
			break;
		}
	}

	UItemDataAsset* Asset = NewObject<UItemDataAsset>(
		GetTransientPackage(), UItemDataAsset::StaticClass(),
		*FString::Printf(TEXT("ItemData_%s"), *ItemId.ToString()));
	if (Def)
	{
		Asset->DisplayName = FText::FromString(Def->DisplayName);
		Asset->Description = FText::FromString(Def->Description);
		Asset->Category = Def->Category;
		Asset->ItemType = Def->UseType;
		Asset->MaxStack = Def->MaxStackSize;
		Asset->RestoreAmount = Def->RestoreAmount;
		Asset->RestoreType = Def->RestoreType;
		Asset->RestoreAmount2 = Def->RestoreAmount2;
		Asset->RestoreType2 = Def->RestoreType2;
		Asset->WorldMesh = Def->WorldMesh;
		Asset->Icon = Def->Icon;
		Asset->HandSocketName = Def->HandSocketName;
		Asset->bCanHold = Def->bCanHold;
		Asset->bInspectable = Def->bInspectable;
		Asset->EquipMontage = Def->EquipMontage;
		Asset->UseMontage = Def->UseMontage;
		Asset->ThrowImpulseRange = Def->ThrowImpulseRange;
		Asset->NoiseRadius = Def->NoiseRadius;
		Asset->bFlashlight = Def->bFlashlight;
	}
	else
	{
		Asset->DisplayName = FText::FromName(ItemId);
		Asset->ItemType = EBackroomsItemUseType::Consume;
	}
	Asset->HeldMesh = Asset->WorldMesh;

	DataAssets.Emplace(ItemId, TStrongObjectPtr<UItemDataAsset>(Asset));
	return Asset;
}

void UBackroomsItemSystem::BuildDefaultItemDefs(TArray<FBackroomsItemDef>& OutDefs)
{
	OutDefs.Reset();

	auto Add = [&OutDefs](FName Id, const FString& Name, EBackroomsItemCategory Cat,
		EBackroomsItemUseType Use, int32 MaxStack, float Restore, int32 RestoreType,
		float Restore2 = 0.0f, int32 RestoreType2 = -1)
	{
		FBackroomsItemDef& D = OutDefs.AddDefaulted_GetRef();
		D.Id = Id;
		D.DisplayName = Name;
		D.Category = Cat;
		D.UseType = Use;
		D.MaxStackSize = MaxStack;
		D.RestoreAmount = Restore;
		D.RestoreType = RestoreType;
		D.RestoreAmount2 = Restore2;
		D.RestoreType2 = RestoreType2;
		D.UseTime = (Use == EBackroomsItemUseType::Consume) ? 2.0f : 1.0f;
	};

	// Миндальная вода: утоляет жажду И успокаивает (в лоре Backrooms — главный
	// источник рассудка). Второй эффект добавлен, чтобы найти её было ценно.
	Add(TEXT("AlmondWater"), TEXT("Миндальная вода"), EBackroomsItemCategory::AlmondWater, EBackroomsItemUseType::Consume, 6, 40.0f, 1, 30.0f, 3);
	Add(TEXT("CanFood"),     TEXT("Тушёнка"),        EBackroomsItemCategory::Food,       EBackroomsItemUseType::Consume, 6, 30.0f, 0);
	Add(TEXT("MedKit"),      TEXT("Аптечка"),        EBackroomsItemCategory::Medicine,   EBackroomsItemUseType::Consume, 3, 50.0f, 2);
	Add(TEXT("Pill"),        TEXT("Успокоительное"), EBackroomsItemCategory::Medicine,   EBackroomsItemUseType::Consume, 5, 70.0f, 3);   // рассудок
	// Энергетик/чай: бодрость + немного рассудка. Отдельный расходник, чтобы
	// рассудок можно было поднимать не только успокоительным.
	Add(TEXT("Energy"),      TEXT("Энергетик"),      EBackroomsItemCategory::Energy,     EBackroomsItemUseType::Consume, 5, 25.0f, 3);
	// Батарейка — ПРЕДМЕТ ДЛЯ ФОНАРИКА, а не еда: никакого восстановления
	// шкал (раньше лечила жажду, как миндальная вода — явный баг).
	Add(TEXT("Battery"),     TEXT("Батарейка"),      EBackroomsItemCategory::Battery,    EBackroomsItemUseType::Consume, 10, 0.0f, 0);
}

UStaticMesh* UBackroomsItemSystem::GetWorldMeshForItem(FName ItemId)
{
	// Визуал расходников в мире. Пути подобраны из Fab/Props-контента проекта;
	// если ассета нет — вернётся nullptr и пикап отрисуется заглушкой.
	static const TMap<FName, FString> Paths =
	{
		{ TEXT("AlmondWater"), TEXT("/Game/Fab/Milks_and_some_juices/milk_and_juice_sketchfab/StaticMeshes/milk_and_juice_sketchfab.milk_and_juice_sketchfab") },
		{ TEXT("CanFood"),     TEXT("/Game/Fab/Tin_cans/tincanssketchfab/StaticMeshes/tincanssketchfab.tincanssketchfab") },
		{ TEXT("MedKit"),      TEXT("/Game/Props/Kit_FirstAid.Kit_FirstAid") },
		{ TEXT("Pill"),        TEXT("/Game/Fab/Prescription_Pill_Bottle/PillBottle_fbx.PillBottle_fbx") },
		{ TEXT("Energy"),      TEXT("/Game/Fab/Soda_Can/soda_can/StaticMeshes/soda_can.soda_can") },
		{ TEXT("Battery"),     TEXT("/Game/Fab/Batteries/batteries/StaticMeshes/batteries.batteries") }
	};

	if (const FString* Path = Paths.Find(ItemId))
	{
		return Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, **Path));
	}
	return nullptr;
}

void UBackroomsItemSystem::TickStats(float DeltaSeconds)
{
	Hunger  = FMath::Clamp(Hunger  - HungerDrainRate  * DeltaSeconds, 0.0f, MaxHunger);
	Thirst  = FMath::Clamp(Thirst  - ThirstDrainRate  * DeltaSeconds, 0.0f, MaxThirst);
	Sanity  = FMath::Clamp(Sanity  - SanityDrainRate  * SanityDrainMultiplier * DeltaSeconds, 0.0f, MaxSanity);

	// Пассивное восстановление рассудка (безопасная зона: рядом нет монстра,
	// не в темноте). Задаётся игроком/статусами по обстановке.
	if (SanityRegenPerSecond > 0.0f)
	{
		Sanity = FMath::Clamp(Sanity + SanityRegenPerSecond * DeltaSeconds, 0.0f, MaxSanity);
	}

	// Голод/жажда на нуле медленно убивают: раньше эти шкалы были безвредны.
	float Damage = 0.0f;
	if (Hunger <= 0.0f)
	{
		Damage += StarvationHealthDrain;
	}
	if (Thirst <= 0.0f)
	{
		Damage += DehydrationHealthDrain;
	}
	if (Damage > 0.0f)
	{
		Health = FMath::Clamp(Health - Damage * DeltaSeconds, 0.0f, MaxHealth);
	}
}

FBackroomsItemDef* UBackroomsItemSystem::FindItemDef(FName ItemId)
{
	for (FBackroomsItemDef& D : ItemDefs)
	{
		if (D.Id == ItemId)
		{
			return &D;
		}
	}
	return nullptr;
}

bool UBackroomsItemSystem::PickUpItem(FName ItemId, int32 Count, int32& Leftover)
{
	Leftover = 0;
	if (Count <= 0)
	{
		return false;
	}
	FBackroomsItemDef* Def = FindItemDef(ItemId);
	if (!Def || !InventoryComponent)
	{
		return false;
	}

	// Слотами владеет UInventoryComponent: стакуется в неполные стаки, остаток —
	// новыми слотами до Capacity. Что не влезло НЕ сжигаем и НЕ спавним здесь:
	// Leftover уходит вызывающему коду, а пикапы (игровые, §5) сами уменьшают
	// свой Count на принятое — «инвентарь полон» остаётся честным в мире.
	const int32 Left = InventoryComponent->AddItemById(ItemId, Count);
	const int32 Added = Count - Left;
	Leftover = Left;

	OnInventoryChanged.Broadcast();
	OnItemPickedUp.Broadcast(*Def, Added);
	return Left <= 0;
}

bool UBackroomsItemSystem::DropItem(FName ItemId, int32 Count, bool bThrow, float Charge,
	ABackroomsItemPickup*& OutPickup)
{
	OutPickup = nullptr;
	if (Count <= 0 || !InventoryComponent)
	{
		return false;
	}

	// Сколько реально сняли со слотов (на складе могло быть меньше Count).
	const int32 Before = GetItemCount(ItemId);
	const bool bRemoved = InventoryComponent->RemoveItemById(ItemId, Count);
	const int32 Removed = FMath::Max(0, Before - GetItemCount(ItemId));
	if (!bRemoved || Removed <= 0)
	{
		return false;
	}

	UWorld* World = GetWorld();
	AActor* Owner = InventoryComponent->GetOwner();
	if (!World || !Owner)
	{
		return true; // слот уже снят; мир недоступен — не считаем ошибкой
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ABackroomsItemPickup* Pickup = World->SpawnActor<ABackroomsItemPickup>(
		ABackroomsItemPickup::StaticClass(),
		Owner->GetActorLocation() + FVector::UpVector * 40.0f,
		Owner->GetActorRotation(), Params);
	if (!Pickup)
	{
		return true;
	}

	Pickup->Initialize(ItemId, Removed, GetWorldMeshForItem(ItemId), 1.0f);
	OutPickup = Pickup;
	if (bThrow)
	{
		// Бросок от игрока: вперёд + подкид. Заряд (0..1) маппится на диапазон
		// ThrowImpulseRange из каталожных данных предмета.
		FBackroomsItemDef Def;
		const float MinImpulse = GetItemDef(ItemId, Def) ? Def.ThrowImpulseRange.X : 120.0f;
		const float MaxImpulse = GetItemDef(ItemId, Def) ? Def.ThrowImpulseRange.Y : 700.0f;
		const float Rel = FMath::Clamp(Charge, 0.0f, 1.0f);
		const FVector Fwd = Owner->GetActorForwardVector();
		Pickup->ThrowAt(Fwd * FMath::Lerp(MinImpulse, MaxImpulse, Rel) + FVector::UpVector * 200.0f);
	}
	return true;
}

bool UBackroomsItemSystem::HasItem(FName ItemId, int32 Count) const
{
	return InventoryComponent ? InventoryComponent->HasItem(ItemId, Count) : false;
}

int32 UBackroomsItemSystem::GetItemCount(FName ItemId) const
{
	return InventoryComponent ? InventoryComponent->GetItemCount(ItemId) : 0;
}

bool UBackroomsItemSystem::GetItemDef(FName ItemId, FBackroomsItemDef& OutDef) const
{
	for (const FBackroomsItemDef& D : ItemDefs)
	{
		if (D.Id == ItemId)
		{
			OutDef = D;
			return true;
		}
	}
	return false;
}

bool UBackroomsItemSystem::UseItem(FName ItemId)
{
	FBackroomsItemDef* Def = FindItemDef(ItemId);
	if (!Def || !HasItem(ItemId, 1))
	{
		return false;
	}

	// Применяем эффект восстановления.
	if (Def->UseType == EBackroomsItemUseType::Consume)
	{
		auto ApplyRestore = [this, Def](int32 RestoreType, float Amount)
		{
			if (Amount <= 0.0f)
			{
				return;
			}
			switch (RestoreType)
			{
			case 0: Hunger = FMath::Clamp(Hunger + Amount, 0.0f, MaxHunger); break;
			case 1: Thirst = FMath::Clamp(Thirst + Amount, 0.0f, MaxThirst); break;
			case 2: Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth); break;
			case 3: Sanity = FMath::Clamp(Sanity + Amount, 0.0f, MaxSanity); break;
			default: break;
			}
		};

		ApplyRestore(Def->RestoreType, Def->RestoreAmount);
		if (Def->RestoreType2 >= 0)
		{
			ApplyRestore(Def->RestoreType2, Def->RestoreAmount2);
		}

		// Батарейка — расходник для фонарика: восстанавливает заряд, а не шкалы.
		if (Def->Category == EBackroomsItemCategory::Battery)
		{
			OnBatteryUsed.Broadcast(50.0f);
		}
	}

	RemoveItem(ItemId, 1);
	OnItemUsed.Broadcast(*Def);
	return true;
}

bool UBackroomsItemSystem::RemoveItem(FName ItemId, int32 Count)
{
	// Слотами управляет UInventoryComponent; тут только делегируем и уведомляем
	// хотбар/HUD, что инвентарь мог измениться.
	const bool bRemoved = InventoryComponent ? InventoryComponent->RemoveItemById(ItemId, Count) : false;
	OnInventoryChanged.Broadcast();
	return bRemoved;
}

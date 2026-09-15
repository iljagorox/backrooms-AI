#include "BackroomsInventoryComponent.h"
#include "BackroomsItemSystem.h"
#include "BackroomsItemPickup.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"

namespace
{
	// Стат-данные предмета из каталога. Каталог маленький и статический — просто
	// пересобираем и ищем; точнее, чем реконструкция из UItemDataAsset (там нет
	// UseTime и текстовой пары имя/описание).
	FBackroomsItemDef GetCatalogDef(FName ItemId)
	{
		TArray<FBackroomsItemDef> Defs;
		UBackroomsItemSystem::BuildDefaultItemDefs(Defs);
		for (const FBackroomsItemDef& D : Defs)
		{
			if (D.Id == ItemId)
			{
				return D;
			}
		}
		return FBackroomsItemDef();
	}
}

bool UInventoryComponent::AddItem(const FInventoryItem& Item)
{
	if (Item.IsEmpty())
	{
		return false;
	}

	FInventoryItem Local = Item;
	if (Local.ItemData.IsNull())
	{
		ResolveItemData(Local);
	}
	UItemDataAsset* Data = Local.ItemData.IsNull() ? nullptr : Local.ItemData.LoadSynchronous();
	const int32 MaxStack = Data ? FMath::Max(1, Data->MaxStack) : 1;

	int32 Count = Local.Quantity;
	TArray<int32> Touched;

	// Добиваем существующие неполные стаки.
	if (MaxStack > 1)
	{
		int32 StackIndex = FindStackWithRoom(Local.ItemID, MaxStack);
		while (StackIndex >= 0 && Count > 0)
		{
			const int32 Room = FMath::Max(0, MaxStack - InventorySlots[StackIndex].Quantity);
			const int32 Taken = FMath::Min(Count, Room);
			InventorySlots[StackIndex].Quantity += Taken;
			Count -= Taken;
			Touched.AddUnique(StackIndex);
			if (Count <= 0)
			{
				break;
			}
			StackIndex = FindStackWithRoom(Local.ItemID, MaxStack);
		}
	}

	// Остаток раскладываем по пустым слотам.
	while (Count > 0)
	{
		int32 Empty = FindFirstEmptySlot();
		if (Empty < 0 || Empty >= Capacity)
		{
			break;
		}
		FInventoryItem& Slot = InventorySlots[Empty];
		Slot = Local;
		Slot.Quantity = FMath::Min(Count, MaxStack);
		Slot.UniqueInstanceID = FGuid::NewGuid();
		Count -= Slot.Quantity;
		Touched.AddUnique(Empty);
	}

	const bool bAllPlaced = (Count <= 0);
	if (bAllPlaced)
	{
		NotifyChanged();
		for (const int32 Idx : Touched)
		{
			NotifySlotChanged(Idx);
		}
	}
	return bAllPlaced;
}

int32 UInventoryComponent::AddItemById(FName ItemID, int32 Quantity)
{
	if (Quantity <= 0 || ItemID.IsNone())
	{
		return 0;
	}

	UItemDataAsset* Data = UBackroomsItemSystem::GetItemDataAsset(ItemID);
	const int32 MaxStack = Data ? FMath::Max(1, Data->MaxStack) : 1;

	int32 Left = Quantity;
	TArray<int32> Touched;

	// Добиваем существующие неполные стаки.
	if (MaxStack > 1)
	{
		int32 StackIndex = FindStackWithRoom(ItemID, MaxStack);
		while (StackIndex >= 0 && Left > 0)
		{
			const int32 Room = FMath::Max(0, MaxStack - InventorySlots[StackIndex].Quantity);
			const int32 Taken = FMath::Min(Left, Room);
			InventorySlots[StackIndex].Quantity += Taken;
			Left -= Taken;
			Touched.AddUnique(StackIndex);
			if (Left <= 0)
			{
				break;
			}
			StackIndex = FindStackWithRoom(ItemID, MaxStack);
		}
	}

	// Остаток — новыми слотами (по MaxStack).
	while (Left > 0)
	{
		int32 Empty = FindFirstEmptySlot();
		if (Empty < 0 || Empty >= Capacity)
		{
			break;
		}
		FInventoryItem& Slot = InventorySlots[Empty];
		Slot.ItemID = ItemID;
		Slot.Quantity = FMath::Min(Left, MaxStack);
		Slot.ItemData = Data;
		Slot.UniqueInstanceID = FGuid::NewGuid();
		Left -= Slot.Quantity;
		Touched.AddUnique(Empty);
	}

	if (Left != Quantity)
	{
		NotifyChanged();
		for (const int32 Idx : Touched)
		{
			NotifySlotChanged(Idx);
		}
	}
	return Left;
}

bool UInventoryComponent::RemoveItem(int32 SlotIndex, int32 Amount)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || Amount <= 0)
	{
		return false;
	}
	FInventoryItem& Slot = InventorySlots[SlotIndex];
	if (Slot.IsEmpty())
	{
		return false;
	}
	Slot.Quantity = FMath::Max(0, Slot.Quantity - Amount);
	if (Slot.IsEmpty())
	{
		Slot = FInventoryItem();
	}
	NotifyChanged();
	NotifySlotChanged(SlotIndex);
	return true;
}

bool UInventoryComponent::RemoveItemById(FName ItemID, int32 Amount)
{
	if (Amount <= 0 || ItemID.IsNone())
	{
		return true;
	}
	TArray<int32> Touched;
	for (int32 i = InventorySlots.Num() - 1; i >= 0 && Amount > 0; --i)
	{
		FInventoryItem& Slot = InventorySlots[i];
		if (Slot.ItemID == ItemID)
		{
			const int32 Taken = FMath::Min(Slot.Quantity, Amount);
			Slot.Quantity -= Taken;
			Amount -= Taken;
			Touched.AddUnique(i);
			if (Slot.IsEmpty())
			{
				Slot = FInventoryItem();
			}
		}
	}
	const bool bRemoved = (Amount <= 0);
	if (bRemoved)
	{
		NotifyChanged();
		for (const int32 Idx : Touched)
		{
			NotifySlotChanged(Idx);
		}
	}
	return bRemoved;
}

FInventoryItem UInventoryComponent::GetItem(int32 SlotIndex) const
{
	if (InventorySlots.IsValidIndex(SlotIndex))
	{
		return InventorySlots[SlotIndex];
	}
	return FInventoryItem();
}

bool UInventoryComponent::IsSlotEmpty(int32 SlotIndex) const
{
	if (!InventorySlots.IsValidIndex(SlotIndex))
	{
		// Слот за пределами массива пуст, лишь бы сам индекс был в лимите.
		return SlotIndex < Capacity;
	}
	return InventorySlots[SlotIndex].IsEmpty();
}

int32 UInventoryComponent::FindFirstEmptySlot() const
{
	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		if (InventorySlots[i].IsEmpty())
		{
			return i;
		}
	}
	// Пустых нет — слот можно добавить, если лимит позволяет.
	return InventorySlots.Num();
}

int32 UInventoryComponent::FindItemByID(FName ItemID) const
{
	if (ItemID.IsNone())
	{
		return -1;
	}
	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		if (InventorySlots[i].ItemID == ItemID && !InventorySlots[i].IsEmpty())
		{
			return i;
		}
	}
	return -1;
}

bool UInventoryComponent::HasItem(FName ItemID, int32 Count) const
{
	return GetItemCount(ItemID) >= Count;
}

int32 UInventoryComponent::GetItemCount(FName ItemID) const
{
	int32 Count = 0;
	if (ItemID.IsNone())
	{
		return 0;
	}
	for (const FInventoryItem& Slot : InventorySlots)
	{
		if (Slot.ItemID == ItemID)
		{
			Count += Slot.Quantity;
		}
	}
	return Count;
}

void UInventoryComponent::Clear()
{
	const int32 OldNum = InventorySlots.Num();
	InventorySlots.Reset();
	NotifyChanged();
	for (int32 i = 0; i < OldNum; ++i)
	{
		NotifySlotChanged(i);
	}
}

void UInventoryComponent::SelectActiveSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= InventorySlots.Num() || InventorySlots[SlotIndex].IsEmpty())
	{
		return;
	}
	if (ActiveSlotIndex == SlotIndex)
	{
		return;
	}
	ActiveSlotIndex = SlotIndex;
	OnActiveSlotChanged.Broadcast(ActiveSlotIndex);
}

bool UInventoryComponent::UseSlot(int32 SlotIndex)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].IsEmpty())
	{
		return false;
	}
	OnSlotUseRequested.Broadcast(InventorySlots[SlotIndex].ItemID);
	return true;
}

bool UInventoryComponent::DropSlot(int32 SlotIndex, int32 Count, const FTransform& SpawnTm)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].IsEmpty() || Count <= 0)
	{
		return false;
	}

	const FInventoryItem& Slot = InventorySlots[SlotIndex];
	const int32 DropCount = FMath::Min(Count, Slot.Quantity);
	const FName ItemId = Slot.ItemID;

	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ABackroomsItemPickup* Pickup = World->SpawnActor<ABackroomsItemPickup>(
			ABackroomsItemPickup::StaticClass(), SpawnTm, Params);
		if (Pickup)
		{
			Pickup->Initialize(ItemId, DropCount, UBackroomsItemSystem::GetWorldMeshForItem(ItemId), 1.0f);
		}
	}
	RemoveItem(SlotIndex, DropCount);
	return true;
}

bool UInventoryComponent::MoveSlot(int32 From, int32 To)
{
	if (From == To || !InventorySlots.IsValidIndex(From) || To < 0 || To >= Capacity)
	{
		return false;
	}
	// Массив расширяем до To: учёт «перспективного» пустого слота.
	while (InventorySlots.Num() <= To)
	{
		InventorySlots.Add(FInventoryItem());
	}

	FInventoryItem& A = InventorySlots[From];
	FInventoryItem& B = InventorySlots[To];

	// Один и тот же предмет в оба слота — сливаем в один стак (перетаскивание
	// стака на стак), иначе честный свап.
	if (!A.IsEmpty() && !B.IsEmpty() && A.ItemID == B.ItemID)
	{
		UItemDataAsset* Data = B.ItemData.IsNull() ? nullptr : B.ItemData.LoadSynchronous();
		const int32 MaxStack = Data ? FMath::Max(1, Data->MaxStack) : 1;
		const int32 OnTop = FMath::Min(A.Quantity, FMath::Max(0, MaxStack - B.Quantity));
		if (OnTop > 0)
		{
			B.Quantity += OnTop;
			A.Quantity -= OnTop;
			if (A.IsEmpty())
			{
				A = FInventoryItem();
			}
			NotifyChanged();
			NotifySlotChanged(From);
			NotifySlotChanged(To);
			return true;
		}
	}

	Swap(A, B);
	NotifyChanged();
	NotifySlotChanged(From);
	NotifySlotChanged(To);
	return true;
}

int32 UInventoryComponent::SplitStack(int32 SlotIndex, int32 Count)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].IsEmpty() || Count <= 0)
	{
		return Count;
	}
	FInventoryItem& S = InventorySlots[SlotIndex];
	if (FMath::Min(Count, S.Quantity) >= S.Quantity)
	{
		// Отделять нечего: это уже весь стак, а не его часть.
		return Count;
	}

	int32 Empty = FindFirstEmptySlot();
	if (Empty < 0 || Empty >= Capacity)
	{
		return Count;
	}

	FInventoryItem NewStack = S;
	NewStack.Quantity = FMath::Min(Count, S.Quantity);
	NewStack.UniqueInstanceID = FGuid::NewGuid();
	InventorySlots[Empty] = NewStack;
	S.Quantity -= NewStack.Quantity;

	NotifyChanged();
	NotifySlotChanged(SlotIndex);
	NotifySlotChanged(Empty);
	return 0;
}

void UInventoryComponent::SpawnLeftoverPickup(FName ItemId, int32 Count)
{
	if (Count <= 0 || ItemId.IsNone())
	{
		return;
	}
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World)
	{
		return;
	}

	FVector Loc = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 70.0f;
	Loc.Z += 45.0f;
	FTransform Tm(OwnerActor->GetActorRotation(), Loc);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (ABackroomsItemPickup* Pickup = World->SpawnActor<ABackroomsItemPickup>(
		ABackroomsItemPickup::StaticClass(), Tm, Params))
	{
		Pickup->Initialize(ItemId, Count, UBackroomsItemSystem::GetWorldMeshForItem(ItemId), 1.0f);
	}
}

UItemDataAsset* UInventoryComponent::ResolveItemData(FInventoryItem& Item)
{
	if (Item.ItemID.IsNone())
	{
		return nullptr;
	}
	if (!Item.ItemData.IsNull())
	{
		return Item.ItemData.LoadSynchronous();
	}
	UItemDataAsset* Data = UBackroomsItemSystem::GetItemDataAsset(Item.ItemID);
	if (Data)
	{
		Item.ItemData = TSoftObjectPtr<UItemDataAsset>(Data);
	}
	return Data;
}

int32 UInventoryComponent::FindStackWithRoom(FName ItemID, int32 MaxStack) const
{
	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		const FInventoryItem& Slot = InventorySlots[i];
		if (Slot.ItemID == ItemID && !Slot.IsEmpty() && Slot.Quantity < MaxStack)
		{
			return i;
		}
	}
	return -1;
}

void UInventoryComponent::NotifyChanged()
{
	OnInventorySlotsChanged.Broadcast();
}

void UInventoryComponent::NotifySlotChanged(int32 SlotIndex)
{
	FBackroomsItemDef Def;
	if (InventorySlots.IsValidIndex(SlotIndex) && !InventorySlots[SlotIndex].IsEmpty())
	{
		Def = GetCatalogDef(InventorySlots[SlotIndex].ItemID);
	}
	OnSlotChanged.Broadcast(SlotIndex, Def);
}
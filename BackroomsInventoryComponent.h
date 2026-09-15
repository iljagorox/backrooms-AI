#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BackroomsInventoryData.h"
#include "BackroomsInventoryComponent.generated.h"

class UItemDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventorySlotsChanged);

// Точечное изменение одного слота (слот, стат-данные; пустой слот — Id=None).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotChanged, int32, SlotIndex, const FBackroomsItemDef&, Def);

// Активный (выбранный) слот изменился.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveInventorySlotChanged, int32, SlotIndex);

// Запрошено использование слота по ItemId (мост в правила UBackroomsItemSystem).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventorySlotUseRequested, FName, ItemId);

// Компонент инвентаря: TArray<FInventoryItem> InventorySlots — единственный
// источник правды по слотам. Статические данные берутся из UItemDataAsset
// (мягкая ссылка в ItemData), логика — стаки/перенос/поиск через массив.
UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class BACKROOMS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Лимит слотов (массив расширяется до этого значения).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Capacity = 24;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryItem> InventorySlots;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventorySlotsChanged OnInventorySlotsChanged;

	// Точечные события: хотбар/виджет обновляют ровно один слот, а не весь массив.
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventorySlotChanged OnSlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnActiveInventorySlotChanged OnActiveSlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventorySlotUseRequested OnSlotUseRequested;

	// Добавить готовый слот (стаки добиваются, затем пустой слот). true — влез
	// целиком.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(const FInventoryItem& Item);

	// Добавить Quantity штук по ID (DataAsset для предмета подставляется из
	// каталога). Возвращает, сколько НЕ влезло (0 — всё добавлено).
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItemById(FName ItemID, int32 Quantity = 1);

	// Удалить Amount из слота (слот очищается, если стал пустым).
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(int32 SlotIndex, int32 Amount = 1);

	// Удалить Amount из всех слотов с ItemID (итерация назад).
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemById(FName ItemID, int32 Amount = 1);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FInventoryItem GetItem(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsSlotEmpty(int32 SlotIndex) const;

	// Первый пустой слот. Если пустых нет, но массив короче Capacity —
	// возвращает перспективный индекс (Num), куда можно положить.
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 FindFirstEmptySlot() const;

	// Первый слот с ItemID (-1 — нет).
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 FindItemByID(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemID, int32 Count = 1) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetNumSlots() const { return InventorySlots.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void Clear();

	// Активный слот (выбор в хотбаре/UI). -1 — ничего не выбрано.
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 ActiveSlotIndex = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectActiveSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	// Использовать предмет из слота: мост в правила (ItemSystem). Сам слот не
	// трогает — правила сами заберут предмет через RemoveItemById.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseSlot(int32 SlotIndex);

	// Выбросить Count штук из слота в мир (спавн ABackroomsItemPickup).
	// true — предмет ушёл из слота (полное/частичное выбрасывание).
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool DropSlot(int32 SlotIndex, int32 Count, const FTransform& SpawnTm);

	// Переместить слот (swap). Индексы биваются в лимит Capacity.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveSlot(int32 From, int32 To);

	// Разделить стак: Count из SlotIndex уходит в новый пустой слот.
	// Возвращает сколько НЕ разделилось (0 — успех).
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 SplitStack(int32 SlotIndex, int32 Count);

	// Спавн не взятого остатка у ног владельца (полный инвентарь). Слотов не
	// касается: предмет в инвентарь не попал, а просто появился рядом.
	void SpawnLeftoverPickup(FName ItemId, int32 Count);

	// Заполнить ItemData слота по ItemID, если мягкая ссылка пуста.
	// Возвращает статданные предмета или nullptr.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemDataAsset* ResolveItemData(FInventoryItem& Item);

private:
	int32 FindStackWithRoom(FName ItemID, int32 MaxStack) const;
	void NotifyChanged();
	// Точечное событие одного слота (пустой слот уходит как Id=None).
	void NotifySlotChanged(int32 SlotIndex);
};
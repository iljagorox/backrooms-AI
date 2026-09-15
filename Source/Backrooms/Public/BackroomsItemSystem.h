#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BackroomsItemSystem.generated.h"

class UStaticMesh;
class UInventoryComponent;
class UItemDataAsset;

// Категория предмета.
UENUM(BlueprintType)
enum class EBackroomsItemCategory : uint8
{
	Food          UMETA(DisplayName = "Еда"),
	Water         UMETA(DisplayName = "Вода"),
	Medicine      UMETA(DisplayName = "Медикаменты"),
	Battery       UMETA(DisplayName = "Батарейки"),
	Tool          UMETA(DisplayName = "Инструмент"),
	Weapon        UMETA(DisplayName = "Оружие"),
	Tent          UMETA(DisplayName = "Тент"),
	Clothing      UMETA(DisplayName = "Одежда"),
	Junk          UMETA(DisplayName = "Мусор"),
	AlmondWater   UMETA(DisplayName = "Миндальная вода"),
	Energy        UMETA(DisplayName = "Энергетик/чай"),
	KeyItem       UMETA(DisplayName = "Ключевой предмет")
};

// Тип использования.
UENUM(BlueprintType)
enum class EBackroomsItemUseType : uint8
{
	Consume       UMETA(DisplayName = "Потребить (удалить)"),
	Equip         UMETA(DisplayName = "Экипировать"),
	Place         UMETA(DisplayName = "Разместить в мире"),
	Read          UMETA(DisplayName = "Прочитать"),
	Toggle        UMETA(DisplayName = "Вкл/Выкл")
};

// Описание предмета.
USTRUCT(BlueprintType)
struct FBackroomsItemDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBackroomsItemCategory Category = EBackroomsItemCategory::Junk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBackroomsItemUseType UseType = EBackroomsItemUseType::Consume;

	// Макс. стак в инвентаре.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStackSize = 1;

	// Восстанавливаемое значение (голод/жажа/здоровье).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestoreAmount = 0.0f;

	// Тип восстановления: 0=голод, 1=жажа, 2=здоровье, 3=рассудок.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RestoreType = 0;

	// Второй эффект предмета (например, миндальная вода: жажда + рассудок).
	// RestoreType2 < 0 — второго эффекта нет.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestoreAmount2 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RestoreType2 = -1;

	// Время использования (сек).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UseTime = 1.0f;

	// Сокет руки для держания (скилет), по умолчанию правая рука.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HandSocketName = TEXT("hand_r_hold");

	// Можно ли держать предмет в руках (еда — да, аптечка — можно не).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanHold = false;

	// Осматриваемость (записки/ключи — да, банка — нет).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInspectable = true;

	// Анимации экипировки в руку и использования (опционально).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UAnimMontage> EquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UAnimMontage> UseMontage;

	// Диапазон импульса броска по заряду удержания (Min..Max, у.е. импульса).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D ThrowImpulseRange = FVector2D(120.0f, 700.0f);

	// Радиус шума при падении (для сущностей). 800 см = 8 м.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NoiseRadius = 800.0f;

	// Сам предмет — фонарик (включает свет и тратит батарейку).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlashlight = false;

	// Меш для отображения в мире.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	// Иконка для инвентаря.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPickedUp, const FBackroomsItemDef&, Item, int32, Count);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUsed, const FBackroomsItemDef&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
// Запрошено восстановление заряда фонарика (батарейка).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBatteryUsed, float, Amount);

UCLASS(BlueprintType, Blueprintable)
class BACKROOMS_API UBackroomsItemSystem : public UObject
{
	GENERATED_BODY()

public:
	// Выбросить предмет из инвентаря (удаляет слот и спавнит пикап в мире).
	// bThrow=false — дроп у ног; bThrow=true — бросок с зарядом: импульс считается
	// из ThrowImpulseRange по Charge (0..1). OutPickup — созданный пикап (может
	// быть null, если мир/ассет недоступен): например, для подписки на его шум.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool DropItem(FName ItemId, int32 Count, bool bThrow, float Charge,
		class ABackroomsItemPickup*& OutPickup);
	// Заполнить каталог предметов по умолчанию. Статический вариант — чтобы
	// и ItemSystem игрока, и виджет-предпросмотр могли получить список без
	// создания отдельного объекта-владельца.
	UFUNCTION(BlueprintCallable, Category = "Items")
	static void BuildDefaultItemDefs(TArray<FBackroomsItemDef>& OutDefs);

	// Мировой меш для предмета (расходники для пикапа). Единая точка правды,
	// чтобы визуал в мире совпадал с каталогом. Может вернуть nullptr.
	static UStaticMesh* GetWorldMeshForItem(FName ItemId);

	// Статданные предмета (UItemDataAsset) по ID. Для встроенных предметов
	// создаётся runtime-ассет из каталога; задача — есть ли ассет в проекте.
	static UItemDataAsset* GetItemDataAsset(FName ItemId);

	// Каталог всех предметов.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	TArray<FBackroomsItemDef> ItemDefs;

	// Массив слотов живёт в UInventoryComponent игрока (источник правды).
	// ItemSystem остаётся каталогом и статами; все операции с предметами
	// делегируются в компонент.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	void SetInventoryComponent(UInventoryComponent* InComponent);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	// Голод (0-100).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Hunger = 100.0f;

	// Жажда (0-100).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Thirst = 100.0f;

	// Здоровье (0-100).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 100.0f;

	// Рассудок (0-100).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Sanity = 100.0f;

	// Максимальные значения.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxThirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxSanity = 100.0f;

	// Скорость убывания (в секунду).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float HungerDrainRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ThirstDrainRate = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float SanityDrainRate = 0.2f;

	// Множитель убывания рассудка (задаётся системой состояний: защита снижает,
	// уязвимость/облучение повышают). 1.0 — нейтрально.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float SanityDrainMultiplier = 1.0f;

	// Пассивное восстановление рассудка в секунду «в безопасной зоне» (нет
	// монстра рядом, не темно). Задаётся игроком/статусами; 0 — выключено.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float SanityRegenPerSecond = 0.0f;

	// Голод/жажда на нуле отнимают здоровье. Множители урона в секунду.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float StarvationHealthDrain = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DehydrationHealthDrain = 1.5f;

	// События.
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnItemPickedUp OnItemPickedUp;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnItemUsed OnItemUsed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInventoryChanged OnInventoryChanged;

	// Запрошено пополнение заряда фонарика (использована батарейка).
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBatteryUsed OnBatteryUsed;

	// Инициализация каталога предметов.
	UFUNCTION(BlueprintCallable, Category = "Items")
	void InitializeDefaultItems();

	// Тик статов (убывание голод/жажда/рассудок).
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void TickStats(float DeltaSeconds);

	// Подобрать предмет в инвентарь. Остаток (не влезло) возвращается в
	// Leftover и сам спавнится у ног игрока как ABackroomsItemPickup.
	UFUNCTION(BlueprintCallable, Category = "Items")
	bool PickUpItem(FName ItemId, int32 Count, int32& Leftover);

	// Использовать предмет из инвентаря.
	UFUNCTION(BlueprintCallable, Category = "Items")
	bool UseItem(FName ItemId);

	// Удалить предмет из инвентаря.
	UFUNCTION(BlueprintCallable, Category = "Items")
	bool RemoveItem(FName ItemId, int32 Count = 1);

	// Есть ли предмет в инвентаре.
	UFUNCTION(BlueprintPure, Category = "Items")
	bool HasItem(FName ItemId, int32 Count = 1) const;

	// Количество предмета в инвентаре.
	UFUNCTION(BlueprintPure, Category = "Items")
	int32 GetItemCount(FName ItemId) const;

	// Получить определение предмета по ID.
	UFUNCTION(BlueprintPure, Category = "Items")
	bool GetItemDef(FName ItemId, FBackroomsItemDef& OutDef) const;

private:
	FBackroomsItemDef* FindItemDef(FName ItemId);
};

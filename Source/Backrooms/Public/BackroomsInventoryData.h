#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BackroomsItemSystem.h"
#include "BackroomsInventoryData.generated.h"

class UStaticMesh;
class UTexture2D;

// Слот инвентаря. ItemData — мягкая ссылка на Data Asset: в редакторе это путь к
// контентному ассету, в рантайме для встроенных предметов каталога подставляется
// ItemSystem-ом автоматически.
USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<class UItemDataAsset> ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGuid UniqueInstanceID;

	bool IsEmpty() const { return ItemID.IsNone() || Quantity <= 0; }
};

// Статические данные предмета. Здесь живут оба меша:
// WorldMesh — полноразмерный (в мире), HeldMesh — уменьшенная версия для руки.
// Если HeldMesh пуст, для руки используется WorldMesh с HeldScale.
UCLASS(BlueprintType)
class BACKROOMS_API UItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EBackroomsItemCategory Category = EBackroomsItemCategory::Junk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EBackroomsItemUseType ItemType = EBackroomsItemUseType::Consume;

	// Меш для отображения в мире (на полу, на полке).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	// Меш для руки (уменьшенная/оптимизированная версия). Если пуст — WorldMesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<UStaticMesh> HeldMesh;

	// Масштаб HeldMesh при экипировке (по умолчанию 0.45 — предмет ~30 см).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FVector HeldScale = FVector(0.45f, 0.45f, 0.45f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxStack = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float Weight = 1.0f;

	// Эффект при потреблении.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float RestoreAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 RestoreType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float RestoreAmount2 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 RestoreType2 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<UTexture2D> Icon;

	// --- Поля держания/броска/осмотра (§6) ---
	// Сокет руки для держания (скилет), по умолчанию правая рука.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hold")
	FName HandSocketName = TEXT("hand_r_hold");

	// Можно ли держать предмет в руках (еда — да, аптечка — можно не).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hold")
	bool bCanHold = false;

	// Осматриваемость (записки/ключи — да, банка — нет).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hold")
	bool bInspectable = true;

	// Анимации экипировки в руку и использования (опционально).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hold")
	TSoftObjectPtr<class UAnimMontage> EquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hold")
	TSoftObjectPtr<class UAnimMontage> UseMontage;

	// Диапазон импульса броска по заряду удержания (Min..Max).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
	FVector2D ThrowImpulseRange = FVector2D(120.0f, 700.0f);

	// Радиус шума при падении (для сущностей). 800 см = 8 м.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
	float NoiseRadius = 800.0f;

	// Сам предмет — фонарик (включает свет и тратит батарейку).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hold")
	bool bFlashlight = false;

	UStaticMesh* GetHeldMesh() const;
	UStaticMesh* GetWorldMesh() const;
};

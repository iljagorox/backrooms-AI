#pragma once

#include "CoreMinimal.h"
#include "BackroomsHandles.h"
#include "BackroomsMacroFields.h"
#include "BackroomsLocationArchetype.h"
#include "BackroomsMaterialTypes.generated.h"

class UMaterialInterface;

// -------------------------------------------------------
// EBackroomsMaterialType — поверхностный тип, задаваемый FloorPlan/Archetype.
// Material system интерпретирует semantics, а не хардкодит материалы.
// -------------------------------------------------------
UENUM(BlueprintType)
enum class EBackroomsMaterialType : uint8
{
	// Базовые типы, соответствующие ролям spaces
	Concrete UMETA(DisplayName = "Concrete"),
	Plaster UMETA(DisplayName = "Plaster"),
	Wood UMETA(DisplayName = "Wood"),
	Metal UMETA(DisplayName = "Metal"),
	Glass UMETA(DisplayName = "Glass"),
	Carpet UMETA(DisplayName = "Carpet"),
	Tile UMETA(DisplayName = "Tile"),
	Wallpaper UMETA(DisplayName = "Wallpaper"),

	// Деривативные типы для конкретных локаций/архетипов
	OfficeFloor UMETA(DisplayName = "OfficeFloor"),
	OfficeWall UMETA(DisplayName = "OfficeWall"),
	HotelFloor UMETA(DisplayName = "HotelFloor"),
	HotelWall UMETA(DisplayName = "HotelWall"),
	HospitalFloor UMETA(DisplayName = "HospitalFloor"),
	HospitalWall UMETA(DisplayName = "HospitalWall"),

	Count
};

// -------------------------------------------------------
// FBackroomsMaterialPalette — правила материала для каждой поверхности.
// Хранятся в LevelPalette -> ZoneVariation -> SpaceVariation -> LocalVariation.
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct FBackroomsMaterialPalette
{
	GENERATED_BODY()

	// Базовый материал (MasterMaterial).
	UPROPERTY(EditAnywhere, Category = "Material")
	TObjectPtr<UMaterialInterface> BaseMaterial;

	// Параметры материала, подаваемые в MasterMaterial через параметры инстанса:
	UPROPERTY(EditAnywhere, Category = "Material Parameters")
	FLinearColor BaseColor = FLinearColor(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Material Parameters")
	float Roughness = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Material Parameters")
	float Metallic = 0.0f;

	// Слой загрязнения/возраста (0.0 = чистый, 1.0 = сильно потерт/загрязнен)
	UPROPERTY(EditAnywhere, Category = "Aging")
	float DirtAmount = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Aging")
	float Age = 0.0f;  // сколько лет/времени прошло с момента создания пространства

	// Уровень влаги/влажности (0.0 = сухо, 1.0 = мокро/влага)
	UPROPERTY(EditAnywhere, Category = "Environment")
	float Moisture = 0.0f;

	// Уровень повреждений (0.0 = целое, 1.0 = 심하게 повреждено)
	UPROPERTY(EditAnywhere, Category = "Damage")
	float Damage = 0.0f;

	// Дисколориация/выцветание (0.0 = нет, 1.0 = сильно выцвело)
	UPROPERTY(EditAnywhere, Category = "Appearance")
	float Discoloration = 0.0f;

	// Шероховатость поверхности на основе текстурного канала
	UPROPERTY(EditAnywhere, Category = "Texture")
	float RoughnessVariation = 0.0f;

	// UV сдвиг для паттернированных материалов
	UPROPERTY(EditAnywhere, Category = "Texture")
	FVector2D UVOffset = FVector2D(0.0f, 0.0f);

	// Вращение паттерна
	UPROPERTY(EditAnywhere, Category = "Texture")
	float PatternRotation = 0.0f;

	// Конструктор по умолчанию
	FBackroomsMaterialPalette() = default;

	// Проверка валидности
	bool IsValid() const { return BaseMaterial != nullptr; }
};

// -------------------------------------------------------
// FBackroomsMaterialState — runtime state материала для конкретного пространства.
// Вычисляется из MacroFields + EnvironmentState + TimeSinceGeneration.
// -------------------------------------------------------
struct FBackroomsMaterialState
{
	// Текущие параметры (могут меняться во времени: dirt накапливается, age растет)
	FLinearColor BaseColor;
	float Roughness;
	float Metallic;
	float DirtAmount;
	float Age;
	float Moisture;
	float Damage;
	float Discoloration;
	float RoughnessVariation;
	FVector2D UVOffset;
	float PatternRotation;

	// Derived state
	bool bIsWet;      // Moisture > 0.5
	bool bIsDamaged;  // Damage > 0.3
	bool bIsOld;      // Age > 5.0 (лет симуляции)

	FBackroomsMaterialState()
		: BaseColor(FLinearColor(1.0f, 1.0f, 1.0f))
		, Roughness(0.5f)
		, Metallic(0.0f)
		, DirtAmount(0.0f)
		, Age(0.0f)
		, Moisture(0.0f)
		, Damage(0.0f)
		, Discoloration(0.0f)
		, RoughnessVariation(0.0f)
		, UVOffset(FVector2D(0.0f, 0.0f))
		, PatternRotation(0.0f)
		, bIsWet(false)
		, bIsDamaged(false)
		, bIsOld(false)
	{
	}

	// Обновление состояния на основе времени и окружения
	void Update(float DeltaSeconds, const FBackroomsMaterialPalette& Palette, const FName& EnvironmentState);
};

// -------------------------------------------------------
// FBackroomsMaterialParams — упаковка параметров для шейдера MasterMaterial.
// -------------------------------------------------------
struct FBackroomsMaterialParams
{
	FLinearColor BaseColor;
	float Roughness;
	float Metallic;
	float DirtAmount;
	float Age;
	float Moisture;
	float Damage;
	float Discoloration;
	float RoughnessVariation;
	FVector2D UVOffset;
	float PatternRotation;

	// Конструкция изPalette + State
	static FBackroomsMaterialParams FromPaletteAndState(
		const FBackroomsMaterialPalette& Palette,
		const FBackroomsMaterialState& State);
};

// -------------------------------------------------------
// FBackroomsMaterialLookup — быстрый lookupPalette+State по SpaceHandle.
// -------------------------------------------------------
struct FBackroomsMaterialLookup
{
	// Находится состояние материала для данного пространства.
	// Если нет — возвращается дефолт.
	static const FBackroomsMaterialState& GetMaterialState(
		const FBackroomsSpaceHandle& SpaceHandle,
		const UBackroomsLocationArchetype* Archetype,
		const FBackroomsMacroFields& MacroFields);

	// Обновлить состояние (вызывать каждый тик или по таймеру)
	static void UpdateMaterialState(
		const FBackroomsSpaceHandle& SpaceHandle,
		const UBackroomsLocationArchetype* Archetype,
		const FBackroomsMacroFields& MacroFields,
		float DeltaSeconds);
};
#pragma once

#include "CoreMinimal.h"
#include "BackroomsLevelConfig.generated.h"

USTRUCT(BlueprintType)
struct FBackroomsGridConfig
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TileSizeM = 0.5f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WallThicknessMinM = 0.3f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FloorHeightM = 3.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Chapter = 0;
};

USTRUCT(BlueprintType)
struct FBackroomsSpawnConfig
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 XTile = 8;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 YTile = 8;
};

USTRUCT(BlueprintType)
struct FBackroomsGenerationConfig
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Algorithm = TEXT("bsp_plus_loops");

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Params;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChunkCells = 8;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeedMin = 0;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeedMax = MAX_int32;
};

USTRUCT(BlueprintType)
struct FBackroomsRoomsConfig
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSizeM = 8.0f;
};

USTRUCT(BlueprintType)
struct FBackroomsCorridorsConfig
{
	GENERATED_BODY()

	// -1 означает «не применимо» (поле есть в YAML как null).
UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WidthMinM = -1.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WidthMaxM = -1.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadEndFrac = 0.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoopGlobal = 0.0f;

	bool HasWidthRange() const { return WidthMinM >= 0.0f; }
};

USTRUCT(BlueprintType)
struct FBackroomsDoorsConfig
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WidthMinM = 1.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WidthMaxM = 1.5f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeightM = 2.1f;
};

USTRUCT(BlueprintType)
struct FBackroomsMaterialsConfig
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Floor;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Wall;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Ceiling;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WaterFloor;
};

USTRUCT(BlueprintType)
struct FBackroomsLightsConfig
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EveryM = 5.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(1.0f, 0.9f, 0.7f);
};

// Трёхслойный декоратор процедурного мира (см. «Слои»):
//  Слой 2 «Инженерка» — розетки/выключатели/вентиляция/плинтус + грязь у стыка
//    стена-пол. Ставится «пучками»: редкие якорные клетки, в пучке — густо.
//  Слой 3 «История» — островки интереса у стен (кучка мусора, бутылка).
//  Пятна — редкие декали «мокроты»/подтёков на полу.
// Все списки ассетов пусты по умолчанию: слой отключается, пока контент не
// назначен в YAML уровня (decor: …). Чанки-процедурщики генерируют местоположение.
USTRUCT(BlueprintType)
struct FBackroomsDecorConfig
{
	GENERATED_BODY()

	// Вероятность «жилой» клетки стать якорем пучка инженерки (редко).
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer2", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ClusterChance = 0.10f;

	// Шанс каждой стенной грани в пучке получить элемент (розетку/решётку).
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer2", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WallGearChance = 0.50f;

	// Меши инженерки: розетки, выключатели, вентиляционные решётки, кабель-каналы.
	// Ставятся на стены, ориентированы фронтом в комнату (см. WallGearYawOffset).
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer2")
	TArray<TSoftObjectPtr<UStaticMesh>> WallGearMeshes;

	// Декали грязи у стыка стена-пол (подтёки, тёмные полосы вдоль плинтуса).
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer2")
	TArray<TSoftObjectPtr<UMaterialInterface>> PlinthDecalMaterials;

	// Диапазон высоты установки элементов инженерки на стене (см).
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer2")
	FVector2D WallGearHeightCm = FVector2D(40.0f, 200.0f);

	// Смещение ориентации мешей инженерки (градусы) — если меш в ассете
	// авторизован «лицом» не вдоль -X, доверни его здесь.
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer2")
	float WallGearYawOffset = 0.0f;

	// Вероятность: у пучка инженерки дополнительно появляется «островок
	// интереса» — кучка мусора/бутылка у стены (Слой 3).
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer3", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HistoryChance = 0.35f;

	// Пятна на полу (мокрота/подтёк): шанс на «жилую» клетку.
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stains", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FloorStainChance = 0.05f;

	// Декали пятен: масляные лужицы, влага на ковре, следы.
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stains")
	TArray<TSoftObjectPtr<UMaterialInterface>> FloorStainMaterials;

	// Размер пятна (ширина/высота), см.
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stains")
	FVector2D StainSizeCm = FVector2D(50.0f, 50.0f);
};

USTRUCT(BlueprintType)
struct FBackroomsSkyConfig
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Material;
};

USTRUCT(BlueprintType)
struct FBackroomsWaterConfig
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DepthM = 0.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(0.2f, 0.4f, 0.9f);
};

USTRUCT(BlueprintType)
struct FBackroomsAcousticsConfig
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Reverb = TEXT("office");

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AmbientLayer = TEXT("lamp_hum");

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEcho = false;
};

USTRUCT(BlueprintType)
struct FBackroomsValidationConfig
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Seeds = 1000;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ErrorTolerancePct = 1.0f;
};

// Runtime-конфиг уровня, собранный из YAML на диске (Content/Config/Levels).
// YAML — источник правды; этот объект строится загрузчиком (BackroomsYamlLoader)
// и не редактируется вручную в редакторе.
UCLASS(BlueprintType)
class BACKROOMS_API UBackroomsLevelConfig : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	int32 Version = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FBackroomsGridConfig Grid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FBackroomsSpawnConfig Spawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FBackroomsGenerationConfig Generation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FBackroomsRoomsConfig Rooms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FBackroomsCorridorsConfig Corridors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Décor")
	FBackroomsDoorsConfig Doors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	FBackroomsMaterialsConfig Materials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
	FBackroomsLightsConfig Lights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Décor")
	FBackroomsDecorConfig Decor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
	FBackroomsSkyConfig Sky;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	FBackroomsWaterConfig Water;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
	FBackroomsAcousticsConfig Acoustics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
	bool bFloodFill3D = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	FBackroomsValidationConfig Validation;

	// Полный путь к YAML-файлу, из которого собран конфиг (для отладки).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString SourcePath;
};
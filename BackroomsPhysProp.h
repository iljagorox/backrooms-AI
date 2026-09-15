#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BackroomsPhysProp.generated.h"

class UStaticMeshComponent;

// Физический предмет «следа жизни»: можно взять в руку, толкнуть, уронить,
// он катится. Вес — числом (кг): лёгкое катится легко, тяжёлое почти не
// сдвинуть. Реалистичный масштаб (или чуть меньше).
UCLASS()
class BACKROOMS_API ABackroomsPhysProp : public AActor
{
	GENERATED_BODY()

public:
	ABackroomsPhysProp();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Prop")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// Вес в кг (записывается в PhysicalMaterial/массу).
	UPROPERTY(EditAnywhere, Category = "Prop")
	float WeightKg = 1.0f;

	// Порог импульса удара (N·s), после которого проп считается «сломанным»
	// (событие OnPropBroken). Полноценного Chaos-damage здесь нет — только хук.
	UPROPERTY(EditAnywhere, Category = "Prop")
	float BrokenImpactThreshold = 900.0f;

	// Инициализация данными из генератора комнат.
	void Initialize(UStaticMesh* Mesh, float Scale, float InWeightKg, const FVector& Pos, const FRotator& Rot);

	// Текущий граб (заполняет игрок): если >0 — предмет «держат».
	float CarryDistance = 0.0f;
	bool bCarried = false;

	// ---- Сюжетные теги (Task 8: события) ----
	// Идентификатор комнаты и сценария — заполняется из SpawnStoryProp.
	// -1 = не тегирован (по умолчанию).
	int32 GetRoomID() const { return RoomID; }
	int32 GetScenarioID() const { return ScenarioID; }
	void SetStoryTag(int32 InRoomID, int32 InScenarioID) { RoomID = InRoomID; ScenarioID = InScenarioID; }

	// Обратный линк на чанк-владельца (для событий OnPropBroken/сюжетного триггера).
	void SetOwnerChunk(class ABackroomsChunkActor* InChunk);
	class ABackroomsChunkActor* GetOwnerChunk() const;

	// Игрок взял проп в руку — сообщает владельцу-чанку (сбор сцены).
	void NotifyPickedUp();

private:
	int32 RoomID = -1;
	int32 ScenarioID = -1;

	UPROPERTY(Transient)
	TWeakObjectPtr<class ABackroomsChunkActor> OwnerChunk;

	// Обработчик сильных ударов: поро-же «сломан» -> OnPropBroken у чанка.
	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	// Обновляет трение по весу: чем тяжелее, тем выше фрикция (стоит на месте);
	// чем легче — тем ниже (катится). Вызывается из Initialize/BeginPlay.
	void ApplyFrictionByWeight();

	// Динамический физмат: фрикция/реституция пересчитываются по весу.
	UPROPERTY(Transient)
	TObjectPtr<UPhysicalMaterial> FrictionMaterial;
};

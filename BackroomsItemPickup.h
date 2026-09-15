#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BackroomsItemPickup.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UPointLightComponent;
class UStaticMesh;

// Шум от брошенного предмета: сущности слышат падение (Location, Radius, см).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBackroomsItemNoise, FVector, Location, float, Radius);

// Состояния пикапа (§4): один актор закрывает и лежание, и полёт, и покой.
UENUM(BlueprintType)
enum class EPickupState : uint8
{
	Idle,    // парение + вращение, интерактивный
	Thrown,  // полёт с физикой, подобрать нельзя
	Rest     // упал, снова интерактивный (шум уже разлетелся)
};

// Расходник, лежащий в мире: игрок подходит и подбирает его в инвентарь
// (по ItemId из каталога UBackroomsItemSystem). Это НЕ физический проп —
// его нельзя толкать/кидать, только поднять. При броске/дропе актор сам себя
// доводит: Thrown (физика + шум по удару) → Rest (покой, интерактивность).
UCLASS()
class BACKROOMS_API ABackroomsItemPickup : public AActor
{
	GENERATED_BODY()

public:
	ABackroomsItemPickup();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ID предмета в каталоге (AlmondWater, CanFood, MedKit, Pill, Energy, Battery).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Count = 1;

	// Радиус шума при ударе о поверхность (см). Берётся из каталога в Initialize.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float NoiseRadius = 800.0f;

	// Текущее состояние актора.
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	EPickupState State = EPickupState::Idle;

	// Событие шума при падении (для слуха сущностей).
	UPROPERTY(BlueprintAssignable, Category = "Item")
	FBackroomsItemNoise OnItemNoise;

	// Высота парения над полом и амплитуда вращения/покачивания (визуальный маркер).
	UPROPERTY(EditAnywhere, Category = "Item")
	float HoverHeight = 7.0f;

	UPROPERTY(EditAnywhere, Category = "Item")
	float SpinSpeed = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Item")
	float BobAmplitude = 2.0f;

	UPROPERTY(VisibleAnywhere, Category = "Item")
	TObjectPtr<USphereComponent> Trigger;

	UPROPERTY(VisibleAnywhere, Category = "Item")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// Настройка из генератора/дропа: предмет + меш (может быть nullptr — отрисуется
	// процедурной заглушкой).
	void Initialize(FName InItemId, int32 InCount, UStaticMesh* Mesh, float Scale);

	// Подобрать предмет игроку (по кнопке). Пикап сам уменьшает Count на принятое
	// и уничтожается на нуле; невлезшее остаётся в мире (честный «инвентарь полон»).
	bool TryPickupBy(class ABackroomsPlayerCharacter* Player);

	// Бросок (§4): Idle -> Thrown. Физика включается через кадр (чтобы не застрять
	// в стене), по удару/засыпанию — Rest + шум.
	void ThrowAt(const FVector& Impulse);

	// Летит ли предмет сейчас (нельзя подобрать/подсветить).
	bool IsInFlight() const { return State == EPickupState::Thrown; }

	// Золотая подсветка при наведении: точечный свет-пульс + CustomDepth-стенсил
	// (для пост-процессной обводки, если ассет в контенте будет).
	void SetGlowEnabled(bool bEnabled);

protected:
	UFUNCTION()
	void OnTriggerBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UStaticMesh* PlaceholderMesh = nullptr;
	FVector BaseLocation;
	float SpinPhase = 0.0f;
	float Scale = 1.0f;
	bool bGlow = false;
	TObjectPtr<UPointLightComponent> GlowLight;
	float GlowTimer = 0.0f;

	// Физика полёта включается с задержкой, чтобы пикап не застрял в точке спавна.
	bool bAwaitingPhysics = false;
	float PhysicsDelay = 0.0f;
	FVector PendingImpulse = FVector::ZeroVector;
	float RestTimeout = 0.0f;
	bool bNoiseSent = false;
	bool bNoiseBound = false;
};

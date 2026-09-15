#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Styling/SlateBrush.h"
#include "BackroomsNoiseMarker.generated.h"

class UWidgetComponent;
class UPointLightComponent;
class UAudioComponent;
class SImage;

// 3D-маркер «откуда пришёл шум»: небольшая полупрозрачная иконка
// (Content/UI/StatusIcons/17_noise.png), всегда повёрнутая к камере игрока.
// Спавнится в точке ЧУЖОГО шума (шаги в темноте, рык, грохот, хлопок двери),
// чтобы игрок видел источник звука. Живёт ровно пока звук играет, плавно гаснет
// при приближении игрока и намеренно не выглядит ярко — чтобы не выделяться из
// мира и не ломать реализм. Собственные шаги игрока маркер НЕ создают.
UCLASS()
class BACKROOMS_API ABackroomsNoiseMarker : public AActor
{
	GENERATED_BODY()

public:
	ABackroomsNoiseMarker();

	// Отрисовщик маркера в мире: плавающая иконка шума.
	UPROPERTY(VisibleAnywhere, Category = "Marker")
	TObjectPtr<UWidgetComponent> NoiseWidget;

	// Мягкое свечение в точке шума, чтобы маркер был заметен в темноте.
	UPROPERTY(VisibleAnywhere, Category = "Marker")
	TObjectPtr<UPointLightComponent> GlowLight;

	// Страховочный предел жизни маркера (используется, когда звук не привязан).
	UPROPERTY(EditAnywhere, Category = "Marker")
	float Lifetime = 6.0f;

	// Радиус «подошли близко»: внутри этой зоны маркер быстро гаснет.
	UPROPERTY(EditAnywhere, Category = "Marker")
	float ProximityRadius = 300.0f;

	// Привязать источник звука: маркер живёт ровно пока этот звук играет.
	void BindSound(class UAudioComponent* InComp);

	// Спавн маркера в точке шума (WorldContext — обычно генератор/игрок).
	static ABackroomsNoiseMarker* SpawnAt(UWorld* World, const FVector& Location, float InLifetime = 6.0f);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Точка появления (для лёгкого покачивания маркера вверх-вниз).
	FVector BaseLocation = FVector::ZeroVector;
	float Age = 0.0f;

	// Привязанный звук (см. BindSound). Слабый указатель: компонент Audio может
	// самоуничтожиться после окончания воспроизведения.
	TWeakObjectPtr<class UAudioComponent> BoundSound;

	bool bFading = false;
	float FadeBeginAge = 0.0f;
	float FadeDuration = 1.0f;

	// Близко ли игрок к источнику (плоская дистанция).
	bool IsPlayerClose() const;

	TSharedPtr<FSlateDynamicImageBrush> IconBrush;
	TSharedPtr<SImage> IconImage;

	// Загрузить 17_noise.png как runtime-кисть (BGRA), как в HUD.
	TSharedPtr<FSlateDynamicImageBrush> LoadIconBrush();
};
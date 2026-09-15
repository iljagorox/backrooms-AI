#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BackroomsStatusComponent.generated.h"

class ABackroomsPlayerCharacter;
class ABackroomsWorldGenerator;
struct FBackroomsItemDef;

// Состояние игрока (бейдж на HUD). Иконка для каждого — PNG в
// Content/UI/StatusIcons (12_overheating ... 23_vulnerability).
UENUM(BlueprintType)
enum class EBackroomsStatus : uint8
{
	None          UMETA(DisplayName = "Нет"),
	Overheating   UMETA(DisplayName = "Перегрев"),
	Poison        UMETA(DisplayName = "Отравление"),
	Radiation     UMETA(DisplayName = "Радиация"),
	Wet           UMETA(DisplayName = "Влажность"),
	Sleepiness    UMETA(DisplayName = "Сонливость"),
	Noise         UMETA(DisplayName = "Шум"),
	Odor          UMETA(DisplayName = "Запах"),
	Adrenaline    UMETA(DisplayName = "Адреналин"),
	Slowing       UMETA(DisplayName = "Замедление"),
	Acceleration  UMETA(DisplayName = "Ускорение"),
	Protection    UMETA(DisplayName = "Защита"),
	Vulnerability UMETA(DisplayName = "Уязвимость")
};

// Текущее состояние одного статуса.
USTRUCT(BlueprintType)
struct BACKROOMS_API FBackroomsStatusState
{
	GENERATED_BODY()

	// Интенсивность 0..1 (насколько ярко горит иконка и силён эффект).
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float Intensity = 0.0f;

	// Остаток времени, сек. <= 0 и bDriven=false — состояние спадает само.
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float TimeRemaining = 0.0f;

	// true — состояние «ведётся» средой (перегрев от бега и т.п.), спадает плавно.
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bDriven = false;
};

// Система состояний игрока. Не набор «кнопок», а живая среда: перегрев от бега,
// шум от шагов, радиация/ядовитый воздух по уровням, адреналин у монстра,
// защита/уязвимость от рассудка. Состояния плавно нарастают и спадают, влияют на
// скорость, урон и громкость для монстра. HUD показывает их иконками.
UCLASS(ClassGroup = (Backrooms), meta = (BlueprintSpawnableComponent))
class BACKROOMS_API UBackroomsStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBackroomsStatusComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Задать состояние на время Duration (<=0 — ведомое средой, спадает плавно).
	UFUNCTION(BlueprintCallable, Category = "Status")
	void AddStatus(EBackroomsStatus Status, float Intensity, float Duration = 0.0f);

	// Убрать состояние.
	UFUNCTION(BlueprintCallable, Category = "Status")
	void RemoveStatus(EBackroomsStatus Status);

	// Интенсивность состояния 0..1.
	UFUNCTION(BlueprintPure, Category = "Status")
	float GetIntensity(EBackroomsStatus Status) const;

	// Активно ли состояние (интенсивность выше порога).
	UFUNCTION(BlueprintPure, Category = "Status")
	bool IsStatusActive(EBackroomsStatus Status) const;

	// Остаток времени состояния.
	UFUNCTION(BlueprintPure, Category = "Status")
	float GetStatusTimeRemaining(EBackroomsStatus Status) const;

	// Множитель скорости передвижения от состояний.
	UFUNCTION(BlueprintPure, Category = "Status")
	float GetSpeedMultiplier() const;

	// Множитель убывания рассудка (защита снижает, уязвимость/перегрев повышают).
	UFUNCTION(BlueprintPure, Category = "Status")
	float GetSanityDrainMultiplier() const;

	// Доп. убывание рассудка в секунду (облучение/отравление).
	UFUNCTION(BlueprintPure, Category = "Status")
	float GetExtraSanityDrainPerSecond() const;

	// Убывание здоровья в секунду (отравление/радиация/перегрев).
	UFUNCTION(BlueprintPure, Category = "Status")
	float GetHealthDrainPerSecond() const;

	// Громкость, с которой игрок себя выдаёт (для слуха монстра). 0..1.
	UFUNCTION(BlueprintPure, Category = "Status")
	float GetNoiseLoudness() const;

	// Насколько «плывёт» камера (сонливость/отравление/радиация). 0..1.
	UFUNCTION(BlueprintPure, Category = "Status")
	float GetCameraDrift() const;

	// Реакция на использованный предмет: лекарства дают защиту и снимают хворь,
	// миндальная вода снимает отравление/влажность и т.п. (живая связь инвентаря
	// и состояний, а не только «кнопка лечит шкалу»).
	void NotifyItemUsed(const FBackroomsItemDef& Item);

	// Сообщить о монстре: близость рождает адреналин и запах.
	void SetMonsterProximity(float InProximity01);

	// Включить автоматический сбор среды (бег/уровень/монстр/рассудок).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool bAutoEnvironment = true;

	// Ниже этого рассудка копится уязвимость/сонливость.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float LowSanityThreshold = 30.0f;

	// Ближе этого расстояния до монстра (см) начинается адреналин.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MonsterPanicDistance = 2600.0f;

	// Порог «активности» иконки, выше которого HUD считает статус включённым.
	static constexpr float ActiveThreshold = 0.05f;

private:
	// Текущие состояния (плоская карта, без рефлексии — только числа).
	TMap<EBackroomsStatus, FBackroomsStatusState> Statuses;

	// --- Накопители среды ---
	float SprintHeat = 0.0f;      // нагрев от бега 0..1
	float Fatigue = 0.0f;         // усталость/сонливость 0..1
	float RestTimer = 0.0f;       // сколько подряд игрок стоит на месте (сек)
	float RadiationPulse = 0.0f;  // пульс радиации на «горячих» уровнях
	float PoisonPulse = 0.0f;     // пульс отравы
	float MonsterProximity = 0.0f;

	// Сгладить значение к цели.
	static float Approach(float Current, float Target, float DeltaTime, float SpeedPerSec);

	// Автосбор среды (вызывается из TickComponent).
	void UpdateEnvironment(ABackroomsPlayerCharacter* Player, float DeltaTime);

	// Найти генератор мира в сцене (для уровня/монстра). Может быть nullptr.
	ABackroomsWorldGenerator* FindGenerator() const;

	// Найти игрока-владельца.
	ABackroomsPlayerCharacter* GetPlayer() const;
};

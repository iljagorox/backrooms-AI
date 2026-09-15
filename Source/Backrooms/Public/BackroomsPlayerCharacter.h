#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BackroomsPlayerCharacter.generated.h"

// ---- События игрока (слой 2 / Task 8) ----
// Транслируются только, ничего не меняют в логике (безопасны для детерминизма).
// Имена типов с префиксом PhysProp — чтобы не конфликтовать с FOnItemPickedUp
// из UBackroomsItemSystem (там другие сигнатуры).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysPropPickedUp, AActor*, PropActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysPropDropped, AActor*, PropActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysPropThrown, AActor*, PropActor, FVector, Impulse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysPropPushed, AActor*, PropActor, FVector, Direction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEnteredRoom, int32, RoomID);

class UCameraComponent;
class USkeletalMeshComponent;
class USkeletalMesh;
class UMaterialInterface;
class UAnimBlueprint;
class UAnimSequence;
class USceneComponent;
class USpotLightComponent;
class ABackroomsPhysProp;
class UBackroomsItemSystem;
class UBackroomsStatusComponent;
class UBackroomsHUDWidget;
class UBackroomsGameOverWidget;
class UInventoryComponent;
class ABackroomsHeldItem;
class UBackroomsInspectComponent;
class UBackroomsEquipmentComponent;

UCLASS()
class BACKROOMS_API ABackroomsPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABackroomsPlayerCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	// Тело персонажа под камерой (первое лицо).
	UPROPERTY(VisibleAnywhere, Category = "Body")
	TObjectPtr<USkeletalMeshComponent> BodyMesh;

	// Статическое тело/руки (если нет SkeletalMesh) с процедурной анимацией шагов/покачивания
	UPROPERTY(VisibleAnywhere, Category = "Body")
	TObjectPtr<UStaticMeshComponent> StaticBodyMesh;

	// Фонарик (F). Крепится к камере, светит вперёд по взгляду.
	UPROPERTY(VisibleAnywhere, Category = "Equipment")
	TObjectPtr<USpotLightComponent> Flashlight;

	// Чувствительность мыши (множитель на MouseX/MouseY). Меняется из меню.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float MouseSensitivity = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMouseSensitivity(float InSensitivity);

	// Показывать/скрывать HUD (состояния + шкалы). В главном меню HUD скрыт,
	// включается только при старте игры, чтобы шкалы не висели поверх меню.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetHUDVisible(bool bVisible);

	// Инвентарь и статы игрока (голод/жажда/здоровье/рассудок). Создаётся в
	// BeginPlay; слоты предметов живут в InventoryComponent (источник правды),
	// ItemSystem — каталог и статы.
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UBackroomsItemSystem> ItemSystem;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UBackroomsItemSystem* GetItemSystem() const { return ItemSystem; }

	// Компонент инвентаря: TArray<FInventoryItem> InventorySlots.
	// Добавление/удаление/поиск/стаки — через массив, статданные из Data Asset.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	// Класс «предмета в руке» (BP_HeldItem): настоящий Actor, спавнится при
	// экипировке слота, крепится к сокету руки/камере.
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<ABackroomsHeldItem> HeldItemClass;

	// Сокет на скелете тела для предметов в руке. Если сокета нет (статическое
	// тело) — предмет крепится к HoldPoint перед камерой.
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	FName ItemSocketName = TEXT("ItemSocket");

	// Экипировать слот инвентаря: спавнит настоящий Actor предмета в руке.
	// Пустой слот или несуществующий индекс — убирает предмет из руки.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void EquipItemFromSlot(int32 InventorySlotIndex);

	// Убрать предмет из руки (уничтожить HeldItemActor).
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UnequipHeldItem();

	// Индекс экипированного слота (-1 — ничего в руке).
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetCurrentHeldSlot() const { return CurrentHeldSlotIndex; }

	// Держит ли игрок предмет из инвентаря в руке.
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsHoldingItem() const { return HeldItemActor != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	ABackroomsHeldItem* GetHeldItem() const { return HeldItemActor; }

	// Система состояний игрока (перегрев/радиация/шум/адреналин/защита и т.д.).
	// Наполняется средой и предметами, влияет на скорость/урон/громкость.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	TObjectPtr<UBackroomsStatusComponent> StatusComponent;

	UFUNCTION(BlueprintPure, Category = "Status")
	UBackroomsStatusComponent* GetStatusComponent() const { return StatusComponent; }

	// В городе (на стартовой платформе) выживальческий HUD не нужен: голод,
	// жажда и т.п. начинают иметь смысл только в Бэкрумсе. Возвращает true,
	// если игрок ещё на поверхности.
	UFUNCTION(BlueprintPure, Category = "HUD")
	bool IsInCity() const;

	// Подсистема достижений (может быть nullptr вне игры).
	class UBackroomsAchievements* GetAchievements() const;

	// Мета-прогрессия (XP/уровни/перки). nullptr — вне игры.
	class UBackroomsProgression* GetProgression() const;

	// Применить бонусы взятых перков к статам (HP/стамина/слоты/расход).
	// Вызывается при старте и при каждом открытии перка.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void ApplyPerkBonuses();

	// Колбэк подсистемы прогрессии: пересобрать бонусы при взятии перка.
	UFUNCTION()
	void OnPerkUnlockedHandler(FName PerkId, const FText& Name, int32 PointsLeft);

	// Подсказка взаимодействия с предметом под прицелом: что можно сделать и
	// какой клавишей. Клавиши берутся из переназначений (BackroomsInput), поэтому
	// после смены привязки текст обновляется сам. Пустая строка — смотрим в пустоту.
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FString GetInteractPrompt() const;

	// Вес предмета под прицелом (кг), -1 если не смотрим на физический предмет.
	UFUNCTION(BlueprintPure, Category = "Interaction")
	class ABackroomsPhysProp* GetFocusedProp() const;

	// Заряд фонарика (0..100).
	UFUNCTION(BlueprintPure, Category = "Equipment")
	float GetFlashlightBattery() const { return FlashlightBattery; }

	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsFlashlightOn() const { return bFlashlightOn; }

	// Экипирован ли сейчас фонарик-предмет (свет от него управляется кнопкой F).
	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsFlashlightItemEquipped() const;

	// Прямое управление камерным лучом (использует EquipmentComponent и предметы).
	void SetFlashlightFromItem(bool bOn);

	// Применить «реализм» фонарика: грязная линза (Light Function), объёмный
	// луч (Volumetric Fog) и интенсивность рассеивания. Зовётся в BeginPlay.
	void ApplyFlashlightRealism();

	// Компонент экипировки (держание, монтажи, фонарик-предмет).
	UFUNCTION(BlueprintPure, Category = "Equipment")
	class UBackroomsEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

	UFUNCTION(BlueprintPure, Category = "Input")
	bool IsSprinting() const { return bSprintHeld; }

	// Выносливость бега (0..100): бег тратит, ходьба/покой восстанавливают.
	UFUNCTION(BlueprintPure, Category = "Input")
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintPure, Category = "Input")
	float GetMaxStamina() const { return MaxStamina; }

	// Подобрать предмет из мира в массив инвентаря (по ID каталога предметов).
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool PickUpInventoryItem(FName ItemId, int32 Count = 1);

	// Использовать предмет из инвентаря (расходники восстанавливают статы).
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseInventoryItem(FName ItemId);

	// Использовать предмет из слота хотбара (1..4 → индекс 0..3).
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseHotbarSlot(int32 SlotIndex);

	// Индекс выбранного слота хотбара (-1 — ничего не выбрано).
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetSelectedHotbarSlot() const { return SelectedHotbarSlot; }

	// ID предмета в слоте хотбара (NAME_None — пусто).
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FName GetHotbarItemId(int32 SlotIndex) const
	{
		return HotbarItemIds.IsValidIndex(SlotIndex) ? HotbarItemIds[SlotIndex] : NAME_None;
	}

	// Взять/положить предмет в руку (E). При взятии предмет можно осматривать.
	void InspectHeld();
	// Осмотр: вращение удерживаемого предмета мышью (вызывается из Turn/LookUp).
	bool IsInspecting() const;

	// Мировая точка «под предметом», к которой HUD крепит текст подсказки.
	// false — смотрим в пустоту, промпт показывать не нужно.
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool GetInteractPromptWorldPoint(FVector& OutPoint) const;

	// Пикап под прицелом (ближайший к камере). Подсвечивается и включается в промпт.
	UFUNCTION(BlueprintPure, Category = "Interaction")
	class ABackroomsItemPickup* GetFocusedPickup() const;

	// Текущее здоровье (для инспектора и прогресса использования).
	UFUNCTION(BlueprintPure, Category = "Status")
	float GetHealth() const;

	// Компонент осмотра (для HUD: имя/описание во время осмотра).
	UFUNCTION(BlueprintPure, Category = "Inspect")
	class UBackroomsInspectComponent* GetInspectComponent() const { return InspectComponent; }

	UFUNCTION(BlueprintPure, Category = "Inspect")
	FName GetInspectItemId() const;

	UFUNCTION(BlueprintPure, Category = "Inspect")
	FText GetInspectItemName() const;

	UFUNCTION(BlueprintPure, Category = "Inspect")
	FString GetInspectItemDescription() const;

	// Якорь осмотра — камера игрока (InspectRoot прицепляется к ней).
	class USceneComponent* GetCameraComponent() const;

	// --- Удержание кнопки «использовать» (UseTime в каталоге предметов) ---
	// Consume-предмет с UseTime>0.15 требует удержания; прогресс 0..1 рисует HUD.
	bool IsUseHeld() const { return bUseHeld; }
	float GetUseHoldProgress() const { return (UseHoldDuration > 0.0f) ? FMath::Clamp(UseHoldTime / UseHoldDuration, 0.0f, 1.0f) : 0.0f; }
	FString GetUseHoldItemName() const;

	// ---- События игрока (BlueprintAssignable) ----
	UPROPERTY(BlueprintAssignable, Category = "Events|Player")
	FOnPhysPropPickedUp OnItemPickedUp;

	UPROPERTY(BlueprintAssignable, Category = "Events|Player")
	FOnPhysPropDropped OnItemDropped;

	UPROPERTY(BlueprintAssignable, Category = "Events|Player")
	FOnPhysPropThrown OnItemThrown;

	UPROPERTY(BlueprintAssignable, Category = "Events|Player")
	FOnPhysPropPushed OnItemPushed;

	UPROPERTY(BlueprintAssignable, Category = "Events|Player")
	FOnPlayerEnteredRoom OnPlayerEnteredRoom;

protected:
	// Загружаемая скелетная модель персонажа (SkeletalMesh).
	UPROPERTY(EditDefaultsOnly, Category = "Body")
	TSoftObjectPtr<USkeletalMesh> BodyMeshAsset;

	// Материал тела. Назначается в рантайме на слот 0 скелетного меша: правка
	// слота в самом ассете не сохранялась и текстура слетала.
	UPROPERTY(EditDefaultsOnly, Category = "Body")
	TObjectPtr<UMaterialInterface> BodyMaterial = nullptr;

	// Поворот модели относительно капсулы. У Mixamo forward = +Y (левая рука на
	// +X), а у UE forward = +X, поэтому меш доворачивается на -90° по yaw,
	// иначе персонаж «бежит боком» (вправо).
	UPROPERTY(EditDefaultsOnly, Category = "Body")
	FRotator BodyMeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);

	// Animation Blueprint, который водит анимации тела (Idle/Walk/Run).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Body|Animation")
	TSoftObjectPtr<UAnimBlueprint> AnimBlueprint;

	// Single-node анимации тела (используются, если AnimBlueprint не задан):
	// подбираются в Tick по скорости/полёту/приседу/смерти.
	UPROPERTY(EditDefaultsOnly, Category = "Body|Animation")
	TSoftObjectPtr<UAnimSequence> IdleAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Body|Animation")
	TSoftObjectPtr<UAnimSequence> WalkAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Body|Animation")
	TSoftObjectPtr<UAnimSequence> RunAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Body|Animation")
	TSoftObjectPtr<UAnimSequence> JumpUpAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Body|Animation")
	TSoftObjectPtr<UAnimSequence> JumpDownAnim;

	// Анимация падения (JumpDown) включается только при падении с высоты,
	// большей MinFallAnimHeight — иначе короткий прыжок показывает обычный
	// бег/шаг, а не «полёт в пропасть».
	UPROPERTY(EditDefaultsOnly, Category = "Body|Animation")
	float MinFallAnimHeight = 150.0f;

	// Z-координата точки, с которой началось текущее падение (для подсчёта
	// высоты падения без лишней кучи полей состояния).
	float FallStartZ = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Body|Animation")
	TSoftObjectPtr<UAnimSequence> CrawlAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Body|Animation")
	TSoftObjectPtr<UAnimSequence> PunchAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Body|Animation")
	TSoftObjectPtr<UAnimSequence> DieAnim;

	// Триггер одноразовой анимации (удар/смерть): проигрывается поверх базовой,
	// затем UpdateBodyAnimation возвращает базовую.
	void PlayBodyOneShot(UAnimSequence* Seq);

	// Одноразовая анимация активна (до конца проигрывания).
	bool bBodyOneShot = false;
	float BodyOneShotTimeLeft = 0.0f;

	UPROPERTY()
	TObjectPtr<UAnimSequence> CurrentBodyAnim;

	void UpdateBodyAnimation(float DeltaTime);

	void LoadBodyVisuals();
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	// Привязка ввода (Enhanced Input / legacy ActionMappings).
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- Take to hand / push / throw ---
	// Взять предмет с пола перед камерой / положить обратно (E).
	void TakeToHand();
	// Толкнуть удерживаемый предмет по направлению камеры (ЛКМ).
	void PushHeld();
	// Бросить удерживаемый предмет (вес влияет на дальность) (ПКМ).
	// Для физических пропов — мгновенно; для предмета из инвентаря — с зарядом:
	// удержание копит силу (0..1 за ~0.5 с), отпускание бросает.
	void ThrowHeld();
	void StartThrowCharge();
	void FireThrowCharge();

	// Открыть/закрыть меню паузы и настроек (ESC / P).
	void TogglePauseMenu();

	// Открыть/закрыть инвентарь-бестиарий (Tab).
	void ToggleInventory();

	// Бег (Shift): меняет MaxWalkSpeed между ходьбой и бегом.
	void StartSprint();
	void StopSprint();

	// Выносливость: расход при беге, восстановление в покое/на ходьбе.
	void UpdateStamina(float DeltaTime);

	// Фонарик (F).
	void ToggleFlashlight();

	// Переключить вид от 1-го / 3-го лица (V).
	void ToggleView();

	// Зум камеры в третьем лице (колесо мыши).
	void ZoomView(float Value);

	// Удар кулаком (Q): одноразовая анимация + толчок перед собой.
	void Attack();

	// Смерть: одноразовая анимация, блокировка ввода.
	void Die();
	bool bDead = false;

	// Вид от третьего лица (плечо): камера сзади. По умолчанию — первое лицо.
	bool bThirdPerson = false;

	// Выбранный индекс слота хотбара (-1 — нет).
	// (определён ниже как SelectedHotbarSlot)

	// Использовать выбранный слот хотбара (R).
	void UseSelectedItem();

	// Выбор слота хотбара (1..4 → индекс 0..3). Кроме выбора — экипирует слот.
	void SelectHotbarSlot(int32 HotbarIndex);

	// Переключение активного слота колёсиком мыши (в первом лице).
	void CycleHotbarSlot(int32 Direction);

	// Инвентарный индекс N-го непустого слота (-1 — нет такого).
	int32 NthNonEmptySlotIndex(int32 N) const;

	// Синхронизация: если экипированный слот опустел — убрать предмет из руки.
	void SyncHeldItemToInventory();

	void SelectHotbarSlot1();
	void SelectHotbarSlot2();
	void SelectHotbarSlot3();
	void SelectHotbarSlot4();

	// Осмотр удерживаемого предмета (клавиша Inspect, удержание): крутим предмет
	// мышью вместо камеры.
	void StartInspect();
	void StopInspect();

	// Пополнить заряд фонарика (использована батарейка).
	UFUNCTION()
	void AddFlashlightCharge(float Amount);

	// Пересобрать хотбар из инвентаря (первые 4 стека).
	UFUNCTION()
	void RebuildHotbar();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UBackroomsPauseMenuWidget> PauseMenuClass;

	UPROPERTY()
	TObjectPtr<class UBackroomsPauseMenuWidget> PauseMenuWidget;

	// Экран смерти.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UBackroomsGameOverWidget> GameOverWidgetClass;

	UPROPERTY()
	TObjectPtr<class UBackroomsGameOverWidget> GameOverWidget;

	// Внутриигровой HUD (состояния + шкалы). Создаётся в BeginPlay.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UBackroomsHUDWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<class UBackroomsHUDWidget> HUDWidget;

	// Инвентарь-бестиарий (сетка предметов + 3D-превью). Открывается клавишей.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UBackroomsInventoryWidget> InventoryWidgetClass;

	UPROPERTY()
	TObjectPtr<class UBackroomsInventoryWidget> InventoryWidget;

	// Точка крепления взятых предметов (к камере, первое лицо).
	UPROPERTY(VisibleAnywhere, Category = "Grab")
	TObjectPtr<USceneComponent> HoldPoint;

	// --- Хотбар ---
	// Выбранный слот (0..3, -1 — нет). Заполняется первыми четырьмя стеками
	// инвентаря; использование — клавишами 1..4.
	int32 SelectedHotbarSlot = -1;

	// Массив слотов хотбара: параллельные массивы ItemId + Count (Count=0 —
	// пусто). Строится из инвентаря при изменении.
	TArray<FName> HotbarItemIds;

	// --- Предмет в руке (экипированный слот инвентаря) ---
	// Индекс слота в InventoryComponent, который сейчас в руке (-1 — нет).
	int32 CurrentHeldSlotIndex = -1;

	// Компонент экипировки: состояние руки, монтажи Equip/Use, фонарик-предмет.
	UPROPERTY(VisibleAnywhere, Category = "Equipment")
	TObjectPtr<UBackroomsEquipmentComponent> EquipmentComponent;

	// Настоящий Actor предмета в руке (BP_HeldItem). null — руки пусты.
	UPROPERTY(Transient)
	TObjectPtr<ABackroomsHeldItem> HeldItemActor;

	// --- Осмотр предмета в руке ---
	// Взят в руку предмет можно поворачивать мышью (осмотр).
	bool bInspecting = false;
	FRotator InspectRotation = FRotator::ZeroRotator;

	// Компонент осмотра Предметов из инвентаря (спека §3): меш на InspectRoot у
	// камеры, блок движения, обязательный срыв по урону.
	UPROPERTY(VisibleAnywhere, Category = "Inspect")
	TObjectPtr<class UBackroomsInspectComponent> InspectComponent;

	// --- Удержание «использовать» ---
	bool bUseHeld = false;
	FName UseHoldItemId = NAME_None;
	float UseHoldTime = 0.0f;

	// --- Заряд кидания предмета из инвентаря (§8.4) ---
	// ПКМ удержание копит Charge (0..1 за ~0.5 с); отпускание бросает пикап
	// с импульсом ThrowImpulseRange. Физические пропы бросаются сразу.
	bool bThrowCharging = false;
	float ThrowHoldTime = 0.0f;
	static constexpr float ThrowChargeDuration = 0.5f;
	float UseHoldDuration = 1.0f;
	float UseHoldStartHealth = -1.0f;

	// --- Подсветка пикапа под прицелом (золотой glow) ---
	TWeakObjectPtr<class ABackroomsItemPickup> FocusedPickup;
	TWeakObjectPtr<class ABackroomsItemPickup> GlowedPickup;
	float FocusUpdateTimer = 0.0f;

	// Обработчики удержания Use (Pressed/Released) и их логика.
	void UseSelectedItemPressed();
	void UseSelectedItemReleased();
	void UpdateUseHold(float DeltaTime);

	// Обновление FocusedPickup + glow (throttled в Tick).
	void UpdateFocusedPickup(float DeltaTime);

	// Сигнализация от InspectComponent: осмотр оборван (в т.ч. уроном).
	void HandleInspectEnded(bool bForced);

	// --- Head Bob ---
	UPROPERTY(EditDefaultsOnly, Category = "Camera|HeadBob")
	float BobAmplitude = 2.0f;      // амплитуда вертикального боба (см)

	UPROPERTY(EditDefaultsOnly, Category = "Camera|HeadBob")
	float BobSideAmplitude = 1.0f;  // амплитуда бокового боба

	UPROPERTY(EditDefaultsOnly, Category = "Camera|HeadBob")
	float BobFrequencyBase = 10.0f; // частота при ходьбе (колебаний/сек)

	float BobTimer = 0.0f;

	// --- Breathing ---
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Breathing")
	float BreathAmplitude = 0.5f;   // амплитуда дыхания (см)

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Breathing")
	float BreathFrequency = 2.0f;   // полных циклов вдох/выдох в секунду

	float BreathTimer = 0.0f;

	// Таймер дрейфа камеры от состояний (сонливость/отравление).
	float CameraDriftTimer = 0.0f;

	// --- Footsteps ---
	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	float WalkStepInterval = 0.5f;  // секунд между шагами при ходьбе

	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	float RunStepInterval = 0.3f;   // секунд между шагами при беге

	float StepAccumulator = 0.0f;
	bool bWasMoving = false;

	void Footstep();
	float GetCurrentStepInterval() const;

	// Выдать шум, который могут услышать монстры (шаги/бросок/толчок).
	void EmitNoise(float Loudness);

	// Предметный пикап (кинутый §8.4) стукнулся о поверхность: его шум
	// ретранслируется монстрам через EmitNoise (радиус → громкость).
	UFUNCTION()
	void OnPickupNoise(FVector Location, float Radius);

	// Присед (Ctrl, удержание): тише шаги и ниже камера.
	void StartCrouch();
	void StopCrouch();

	// Рассудок от среды: темнота и вид монстра роняют, свет + тишина лечат.
	void UpdateFear(float DeltaTime);
	// Постпроцесс камеры по уровню страха (расщепление/виньетка/цвет).
	void UpdateSanityPostProcess(float DeltaTime);

private:
	// Базовая позиция камеры (относительно капсулы): ровно над капсулой, чуть
	// выше уровня макушки — при наклоне взгляда макушка/тело не мешают, а сама
	// камера не проваливается в стены и не оказывается внутри тела (X=0).
	FVector CameraBaseOffset = FVector(0.0f, 0.0f, 64.0f);

	// Смещение камеры в третьем лице (назад и чуть вверх от уровня глаз).
	FVector ThirdPersonOffset = FVector(-260.0f, 0.0f, 90.0f);

	// Текущая дистанция камеры в третьем лице (зум колесом мыши).
	float ThirdPersonDistance = 260.0f;
	float MinThirdPersonDistance = 80.0f;
	float MaxThirdPersonDistance = 500.0f;

	// --- Тряски камеры (у каждой события — своя частота/амплитуда/вибрация) ---
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
	float WalkShakeAmplitude = 0.45f;            // ходьба/бег: едва заметная дрожь
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
	float PunchShakeAmplitude = 1.8f;            // попадание кулаком в существо/проп
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
	float PunchShakeFrequency = 22.0f;           // короткий резкий «толчок»
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
	float PunchShakeDuration = 0.25f;
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
	float FallShakeMaxAmplitude = 2.6f;          // касание земли (сила — от высоты)
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
	float FallShakeFrequency = 11.0f;            // низкий глухой удар
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
	float FallShakeDuration = 0.55f;
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
	float MonsterShakeRadius = 320.0f;           // вибрация страха при близости твари
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
	float MonsterShakeMaxAmplitude = 2.2f;
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
	float MonsterShakeBaseFrequency = 9.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
	float PunchKnockbackForce = 600.0f;          // сила отброса существа (импульс*силу)

	// Импульсная тряска (удар/падение): затухающая огибающая во времени.
	struct FShakePulse
	{
		bool bActive = false;
		float Age = 0.0f;
		float Amplitude = 0.0f;
		float Frequency = 0.0f;
		float Duration = 0.0f;
		FVector Axis = FVector::ZeroVector;
	};
	FShakePulse PunchShake;
	FShakePulse FallShake;
	float ShakeClock = 0.0f;
	float MonsterProx = 0.0f;                    // близость монстра 0..1 (каждый кадр)
	bool bWasFallingShake = false;
	float FallImpactMax = 0.0f;                  // макс. скорость падения (сила тряски)

	void TriggerPunchShake();
	void TriggerFallShake();
	FVector ComputeShakeOffset(float DeltaTime, float Speed01);

	// Скорости ходьбы/бега (см/с).
	float WalkSpeed = 350.0f;
	float RunSpeed = 700.0f;
	bool bSprintHeld = false;

	// --- Выносливость бега ---
	// Бег тратит запас; когда он на нуле, бег блокируется до восстановления
	// до порога (гистерезис), чтобы шкала не «дребезжала» на нуле.
	float Stamina = 100.0f;
	float MaxStamina = 100.0f;
	float StaminaDrainPerSecond = 7.5f;
	float StaminaRegenPerSecond = 14.0f;
	float StaminaRegenDelay = 0.7f;
	float StaminaRegenDelayTimer = 0.0f;
	float StaminaRecoverThreshold = 15.0f;
	bool bStaminaExhausted = false;
	bool bFlashlightOn = false;

	// Поблизости есть комнатные лампы (без учёта фонарика) —
	// для динамической адапции камеры к уровню освещённости.
	bool bCurrentlyLit = false;

	// Заряд фонарика. Батарейки — расходник; на нуле фонарик гаснет.
	float FlashlightBattery = 100.0f;
	float FlashlightDrainPerSecond = 0.55f;

	// --- Фонарик: реализм (Light Function + объёмный луч) ---
	// Материал «грязной линзы» (Light Function): дефекты стекла, неровности
	// пятна. Пустая ссылка — фонарь остаётся чистым (ассет в контенте опционален).
	UPROPERTY(EditDefaultsOnly, Category = "Flashlight")
	TSoftObjectPtr<class UMaterialInterface> FlashlightLensMaterial;

	// Объёмный туман в мире для осязаемого луча (включается для ExponentialHeightFog).
	UPROPERTY(EditDefaultsOnly, Category = "Flashlight")
	bool bFlashlightVolumetricFog = true;

	// «Вес» луча в объёмном тумане (VolumetricScatteringIntensity).
	UPROPERTY(EditDefaultsOnly, Category = "Flashlight")
	float FlashlightVolumetricScattering = 1.8f;

	// --- Рендерер: Lumen / Virtual Shadow Maps / контактные тени ---
	// Включаются консольными командами из BeginPlay (иначе просятся галочки в
	// Project Settings). На слабых GPU можно снять, чтобы не платить за GI/VSM.
	UPROPERTY(EditDefaultsOnly, Category = "Renderer")
	bool bEnableLumen = true;

	UPROPERTY(EditDefaultsOnly, Category = "Renderer")
	bool bEnableVirtualShadowMaps = true;

	UPROPERTY(EditDefaultsOnly, Category = "Renderer")
	bool bEnableContactShadows = true;

	void ApplyRendererDefaults();

	// --- Взаимодействие с предметами ---
	// Текущий удерживаемый физический проп (nullptr — руки пусты).
	TObjectPtr<ABackroomsPhysProp> HeldProp;
	float GrabDistance = 260.0f;   // дальность трассировки взять
	float PushImpulse = 260.0f;    // базовая сила толчка (для лёгких)
	float HoldOffset = 80.0f;      // дистанция предмета перед камерой при холде

	// --- Комнатная навигация (Task 8: OnPlayerEnteredRoom) ---
	// Проба клетки пола по образцу генератора: клетка = профильная CellSize
	// (при tile_size_m=0.5 это 500 см). Значение обновляется в BeginPlay из
	// активного профиля генератора; 500 здесь — фолбэк, не хардкод геометрии.
	// RoomID — хеш пары клеток (ComputeRoomID), без коллизий Y*10000+X.
	float RoomProbeCellSize = 500.0f;
	int32 LastRoomID = TNumericLimits<int32>::Min();
	// Кэш параметров генератора для XP-хуков (уровень и размер чанка в клетках).
	int32 ProbedLevelIndex = 0;
	int32 ProbedChunkSizeCells = 8;

	// --- Страх/рассудок от среды ---
	// Сглаженный уровень страха 0..1 (используется постпроцессом).
	float FearValue = 0.0f;
	// Бонусы перков (кэш из UBackroomsProgression), применяются в Tick.
	float PerkSanityDrainMult = 1.0f;
	float PerkFearGrowthMult = 1.0f;
	// Убывание рассудка в темноте (в секунду).
	float DarknessSanityDrain = 1.4f;
	// Убывание рассудка, когда монстр в поле зрения (в секунду).
	float MonsterSightSanityDrain = 6.0f;
	// Восстановление рассудка в безопасной, освещённой зоне (в секунду).
	float SafeSanityRegen = 1.2f;
	// Радиус, в котором комнатный свет считается «при мне». Берётся из активного
	// профиля через генератор (GetActiveLightRadius); 450 = фолбэк для every_m 5 м.
	float SafeLightRadius = 450.0f;
	// Во сколько раз присед снижает шум шагов.
	float CrouchNoiseScale = 0.45f;
};

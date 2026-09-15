#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BackroomsRiggedMonster.generated.h"

class USkeletalMeshComponent;
class USkeletalMesh;
class UAnimationAsset;
class UAnimBlueprint;
class UWidgetComponent;
class STextBlock;

// Скелетный монстр-патрон с РИГИНГОМ.
//
// Сюда садится скачанная моделька (SkeletalMesh, у которой вероятно есть
// ригинг/скелет + анимации). Класс наследует ACharacter, поэтому:
//   * у него есть капсула + движение (ACharacter уже даёт их из коробки);
//   * контроллер ABackroomsMonsterAIController штатно садится на него через
//     Possess и водит MoveToActor/MoveToLocation по нав-мешу;
//   * скелетный меш управляется USkeletalMeshComponent -> ригинг/анимации
//     проигрываются сами, AI их не ломает.
//
// ПОВЕДЕНИЕ — «МЯГЧЕ» (как просил игрок): монстр НЕ атакует по скрипту и не
// прыгает на игрока. Он ведёт себя как угроза: патрулирует, исследует,
// а при контакте просто подходит вплотную и «нависает» (standoff), давая
// игроку шанс сбежать. Агрессия/скорость настраиваются тут.
UCLASS(Blueprintable, ClassGroup = (Backrooms))
class BACKROOMS_API ABackroomsRiggedMonster : public ACharacter
{
	GENERATED_BODY()

public:
	ABackroomsRiggedMonster();

	// Скелет (сетка + ригинг) — присваивается из загруженного архива.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Visuals")
	TObjectPtr<USkeletalMesh> EnemySkeletalMesh;

	// Анимация покоя (idle) — проигрывается на скелете.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Visuals")
	TObjectPtr<UAnimationAsset> IdleAnimation;

	// Animation Blueprint, который сам водит анимации по скорости павна
	// (Idle/Walk/Chase) — как у игрока. Если задан, IdleAnimation не нужен
	// и PlayAnimation не вызывается (конфликтует с AnimInstanceClass).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Visuals")
	TSoftObjectPtr<UAnimBlueprint> AnimBlueprint;

	// --- «Мягкое» поведение: скорости ---
	// Скорость ходьбы при патруле/исследовании (медленно, ненавязчиво).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Soft")
	float PatrolWalkSpeed = 150.0f;

	// Скорость в режиме тревоги (подход к игроку) — заметно быстрее, но
	// всё равно не «пуля»: игрок успевает скрыться.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Soft")
	float ApproachWalkSpeed = 250.0f;

	// Дистанция «нависания»: на какой дистанции монстр останавливается
	// перед игроком в режиме тревоги (жёсткости нет — он просто близко).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Soft")
	float StandoffDistance = 170.0f;

	// Применить модель + анимацию к скелетному мешу (вызывается после того,
	// как EnemySkeletalMesh/IdleAnimation назначены в редакторе/Blueprints).
	UFUNCTION(BlueprintCallable, Category = "Monster|Visuals")
	void ApplyEnemyVisuals();

	// Включить «мягкую» скорость (патруль) или скоростной подход (тревога).
	UFUNCTION(BlueprintCallable, Category = "Monster|Soft")
	void SetAggressiveMode(bool bAggressive);

	// Сообщить монстру о звуке (шаги/падение/бросок). Пробрасывается в
	// сенсор AI-контроллера, который решает, слышно ли и куда идти.
	UFUNCTION(BlueprintCallable, Category = "Monster|Sense")
	void HearNoise(const FVector& Location, float Loudness);

	// Исчезнуть: спрятать меш/коллайдер и замереть (состояние Disappearing).
	UFUNCTION(BlueprintCallable, Category = "Monster|Presence")
	void Vanish();

	// Появиться заново в указанной точке (возврат из исчезновения).
	UFUNCTION(BlueprintCallable, Category = "Monster|Presence")
	void ReappearAt(const FVector& Location);

	UFUNCTION(BlueprintPure, Category = "Monster|Presence")
	bool IsVanished() const { return bVanished; }

	// Отброс от удара игрока (кулак/толчок). Существо НЕ умирает и не получает
	// урона (хоррор): его тело чуть «откланивается» назад (анимация) + лёгкий
	// пинок, остаток силы гаснет экспоненциально с каждым кадром.
	UFUNCTION(BlueprintCallable, Category = "Monster|Physics")
	void TakePunch(const FVector& HitLocation, const FVector& Direction, float Force);

	// Режим «осмотра» (из Investigating-стадии AI): монстр приподнимает и
	// крутит голову по сторонам, оглядываясь. Вызывается контроллером.
	UFUNCTION(BlueprintCallable, Category = "Monster|ProceduralAnim")
	void SetInspectMode(bool bOn);

	// Включить/выключить процедурную анимацию (походка/бег/осмотр кодом).
	UFUNCTION(BlueprintCallable, Category = "Monster|ProceduralAnim")
	void EnableProceduralAnimation(bool bEnable);

	// Сесть/встать (AI Sitting). Себя позу пишет процедурный AnimInstance;
	// здесь — передача режима и расчет опускания таза под высоту сиденья.
	UFUNCTION(BlueprintCallable, Category = "Monster|ProceduralAnim")
	void SetSitting(bool bInSit, float InSeatHeightCm);

	// Имя текущего состояния ИИ для плавающего ярлыка над головой.
	UFUNCTION(BlueprintCallable, Category = "Monster|ProceduralAnim")
	void SetStateDisplayName(const FString& InName);

	UFUNCTION(BlueprintPure, Category = "Monster|ProceduralAnim")
	bool IsSitting() const { return bSittingAnim; }

protected:
	// Физический отброс окна в Tick (затухание импульса каждый кадр).
	virtual void Tick(float DeltaSeconds) override;

	// Сам скелетный компонент (ригинг/анимации сидят здесь).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Visuals")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

private:
	// --- Отброс (TakePunch) ---
	bool bKnockbackActive = false;
	bool bKnockbackPhysics = false;   // скелет сейчас симулируется (рагдолл)
	FVector KnockbackDir = FVector::ZeroVector;
	FVector KnockbackHitPoint = FVector::ZeroVector;
	float KnockbackForce = 0.0f;      // текущий остаток силы (затухает)
	// Скорость затухания (1/сек): остаток умножается на e^-rate*dt.
	float KnockbackDecayRate = 6.0f;
	// Множитель «сила в секунду» для импульса каждого кадра.
	float KnockbackApplyScale = 1.0f;
	float KnockbackRecoverTimer = 0.0f;
	float KnockbackRecoverDelay = 1.2f;

	void UpdateKnockback(float DeltaSeconds);
	void DisableKnockbackPhysics();

	// «Отклонение» после удара: меш на секунду наклоняется назад (пружинно).
	float RecoilLean = 0.0f;
	float RecoilPitchMax = 9.0f;     // градусы наклона назад в полном отбросе
	float RecoilRecoveryRate = 2.2f; // скорость возврата к вертикали

	// Состояние исчезновения (Vanish/ReappearAt).
	bool bVanished = false;

	// --- Процедурная женская анимация (походка/бег/осмотр кодом через
	// UBackroomsProceduralAnimInstance). Модель Karelia крупная — приводим
	// масштаб к человеческому.
	UPROPERTY(EditAnywhere, Category = "Monster|ProceduralAnim")
	bool bProceduralAnimation = true;

	// Истинный рост тела модели (см) в её собственных единицах при масштабе 1.
	UPROPERTY(EditAnywhere, Category = "Monster|ProceduralAnim")
	float KareliaBodyHeightCm = 535.0f;
	// Желаемый рост тела монстра в мире (см), ~рост игрока.
	UPROPERTY(EditAnywhere, Category = "Monster|ProceduralAnim")
	float MonsterVisualHeightCm = 182.0f;

	// Режим «осмотра» (AI Investigating): голова крутится по сторонам.
	bool bInspecting = false;

	// --- Режим сидения (AI Sitting) ---
	bool bSittingAnim = false;
	float SitDropCm = 0.0f;            // опускание таза при посадке (см)

	// Плавающий ярлык состояния над головой (Slate-виджет, как NoiseMarker).
	UPROPERTY(Transient)
	TObjectPtr<class UWidgetComponent> StateWidget;
	TSharedPtr<STextBlock> StateTextLabel;
	FString StateDisplayName;
	float StateLabelBaseZ = 190.0f;
	float StateLabelAlpha = 0.0f;
};

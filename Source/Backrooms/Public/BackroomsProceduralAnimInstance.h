#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"

#include "BackroomsProceduralAnimInstance.generated.h"

// Прокси анимации монстра: собственный Evaluate пишет ВСЮ позу процедурно
// (женская походка/бег/осмотр) без анимационного графа. Данные латчуются
// с игрового потока каждый кадр (см. NativeUpdateAnimation), вычисления —
// со стороны воркера.
class FBackroomsProceduralAnimProxy final : public FAnimInstanceProxy
{
public:
	FBackroomsProceduralAnimProxy() {}
	explicit FBackroomsProceduralAnimProxy(UAnimInstance* InInstance) : FAnimInstanceProxy(InInstance) {}

	virtual bool Evaluate(FPoseContext& Output) override;

	// --- Настройки/состояние, обновляются с игрового потока ---
	float WalkCycleTime = 2.4f;
	float RunCycleTime = 0.85f;
	float WalkSpeedCmS = 160.0f;
	float RunSpeedCmS = 430.0f;
	float SwingPolarity = 1.0f;
	float LegSwingAmp = 26.0f;
	float KneeBendDeg = 26.0f;
	float HipSwayAmp = 4.0f;
	float HipRollAmp = 7.0f;
	float BodyBobAmp = 3.5f;
	float ArmSwingAmp = 11.0f;
	float ElbowBendDeg = 20.0f;
	float RunForwardLeanDeg = 14.0f;
	float BreastBounceAmp = 0.9f;
	float HairSwayAmp = 8.0f;
	float InspectLookAmpYaw = 52.0f;
	float InspectLookFreq = 1.1f;

	float Speed2D = 0.0f;
	float LastDelta = 0.0f;
	float PropAnimTime = 0.0f;
	float InspectPhase = 0.0f;
	bool bInspecting = false;

	// --- Режим сидения ---
	float SitBlendSpeed = 2.2f;        // скорость входа/выхода из позы сидя
	float SitSwayFreq = 0.7f;          // Гц лёгкого покачивания сидя
	float SitSwayAmpDeg = 3.0f;        // град амплитуда покачивания
	float SitPelvisRockDeg = 2.5f;     // град перекат таза сидя
	float SitHeadLookPitchDeg = 8.0f;  // град взгляд слегка вниз
	float CrossLegBlendSpeed = 2.0f;   // скорость скрещивания ног

	// Состояние/входные данные (с игрового потока).
	bool bSitting = false;
	float SitDropCm = 0.0f;            // на сколько опускаем таз под сиденье
	// Таймеры/бленды (счёт в Evaluate, воркер).
	float SitBlend = 0.0f;
	float CrossLegBlend = 0.0f;
	float SitPhase = 0.0f;
};

UCLASS()
class BACKROOMS_API UBackroomsProceduralAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// AI: включить режим осмотра (голова крутится по сторонам).
	void SetInspecting(bool bInInspecting) { bInspecting = bInInspecting; }

	// AI: сесть/встать. InSitDropCm — на сколько опустить таз под сиденье.
	void SetSitting(bool bInSit, float InSitDropCm);

	// --- Настройки сидения ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Sit")
	float SitBlendSpeed = 2.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Sit")
	float SitSwayFreq = 0.7f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Sit")
	float SitSwayAmpDeg = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Sit")
	float SitPelvisRockDeg = 2.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Sit")
	float SitHeadLookPitchDeg = 8.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Sit")
	float CrossLegBlendSpeed = 2.0f;

	// --- Настройки процедурной походки ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float WalkCycleTime = 2.4f;       // с/цикл: медленная, грациозная
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float RunCycleTime = 0.85f;       // с/цикл: энергичный бег
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float WalkSpeedCmS = 160.0f;      // скорость перехода в бег
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float RunSpeedCmS = 430.0f;       // скорость полного бега
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float SwingPolarity = 1.0f;       // 1/-1: направление маха ног/рук
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float LegSwingAmp = 26.0f;        // град: мах бедра
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float KneeBendDeg = 26.0f;        // град: сгиб колена
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float HipSwayAmp = 4.0f;          // см: боковое покачивание таза
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float HipRollAmp = 7.0f;          // град: перекат таза (женственность)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float BodyBobAmp = 3.5f;          // см: вертикальный подъём на шаге
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float ArmSwingAmp = 11.0f;        // град: мягкий мах рук
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float ElbowBendDeg = 20.0f;       // град: мягко согнутые локти
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Gait")
	float RunForwardLeanDeg = 14.0f;  // град: наклон туловища в беге

	// --- Женская вторичная динамика / осмотр ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Secondary")
	float BreastBounceAmp = 0.9f;     // см: динамика груди
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Secondary")
	float HairSwayAmp = 8.0f;         // град: развевание волос
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Inspect")
	float InspectLookAmpYaw = 52.0f;  // град: размах взгляда при осмотре
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural|Inspect")
	float InspectLookFreq = 1.1f;     // Гц: частота качания головы

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	bool bInspecting = false;
	bool bSitting = false;
	float SitDropCm = 0.0f;
};
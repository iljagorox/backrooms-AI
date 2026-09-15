#include "BackroomsProceduralAnimInstance.h"

#include "Animation/AnimNodeBase.h"
#include "BoneContainer.h"
#include "BonePose.h"

FAnimInstanceProxy* UBackroomsProceduralAnimInstance::CreateAnimInstanceProxy()
{
	return new FBackroomsProceduralAnimProxy(this);
}

void UBackroomsProceduralAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{
	delete InProxy;
}

void UBackroomsProceduralAnimInstance::SetSitting(bool bInSit, float InSitDropCm)
{
	bSitting = bInSit;
	SitDropCm = InSitDropCm;
}

void UBackroomsProceduralAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	FBackroomsProceduralAnimProxy& P = GetProxyOnGameThread<FBackroomsProceduralAnimProxy>();
	P.PropAnimTime = 0.0f;
	P.InspectPhase = 0.0f;
	P.bInspecting = false;
	P.Speed2D = 0.0f;
	P.LastDelta = 0.016f;
	P.SitBlend = 0.0f;
	P.CrossLegBlend = 0.0f;
	P.SitPhase = 0.0f;
}

void UBackroomsProceduralAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Скорость павна для бленда ходьба/бег.
	float Speed2D = 0.0f;
	if (const AActor* OwningActor = GetOwningActor())
	{
		Speed2D = OwningActor->GetVelocity().Size2D();
	}

	// Латчим настройки и состояние в прокси-копию (воркер прочитает в Evaluate).
	FBackroomsProceduralAnimProxy& P = GetProxyOnGameThread<FBackroomsProceduralAnimProxy>();
	P.WalkCycleTime = WalkCycleTime;
	P.RunCycleTime = RunCycleTime;
	P.WalkSpeedCmS = WalkSpeedCmS;
	P.RunSpeedCmS = RunSpeedCmS;
	P.SwingPolarity = SwingPolarity;
	P.LegSwingAmp = LegSwingAmp;
	P.KneeBendDeg = KneeBendDeg;
	P.HipSwayAmp = HipSwayAmp;
	P.HipRollAmp = HipRollAmp;
	P.BodyBobAmp = BodyBobAmp;
	P.ArmSwingAmp = ArmSwingAmp;
	P.ElbowBendDeg = ElbowBendDeg;
	P.RunForwardLeanDeg = RunForwardLeanDeg;
	P.BreastBounceAmp = BreastBounceAmp;
	P.HairSwayAmp = HairSwayAmp;
	P.InspectLookAmpYaw = InspectLookAmpYaw;
	P.InspectLookFreq = InspectLookFreq;
	P.SitBlendSpeed = SitBlendSpeed;
	P.SitSwayFreq = SitSwayFreq;
	P.SitSwayAmpDeg = SitSwayAmpDeg;
	P.SitPelvisRockDeg = SitPelvisRockDeg;
	P.SitHeadLookPitchDeg = SitHeadLookPitchDeg;
	P.CrossLegBlendSpeed = CrossLegBlendSpeed;
	P.bSitting = bSitting;
	P.SitDropCm = SitDropCm;
	P.Speed2D = Speed2D;
	P.bInspecting = bInspecting;
	P.LastDelta = DeltaSeconds;
}



bool FBackroomsProceduralAnimProxy::Evaluate(FPoseContext& Output)
{
	FCompactPose& Pose = Output.Pose;
	if (!Pose.IsValid() || Pose.GetNumBones() == 0)
	{
		return true;
	}
	const FBoneContainer& Bones = Pose.GetBoneContainer();
	const FReferenceSkeleton& Ref = Bones.GetReferenceSkeleton();

	// +1 цикл за кадр (фаза общая для ног/рук/таза).
	const float Gait = FMath::Clamp(
		(Speed2D - WalkSpeedCmS) / FMath::Max(1.0f, RunSpeedCmS - WalkSpeedCmS), 0.0f, 1.0f);
	const float Cycle = FMath::Lerp(WalkCycleTime, RunCycleTime, Gait);
	const float Dt = FMath::Max(0.001f, LastDelta);
	PropAnimTime += Dt / Cycle;
	if (bInspecting)
	{
		InspectPhase += Dt * InspectLookFreq;
	}
	const float PhaseS = FMath::Fmod(PropAnimTime, 1.0f);
	const float S = PhaseS * 2.0f * PI;
	const float Sp = SwingPolarity;

	const float SinS = FMath::Sin(S);
	const float CosS = FMath::Cos(S);
	const float BodyBob = (1.0f - FMath::Cos(2.0f * S)) * 0.5f;
	const float Breath = FMath::Sin(PropAnimTime * 0.6f * 2.0f * PI);

	// --- Режим сидения: таймеры и бленды ---
	if (bSitting)
	{
		SitPhase += Dt * SitSwayFreq;
	}
	const float SitStart = bSitting ? 1.0f : 0.0f;
	if (bSitting)
	{
		SitBlend = FMath::Min(1.0f, SitBlend + SitBlendSpeed * Dt);
	}
	else
	{
		SitBlend = FMath::Max(0.0f, SitBlend - SitBlendSpeed * Dt);
	}
	const float CrossTarget = (bSitting && SitBlend >= 0.6f) ? 1.0f : 0.0f;
	CrossLegBlend = FMath::Clamp(
		CrossLegBlend + FMath::Sign(CrossTarget - CrossLegBlend)
			* FMath::Min(FMath::Abs(CrossTarget - CrossLegBlend), CrossLegBlendSpeed * Dt),
		0.0f, 1.0f);

	// Сидячие «медленные» знаки: покачивание корпуса и головы.
	const float SW = SitSwayAmpDeg * 0.5f * FMath::Sin(SitPhase);
	const float SitBreath = 0.5f * (bSitting ? Breath : 0.0f);
	const float C = CrossLegBlend;

	const float SwingAmp = LegSwingAmp * (1.0f + 0.35f * Gait);
	const float ArmAmp = ArmSwingAmp * (1.0f + 0.4f * Gait);
	const float CalfBend = KneeBendDeg + 6.0f * (0.5f - 0.5f * FMath::Cos(2.0f * S));
	const float Bounce = FMath::Sin(2.0f * S);

	float HeadYaw = 0.0f;
	float HeadPitch = 0.0f;
	if (bInspecting)
	{
		const float IP = InspectPhase * 2.0f * PI;
		HeadYaw = FMath::Sin(IP) * InspectLookAmpYaw;
		HeadPitch = FMath::Sin(IP * 0.7f + 1.0f) * 18.0f;
	}
	else
	{
		HeadYaw = FMath::Sin(PropAnimTime * 0.35f) * 6.0f;
	}

	const FCompactPoseBoneIndex NumPoseBones(Pose.GetNumBones());

	// Один проход по всем костям: матчим префикс и накидываем дельту на рест-позу.
	for (int32 i = 0; i < NumPoseBones.GetInt(); ++i)
	{
		const FCompactPoseBoneIndex CIdx(i);
		const FSkeletonPoseBoneIndex SIdx = Bones.GetSkeletonPoseIndexFromCompactPoseIndex(CIdx);
		const FName N = Ref.GetBoneName(SIdx.GetInt());
		const FString NS = N.ToString();
		float R = 0.0f, P = 0.0f, Y = 0.0f;
		FVector Loc = FVector::ZeroVector;

		if (NS.StartsWith(TEXT("Thigh_R_")))
		{
			P = Sp * SwingAmp * SinS;
		}
		else if (NS.StartsWith(TEXT("Thigh_L_")))
		{
			P = -Sp * SwingAmp * SinS;
		}
		else if (NS.StartsWith(TEXT("Calf_R_")))
		{
			P = CalfBend * (0.35f + 0.25f * CosS);
		}
		else if (NS.StartsWith(TEXT("Calf_L_")))
		{
			P = CalfBend * (0.35f - 0.25f * CosS);
		}
		else if (NS.StartsWith(TEXT("Foot_R_")))
		{
			P = -Sp * SwingAmp * 0.4f * SinS;
		}
		else if (NS.StartsWith(TEXT("Foot_L_")))
		{
			P = Sp * SwingAmp * 0.4f * SinS;
		}
		else if (NS.StartsWith(TEXT("Toe_R_")))
		{
			P = -Sp * SwingAmp * 0.3f * SinS;
		}
		else if (NS.StartsWith(TEXT("Toe_L_")))
		{
			P = Sp * SwingAmp * 0.3f * SinS;
		}
		else if (NS.StartsWith(TEXT("Root_01")))
		{
			// Таз: лёгкий наклон в бег + боковое покачивание + подъём на шаге.
			P = -Gait * 4.0f;
			Y = HipRollAmp * (1.0f - 0.4f * Gait) * SinS;
			Loc = FVector(0.0f, HipSwayAmp * SinS, -BodyBob * BodyBobAmp);
		}
		else if (NS.StartsWith(TEXT("Spine_02")))
		{
			R = -HipRollAmp * SinS * 0.35f;
		}
		else if (NS.StartsWith(TEXT("Chest_03")))
		{
			P = -Gait * RunForwardLeanDeg + Breath * 1.5f;
		}
		else if (NS.StartsWith(TEXT("Neck_00")))
		{
			P = HeadPitch * 0.4f;
			Y = HeadYaw * 0.4f;
		}
		else if (NS.StartsWith(TEXT("Head_04")))
		{
			P = HeadPitch * 0.6f;
			Y = HeadYaw * 0.6f;
		}
		else if (NS.StartsWith(TEXT("Shoulder_R_")))
		{
			P = -Sp * ArmAmp * SinS; // против-фаза к ногам
		}
		else if (NS.StartsWith(TEXT("Shoulder_L_")))
		{
			P = Sp * ArmAmp * SinS;
		}
		else if (NS.StartsWith(TEXT("Upperarm_R_")))
		{
			P = Sp * ArmAmp * 0.25f * SinS;
		}
		else if (NS.StartsWith(TEXT("Upperarm_L_")))
		{
			P = -Sp * ArmAmp * 0.25f * SinS;
		}
		else if (NS.StartsWith(TEXT("Forearm_R_")))
		{
			R = ElbowBendDeg + 4.0f;
		}
		else if (NS.StartsWith(TEXT("Forearm_L_")))
		{
			R = ElbowBendDeg + 4.0f;
		}
		else if (NS.StartsWith(TEXT("Hand_R_")))
		{
			P = -Sp * ArmAmp * 0.15f * SinS;
		}
		else if (NS.StartsWith(TEXT("Hand_L_")))
		{
			P = Sp * ArmAmp * 0.15f * SinS;
		}
		else if (NS.StartsWith(TEXT("Breast_R_")) || NS.StartsWith(TEXT("Breast_L_")))
		{
			P = Bounce * 3.0f * (0.5f + 0.5f * Gait);
			Loc = FVector(0.0f, 0.0f, -BreastBounceAmp * (0.5f + 0.5f * FMath::Max(0.0f, Bounce)));
		}
		else if (NS.StartsWith(TEXT("Belly1")))
		{
			P = -Breath;
		}
		else if (NS.StartsWith(TEXT("Butt_R_")) || NS.StartsWith(TEXT("Butt_L_")))
		{
			P = -Bounce * 1.8f;
		}
		else if (NS.StartsWith(TEXT("HairRoot")))
		{
			Y = HairSwayAmp * (0.3f + 1.2f * Gait) * SinS;
		}
		else
		{
			continue;
		}

		// --- Сидячая поза: значения «сидя» для этой кости ---
		float sR = 0.0f, sP = 0.0f, sY = 0.0f;
		FVector sLoc = FVector::ZeroVector;
		if (NS.StartsWith(TEXT("Thigh_R_")))
		{
			sP = FMath::Lerp(86.0f, 84.0f, C);
			sY = FMath::Lerp(-4.0f, 22.0f, C);  // поворот внутрь / в скрест
			sR = FMath::Lerp(-2.0f, 8.0f, C);
		}
		else if (NS.StartsWith(TEXT("Thigh_L_")))
		{
			sP = FMath::Lerp(86.0f, 80.0f, C);
			sY = FMath::Lerp(4.0f, -6.0f, C);
			sR = FMath::Lerp(-2.0f, -6.0f, C);
		}
		else if (NS.StartsWith(TEXT("Calf_R_")))
		{
			sP = FMath::Lerp(86.0f, 80.0f, C); // лодыжка на левое колено
		}
		else if (NS.StartsWith(TEXT("Calf_L_")))
		{
			sP = FMath::Lerp(86.0f, 76.0f, C);
		}
		else if (NS.StartsWith(TEXT("Foot_R_")))
		{
			sP = FMath::Lerp(-70.0f, -74.0f, C);
			sY = FMath::Lerp(0.0f, 10.0f, C);
		}
		else if (NS.StartsWith(TEXT("Foot_L_")))
		{
			sP = FMath::Lerp(-70.0f, -66.0f, C);
			sY = FMath::Lerp(0.0f, -4.0f, C);
		}
		else if (NS.StartsWith(TEXT("Toe_R_")))
		{
			sP = FMath::Lerp(-6.0f, -8.0f, C);
		}
		else if (NS.StartsWith(TEXT("Toe_L_")))
		{
			sP = -6.0f;
		}
		else if (NS.StartsWith(TEXT("Root_01")))
		{
			// Таз опускается на сиденье, лёгкий наклон + перекат.
			sP = 2.0f;
			sY = SitPelvisRockDeg * FMath::Sin(SitPhase);
			sLoc = FVector(0.0f, 0.0f, -SitDropCm);
		}
		else if (NS.StartsWith(TEXT("Spine_02")))
		{
			sP = 3.0f; // чуть округлая спина
		}
		else if (NS.StartsWith(TEXT("Chest_03")))
		{
			sP = 1.0f + SitBreath;
			sR = SW * 0.6f;
		}
		else if (NS.StartsWith(TEXT("Neck_00")))
		{
			sP = SitHeadLookPitchDeg * 0.4f;
			sY = SW * 0.4f;
		}
		else if (NS.StartsWith(TEXT("Head_04")))
		{
			sP = SitHeadLookPitchDeg * 0.6f;
			sY = SW * 0.6f;
		}
		else if (NS.StartsWith(TEXT("Shoulder_R_")) || NS.StartsWith(TEXT("Shoulder_L_")))
		{
			sP = -6.0f; // руки на бёдра
		}
		else if (NS.StartsWith(TEXT("Upperarm_R_")) || NS.StartsWith(TEXT("Upperarm_L_")))
		{
			sP = 12.0f;
		}
		else if (NS.StartsWith(TEXT("Forearm_R_")) || NS.StartsWith(TEXT("Forearm_L_")))
		{
			sR = 60.0f; // локти согнуты, кисти на бёдрах
		}
		else if (NS.StartsWith(TEXT("Hand_R_")) || NS.StartsWith(TEXT("Hand_L_")))
		{
			sR = -10.0f;
		}
		else if (NS.StartsWith(TEXT("Breast_R_")) || NS.StartsWith(TEXT("Breast_L_")))
		{
			sP = -6.0f;
			sLoc = FVector(0.0f, 0.0f, -FMath::Clamp(SitDropCm * 0.3f, 0.0f, 12.0f));
		}
		else if (NS.StartsWith(TEXT("Belly1")))
		{
			sP = -2.0f + SitBreath * 0.3f;
		}
		else if (NS.StartsWith(TEXT("Butt_R_")) || NS.StartsWith(TEXT("Butt_L_")))
		{
			sP = -3.0f;
		}
		else if (NS.StartsWith(TEXT("HairRoot")))
		{
			sY = SW * 0.8f;
		}

		// Смешиваем ходовую и сидячую позу.
		const float M = SitBlend;
		R = FMath::Lerp(R, sR, M);
		P = FMath::Lerp(P, sP, M);
		Y = FMath::Lerp(Y, sY, M);
		Loc = FMath::Lerp(Loc, sLoc, M);

		FTransform& T = Pose[CIdx];
		if (R != 0.0f || P != 0.0f || Y != 0.0f)
		{
			T.SetRotation(T.GetRotation() * FRotator(P, Y, R).Quaternion());
		}
		if (!Loc.IsZero())
		{
			T.AddToTranslation(Loc);
		}
	}

	return true; // график/ноды не оцениваются — поза уже готова.
}
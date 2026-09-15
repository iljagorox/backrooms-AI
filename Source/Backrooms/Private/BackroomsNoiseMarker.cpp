#include "BackroomsNoiseMarker.h"
#include "Components/WidgetComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SNullWidget.h"
#include "Styling/CoreStyle.h"
#include "Modules/ModuleManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

namespace
{
	constexpr float GIconSize = 96.0f;
	constexpr float GMaxAlpha = 0.55f;
	constexpr float GFadeInTime = 0.4f;
	constexpr float GFadeOutTime = 1.4f;
	constexpr float GFadeOutQuick = 0.45f;
}

ABackroomsNoiseMarker::ABackroomsNoiseMarker()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.016f;

	NoiseWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NoiseWidget"));
	RootComponent = NoiseWidget;
	NoiseWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NoiseWidget->SetCastShadow(false);
	// Биллборд: компонент виджета разворачивается к камере сам (Plane сохраняет
	// мировую ось экрана), но мы дополнительно крутим его на камеру в Tick,
	// чтобы иконка всегда «смотрела» на игрока и была читаемой.
	NoiseWidget->SetGeometryMode(EWidgetGeometryMode::Plane);
	NoiseWidget->SetDrawSize(FVector2D(GIconSize, GIconSize));

	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(NoiseWidget);
	// Приглушённое холодное свечение — едва заметный намёк на источник шума.
	GlowLight->SetLightColor(FLinearColor(0.85f, 0.90f, 1.00f));
	GlowLight->SetIntensity(620.0f);
	GlowLight->SetAttenuationRadius(430.0f);
	GlowLight->SetCastShadows(false);
}

TSharedPtr<FSlateDynamicImageBrush> ABackroomsNoiseMarker::LoadIconBrush()
{
	const FString Path = FPaths::ProjectContentDir() / TEXT("UI/StatusIcons") / TEXT("17_noise.png");

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *Path))
	{
		UE_LOG(LogTemp, Warning, TEXT("BackroomsNoiseMarker: icon PNG not found: %s"), *Path);
		return nullptr;
	}

	IImageWrapperModule& Module = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	TSharedPtr<IImageWrapper> Wrapper = Module.CreateImageWrapper(EImageFormat::PNG);
	if (!Wrapper.IsValid() || !Wrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Warning, TEXT("BackroomsNoiseMarker: failed to decode PNG: %s"), *Path);
		return nullptr;
	}

	const int32 Width = Wrapper->GetWidth();
	const int32 Height = Wrapper->GetHeight();
	TArray64<uint8> Raw;
	if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, Raw) || Width <= 0 || Height <= 0)
	{
		return nullptr;
	}

	TArray<uint8> Bgra;
	Bgra.Append(Raw.GetData(), (int32)Raw.Num());
	return FSlateDynamicImageBrush::CreateWithImageData(FName(TEXT("BackroomsNoiseMarker_Icon")), FVector2D(Width, Height), Bgra);
}

void ABackroomsNoiseMarker::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();

	if (NoiseWidget)
	{
		IconBrush = LoadIconBrush();
		const FSlateBrush* BrushPtr = IconBrush.IsValid() ? static_cast<const FSlateBrush*>(IconBrush.Get())
			: FCoreStyle::Get().GetBrush(TEXT("GenericWhiteBox"));

		SAssignNew(IconImage, SImage)
			.Image(BrushPtr)
			.DesiredSizeOverride(FVector2D(GIconSize, GIconSize))
			.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));

		// Тонкая подложка, чтобы иконка едва читалась и на светлом фоне;
		// без яркой рамки — маркер должен сливаться с миром, а не выделяться.
		TSharedRef<SDPIScaler> Container = SNew(SDPIScaler)
			.DPIScale(1.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.10f))
				.Padding(FMargin(4.0f))
				[
					IconImage.ToSharedRef()
				]
			];

		NoiseWidget->SetSlateWidget(Container);
	}
}

void ABackroomsNoiseMarker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Age += DeltaSeconds;

	// Жизнь маркера привязана к звуку: как только привязанный звук закончился
	// либо игрок подошёл к источнику — начинаем быстро гасить. Страховочный
	// Lifetime срабатывает только когда звук не привязан.
	const bool bSoundDone = Age > 0.25f && (!BoundSound.IsValid() || !BoundSound->IsPlaying());
	const bool bPlayerClose = IsPlayerClose();
	const bool bLifetimeOut = !BoundSound.IsValid() && Age >= Lifetime - GFadeOutTime;
	if (!bFading && (bSoundDone || bPlayerClose || bLifetimeOut))
	{
		bFading = true;
		FadeBeginAge = Age;
		FadeDuration = (bSoundDone || bPlayerClose) ? GFadeOutQuick : GFadeOutTime;
	}

	// Альфа по фазе: медленное появление до полупрозрачного уровня, далее
	// плавное (или резкое при приближении) затухание.
	float Alpha = GMaxAlpha;
	if (bFading)
	{
		Alpha = GMaxAlpha * (1.0f - FMath::Clamp((Age - FadeBeginAge) / FadeDuration, 0.0f, 1.0f));
	}
	else if (Age < GFadeInTime)
	{
		Alpha = GMaxAlpha * FMath::Clamp(Age / GFadeInTime, 0.0f, 1.0f);
	}

	if (Alpha <= 0.0f)
	{
		SetActorLocation(BaseLocation);
		Destroy();
		return;
	}

	if (IconImage.IsValid())
	{
		IconImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, Alpha));
	}

	// Лёгкое «дыхание» вверх-вниз, чтобы маркер был заметнее.
	const float Bob = FMath::Sin(Age * 1.6f) * 8.0f;
	SetActorLocation(BaseLocation + FVector(0.0f, 0.0f, Bob));

	// Биллборд: поворачиваемся к камере игрока.
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		FVector CamLoc = BaseLocation;
		if (PC->PlayerCameraManager)
		{
			CamLoc = PC->PlayerCameraManager->GetCameraLocation();
		}
		else if (PC->GetPawn())
		{
			CamLoc = PC->GetPawn()->GetActorLocation();
		}
		const FVector ToCam = CamLoc - GetActorLocation();
		if (ToCam.SizeSquared() > 1.0f)
		{
			SetActorRotation(ToCam.Rotation());
		}
	}
}

bool ABackroomsNoiseMarker::IsPlayerClose() const
{
	if (ProximityRadius <= 0.0f)
	{
		return false;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	FVector PlayerLoc = FVector::ZeroVector;
	if (PC && PC->GetPawn())
	{
		PlayerLoc = PC->GetPawn()->GetActorLocation();
	}
	else if (PC && PC->PlayerCameraManager)
	{
		PlayerLoc = PC->PlayerCameraManager->GetCameraLocation();
	}

	// Маркер висит над источником на ~90 см выше; сравниваем только по горизонтали.
	FVector Delta = GetActorLocation() - PlayerLoc;
	Delta.Z = 0.0f;
	return Delta.SizeSquared() < ProximityRadius * ProximityRadius;
}

void ABackroomsNoiseMarker::BindSound(UAudioComponent* InComp)
{
	BoundSound = InComp;
}

void ABackroomsNoiseMarker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Освобождаем Slate-виджет до того, как умрёт кисть (SImage держит только
	// сырой указатель на кисть).
	if (NoiseWidget && IconImage.IsValid())
	{
		NoiseWidget->SetSlateWidget(SNullWidget::NullWidget);
	}
	IconImage.Reset();
	IconBrush.Reset();
	Super::EndPlay(EndPlayReason);
}

ABackroomsNoiseMarker* ABackroomsNoiseMarker::SpawnAt(UWorld* World, const FVector& Location, float InLifetime)
{
	if (!World)
	{
		return nullptr;
	}

	// Небольшой подъём маркера над полом, чтобы его было видно из-за
	// ящиков/мебели, но почти вплотную к источнику.
	FVector SpawnLoc = Location + FVector(0.0f, 0.0f, 90.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ABackroomsNoiseMarker* Marker = World->SpawnActor<ABackroomsNoiseMarker>(
		ABackroomsNoiseMarker::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
	if (Marker)
	{
		Marker->Lifetime = FMath::Max(0.5f, InLifetime);
	}
	return Marker;
}
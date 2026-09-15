#include "BackroomsGameMode.h"
#include "BackroomsPlayerCharacter.h"
#include "BackroomsMainMenuWidget.h"
#include "Kismet/GameplayStatics.h"

ABackroomsGameMode::ABackroomsGameMode()
{
	// Свой игрок (ACharacter с камерой и телом SM_Character под ней)
	// вместо шаблонного FirstPerson.
	DefaultPawnClass = ABackroomsPlayerCharacter::StaticClass();
}

void ABackroomsGameMode::BeginPlay()
{
	Super::BeginPlay();

	// skipmenu (переход между картами, OpenLevel "seed=N&skipmenu"): меню не
	// нужно — игрок попадает в уровень сразу, с авто-стартом HUD (см.
	// ABackroomsPlayerCharacter::BeginPlay).
	if (UWorld* World = GetWorld())
	{
		if (World->URL.HasOption(TEXT("skipmenu")))
		{
			return;
		}
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		MainMenuWidget = CreateWidget<UBackroomsMainMenuWidget>(PC, UBackroomsMainMenuWidget::StaticClass());
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport(1000);
			MainMenuWidget->ShowMenu(true);
		}
	}
}

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BackroomsGameMode.generated.h"

UCLASS()
class BACKROOMS_API ABackroomsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABackroomsGameMode();
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<class UBackroomsMainMenuWidget> MainMenuWidget;
};

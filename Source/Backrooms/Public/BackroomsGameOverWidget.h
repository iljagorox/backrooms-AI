#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "BackroomsGameOverWidget.generated.h"

class SButton;
class STextBlock;
class SOverlay;

// Экран смерти: затемнение, надпись и кнопки «Заново» / «Выход». Показывается
// игроком в Die(). Рестарт — тот же уровень (и тот же seed генерации из
// сохранения), чтобы смерть не «перекатывала» мир.
UCLASS()
class BACKROOMS_API UBackroomsGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void ShowGameOver();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
	FReply OnRestartClicked();
	FReply OnQuitClicked();

	TSharedPtr<STextBlock> TitleText;
};

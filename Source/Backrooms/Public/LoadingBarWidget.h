#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "LoadingBarWidget.generated.h"

UCLASS()
class BACKROOMS_API ULoadingBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetProgress(float Percent);
	void SetLabel(const FString& Text);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	TSharedPtr<SProgressBar> ProgressBar;
	TSharedPtr<STextBlock> LabelText;
	TSharedPtr<SVerticalBox> Box;

	// Целевой прогресс, который задаёт внешний код, и сглаженное значение,
	// которое показывается игроку (NativeTick подводит одно к другому).
	float CurrentPercent = 0.0f;
	float DisplayPercent = 0.0f;
	FString CurrentLabel = TEXT("");
};
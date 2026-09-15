#include "BackroomsGameOverWidget.h"
#include "BackroomsLocalization.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Styling/CoreStyle.h"

void UBackroomsGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Collapsed);
}

TSharedRef<SWidget> UBackroomsGameOverWidget::RebuildWidget()
{
	// Полное затемнение с текстом по центру.
	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.82f))
			[
				SNew(SSpacer)
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(10.0f)
			[
				SAssignNew(TitleText, STextBlock)
				.Text(BackroomsLoc::Text(TEXT("GameOver.Title")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
				.ColorAndOpacity(FLinearColor(0.80f, 0.12f, 0.10f))
				.ShadowOffset(FVector2D(2.0f, 2.0f))
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(10.0f)
			[
				SNew(STextBlock)
				.Text(BackroomsLoc::Text(TEXT("GameOver.Subtitle")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
				.ColorAndOpacity(FLinearColor(0.75f, 0.72f, 0.62f))
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(20.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { return OnRestartClicked(); })
					[
						SNew(STextBlock).Text(BackroomsLoc::Text(TEXT("GameOver.Restart")))
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { return OnQuitClicked(); })
					[
						SNew(STextBlock).Text(BackroomsLoc::Text(TEXT("GameOver.Quit")))
					]
				]
			]
		];
}

void UBackroomsGameOverWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	TitleText.Reset();
}

void UBackroomsGameOverWidget::ShowGameOver()
{
	SetVisibility(ESlateVisibility::Visible);

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->bShowMouseCursor = true;
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
	}
}

FReply UBackroomsGameOverWidget::OnRestartClicked()
{
	// Рестарт того же уровня: seed восстанавливается из Save.dat генератором,
	// поэтому мир остаётся тем же (смерть не «перекатывает» подземелье).
	FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*CurrentLevel));
	return FReply::Handled();
}

FReply UBackroomsGameOverWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
	return FReply::Handled();
}

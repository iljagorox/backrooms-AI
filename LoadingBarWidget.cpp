#include "LoadingBarWidget.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SBoxPanel.h"

TSharedRef<SWidget> ULoadingBarWidget::RebuildWidget()
{
	// Slate-дерево собираем только при первом запросе: последующие вызовы
	// RebuildWidget (invalidate/re-take) возвращают уже готовый виджет, а
	// обновления идут через SetProgress/SetLabel без пересоздания.
	if (Box.IsValid())
	{
		return Box.ToSharedRef();
	}

	Box = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(FMargin(40.0f, 20.0f, 40.0f, 60.0f))
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Bottom)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SAssignNew(LabelText, STextBlock)
				.Text(FText::FromString(CurrentLabel))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
				.ShadowOffset(FVector2D(2.0f, 2.0f))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
			[
				SAssignNew(ProgressBar, SProgressBar)
				.Percent(CurrentPercent)
				.FillColorAndOpacity(FLinearColor(0.8f, 0.7f, 0.3f, 1.0f))
			]
		];

	if (LabelText.IsValid())
	{
		LabelText->SetText(FText::FromString(CurrentLabel));
	}
	if (ProgressBar.IsValid())
	{
		ProgressBar->SetPercent(CurrentPercent);
	}

	return Box.ToSharedRef();
}

void ULoadingBarWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	ProgressBar.Reset();
	LabelText.Reset();
	Box.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULoadingBarWidget::SetProgress(float Percent)
{
	// Цель обновляем мгновенно, а полосу к ней подводит NativeTick, поэтому
	// прогресс (часто скачущий по чанкам) не дёргается, а плавно заполняется.
	CurrentPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
}

void ULoadingBarWidget::SetLabel(const FString& Text)
{
	CurrentLabel = Text;
	if (LabelText.IsValid())
	{
		LabelText->SetText(FText::FromString(CurrentLabel));
	}
}

void ULoadingBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!ProgressBar.IsValid())
	{
		return;
	}

	if (FMath::IsNearlyEqual(DisplayPercent, CurrentPercent, 1e-3f))
	{
		DisplayPercent = CurrentPercent;
		return;
	}

	// ~8 обновлений в секунду: полоса заполняется за ~0.5 с, даже если чанки
	// готовятся рывками (фоновые фризы синхронной загрузки не видны игроку).
	DisplayPercent = FMath::FInterpTo(DisplayPercent, CurrentPercent, InDeltaTime, 8.0f);
	ProgressBar->SetPercent(DisplayPercent);
}
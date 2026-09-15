#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackroomsStatusComponent.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "BackroomsHUDWidget.generated.h"

class ABackroomsPlayerCharacter;
class SOverlay;
class SVerticalBox;
class SBorder;

// Приборная шкала HUD. Это НЕ полоса загрузки: значение разбито на дискретные
// ячейки-сегменты, которые загораются по одной. Так индикатор читается как
// «устройство» (уровень заряда/здоровья), а не как прогресс-бар.
struct FHudMeter
{
	TArray<TSharedPtr<SImage>> Cells;
	// Цвет зажжённого сегмента; погашенный всегда тёмный.
	FLinearColor OnColor = FLinearColor::White;
	// Последняя применённая доля — чтобы не трогать Slate каждый тик.
	float LastFrac = -1.0f;
};

// Внутриигровой HUD: состояния (иконки), выживальческие шкалы и хотбар.
// В городе (на стартовой платформе) выживальческие панели скрыты: голод/жажда/
// рассудок обретают смысл только в Бэкрумсе.
UCLASS()
class BACKROOMS_API UBackroomsHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Привязать игрока, чьи состояния показываем.
	void SetPlayer(ABackroomsPlayerCharacter* InPlayer);

	// Обновить шкалы и иконки (вызывается из Tick игрока).
	void Refresh();

	// Всплывающее уведомление (открытый перк, уровень, очки лидерборда).
	void ShowToast(const FString& Message);

	// XP-полоса прогрессии: скрыта по умолчанию, всплывает на пару секунд
	// при начислении XP / новом уровне.
	TSharedPtr<class STextBlock> XPText;
	TSharedPtr<class SBorder> XPBarPanel;
	TSharedPtr<class SBorder> ToastPanel;
	TSharedPtr<class STextBlock> ToastText;
	float ToastUntil = 0.0f;

	// Прогрессия: следим за XP/уровнем, чтобы всплывать только на изменениях.
	int32 LastSeenLevel = 0;
	int32 LastSeenXP = -1;
	float XPRevealUntil = 0.0f;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
	TWeakObjectPtr<ABackroomsPlayerCharacter> Player;

	// Панели, которые прячутся в городе.
	TSharedPtr<SVerticalBox> SurvivalBarsBox; // рассудок/фонарик/голод/жажда
	TSharedPtr<SVerticalBox> StaminaBox;
	TSharedPtr<class SBorder> HealthPanel;

	FHudMeter SanityMeter;
	FHudMeter FlashlightMeter;
	FHudMeter HealthMeter;
	FHudMeter HungerMeter;
	FHudMeter ThirstMeter;
	FHudMeter StaminaMeter;

	TSharedPtr<STextBlock> SanityText;
	TSharedPtr<STextBlock> FlashlightText;
	TSharedPtr<STextBlock> StaminaText;
	TSharedPtr<STextBlock> HealthText;

	// Хотбар: 4 квадратных слота. Каждый слот — бордер (подсветка выбора),
	// иконка предмета (runtime-рендер меша) + номер/количество.
	struct FHotbarSlot
	{
		TSharedPtr<class SBorder> Border;
		TSharedPtr<class SImage> Icon;
		TSharedPtr<STextBlock> Text;
		TSharedPtr<STextBlock> CountText;
	};
	TArray<FHotbarSlot> HotbarSlots;

	// Прогресс удержания кнопки использования (UseTime): по центру над прицелом.
	// Виден, только пока игрок удерживает клавишу и предмет не «доглотнулся».
	TSharedPtr<class SBorder> UseProgressPanel;
	TSharedPtr<class SProgressBar> UseProgressBar;
	TSharedPtr<STextBlock> UseProgressName;
	void UpdateUseProgress(ABackroomsPlayerCharacter* P);

	// Осмотр предмета (спека §3): имя + описание из UItemDataAsset под центром.
	TSharedPtr<class SBorder> InspectInfoPanel;
	TSharedPtr<STextBlock> InspectNameText;
	TSharedPtr<STextBlock> InspectDescText;
	void UpdateInspectInfo(ABackroomsPlayerCharacter* P);

	// Время последнего Refresh — нужен delta для движения очереди рендера иконок.
	float LastRefreshTime = 0.0f;

	// --- Контекстные подсказки-вызовы ---
	// Не список «кнопок», а редкие сообщения по фактическому состоянию игрока.
	// Показывается ровно одна подсказка; приоритет решает, какая из активных
	// важнее, а кулдаун не даёт одной и той же фразе спамить. Подсказки живут
	// только в Бэкрумсе (в городе игроку ничего не угрожает).
	TSharedPtr<SBorder> HintPanel;
	TSharedPtr<STextBlock> HintText;
	FName HintActiveId;
	int32 HintActivePriority = 0;
	float HintShownUntil = 0.0f;      // мир. время, до которого держим текущую
	float HintFadeInAt = 0.0f;        // мир. время начала показа (для плавного появления)
	TMap<FName, float> HintCooldownUntil;

	// Выбрать и обновить подсказку по текущему миру.
	void UpdateHint(ABackroomsPlayerCharacter* P);

	// Подсказка взаимодействия: что можно сделать с предметом под прицелом и
	// какой клавишей. Обновляется с небольшим троттлингом (трассировка).
	TSharedPtr<SBorder> InteractPromptPanel;
	TSharedPtr<STextBlock> InteractPromptText;
	float InteractPromptTimer = 0.0f;
	FString LastInteractPrompt;
	void UpdateInteractPrompt(ABackroomsPlayerCharacter* P, float DeltaTime);

	TSharedPtr<STextBlock> FpsText;
	TArray<float> FpsHistory;
	float FpsAvg = 0.0f;
	float FpsOnePctLow = 0.0f;
	void UpdateFpsGraph(float DeltaTime);

	// Иконки состояний в порядке массива StatusOrder.
	TArray<TSharedPtr<SImage>> StatusIcons;
	// Кисти нужно держать живыми: SImage хранит только сырой указатель.
	TArray<TSharedPtr<FSlateDynamicImageBrush>> IconBrushes;
	// Заглушка, если PNG не декодировался.
	TSharedPtr<FSlateColorBrush> PlaceholderBrush;
	// Погашенный сегмент приборной шкалы (общая кисть для всех ячеек).
	TSharedPtr<FSlateColorBrush> CellBrush;

	// Обновить ячейки одной шкалы по доле 0..1.
	static void RefreshMeter(FHudMeter& Meter, float Frac);

	// Загрузить иконку из Content/UI/StatusIcons как runtime-кисть (BGRA).
	TSharedPtr<FSlateDynamicImageBrush> LoadIconBrush(const FString& FileName, const FName& ResourceName);
};

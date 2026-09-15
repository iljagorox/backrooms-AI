#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackroomsInventoryWidget.generated.h"

class ABackroomsPlayerCharacter;
class UStaticMesh;
class UStaticMeshComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class AActor;
class SImage;
class STextBlock;
class SHorizontalBox;
class SVerticalBox;
class SOverlay;
struct FSlateBrush;

// Инвентарь-бестиарий: сетка предметов слева/сверху, выбранный предмет можно
// «потыкать» — справа вращается его 3D-модель, слева описание и эффект.
// Всё на чистом C++ Slate, как и остальные меню проекта.
//
// 3D-превью делается через SceneCapture2D: вне уровня спавнится скрытый актёр
// с копией меша, светом и камерой, кадр идёт в RenderTarget, который показывается
// SImage-ом. Это даёт настоящую вращающуюся модель без правок вьюпорта.
UCLASS()
class BACKROOMS_API UBackroomsInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ToggleInventory();
	void ShowInventory(bool bShow);
	bool IsInventoryOpen() const { return bIsOpen; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void RebuildUI();
	void SelectItem(int32 Index);

	// Подготовить/обновить 3D-превью выбранного предмета.
	void EnsurePreviewSetup();
	void SetPreviewMesh(UStaticMesh* Mesh);

	// Локализованные имя/описание предмета по его Id.
	FString ItemName(const struct FBackroomsItemDef& Def) const;
	FString ItemDesc(const struct FBackroomsItemDef& Def) const;

	ABackroomsPlayerCharacter* GetPlayer() const;

	TWeakObjectPtr<ABackroomsPlayerCharacter> Player;
	bool bIsOpen = false;
	int32 SelectedIndex = 0;

	// Кэш каталога предметов (не пересобираем каждый кадр).
	TArray<struct FBackroomsItemDef> Catalog;

	TSharedPtr<SOverlay> MenuRoot;
	TSharedPtr<SHorizontalBox> GridBox;
	TSharedPtr<STextBlock> DetailName;
	TSharedPtr<STextBlock> DetailDesc;
	TSharedPtr<STextBlock> DetailStats;
	TSharedPtr<SImage> PreviewImage;
	TSharedPtr<FSlateBrush> PreviewBrush;

	UPROPERTY(Transient) TObjectPtr<UTextureRenderTarget2D> PreviewRT;
	UPROPERTY(Transient) TObjectPtr<AActor> PreviewActor;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> PreviewMeshComp;
	UPROPERTY(Transient) TObjectPtr<USceneCaptureComponent2D> SceneCapture;
	float PreviewYaw = 0.0f;
};

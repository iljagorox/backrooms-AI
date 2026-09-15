#pragma once

#include "CoreMinimal.h"

class UWorld;
struct FSlateBrush;

// Runtime-иконки предметов: рендер WorldMesh в маленький RenderTarget через
// SceneCapture2D (как 3D-превью инвентаря). В контенте нет готовых иконок —
// предметы рисуются из собственных мешей, поэтому иконка всегда честная и
// не расходится с каталогом. Общий кэш для инвентаря и хотбара.
namespace BackroomsItemIcons
{
	// Кисть иконки предмета или nullptr (ещё не отрендерена / нет меша).
	// Инициализирует рендерер под World при первом вызове.
	BACKROOMS_API TSharedPtr<FSlateBrush> GetItemIcon(class UWorld* World, FName ItemId);

	// Прогнать очередь рендера. Вызывайте каждый тик из виджета/HUD.
	BACKROOMS_API void TickRender(class UWorld* World, float DeltaTime);

	// Отчистить рендерер (убить актёра-капчер и все кэши) — при смене мира/тира.
	BACKROOMS_API void DestroyForWorld(class UWorld* World);
}
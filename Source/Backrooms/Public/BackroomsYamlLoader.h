#pragma once

#include "CoreMinimal.h"

class UBackroomsLevelConfig;

// Минимальный YAML-подмножеств (ключ: значение / карты / inline-списки / комментарии).
// Сетевое состояние для рекурсивного разбора.

struct FBackroomsYamlValue
{
	enum class EKind : uint8 { Scalar, List, Map };

	EKind Kind = EKind::Scalar;
	FString Scalar;
	TArray<TSharedRef<FBackroomsYamlValue>> List;
	TMap<FString, TSharedRef<FBackroomsYamlValue>> Map;
};

namespace BackroomsYaml
{
	// Разобрать YAML-файл. OutError пуст при успехе.
	TSharedPtr<FBackroomsYamlValue> ParseFile(const FString& FilePath, FString& OutError);

	// Загрузить и собрать UBackroomsLevelConfig по ID уровня ("L00"…"L16").
	// Ищет файл Content/Config/Levels/LXX_*.yaml (только режим разработки).
	UBackroomsLevelConfig* LoadLevelConfig(const FString& LevelId);
}
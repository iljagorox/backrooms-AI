#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BackroomsDevTool.generated.h"

class ABackroomsWorldGenerator;

// Инструмент разработчика: позволяет из консоли/кода управлять генерацией и
// получать текстовый отчёт о том, что «настроил» проект по каждому уровню.
UCLASS()
class BACKROOMS_API ABackroomsDevTool : public AActor
{
	GENERATED_BODY()

public:
	ABackroomsDevTool();

	// Куда писать текстовые отчёты о генерации.
	UPROPERTY(EditAnywhere, Category = "Dev")
	FString ReportDir = TEXT("C:/Users/iljag/AppData/Local/Temp/opencode");

	// Сбросить генератор (найти его в мире).
	UFUNCTION(BlueprintCallable, Category = "Dev")
	ABackroomsWorldGenerator* ResolveGenerator() const;

	// Записать отчёт о текущем уровне в файл (проект «рассказывает», что настроил).
	UFUNCTION(BlueprintCallable, Category = "Dev")
	void DumpReportToFile(const FString& FullPath = TEXT(""));

	// Переключить генератор на другой уровень (регенерация мира).
	UFUNCTION(BlueprintCallable, Category = "Dev")
	void SetLevel(int32 InLevelIndex);

	// Сменить seed и перегенерировать.
	UFUNCTION(BlueprintCallable, Category = "Dev")
	void SetSeed(int32 InSeed);
};

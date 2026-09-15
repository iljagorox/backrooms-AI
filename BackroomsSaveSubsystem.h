#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BackroomsSaveSubsystem.generated.h"

// Подсистема GameInstance, хранящая Seed и уровень между сессиями.
// При первом запуске — случайные значения; при перезапуске — из SaveGame.
// Вызывается из ABackroomsWorldGenerator::BeginPlay для восстановления состояния.
UCLASS()
class BACKROOMS_API UBackroomsSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Сохранить текущий Seed и LevelIndex.
	UFUNCTION(BlueprintCallable, Category = "Backrooms|Save")
	void SaveProgress(int32 InSeed, int32 InLevelIndex);

	// Загрузить Seed и LevelIndex из сохранения.
	UFUNCTION(BlueprintCallable, Category = "Backrooms|Save")
	void LoadProgress(int32& OutSeed, int32& OutLevelIndex);

	// Есть ли сохранение (для UI: «Продолжить» vs «Новая игра»).
	UFUNCTION(BlueprintCallable, Category = "Backrooms|Save")
	bool HasSave() const;

	// Удалить сохранение (новая игра).
	UFUNCTION(BlueprintCallable, Category = "Backrooms|Save")
	void DeleteSave();

private:
	// Путь к файлу сохранения (AppData/Local/Backrooms/Save.sav).
	static FString GetSavePath();
};

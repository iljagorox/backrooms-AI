#include "BackroomsSaveSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

FString UBackroomsSaveSubsystem::GetSavePath()
{
	return FPaths::ProjectSavedDir() / TEXT("Backrooms/Save.dat");
}

void UBackroomsSaveSubsystem::SaveProgress(int32 InSeed, int32 InLevelIndex)
{
	const FString Path = GetSavePath();
	const FString Dir = FPaths::GetPath(Path);

	// Создаём директорию, если нет.
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Dir))
	{
		PlatformFile.CreateDirectoryTree(*Dir);
	}

	// Простой бинарный формат: Seed (int32) + LevelIndex (int32).
	TArray<uint8> Data;
	Data.AddUninitialized(8);
	FMemory::Memcpy(Data.GetData(), &InSeed, 4);
	FMemory::Memcpy(Data.GetData() + 4, &InLevelIndex, 4);

	if (FFileHelper::SaveArrayToFile(Data, *Path))
	{
		UE_LOG(LogTemp, Display, TEXT("BackroomsSave: saved Seed=%d, Level=%d -> %s"), InSeed, InLevelIndex, *Path);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BackroomsSave: failed to save to %s"), *Path);
	}
}

void UBackroomsSaveSubsystem::LoadProgress(int32& OutSeed, int32& OutLevelIndex)
{
	const FString Path = GetSavePath();
	TArray<uint8> Data;

	if (!FFileHelper::LoadFileToArray(Data, *Path) || Data.Num() < 8)
	{
		// Нет сохранения или битый файл — дефолт.
		OutSeed = FMath::RandRange(1, 99999);
		OutLevelIndex = 0;
		return;
	}

	FMemory::Memcpy(&OutSeed, Data.GetData(), 4);
	FMemory::Memcpy(&OutLevelIndex, Data.GetData() + 4, 4);

	UE_LOG(LogTemp, Display, TEXT("BackroomsSave: loaded Seed=%d, Level=%d from %s"), OutSeed, OutLevelIndex, *Path);
}

bool UBackroomsSaveSubsystem::HasSave() const
{
	return FPaths::FileExists(GetSavePath());
}

void UBackroomsSaveSubsystem::DeleteSave()
{
	const FString Path = GetSavePath();
	if (FPaths::FileExists(Path))
	{
		IFileManager::Get().Delete(*Path);
		UE_LOG(LogTemp, Display, TEXT("BackroomsSave: save removed"));
	}
}

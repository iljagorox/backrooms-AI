#include "BackroomsLevelBook.h"
#include "BackroomsLevelThemeConfig.h"
#include "BackroomsLocalization.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UnrealClient.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	// Каталог, где лежат готовые превью уровней.
	FString ThumbDir()
	{
		return FPaths::ProjectSavedDir() / TEXT("Backrooms/Levels");
	}

	FString ThumbPathFor(int32 LevelIndex)
	{
		return ThumbDir() / FString::Printf(TEXT("L%d.png"), LevelIndex);
	}
}

FString UBackroomsLevelBook::SavePath()
{
	return FPaths::ProjectSavedDir() / TEXT("Backrooms/LevelBook.dat");
}

FText UBackroomsLevelBook::LevelName(int32 LevelIndex)
{
	if (LevelIndex < 0 || LevelIndex > 9)
	{
		return FText::FromString(FString::Printf(TEXT("L%d"), LevelIndex));
	}
	return BackroomsLoc::Text(*FString::Printf(TEXT("Level.W%d"), LevelIndex));
}

FLinearColor UBackroomsLevelBook::LevelAccent(int32 LevelIndex)
{
	// Акцент карточки совпадает с палитрой уровня — карточка без скриншота
	// всё равно узнаваема.
	switch (LevelIndex)
	{
	case 0: return FLinearColor(0.84f, 0.78f, 0.52f);
	case 1: return FLinearColor(0.70f, 0.66f, 0.60f);
	case 2: return FLinearColor(0.55f, 0.62f, 0.70f);
	case 3: return FLinearColor(0.85f, 0.45f, 0.20f);
	case 4: return FLinearColor(0.88f, 0.88f, 0.86f);
	case 5: return FLinearColor(0.70f, 0.28f, 0.22f);
	case 6: return FLinearColor(0.16f, 0.16f, 0.20f);
	case 7: return FLinearColor(0.20f, 0.35f, 0.55f);
	case 8: return FLinearColor(0.55f, 0.42f, 0.26f);
	case 9: return FLinearColor(0.80f, 0.86f, 0.88f);
	default: return FLinearColor(0.6f, 0.6f, 0.6f);
	}
}

FString UBackroomsLevelBook::GetThumbnailPath(int32 LevelIndex) const
{
	return ThumbPathFor(LevelIndex);
}

bool UBackroomsLevelBook::HasThumbnail(int32 LevelIndex) const
{
	return FPaths::FileExists(ThumbPathFor(LevelIndex));
}

void UBackroomsLevelBook::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Load();
}

void UBackroomsLevelBook::Deinitialize()
{
	Save();
	Super::Deinitialize();
}

void UBackroomsLevelBook::MarkVisited(int32 LevelIndex)
{
	if (LevelIndex < 0)
	{
		return;
	}
	const bool bNew = !Visited.Contains(LevelIndex);
	Visited.Add(LevelIndex);
	if (LevelIndex > FurthestLevel)
	{
		FurthestLevel = LevelIndex;
	}
	if (bNew)
	{
		Save();
	}
}

bool UBackroomsLevelBook::IsUnlocked(int32 LevelIndex) const
{
	if (LevelIndex < 0)
	{
		return false;
	}
	// Уровень открыт, если игрок доходил до него ИЛИ до любого дальше:
	// прогрессия линейна, назад всегда можно вернуться.
	return LevelIndex <= FurthestLevel || Visited.Contains(LevelIndex);
}

void UBackroomsLevelBook::RequestThumbnail(int32 LevelIndex)
{
	if (LevelIndex < 0 || HasThumbnail(LevelIndex) || PendingThumbnail.Contains(LevelIndex))
	{
		return; // уже есть или уже в процессе
	}
	PendingThumbnail.Add(LevelIndex);

	// Просим движок снять кадр. Без суффикса — точное имя, чтобы потом найти.
	FScreenshotRequest::RequestScreenshot(
		FString::Printf(TEXT("BackroomsLevel_%d"), LevelIndex), false, false);

	// Скриншот пишется не мгновенно. Через секунду переносим готовый файл в
	// наш каталог — к этому моменту кадр уже на диске.
	if (UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
	{
		World->GetTimerManager().SetTimer(ThumbnailTimer,
			FTimerDelegate::CreateUObject(this, &UBackroomsLevelBook::FinalizeThumbnail, LevelIndex),
			1.0f, false);
	}
}

void UBackroomsLevelBook::FinalizeThumbnail(int32 LevelIndex)
{
	PendingThumbnail.Remove(LevelIndex);

	// FScreenshotRequest кладёт PNG в каталог скриншотов игры.
	const FString Src = FPaths::ScreenShotDir() / FString::Printf(TEXT("BackroomsLevel_%d.png"), LevelIndex);
	if (!FPaths::FileExists(Src))
	{
		return;
	}

	const FString Dir = ThumbDir();
	IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
	if (!PF.DirectoryExists(*Dir))
	{
		PF.CreateDirectoryTree(*Dir);
	}
	const FString Dst = ThumbPathFor(LevelIndex);

	// Перемещаем (если уже был — перезаписываем).
	PF.DeleteFile(*Dst);
	if (!PF.MoveFile(*Dst, *Src))
	{
		// Некоторые платформы не дают move — копируем.
		FFileHelper::SaveArrayToFile(TArray<uint8>(), *Dst); // создать
		IFileManager::Get().Copy(*Dst, *Src);
		PF.DeleteFile(*Src);
	}
	UE_LOG(LogTemp, Display, TEXT("BackroomsLevelBook: level preview %d saved"), LevelIndex);
}

void UBackroomsLevelBook::ResetAll()
{
	Visited.Reset();
	FurthestLevel = 0;
	PendingThumbnail.Reset();
	Save();
}

void UBackroomsLevelBook::Save() const
{
	const FString Path = SavePath();
	const FString Dir = FPaths::GetPath(Path);
	IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
	if (!PF.DirectoryExists(*Dir))
	{
		PF.CreateDirectoryTree(*Dir);
	}

	FString Out;
	Out += TEXT("# Backrooms level book v1\n");
	Out += FString::Printf(TEXT("FURTHEST %d\n"), FurthestLevel);
	for (int32 L : Visited)
	{
		Out += FString::Printf(TEXT("VISITED %d\n"), L);
	}
	FFileHelper::SaveStringToFile(Out, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

void UBackroomsLevelBook::Load()
{
	FString Content;
	if (!FFileHelper::LoadFileToString(Content, *SavePath()))
	{
		return;
	}
	TArray<FString> Lines;
	Content.ParseIntoArrayLines(Lines, false);
	for (const FString& Raw : Lines)
	{
		const FString Line = Raw.TrimStartAndEnd();
		if (Line.IsEmpty() || Line.StartsWith(TEXT("#")))
		{
			continue;
		}
		FString Key, Args;
		Line.Split(TEXT(" "), &Key, &Args);
		if (Key == TEXT("FURTHEST"))
		{
			FurthestLevel = FCString::Atoi(*Args);
		}
		else if (Key == TEXT("VISITED"))
		{
			Visited.Add(FCString::Atoi(*Args));
		}
	}
}

// ---- Реестр карт ----

FString UBackroomsLevelBook::GetMapName(int32 LevelIndex)
{
	return FString::Printf(TEXT("/Game/Runtime/Lvl_L%d"), LevelIndex);
}

int32 UBackroomsLevelBook::GetLevelIndexFromMapName(const FString& MapName)
{
	// Принимаем и полный путь пакета (/Game/Runtime/Lvl_L3.Lvl_L3),
	// и усечённое имя с опциями URL.
	const FString Base = FPackageName::GetShortName(MapName);
	FString Clean = Base;
	int32 Question = INDEX_NONE;
	Clean.FindChar(TEXT('?'), Question);
	if (Question != INDEX_NONE)
	{
		Clean = Clean.Left(Question);
	}
	if (!Clean.StartsWith(TEXT("Lvl_L")))
	{
		return -1;
	}
	const int32 LevelIndex = FCString::Atoi(*Clean.RightChop(5));
	return (LevelIndex >= 0 && LevelIndex <= 9) ? LevelIndex : -1;
}

FString UBackroomsLevelBook::GetMenuMapName()
{
	return GetMapName(0);
}

bool UBackroomsLevelBook::GetSeedFromUrl(const FURL& Url, int32& OutSeed)
{
	if (const TCHAR* SeedStr = Url.GetOption(TEXT("seed="), nullptr))
	{
		OutSeed = FCString::Atoi(SeedStr);
		return true;
	}
	return false;
}

const UBackroomsLevelThemeConfig* UBackroomsLevelBook::GetConfig(int32 LevelIndex) const
{
	if (LevelIndex < 0 || LevelIndex > 9)
	{
		return nullptr;
	}
	if (const TWeakObjectPtr<UBackroomsLevelThemeConfig>* Cached = ConfigCache.Find(LevelIndex))
	{
		return Cached->Get();
	}

	const FSoftObjectPath Path(*FString::Printf(TEXT("/Game/Data/L%d_Config"), LevelIndex));
	UObject* Loaded = Path.TryLoad();
	UBackroomsLevelThemeConfig* Config = Cast<UBackroomsLevelThemeConfig>(Loaded);
	ConfigCache.Add(LevelIndex, Config);
	return Config;
}

// ---- Консоль ----
namespace
{
	UBackroomsLevelBook* FindBook()
	{
		if (!GEngine)
		{
			return nullptr;
		}
		UWorld* World = GEngine->GetCurrentPlayWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UBackroomsLevelBook>() : nullptr;
	}

	static FAutoConsoleCommand CmdUnlockAll(
		TEXT("BR.Levels.UnlockAll"),
		TEXT("Open all levels (debug levels menu)."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			if (UBackroomsLevelBook* B = FindBook())
			{
				for (int32 i = 0; i <= 9; ++i)
				{
					B->MarkVisited(i);
				}
			}
		})
	);
}

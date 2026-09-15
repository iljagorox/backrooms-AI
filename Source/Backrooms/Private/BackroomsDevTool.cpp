#include "BackroomsDevTool.h"
#include "BackroomsWorldGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

ABackroomsDevTool::ABackroomsDevTool()
{
	PrimaryActorTick.bCanEverTick = false;
}

ABackroomsWorldGenerator* ABackroomsDevTool::ResolveGenerator() const
{
	return Cast<ABackroomsWorldGenerator>(
		UGameplayStatics::GetActorOfClass(this, ABackroomsWorldGenerator::StaticClass()));
}

void ABackroomsDevTool::DumpReportToFile(const FString& FullPath)
{
	ABackroomsWorldGenerator* Gen = ResolveGenerator();
	if (!Gen)
	{
		UE_LOG(LogTemp, Warning, TEXT("BR DevTool: generator not found in the world."));
		return;
	}

	const FString Report = Gen->GenerateReport();
	UE_LOG(LogTemp, Display, TEXT("\n%s"), *Report);

	FString Path = FullPath;
	if (Path.IsEmpty())
	{
		Path = FString::Printf(TEXT("%s/gen_report_L%d.txt"), *ReportDir, Gen->LevelIndex);
	}
	FFileHelper::SaveStringToFile(Report, *Path, FFileHelper::EEncodingOptions::ForceUTF8);
	UE_LOG(LogTemp, Display, TEXT("BR DevTool: report written to %s"), *Path);
}

void ABackroomsDevTool::SetLevel(int32 InLevelIndex)
{
	ABackroomsWorldGenerator* Gen = ResolveGenerator();
	if (!Gen)
	{
		UE_LOG(LogTemp, Warning, TEXT("BR DevTool: generator not found."));
		return;
	}
	Gen->SetLevel(InLevelIndex);
	UE_LOG(LogTemp, Display, TEXT("BR DevTool: switched to level %d"), InLevelIndex);
}

void ABackroomsDevTool::SetSeed(int32 InSeed)
{
	ABackroomsWorldGenerator* Gen = ResolveGenerator();
	if (!Gen)
	{
		UE_LOG(LogTemp, Warning, TEXT("BR DevTool: generator not found."));
		return;
	}
	Gen->Seed = InSeed;
	Gen->SetLevel(Gen->LevelIndex);
	UE_LOG(LogTemp, Display, TEXT("BR DevTool: seed = %d"), InSeed);
}

namespace
{
	ABackroomsWorldGenerator* FindGenerator()
	{
		UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
		if (!World)
		{
			return nullptr;
		}
		return Cast<ABackroomsWorldGenerator>(
			UGameplayStatics::GetActorOfClass(World, ABackroomsWorldGenerator::StaticClass()));
	}

	FString ReportPathFor(int32 LevelIndex)
	{
		// Отчёты пишем в папку Saved проекта, а не в локальный путь разработчика.
		return FString::Printf(TEXT("%s/gen_report_L%d.txt"),
			*FPaths::ProjectSavedDir(), LevelIndex);
	}

	// Отчёт записывается всегда, независимо от того, размещён ли DevTool актор.
	static FAutoConsoleCommand CmdDump(
		TEXT("BR.Dev.Dump"),
		TEXT("Write generator report (current level) to Saved/gen_report_L<N>.txt"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			if (ABackroomsWorldGenerator* Gen = FindGenerator())
			{
				const FString Report = Gen->GenerateReport();
				UE_LOG(LogTemp, Display, TEXT("\n%s"), *Report);
				const FString Path = ReportPathFor(Gen->LevelIndex);
				FFileHelper::SaveStringToFile(Report, *Path, FFileHelper::EEncodingOptions::ForceUTF8);
				UE_LOG(LogTemp, Display, TEXT("BR DevTool: report written to %s"), *Path);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("BR DevTool: generator not found in the world."));
			}
		})
	);

	static FAutoConsoleCommand CmdSetLevel(
		TEXT("BR.Dev.SetLevel"),
		TEXT("Switch generator to level: BR.Dev.SetLevel 5"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				return;
			}
			if (ABackroomsWorldGenerator* Gen = FindGenerator())
			{
				Gen->SetLevel(FCString::Atoi(*Args[0]));
			}
		})
	);

	static FAutoConsoleCommand CmdSetSeed(
		TEXT("BR.Dev.SetSeed"),
		TEXT("Change seed and regenerate: BR.Dev.SetSeed 12345"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				return;
			}
			if (ABackroomsWorldGenerator* Gen = FindGenerator())
			{
				Gen->Seed = FCString::Atoi(*Args[0]);
				Gen->SetLevel(Gen->LevelIndex);
			}
		})
	);

	// Self-check of the generation pipeline (determinism + per-stage invariants)
	// over a fixed sample of chunks for the active level/profile.
	static FAutoConsoleCommand CmdVerify(
		TEXT("BR.Dev.Verify"),
		TEXT("Self-check generation determinism and per-stage invariants."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			if (ABackroomsWorldGenerator* Gen = FindGenerator())
			{
				const FString Report = Gen->GenerateVerificationReport();
				UE_LOG(LogTemp, Display, TEXT("\n%s"), *Report);
				const FString Path = FString::Printf(TEXT("%s/gen_verify_L%d.txt"),
					*FPaths::ProjectSavedDir(), Gen->LevelIndex);
				FFileHelper::SaveStringToFile(Report, *Path, FFileHelper::EEncodingOptions::ForceUTF8);
				UE_LOG(LogTemp, Display, TEXT("BR DevTool: verify report -> %s"), *Path);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("BR DevTool: generator not found."));
			}
		})
	);
}

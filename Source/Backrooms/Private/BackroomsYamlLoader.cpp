#include "BackroomsYamlLoader.h"
#include "BackroomsLevelConfig.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

// ---------------------------------------------------------------------------
// Internal: построчный узел
// ---------------------------------------------------------------------------

struct FYamlLine
{
	int32 Indent;
	FString Text;
};

static TArray<FYamlLine> Tokenize(const FString& Content)
{
	TArray<FString> Raw;
	Content.ParseIntoArrayLines(Raw, /*bCullEmpty=*/true);

	TArray<FYamlLine> Out;
	Out.Reserve(Raw.Num());
	for (const FString& L : Raw)
	{
		FString Trimmed = L;
		// Отбрасываем комментарии (все – на отдельной строке, т.к. YAML.
		const int32 Hash = Trimmed.Find(TEXT("#"));
		if (Hash != INDEX_NONE)
		{
			Trimmed = Trimmed.Left(Hash);
		}
		Trimmed = Trimmed.TrimStartAndEnd();
		if (Trimmed.IsEmpty())
		{
			continue;
		}
		const int32 Indent = L.Len() - L.TrimStart().Len();
		FYamlLine Line;
		Line.Indent = Indent;
		Line.Text = MoveTemp(Trimmed);
		Out.Add(MoveTemp(Line));
	}
	return Out;
}

// ---------------------------------------------------------------------------
// Internal: разбор скаляра/списка
// ---------------------------------------------------------------------------

static TSharedRef<FBackroomsYamlValue> MakeScalar(const FString& Raw)
{
	TSharedRef<FBackroomsYamlValue> N = MakeShared<FBackroomsYamlValue>();
	N->Kind = FBackroomsYamlValue::EKind::Scalar;
	N->Scalar = Raw;
	return N;
}

static TSharedRef<FBackroomsYamlValue> MakeList(const FString& Inner)
{
	TSharedRef<FBackroomsYamlValue> N = MakeShared<FBackroomsYamlValue>();
	N->Kind = FBackroomsYamlValue::EKind::List;
	TArray<FString> Items;
	Inner.ParseIntoArray(Items, TEXT(","), /*bCullEmpty=*/true);
	N->List.Reserve(Items.Num());
	for (const FString& It : Items)
	{
		N->List.Add(MakeScalar(It.TrimStartAndEnd()));
	}
	return N;
}

static TSharedRef<FBackroomsYamlValue> ParseScalar(const FString& Rest)
{
	FString S = Rest.TrimStartAndEnd();
	if (S.StartsWith(TEXT("[")) && S.EndsWith(TEXT("]")))
	{
		return MakeList(S.LeftChop(1).RightChop(1));
	}
	return MakeScalar(S);
}

// ---------------------------------------------------------------------------
// Internal: рекурсивный разбор карты
// ---------------------------------------------------------------------------

static TSharedRef<FBackroomsYamlValue> ParseMap(TArray<FYamlLine>& Lines, int32& Idx, const int32 BaseIndent)
{
	TMap<FString, TSharedRef<FBackroomsYamlValue>> Map;

	while (Idx < Lines.Num())
	{
		const FYamlLine& Cur = Lines[Idx];
		if (Cur.Indent < BaseIndent)
		{
			break;
		}
		if (Cur.Indent > BaseIndent)
		{
			++Idx;
			continue;
		}

		const int32 Colon = Cur.Text.Find(TEXT(":"));
		if (Colon == INDEX_NONE)
		{
			++Idx;
			continue;
		}

		const FString Key = Cur.Text.Left(Colon).TrimStartAndEnd();
		const FString Rest = Cur.Text.RightChop(Colon + 1).TrimStartAndEnd();
		++Idx;

		if (Rest.IsEmpty())
		{
			// Дочерняя карта: вычисляем отступ первого ребёнка.
			const int32 ChildBase = (Idx < Lines.Num()) ? Lines[Idx].Indent : BaseIndent + 2;
			Map.Add(Key, ParseMap(Lines, Idx, ChildBase));
		}
		else
		{
			Map.Add(Key, ParseScalar(Rest));
		}
	}

	TSharedRef<FBackroomsYamlValue> Node = MakeShared<FBackroomsYamlValue>();
	Node->Kind = FBackroomsYamlValue::EKind::Map;
	Node->Map = MoveTemp(Map);
	return Node;
}

// ---------------------------------------------------------------------------
// Internal: навигация по dotted-пути
// ---------------------------------------------------------------------------

static TSharedPtr<FBackroomsYamlValue> Resolve(const TSharedRef<FBackroomsYamlValue>& Root,
                                               const TCHAR* Path)
{
	TArray<FString> Parts;
	FString(Path).ParseIntoArray(Parts, TEXT("."), /*bCullEmpty=*/true);

	TSharedPtr<FBackroomsYamlValue> Cur = Root;
	for (const FString& P : Parts)
	{
		if (!Cur.IsValid() || Cur->Kind != FBackroomsYamlValue::EKind::Map)
		{
			return nullptr;
		}
		const TSharedRef<FBackroomsYamlValue>* Found = Cur->Map.Find(P);
		if (!Found)
		{
			return nullptr;
		}
		Cur = TSharedPtr<FBackroomsYamlValue>(*Found);
	}
	return Cur;
}

// ---------------------------------------------------------------------------
// Internal: типизированные извлечения
// ---------------------------------------------------------------------------

static FString Str(const TSharedRef<FBackroomsYamlValue>& Root, const TCHAR* Path,
                   const TCHAR* Def = TEXT(""))
{
	const TSharedPtr<FBackroomsYamlValue> N = Resolve(Root, Path);
	if (!N.IsValid() || N->Kind != FBackroomsYamlValue::EKind::Scalar
	    || N->Scalar == TEXT("null"))
	{
		return FString(Def);
	}
	return N->Scalar;
}

static float Flt(const TSharedRef<FBackroomsYamlValue>& Root, const TCHAR* Path, const float Def)
{
	const TSharedPtr<FBackroomsYamlValue> N = Resolve(Root, Path);
	if (!N.IsValid() || N->Kind != FBackroomsYamlValue::EKind::Scalar
	    || N->Scalar == TEXT("null") || N->Scalar.IsEmpty())
	{
		return Def;
	}
	return FCString::Atof(*N->Scalar);
}

static int32 Int(const TSharedRef<FBackroomsYamlValue>& Root, const TCHAR* Path, const int32 Def)
{
	const TSharedPtr<FBackroomsYamlValue> N = Resolve(Root, Path);
	if (!N.IsValid() || N->Kind != FBackroomsYamlValue::EKind::Scalar
	    || N->Scalar == TEXT("null") || N->Scalar.IsEmpty())
	{
		return Def;
	}
	return FCString::Atoi(*N->Scalar);
}

static bool Bool(const TSharedRef<FBackroomsYamlValue>& Root, const TCHAR* Path, const bool Def)
{
	const TSharedPtr<FBackroomsYamlValue> N = Resolve(Root, Path);
	if (!N.IsValid() || N->Kind != FBackroomsYamlValue::EKind::Scalar
	    || N->Scalar == TEXT("null") || N->Scalar.IsEmpty())
	{
		return Def;
	}
	const FString& S = N->Scalar;
	return S == TEXT("true") || S == TEXT("True") || S == TEXT("TRUE");
}

static FLinearColor Col(const TSharedRef<FBackroomsYamlValue>& Root, const TCHAR* Path,
                        const FLinearColor& Def)
{
	const TSharedPtr<FBackroomsYamlValue> N = Resolve(Root, Path);
	if (!N.IsValid() || N->Kind != FBackroomsYamlValue::EKind::List || N->List.Num() < 3)
	{
		return Def;
	}
	const float R = FCString::Atof(*N->List[0]->Scalar);
	const float G = FCString::Atof(*N->List[1]->Scalar);
	const float B = FCString::Atof(*N->List[2]->Scalar);
	return FLinearColor(R, G, B);
}

static int32 IntListItem(const TSharedRef<FBackroomsYamlValue>& Root, const TCHAR* Path,
                         const int32 Index, const int32 Def)
{
	const TSharedPtr<FBackroomsYamlValue> N = Resolve(Root, Path);
	if (!N.IsValid() || N->Kind != FBackroomsYamlValue::EKind::List
	    || !N->List.IsValidIndex(Index))
	{
		return Def;
	}
	const FString& S = N->List[Index]->Scalar;
	if (S == TEXT("null") || S.IsEmpty())
	{
		return Def;
	}
	return FCString::Atoi(*S);
}

// ---------------------------------------------------------------------------
// Internal: сборка карты params
// ---------------------------------------------------------------------------

static void ReadParams(const TSharedRef<FBackroomsYamlValue>& Root,
                       TMap<FString, FString>& Out)
{
	const TSharedPtr<FBackroomsYamlValue> ParamsNode = Resolve(Root, TEXT("generation.params"));
	if (!ParamsNode.IsValid() || ParamsNode->Kind != FBackroomsYamlValue::EKind::Map)
	{
		return;
	}
	for (const auto& Pair : ParamsNode->Map)
	{
		const FString& Key = Pair.Key;
		const FBackroomsYamlValue& Val = *Pair.Value;
		FString Raw;
		if (Val.Kind == FBackroomsYamlValue::EKind::List)
		{
			TArray<FString> Items;
			for (const auto& It : Val.List)
			{
				Items.Add(It->Scalar);
			}
			Raw = FString::Join(Items, TEXT(","));
		}
		else
		{
			Raw = Val.Scalar;
		}
		Out.Add(Key, MoveTemp(Raw));
	}
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

TSharedPtr<FBackroomsYamlValue> BackroomsYaml::ParseFile(const FString& FilePath,
                                                         FString& OutError)
{
	FString Content;
	if (!FFileHelper::LoadFileToString(Content, *FilePath))
	{
		OutError = FString::Printf(TEXT("Не удалось прочитать %s"), *FilePath);
		return nullptr;
	}

	TArray<FYamlLine> Lines = Tokenize(Content);
	if (Lines.Num() == 0)
	{
		OutError = FString::Printf(TEXT("Файл пуст: %s"), *FilePath);
		return nullptr;
	}

	int32 Idx = 0;
	return ParseMap(Lines, Idx, 0);
}

UBackroomsLevelConfig* BackroomsYaml::LoadLevelConfig(const FString& LevelId)
{
	// Директория Content/Config/Levels в développement-режиме.
	// Приоритет: сначала внутри проекта (Content/Config/Levels — после упаковки),
	// затем репозиторий-корень (../../Content/Config/Levels от корня проекта —
	// в разработке конфиги живут рядом с валидатором, а не внутри проекта).
	FString LevelsDir;
	TArray<FString> Files;
	{
		TArray<FString> Candidates;
		Candidates.Add(FPaths::ProjectContentDir() + TEXT("Config/Levels"));
		Candidates.Add(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + TEXT("../../Content/Config/Levels")));
		for (const FString& Cand : Candidates)
		{
			TArray<FString> Probe;
			IFileManager::Get().FindFiles(Probe, *Cand, TEXT("yaml"));
			if (Probe.Num() > 0)
			{
				LevelsDir = Cand;
				Files = MoveTemp(Probe);
				break;
			}
		}
	}

	FString FoundPath;
	const FString Prefix = LevelId + TEXT("_");
	for (const FString& Name : Files)
	{
		if (Name.StartsWith(Prefix))
		{
			FoundPath = LevelsDir / Name;
			break;
		}
	}
	if (FoundPath.IsEmpty())
	{
		return nullptr;
	}

	FString Error;
	TSharedPtr<FBackroomsYamlValue> Root = ParseFile(FoundPath, Error);
	if (!Root.IsValid() || Root->Kind != FBackroomsYamlValue::EKind::Map)
	{
		return nullptr;
	}

	UBackroomsLevelConfig* Cfg = NewObject<UBackroomsLevelConfig>();
	Cfg->SourcePath = MoveTemp(FoundPath);

	const TSharedRef<FBackroomsYamlValue> RootRef = Root.ToSharedRef();

	// Identity
	Cfg->Id = Str(RootRef, TEXT("id"), TEXT(""));
	Cfg->Name = Str(RootRef, TEXT("name"), TEXT(""));
	Cfg->Version = Int(RootRef, TEXT("version"), 1);

	// Grid
	Cfg->Grid.TileSizeM = Flt(RootRef, TEXT("grid.tile_size_m"), 0.5f);
	Cfg->Grid.WallThicknessMinM = Flt(RootRef, TEXT("grid.wall_thickness_min_m"), 0.3f);
	Cfg->Grid.FloorHeightM = Flt(RootRef, TEXT("grid.floor_height_m"), 3.0f);
	Cfg->Grid.Chapter = Int(RootRef, TEXT("grid.chapter"), 0);

	// Spawn
	Cfg->Spawn.XTile = Int(RootRef, TEXT("spawn.x_tile"), 8);
	Cfg->Spawn.YTile = Int(RootRef, TEXT("spawn.y_tile"), 8);

	// Generation
	Cfg->Generation.Algorithm = Str(RootRef, TEXT("generation.algorithm"), TEXT(""));
	Cfg->Generation.ChunkCells = Int(RootRef, TEXT("generation.chunk_cells"), 8);
	Cfg->Generation.SeedMin = IntListItem(RootRef, TEXT("generation.seed_range"), 0, 0);
	Cfg->Generation.SeedMax = IntListItem(RootRef, TEXT("generation.seed_range"), 1, MAX_int32);
	ReadParams(RootRef, Cfg->Generation.Params);

	// Rooms
	Cfg->Rooms.MaxSizeM = Flt(RootRef, TEXT("rooms.max_size_m"), 8.0f);

	// Corridors
	Cfg->Corridors.WidthMinM = Flt(RootRef, TEXT("corridors.width_min_m"), -1.0f);
	Cfg->Corridors.WidthMaxM = Flt(RootRef, TEXT("corridors.width_max_m"), -1.0f);
	Cfg->Corridors.DeadEndFrac = Flt(RootRef, TEXT("corridors.dead_end_frac"), 0.0f);
	Cfg->Corridors.LoopGlobal = Flt(RootRef, TEXT("corridors.loop_glob"), 0.0f);

	// Doors
	Cfg->Doors.WidthMinM = Flt(RootRef, TEXT("doors.width_min_m"), 1.0f);
	Cfg->Doors.WidthMaxM = Flt(RootRef, TEXT("doors.width_max_m"), 1.5f);
	Cfg->Doors.HeightM = Flt(RootRef, TEXT("doors.height_m"), 2.1f);

	// Materials
	Cfg->Materials.Floor = Str(RootRef, TEXT("materials.floor"));
	Cfg->Materials.Wall = Str(RootRef, TEXT("materials.wall"));
	Cfg->Materials.Ceiling = Str(RootRef, TEXT("materials.ceiling"));
	Cfg->Materials.WaterFloor = Str(RootRef, TEXT("materials.water_floor"));

	// Lights
	Cfg->Lights.EveryM = Flt(RootRef, TEXT("lights.every_m"), 5.0f);
	Cfg->Lights.Color = Col(RootRef, TEXT("lights.color"), FLinearColor(1.f, 0.9f, 0.7f));

	// Sky
	Cfg->Sky.bEnabled = Bool(RootRef, TEXT("sky.enabled"), false);
	Cfg->Sky.Material = Str(RootRef, TEXT("sky.material"));

	// Water
	Cfg->Water.bEnabled = Bool(RootRef, TEXT("water.enabled"), false);
	Cfg->Water.DepthM = Flt(RootRef, TEXT("water.depth_m"), 0.0f);
	Cfg->Water.Color = Col(RootRef, TEXT("water.color"), FLinearColor(0.2f, 0.4f, 0.9f));

	// Acoustics
	Cfg->Acoustics.Reverb = Str(RootRef, TEXT("acoustics.reverb"), TEXT("office"));
	Cfg->Acoustics.AmbientLayer = Str(RootRef, TEXT("acoustics.ambient_layer"), TEXT("lamp_hum"));
	Cfg->Acoustics.bEcho = Bool(RootRef, TEXT("acoustics.echo"), false);

	// Misc
	Cfg->bFloodFill3D = Bool(RootRef, TEXT("flood_fill_3d"), false);
	Cfg->Validation.Seeds = Int(RootRef, TEXT("validation.seeds"), 1000);
	Cfg->Validation.ErrorTolerancePct = Flt(RootRef, TEXT("validation.error_tolerance_pct"), 1.0f);

	return Cfg;
}
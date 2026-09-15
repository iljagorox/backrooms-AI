#include "Misc/AutomationTest.h"
#include "BackroomsYamlLoader.h"
#include "BackroomsLevelConfig.h"
#include "LevelGeneratorProfile.h"
#include "BackroomsGenerationData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBackroomsYamlConfigTest, "Backrooms.Config.YamlL00Load",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBackroomsYamlConfigTest::RunTest(const FString& Parameters)
{
	UBackroomsLevelConfig* Cfg = BackroomsYaml::LoadLevelConfig(TEXT("L00"));
	if (!Cfg)
	{
		AddError(TEXT("Не удалось загрузить L00 из YAML"));
		return false;
	}

	TestEqual(TEXT("id"), Cfg->Id, FString(TEXT("L00")));
	TestEqual(TEXT("name"), Cfg->Name, FString(TEXT("Lobby")));
	TestEqual(TEXT("chapter"), Cfg->Grid.Chapter, 0);
	TestEqual(TEXT("tile_size_m"), Cfg->Grid.TileSizeM, 3.0f);
	TestEqual(TEXT("wall_thickness_min_m"), Cfg->Grid.WallThicknessMinM, 0.3f);
	TestEqual(TEXT("algorithm"), Cfg->Generation.Algorithm, FString(TEXT("bsp_plus_loops")));
	TestEqual(TEXT("chunk_cells"), Cfg->Generation.ChunkCells, 8);
	TestEqual(TEXT("seed_min"), Cfg->Generation.SeedMin, 0);
	TestEqual(TEXT("corridors.width_min_m"), Cfg->Corridors.WidthMinM, 3.0f);
	TestEqual(TEXT("corridors.width_max_m"), Cfg->Corridors.WidthMaxM, 3.0f);
	TestEqual(TEXT("doors.height_m"), Cfg->Doors.HeightM, 2.1f);
	TestEqual(TEXT("lights.every_m"), Cfg->Lights.EveryM, 5.0f);
	TestEqual(TEXT("acoustics.reverb"), Cfg->Acoustics.Reverb, FString(TEXT("office")));
	TestEqual(TEXT("acoustics.ambient_layer"), Cfg->Acoustics.AmbientLayer,
	          FString(TEXT("lamp_hum")));
	TestFalse(TEXT("sky.enabled по умолч."), Cfg->Sky.bEnabled);
	TestFalse(TEXT("flood_fill_3d по умолч."), Cfg->bFloodFill3D);
	TestEqual(TEXT("validation.seeds"), Cfg->Validation.Seeds, 1000);
	TestTrue(TEXT("SourcePath заполнен"), !Cfg->SourcePath.IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBackroomsYamlConfigAllTest,
	"Backrooms.Config.AllLevelsLoad",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBackroomsYamlConfigAllTest::RunTest(const FString& Parameters)
{
	for (int32 I = 0; I < 17; ++I)
	{
		const FString Id = FString::Printf(TEXT("L%02d"), I);
		UBackroomsLevelConfig* Cfg = BackroomsYaml::LoadLevelConfig(Id);
		if (!Cfg)
		{
			AddError(FString::Printf(TEXT("Не удалось загрузить %s из YAML"), *Id));
			continue;
		}
		TestEqual(TEXT("id файла x id конфига"), Cfg->Id, Id);
		if (Cfg->Grid.TileSizeM != 3.0f)
		{
			AddError(FString::Printf(TEXT("%s: tile_size_m != 3.0"), *Id));
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBackroomsProfileConfigTest,
	"Backrooms.Config.ProfileFromL00",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBackroomsProfileConfigTest::RunTest(const FString& Parameters)
{
	UBackroomsLevelConfig* Cfg = BackroomsYaml::LoadLevelConfig(TEXT("L00"));
	if (!Cfg)
	{
		AddError(TEXT("Не удалось загрузить L00 из YAML"));
		return false;
	}

	ULevelGeneratorProfile* P = NewObject<ULevelGeneratorProfile>();
	P->ApplyLevelConfig(Cfg);

	// Метры -> сантиметры: 1 тайл = 3.0 м = 300 см (константа сетки).
	TestEqual(TEXT("cell_size_cm"), P->CellSize, 300.0f);
	TestEqual(TEXT("wall_height_cm"), P->WallHeight, 300.0f);
	TestEqual(TEXT("wall_thickness_cm"), P->WallThickness, 30.0f);
	TestEqual(TEXT("door_width_cm"), P->DoorWidth, 125.0f);
	TestEqual(TEXT("door_height_cm"), P->DoorHeight, 210.0f);
	TestEqual(TEXT("chunk_cells"), P->ChunkSizeCells, 8);
	TestEqual(TEXT("config_id"), P->ConfigId, FString(TEXT("L00")));
	TestEqual(TEXT("layout_pattern"), (int32)P->LayoutPattern, (int32)ELevelLayoutPattern::GridRooms);

	const FChunkGenerationData::FParams FP = P->ToGenParams(0);
	TestEqual(TEXT("params.cell_count"), FP.CellCount, 8);
	TestEqual(TEXT("params.min_room"), FP.MinRoomCells, 1);
	TestEqual(TEXT("params.max_room (8м/3м)"), FP.MaxRoomCells, 3);
	TestEqual(TEXT("params.pattern"), (int32)FP.Pattern,
	          (int32)FChunkGenerationData::EChunkLayoutPattern::GridRooms);

	return true;
}
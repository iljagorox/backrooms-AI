#pragma once

#include "CoreMinimal.h"
#include "BackroomsFloorPlan.h"
#include "BackroomsChunkCoord.h"
#include "Materials/Material.h"

// -------------------------------------------------------
// Step F: FloorPlan Debug Visualization
// 
// Provides DrawDebug functions to visualize the FloorPlan
// before polished walls/materials are implemented.
// Users can inspect a seed and immediately understand
// WHY the generator produced that architecture.
// -------------------------------------------------------
class BACKROOMS_API FBackroomsFloorPlanDebug
{
public:
	// -------------------------------------------------------
	// DrawFloorPlan — main debug visualization function
	// Draws to screen every frame when called from Tick or editor
	// -------------------------------------------------------
	static void DrawFloorPlan(
		const UBackroomsFloorPlan* FloorPlan,
		const FIntPoint& RegionCoordinate,
		const FChunkCoord& ChunkExtent,
		const FBackroomsMacroFields& MacroFields,
		const FChunkCoord& CurrentChunkCoord = FChunkCoord(0, 0));

	// -------------------------------------------------------
	// DrawSpaceBounds — draw bounding boxes for all spaces
	// -------------------------------------------------------
	static void DrawSpaceBounds(
		const UBackroomsFloorPlan* FloorPlan,
		const FColor ColorModifier = FColor::White);

	// -------------------------------------------------------
	// DrawSpaceIDs — draw text labels with Space IDs
	// -------------------------------------------------------
	static void DrawSpaceIDs(
		const UBackroomsFloorPlan* FloorPlan);

	// -------------------------------------------------------
	// DrawOpenings — draw all openings as dashed lines
	// -------------------------------------------------------
	static void DrawOpenings(
		const UBackroomsFloorPlan* FloorPlan,
		const FColor Color = FColor::Yellow);

	// -------------------------------------------------------
	// DrawConnections — draw graph connections between spaces
	// -------------------------------------------------------
	static void DrawConnections(
		const UBackroomsFloorPlan* FloorPlan,
		const FColor Color = FColor::Cyan);

	// -------------------------------------------------------
	// DrawBoundaryContracts — draw canonical boundary contracts
	// between regions (shows A.East == B.West by construction)
	// -------------------------------------------------------
	static void DrawBoundaryContracts(
		const UBackroomsFloorPlan* FloorPlan,
		const FIntPoint& RegionCoord);

	// -------------------------------------------------------
	// DrawMacroFields — visualize slow coherent fields
	// (OpennessField, RegularityField, BrightnessField, DecayField)
	// Shows gradual variation across regions, NOT abrupt changes at boundaries
	// -------------------------------------------------------
	static void DrawMacroFields(
		const UBackroomsFloorPlan* FloorPlan,
		const FIntPoint& RegionCoordinate);

	// -------------------------------------------------------
	// DrawChunkBoundaries — overlay chunk grid on floor plan
	// -------------------------------------------------------
	static void DrawChunkBoundaries(
		const UBackroomsFloorPlan* FloorPlan,
		const FChunkCoord& ChunkExtent,
		const FChunkCoord& CurrentChunkCoord);

	// -------------------------------------------------------
	// DrawPrimaryFlow — show primary exploration flow direction
	// derived from MacroFields
	// -------------------------------------------------------
	static void DrawPrimaryFlow(
		const UBackroomsFloorPlan* FloorPlan,
		const FIntPoint& RegionCoordinate);

	// -------------------------------------------------------
	// DrawSpaceRoles — color spaces by their architectural role
	// OpenSpace=Green, PartitionedSpace=Blue, TransitionSpace=Cyan,
	// ServiceSpace=Yellow, StructuralVoid=Magenta
	// -------------------------------------------------------
	static TMap<EBackroomsSpaceRole, FColor> GetRoleColors();

private:
	// Helper: draw a single space with its footprint and ID
	static void DrawSingleSpace(
		const FBackroomsSpaceData& Space,
		const FColor& Color,
		const FIntPoint& RegionCoord);

	// Helper: draw an opening in a space boundary
	static void DrawSingleOpening(
		const FBackroomsOpeningData& Opening,
		const FColor& Color);

	// Helper: draw flow arrow at space center
	static void DrawFlowArrow(
		const FBackroomsSpaceData& Space,
		const FBackroomsMacroFields& MacroFields,
		const FColor& Color);
};
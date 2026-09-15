#include "BackroomsInventoryData.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"

UStaticMesh* UItemDataAsset::GetHeldMesh() const
{
	if (!HeldMesh.IsNull())
	{
		return HeldMesh.LoadSynchronous();
	}
	return WorldMesh.LoadSynchronous();
}

UStaticMesh* UItemDataAsset::GetWorldMesh() const
{
	return WorldMesh.LoadSynchronous();
}
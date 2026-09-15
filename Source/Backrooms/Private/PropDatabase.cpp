#include "PropDatabase.h"

const FPropRule* UPropDatabase::FindPropByName(FName PropID) const
{
	for (const FPropRule& Rule : Props)
	{
		if (Rule.PropID == PropID)
		{
			return &Rule;
		}
	}
	return nullptr;
}

TArray<const FPropRule*> UPropDatabase::GetPropsForPlacement(EPropPlacement Placement) const
{
	TArray<const FPropRule*> Result;
	for (const FPropRule& Rule : Props)
	{
		if (Rule.Placement == Placement)
		{
			Result.Add(&Rule);
		}
	}
	return Result;
}

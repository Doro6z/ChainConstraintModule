#include "ChainProfile.h"

UChainProfile::UChainProfile()
{
	// Default LOD: single level spanning a large distance range.
	if (LODLevels.Num() == 0)
	{
		FChainLODLevel DefaultLOD;
		DefaultLOD.MinDistance = 0.0f;
		DefaultLOD.MaxDistance = 100000.0f;
		DefaultLOD.SegmentCountOverride = 0; // use Visual.DefaultSegmentCount
		DefaultLOD.bSimulatePhysics = true;
		DefaultLOD.bEnableCollisions = true;
		DefaultLOD.SimulationRateFactor = 1.0f;

		LODLevels.Add(DefaultLOD);
	}
}

int32 UChainProfile::GetBaseSegmentCount() const
{
	return FMath::Max(2, Visual.DefaultSegmentCount);
}

float UChainProfile::GetBaseLength() const
{
	return FMath::Max(1.0f, Visual.DefaultLength);
}

int32 UChainProfile::GetLODIndexForDistance(float Distance) const
{
	const float SafeDistance = FMath::Max(0.0f, Distance);

	for (int32 Index = 0; Index < LODLevels.Num(); ++Index)
	{
		const FChainLODLevel& LOD = LODLevels[Index];
		if (SafeDistance >= LOD.MinDistance && SafeDistance <= LOD.MaxDistance)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

int32 UChainProfile::GetSegmentCountAtDistance(float Distance) const
{
	const int32 BaseSegments = GetBaseSegmentCount();
	const int32 LODIndex = GetLODIndexForDistance(Distance);

	if (LODIndex == INDEX_NONE)
	{
		return BaseSegments;
	}

	const FChainLODLevel& LOD = LODLevels[LODIndex];
	if (LOD.SegmentCountOverride <= 0)
	{
		return BaseSegments;
	}

	// Ensure at least 2 segments for a valid chain.
	return FMath::Max(2, LOD.SegmentCountOverride);
}

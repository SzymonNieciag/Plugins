#include "SPDefines.h"
#include "../Components/PathSplineMetadata.h"
#include "../Components/PathSplineComponent.h"
#include "../SplinePathActor.h"

const FSplinePathPointData& FSplineActorConnection::GetSplinePathPointData() const
{
	// If the SplinePathActor is invalid, return an empty FSplinePathPointData
	if (!SplinePathActor)
	{
		static FSplinePathPointData EmptyData;
		return EmptyData;
	}

	// Get the spline component from the actor
	UPathSplineComponent* SplineComp = SplinePathActor->SplineComponent;
	if (!SplineComp)
	{
		static FSplinePathPointData EmptyData;
		return EmptyData;
	}

	// Cast to the appropriate metadata type
	UPathSplineMetadata* PathSplineMetadata = Cast<UPathSplineMetadata>(SplineComp->GetSplinePointsMetadata());
	if (!PathSplineMetadata)
	{
		static FSplinePathPointData EmptyData;
		return EmptyData;
	}

	// Check if the index is within a valid range
	if (IndexPoint >= 0 && IndexPoint < PathSplineMetadata->PointData.Num())
	{
		return PathSplineMetadata->PointData[IndexPoint];
	}
	else
	{
		// Return an empty FSplinePathPointData if the index is out of range
		static FSplinePathPointData EmptyData;
		return EmptyData;
	}
}
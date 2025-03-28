// Copyright Epic Games, Inc. All Rights Reserved.

#include "SplinePathVisualizer.h"
#include "Components/SplineComponent.h"
#include "SplinePathSystem/Components/PathSplineComponent.h"
#include "SplinePathSystem/Components/PathSplineMetadata.h"
#include "SplineComponentVisualizer.h"
#include "SplinePathSystem/SplinePathActor.h"

FSplinePathVisualizer::FSplinePathVisualizer() {}
FSplinePathVisualizer::~FSplinePathVisualizer() {}

void FSplinePathVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FSplineComponentVisualizer::DrawVisualization(Component, View, PDI);

	const UPathSplineComponent* PathSplineComponent = Cast<const UPathSplineComponent>(Component);
	if (PathSplineComponent)
	{
		if (const UPathSplineMetadata* Metadata = Cast<UPathSplineMetadata>(PathSplineComponent->GetSplinePointsMetadata()))
		{
			const TArray<FSplinePathPointData>& PointData = Metadata->PointData;

			const int32 NumPoints = PointData.Num();
			for (int32 i = 0; i < NumPoints; ++i)
			{
				if (!SelectionState->GetSelectedKeys().Contains(i))
				{
					continue;
				}
				const FVector StartPoint = PathSplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

				for (const FSplinePathConnection& SplinePathConnection : PointData[i].SplinePathConnections)
				{
					if (SplinePathConnection.ConnectedSplinePathActor)
					{
						const FVector EndPoint = SplinePathConnection.ConnectedSplinePathActor->SplineComponent->GetLocationAtSplinePoint(SplinePathConnection.SplineIndex, ESplineCoordinateSpace::World);
						// Rysowanie strza³ki
						DrawArrow(PDI, StartPoint, EndPoint);
					}
				}
			}
		}
	}
}

void FSplinePathVisualizer::DrawArrow(FPrimitiveDrawInterface* PDI, const FVector& StartPoint, const FVector& EndPoint)
{
	// We calculate the direction of the line
	const FVector Direction = (EndPoint - StartPoint).GetSafeNormal();
	const float ArrowLength = 20.0f; // D³ugoœæ strza³ki
	const float ArrowAngle = 30.0f; // K¹t w stopniach strza³ki

	// We calculate the end point of the triangle (arrows)
	const FVector ArrowEnd = EndPoint - Direction * ArrowLength;

	// We draw the main line
	PDI->DrawLine(StartPoint, EndPoint, FColor::Green, SDPG_Foreground);

	// We calculate vectors for a triangle
	FVector Left = (Direction.RotateAngleAxis(ArrowAngle, FVector(0, 0, 1))).GetSafeNormal();
	FVector Right = (Direction.RotateAngleAxis(-ArrowAngle, FVector(0, 0, 1))).GetSafeNormal();

	// We draw two lines representing the arrow
	PDI->DrawLine(EndPoint, ArrowEnd + Left * ArrowLength * 0.5f, FColor::Green, SDPG_Foreground);
	PDI->DrawLine(EndPoint, ArrowEnd + Right * ArrowLength * 0.5f, FColor::Green, SDPG_Foreground);
}
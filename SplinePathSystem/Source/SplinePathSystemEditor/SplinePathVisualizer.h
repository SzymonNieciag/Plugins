// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/SplineComponent.h"
#include "SplineComponentVisualizer.h"

class SPLINEPATHSYSTEMEDITOR_API FSplinePathVisualizer : public FSplineComponentVisualizer
{
public:
    FSplinePathVisualizer();
    virtual ~FSplinePathVisualizer();

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

private:
    void DrawArrow(FPrimitiveDrawInterface* PDI, const FVector& StartPoint, const FVector& EndPoint);
};

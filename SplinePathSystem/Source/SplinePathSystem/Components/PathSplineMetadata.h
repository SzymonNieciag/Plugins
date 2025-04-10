#pragma once

#include "CoreMinimal.h"
#include "../Defines/SPDefines.h"
#include "Components/SplineComponent.h"
#include "PathSplineMetadata.generated.h"

/**
 * Custom metadata class for spline, extending USplineMetadata.
 */
UCLASS(Blueprintable)
class SPLINEPATHSYSTEM_API UPathSplineMetadata : public USplineMetadata
{
    GENERATED_BODY()

	// Konstruktor
	UPathSplineMetadata();

public:
	virtual void InsertPoint(int32 Index, float t, bool bClosedLoop) override;
	virtual void UpdatePoint(int32 Index, float t, bool bClosedLoop) override;
	virtual void AddPoint(float InputKey) override;
	virtual void RemovePoint(int32 Index) override;
	virtual void DuplicatePoint(int32 Index) override;
	virtual void CopyPoint(const USplineMetadata* FromSplineMetadata, int32 FromIndex, int32 ToIndex) override;
	virtual void Reset(int32 NumPoints) override;
	virtual void Fixup(int32 NumPoints, USplineComponent* SplineComp) override;

	void GenerateDefaultPoints(USplineComponent* SplineComp, int32 Index, FSplinePathPointData& InCurrentPointData);
	void UpdatePointsDistance(const FVector& CurrentPointLocation, FSplinePathPointData& InCurrentPointData);

	UPROPERTY(EditAnywhere)
	TArray<struct FSplinePathPointData> PointData; 

	inline void SetSplineComponent(USplineComponent* NewSplineComponent);
	USplineComponent* GetSplineComponent(){return SplineComponent;}

private:
	UPROPERTY()
    TObjectPtr<USplineComponent> SplineComponent;
};
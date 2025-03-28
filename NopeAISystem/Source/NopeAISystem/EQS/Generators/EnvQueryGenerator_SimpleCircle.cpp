// Copyright Epic Games, Inc. All Rights Reserved.
#include "EnvQueryGenerator_SimpleCircle.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"

#define LOCTEXT_NAMESPACE "EnvQueryGenerator"

UEnvQueryGenerator_SimpleCircle::UEnvQueryGenerator_SimpleCircle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Center = UEnvQueryContext_Querier::StaticClass();
	InnerRadius.DefaultValue = 500.0f;
	OuterRadius.DefaultValue = 2000.0f;
	DistanceToGeneratePoints.DefaultValue = 300;

	ProjectionData.TraceMode = EEnvQueryTrace::None;
}

void UEnvQueryGenerator_SimpleCircle::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	TArray<FVector> CenterPoints;
	QueryInstance.PrepareContext(Center, CenterPoints);

	if (CenterPoints.Num() <= 0)
	{
		return;
	}

	UObject* BindOwner = QueryInstance.Owner.Get();
	InnerRadius.BindData(BindOwner, QueryInstance.QueryID);
	OuterRadius.BindData(BindOwner, QueryInstance.QueryID);
	DistanceToGeneratePoints.BindData(BindOwner, QueryInstance.QueryID);

	FVector::FReal InnerRadiusValue = InnerRadius.GetValue();
	FVector::FReal OuterRadiusValue = OuterRadius.GetValue();

	float DistanceBeetweenPoint = DistanceToGeneratePoints.GetValue();

	if ((InnerRadiusValue < 0.) || (OuterRadiusValue <= 0.) ||
		(InnerRadiusValue > OuterRadiusValue) || (DistanceBeetweenPoint < 10))
	{
		return;
	}

	int XnumPoints = OuterRadiusValue * 2 / DistanceBeetweenPoint;
	int YnumPoints = OuterRadiusValue * 2 / DistanceBeetweenPoint;
	int MaxRange = OuterRadiusValue / DistanceBeetweenPoint;
	int MinRange = InnerRadiusValue / DistanceBeetweenPoint;
	int MaxRangeSqr = MaxRange * MaxRange;
	int MinRangeSqr = MinRange * MinRange;

	TArray<FNavLocation> Points;

	for (int32 x = -XnumPoints; x <= XnumPoints; x++)
	{
		for (int32 y = -YnumPoints; y <= YnumPoints; y++)
		{
			int value = x * x + y * y;
			if (value >= MinRangeSqr && value <= MaxRangeSqr)
			{
				for (int32 ContextIdx = 0; ContextIdx < CenterPoints.Num(); ContextIdx++)
				{
					const FVector RingPos(x * DistanceBeetweenPoint, y * DistanceBeetweenPoint, 0.);

					const FNavLocation PointPos = FNavLocation(CenterPoints[ContextIdx] + RingPos);
					Points.Add(PointPos);
				}
			}
		}
	}

	ProjectAndFilterNavPoints(Points, QueryInstance);
	StoreNavPoints(Points, QueryInstance);
}

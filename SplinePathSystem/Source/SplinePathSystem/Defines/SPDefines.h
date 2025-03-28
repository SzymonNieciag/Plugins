// Copyright 2018 DRAGO. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SPDefines.generated.h"

USTRUCT(BlueprintType)
struct FSplinePathConnection
{
	GENERATED_USTRUCT_BODY()

	// The actor representing the connected spline path
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class ASplinePathActor* ConnectedSplinePathActor;

	// The index of the spline point within the connected spline path
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SplineIndex;

	// The default cost associated with the connection between spline points
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefaultCost = 1;

	// The distance between two connected spline points (visible only during runtime)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	float Distance;

	// The total cost calculated for the connection (visible only during runtime)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	float TotalCost;

	// Default constructor
	FSplinePathConnection()
		: ConnectedSplinePathActor(nullptr), SplineIndex(0), DefaultCost(1), Distance(0.0f), TotalCost(0.0f)
	{
	}

	// Constructor with parameters
	FSplinePathConnection(ASplinePathActor* InConnectedSplinePathActor, int32 InSplineIndex, float InDefaultCost = 1.0f, float InDistance = 0.0f, float InTotalCost = 0.0f)
		: ConnectedSplinePathActor(InConnectedSplinePathActor), SplineIndex(InSplineIndex), DefaultCost(InDefaultCost), Distance(InDistance), TotalCost(InTotalCost)
	{
	}
};

USTRUCT(BlueprintType)
struct FSplinePathPointData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSplinePathConnection> SplinePathConnections;

	FSplinePathPointData()
		: SplinePathConnections() {
	}

	FSplinePathPointData(const FSplinePathPointData& Other)
		: SplinePathConnections(Other.SplinePathConnections) {
	}

	FSplinePathPointData& operator=(const FSplinePathPointData& Other)
	{
		if (this != &Other)
		{
			SplinePathConnections = Other.SplinePathConnections;
		}
		return *this;
	}

	FSplinePathPointData(FSplinePathPointData&& Other) noexcept
		: SplinePathConnections(MoveTemp(Other.SplinePathConnections)) {
	}

	FSplinePathPointData& operator=(FSplinePathPointData&& Other) noexcept
	{
		if (this != &Other)
		{
			SplinePathConnections = MoveTemp(Other.SplinePathConnections);
		}
		return *this;
	}
};

USTRUCT(BlueprintType)
struct SPLINEPATHSYSTEM_API FSplineActorConnection
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|SmartPoint")
	class ASplinePathActor* SplinePathActor;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|SmartPoint")
	int IndexPoint;

	FSplineActorConnection()
		: SplinePathActor(nullptr), IndexPoint(0)
	{
	}

	FSplineActorConnection(ASplinePathActor* InSplinePathActor, int InIndexPoint)
		: SplinePathActor(InSplinePathActor), IndexPoint(InIndexPoint)
	{
	}

	bool operator==(const FSplineActorConnection& Other) const
	{
		return SplinePathActor == Other.SplinePathActor && IndexPoint == Other.IndexPoint;
	}

	// Method returning the spline path point data
	// Retrieves the data related to the spline point from the associated SplinePathActor
	const FSplinePathPointData& GetSplinePathPointData() const;
};

// Funkcja hashuj¹ca dla TMap
FORCEINLINE uint32 GetTypeHash(const FSplineActorConnection& Key)
{
	return HashCombine(GetTypeHash(Key.SplinePathActor), GetTypeHash(Key.IndexPoint));
}

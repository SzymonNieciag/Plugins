//Copyright © 2022 DRAGO Entertainment.All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "NPPathFollowingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCurrentAcceptanceRadiusChange, float, OutCurrentAcceptanceRadius);

UCLASS()
class NOPEAISYSTEM_API UNPPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()

public:
	UNPPathFollowingComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void SetMoveSegment(int32 SegmentStartIndex) override;

	UPROPERTY(BlueprintAssignable, Category = "Animals")
	FCurrentAcceptanceRadiusChange OnCurrentAcceptanceRadiusChanged;

	bool IsPointReached(const FVector& GoalLocation, float GoalRadius, float GoalHalfHeight, const FVector& AgentLocation, float RadiusThreshold);
};



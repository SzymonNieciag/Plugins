// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_NPRotateCharacterTo.generated.h"

/**
 * Make Noise task node.
 * A task node that calls MakeNoise() on this Pawn when executed.
 */

UCLASS()
class NOPEAISYSTEM_API UBTTask_NPRotateCharacterTo : public UBTTaskNode
{
	GENERATED_UCLASS_BODY()

public:
	/** initialize any asset related data */
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	float LimitRotation = 25.0f;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	float Precision = 5.0f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.01", ClampMax = "0.1", UIMin = "0.01", UIMax = "0.1"))
	float MinSpeed = 0.1f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.01", ClampMax = "0.1", UIMin = "0.01", UIMax = "0.1"))
	float MaxSpeed = 1.0f;

	UPROPERTY(EditAnywhere)
	bool bUseTargetRotation = true;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector KeyActorSelector;
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
private:
	FRotator GetRotationPoint(class ANPAICharacterBase* AICharacterBase, UBehaviorTreeComponent& OwnerComp);
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "NopeAISystem/AI/NPAIDefines.h"
#include "BTTask_AICharacterMoveTo.generated.h"

/**
 * Move To task node.
 * Moves the AI pawn toward the specified Actor or Location blackboard entry using the navigation system.
 */

UCLASS()
class NOPEAISYSTEM_API UBTTask_AICharacterMoveTo : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTTask_AICharacterMoveTo(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = Blackboard)
	bool bUseTaskMovementSetup = true;

	UPROPERTY(Category = Node, EditAnywhere, meta = (EditCondition = "!bUseTaskMovementSetup", EditConditionHides))
	ENPAIMovementSpeed AICharacterMovementSpeed = ENPAIMovementSpeed::Jog;

protected:
	virtual EBTNodeResult::Type PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
};

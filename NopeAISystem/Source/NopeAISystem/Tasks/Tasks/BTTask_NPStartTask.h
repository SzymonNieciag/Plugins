// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BehaviorTree/BTTaskNode.h"
#include "../../AI/NPAIDefines.h"
#include "BTTask_NPStartTask.generated.h"

/**
 *	Play indicated AnimationAsset on Pawn controlled by BT 
 *	Note that this node is generic and is handing multiple special cases,
 *	If you want a more efficient solution you'll need to implement it yourself (or wait for our BTTask_PlayCharacterAnimation)
 */
UCLASS()
class NOPEAISYSTEM_API UBTTask_NPStartTask : public UBTTaskNode
{
	GENERATED_UCLASS_BODY()

	/** Animation asset to play. Note that it needs to match the skeleton of pawn this BT is controlling */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	ENPAIActionState NPAIActionState = ENPAIActionState::Move;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_NPStartTask.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include <AIController.h>
#include "NopeAISystem/Characters/NPAICharacterBase.h"
#include "NopeAISystem/Tasks/NPAITaskBase.h"

//----------------------------------------------------------------------//
// UBTTask_PlayAnimation
//----------------------------------------------------------------------//
UBTTask_NPStartTask::UBTTask_NPStartTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "BTT_NP_AIStartTask";
	// instantiating to be able to use Timers
	bCreateNodeInstance = false;
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_NPStartTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ANPAICharacterBase* AICharacterBase = Cast<ANPAICharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (AICharacterBase->GetCurrentActiveAITask())
	{
		AICharacterBase->GetCurrentActiveAITask()->StartTask(NPAIActionState);
	}
	return EBTNodeResult::Type::Succeeded;
}

EBTNodeResult::Type UBTTask_NPStartTask::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::Type::Succeeded;
}

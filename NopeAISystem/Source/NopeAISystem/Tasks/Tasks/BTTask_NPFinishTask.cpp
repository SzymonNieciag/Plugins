// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_NPFinishTask.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include <AIController.h>
#include "NopeAISystem/Characters/NPAICharacterBase.h"

//----------------------------------------------------------------------//
// UBTTask_PlayAnimation
//----------------------------------------------------------------------//
UBTTask_NPFinishTask::UBTTask_NPFinishTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "BTT_NP_AIFinishTask";
	// instantiating to be able to use Timers
	bCreateNodeInstance = false;
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_NPFinishTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	ANPAICharacterBase* AICharacterBase = Cast<ANPAICharacterBase>(AIController->GetPawn());
	AICharacterBase->FinishAITask();
	return EBTNodeResult::Type::Succeeded;
}

EBTNodeResult::Type UBTTask_NPFinishTask::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::Type::Succeeded;
}

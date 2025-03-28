// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_AICharacterMoveTo.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "NopeAISystem/Tasks/NPAITaskBase.h"
#include "NopeAISystem/AI/AITargetPoint.h"
#include "NopeAISystem/AI/Components/NPCharacterMovementComponent.h"
#include "NopeAISystem/Characters/NPAICharacterBase.h"
#include <../../../../../../../Source/Runtime/AIModule/Classes/Tasks/AITask_MoveTo.h>
#include "NavFilters/NavigationQueryFilter.h"

UBTTask_AICharacterMoveTo::UBTTask_AICharacterMoveTo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "AICharacter Move To";
}

NOPEAISYSTEM_API EBTNodeResult::Type UBTTask_AICharacterMoveTo::PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	FBTMoveToTaskMemory* MyMemory = CastInstanceNodeMemory<FBTMoveToTaskMemory>(NodeMemory);
	AAIController* MyController = OwnerComp.GetAIOwner();

	EBTNodeResult::Type NodeResult = EBTNodeResult::Failed;
	if (MyController && MyBlackboard)
	{
		ANPAICharacterBase* AICharacterBase = Cast<ANPAICharacterBase>(MyController->GetPawn());
		if (!AICharacterBase)
		{
			return NodeResult;
		}

		if (AICharacterBase->GetCurrentActiveAITask() && bUseTaskMovementSetup)
		{
			AICharacterBase->SetCharacterMovementSpeed((ENPAIMovementSpeed)AICharacterBase->GetCurrentActiveAITask()->GetAIMovementSpeed());
			AcceptableRadius = AICharacterBase->GetCurrentActiveAITask()->AcceptableRadius;
		}
		else
		{
			if (AICharacterBase->GetNPCharacterMovement()->GetCurrentMovementDirection() == ENPAIMovementDirection::Back)
			{
				AICharacterBase->SetCharacterMovementSpeed(ENPAIMovementSpeed::WalkSlow);
			}
			else
			{
				AICharacterBase->SetCharacterMovementSpeed(AICharacterMovementSpeed);
			}
			AICharacterBase->GetTargetPoint()->SetAcceptableRadius(AcceptableRadius.GetValue(MyBlackboard));
		}

		TSubclassOf<UNavigationQueryFilter> OverrideFiler = FilterClass.GetValue(*MyBlackboard);

		FAIMoveRequest MoveReq;
		MoveReq.SetNavigationFilter(*OverrideFiler ? OverrideFiler : MyController->GetDefaultNavigationFilterClass());
		MoveReq.SetAllowPartialPath(bAllowPartialPath.GetValue(MyBlackboard));
		MoveReq.SetAcceptanceRadius(AcceptableRadius.GetValue(MyBlackboard));
		MoveReq.SetCanStrafe(bAllowStrafe.GetValue(MyBlackboard));
		MoveReq.SetReachTestIncludesAgentRadius(bReachTestIncludesAgentRadius.GetValue(MyBlackboard));
		MoveReq.SetReachTestIncludesGoalRadius(bReachTestIncludesGoalRadius.GetValue(MyBlackboard));
		MoveReq.SetRequireNavigableEndLocation(bRequireNavigableEndLocation.GetValue(MyBlackboard));
		MoveReq.SetProjectGoalLocation(bProjectGoalLocation.GetValue(MyBlackboard));
		MoveReq.SetUsePathfinding(bUsePathfinding);

		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			UObject* KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID());
			AActor* TargetActor = Cast<AActor>(KeyValue);
			if (TargetActor)
			{
				if (bTrackMovingGoal.GetValue(MyBlackboard))
				{
					MoveReq.SetGoalActor(TargetActor);
				}
				else
				{
					MoveReq.SetGoalLocation(TargetActor->GetActorLocation());
				}
			}
			else
			{
				UE_VLOG(MyController, LogBehaviorTree, Warning, TEXT("UBTTask_MoveTo::UBTTask_AICharacterMoveTo tried to go to actor while BB %s entry was empty"), *BlackboardKey.SelectedKeyName.ToString());
			}
		}
		else if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		{
			const FVector TargetLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID());
			MoveReq.SetGoalLocation(TargetLocation);

			MyMemory->PreviousGoalLocation = TargetLocation;
		}

		if (MoveReq.IsValid())
		{
			UAITask_MoveTo* MoveTask = MyMemory->Task.Get();
			const bool bReuseExistingTask = (MoveTask != nullptr);

			MoveTask = PrepareMoveTask(OwnerComp, MoveTask, MoveReq);
			if (MoveTask)
			{
				MyMemory->bObserverCanFinishTask = false;

				if (bReuseExistingTask)
				{
					if (MoveTask->IsActive())
					{
						UE_VLOG(MyController, LogBehaviorTree, Verbose, TEXT("\'%s\' reusing AITask %s"), *GetNodeName(), *MoveTask->GetName());
						MoveTask->ConditionalPerformMove();
					}
					else
					{
						UE_VLOG(MyController, LogBehaviorTree, Verbose, TEXT("\'%s\' reusing AITask %s, but task is not active - handing over move performing to task mechanics"), *GetNodeName(), *MoveTask->GetName());
					}
				}
				else
				{
					MyMemory->Task = MoveTask;
					UE_VLOG(MyController, LogBehaviorTree, Verbose, TEXT("\'%s\' task implementing move with task %s"), *GetNodeName(), *MoveTask->GetName());
					MoveTask->ReadyForActivation();
				}

				MyMemory->bObserverCanFinishTask = true;
				NodeResult = (MoveTask->GetState() != EGameplayTaskState::Finished) ? EBTNodeResult::InProgress :
					MoveTask->WasMoveSuccessful() ? EBTNodeResult::Succeeded :
					EBTNodeResult::Failed;
			}
		}
	}

	return NodeResult;
}


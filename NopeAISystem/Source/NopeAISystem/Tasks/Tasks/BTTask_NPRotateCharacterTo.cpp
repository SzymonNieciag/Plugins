// Copyright Epic Games, Inc. All Rights Reserved.
#include "BTTask_NPRotateCharacterTo.h"
#include "NopeAISystem/Characters/NPAICharacterBase.h"
#include <AIController.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include "NopeAISystem/AI/Components/NPCharacterMovementComponent.h"
#include "NopeAISystem/AI/AITargetPoint.h"
#include "../../../../../../../Source/Runtime/AIModule/Classes/BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "../../../../../../../Source/Runtime/AIModule/Classes/BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

///** Pitch (degrees) around Y axis */
///** Yaw (degrees) around Z axis */
///** Roll (degrees) around X axis */

UBTTask_NPRotateCharacterTo::UBTTask_NPRotateCharacterTo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Rotate Character To";
	bNotifyTick = true;
	KeyActorSelector.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_NPRotateCharacterTo, KeyActorSelector), AActor::StaticClass());
	KeyActorSelector.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_NPRotateCharacterTo, KeyActorSelector));
}

void UBTTask_NPRotateCharacterTo::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (BBAsset)
	{
		KeyActorSelector.ResolveSelectedKey(*BBAsset);
	}
	else
	{
		UE_LOG(LogBehaviorTree, Warning, TEXT("Can't initialize task: %s, make sure that behavior tree specifies blackboard asset!"), *GetName());
	}
}

EBTNodeResult::Type UBTTask_NPRotateCharacterTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type NodeResult = EBTNodeResult::InProgress;

	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	ANPAICharacterBase* AICharacterBase = Cast<ANPAICharacterBase>(AIController->GetPawn());

	FRotator Rotator = GetRotationPoint(AICharacterBase, OwnerComp);

	FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(Rotator, AICharacterBase->GetActorRotation());
	float AbsDeltaRotator = UKismetMathLibrary::Abs(DeltaRotator.Yaw);

	if (LimitRotation > AbsDeltaRotator)
	{
		NodeResult = EBTNodeResult::Succeeded;
	}
	return NodeResult;
}

EBTNodeResult::Type UBTTask_NPRotateCharacterTo::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner()))
	{
		if (ANPAICharacterBase* AICharacterBase = Cast<ANPAICharacterBase>(AIController->GetPawn()))
		{
			AICharacterBase->GetNPCharacterMovement()->OnCharacterRotated.Broadcast(AICharacterBase, 0, 0);
		}
	}
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_NPRotateCharacterTo::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_NPRotateCharacterTo::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!OwnerComp.IsPaused())
	{
		const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
		ANPAICharacterBase* AICharacterBase = Cast<ANPAICharacterBase>(AIController->GetPawn());

		if (!AIController->ShouldPostponePathUpdates())
		{
			FRotator Rotator = GetRotationPoint(AICharacterBase, OwnerComp);
			float Yaw = Rotator.Yaw;

			FRotator rotator = FRotator(AICharacterBase->GetActorRotation().Pitch, Yaw, AICharacterBase->GetActorRotation().Roll);

			FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(rotator, AICharacterBase->GetActorRotation());
			float AbsDeltaRotator = UKismetMathLibrary::Abs(DeltaRotator.Yaw);

			float SpeedRotation = DeltaSeconds * AICharacterBase->GetNPCharacterMovement()->RotationSpeed;
			float Direction = 0;
			FRotator DesinateRotator = AICharacterBase->GetActorRotation();

			SpeedRotation = FMath::Clamp(SpeedRotation, MinSpeed, MaxSpeed);

			bool bIsRotating = true;

			if (DeltaRotator.Yaw > Precision)
			{
				DesinateRotator = UKismetMathLibrary::RLerp(AICharacterBase->GetActorRotation(), rotator, SpeedRotation, true);
				Direction = 1;
			}
			else if (DeltaRotator.Yaw < -Precision)
			{
				DesinateRotator = UKismetMathLibrary::RLerp(AICharacterBase->GetActorRotation(), rotator, SpeedRotation, true);
				Direction = -1;
			}
			else
			{
				bIsRotating = false;
			}

			FQuat LastRotation = AICharacterBase->GetActorRotation().Quaternion();
			FQuat CurrentRotation = DesinateRotator.Quaternion();
			FQuat DeltaRotation = CurrentRotation.Inverse() * LastRotation;
			float RotationAngle = FMath::RadiansToDegrees(DeltaRotation.AngularDistance(FQuat::Identity));

			AICharacterBase->SetActorRotation(DesinateRotator);

			if (bIsRotating)
			{
				AICharacterBase->GetNPCharacterMovement()->OnCharacterRotated.Broadcast(AICharacterBase, Direction, RotationAngle);
				FinishLatentTask(OwnerComp, EBTNodeResult::InProgress);
			}
			else
			{
				AICharacterBase->GetNPCharacterMovement()->OnCharacterRotated.Broadcast(AICharacterBase, Direction, 0);
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		}
	}
}

FRotator UBTTask_NPRotateCharacterTo::GetRotationPoint(ANPAICharacterBase* AICharacterBase, UBehaviorTreeComponent& OwnerComp)
{
	FRotator LookAtRotation = FRotator();
	if (KeyActorSelector.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		FVector LookAtLocation = OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Vector>(KeyActorSelector.GetSelectedKeyID());
		LookAtRotation = UKismetMathLibrary::FindLookAtRotation(AICharacterBase->GetActorLocation(), LookAtLocation);
	}
	else if (KeyActorSelector.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		if (AActor* Actor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Object>(KeyActorSelector.GetSelectedKeyID())))
		{
			if (bUseTargetRotation)
			{
				LookAtRotation = Actor->GetActorRotation();
			}
			else
			{
				LookAtRotation = UKismetMathLibrary::FindLookAtRotation(AICharacterBase->GetActorLocation(), Actor->GetActorLocation());
			}
		}
	}
	return LookAtRotation;
}






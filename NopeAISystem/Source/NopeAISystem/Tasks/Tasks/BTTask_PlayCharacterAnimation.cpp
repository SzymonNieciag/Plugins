// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_PlayCharacterAnimation.h"

//----------------------------------------------------------------------//
// UBTTask_PlayAnimation
//----------------------------------------------------------------------//
UBTTask_PlayCharacterAnimation::UBTTask_PlayCharacterAnimation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "PlayCharacterAnimation";
	// instantiating to be able to use Timers
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PlayCharacterAnimation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = EBTNodeResult::InProgress;

	/*AAIController* const MyController = OwnerComp.GetAIOwner();
	MyOwnerComp = &OwnerComp;

	float CurrentTime = 0.0f;
	bIsEnding = false;

	if (PlayTaskMontage)
	{
		APostapoBaseAIController* PostapoBaseAIController = Cast<APostapoBaseAIController>(OwnerComp.GetAIOwner());
		if (PostapoBaseAIController)
		{
			FAnimalTask AnimalTask = PostapoBaseAIController->GetCurrentActiveAITask();

			FTaskProgress TaskProgress = PostapoBaseAIController->GetTaskProgress(AnimalTask, TaskName);
			if (TaskProgress.TaskName != TaskName)
			{
				TaskProgress.TaskName = TaskName;
				TaskProgress.LeftTaskTime = AnimalTask.TimeTask;
				PostapoBaseAIController->UpdateTask(TaskProgress);
			}
			CurrentTime = TaskProgress.LeftTaskTime;
		}
		if (CurrentTime <= 0)
		{
			return EBTNodeResult::Succeeded;
		}
	}

	if (AnimMontage && MyController && MyController->GetPawn())
	{
		USkeletalMeshComponent* SkelMesh = nullptr;
		APostapoBaseAnimal* const PostapoBaseAnimal = Cast<APostapoBaseAnimal>(MyController->GetPawn());
		if (PostapoBaseAnimal)
		{
			SkelMesh = PostapoBaseAnimal->GetMesh();
		}

		if (SkelMesh == nullptr || SkelMesh->GetAnimInstance() == nullptr)
		{
			return EBTNodeResult::Succeeded;
		}

		if (!SkelMesh->GetAnimInstance()->OnMontageEnded.IsAlreadyBound(this, &UBTTask_PlayCharacterAnimation::OnMontageEnded))
		{
			SkelMesh->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &UBTTask_PlayCharacterAnimation::OnMontageEnded);
		}

		ACharacter* Character = Cast<ACharacter>(MyController->GetPawn());
		if (Character)
		{
			float Duration = Character->PlayAnimMontage(AnimMontage, 1);
			if (Duration == 0.0f)
			{
				return EBTNodeResult::Succeeded;
			}
		}
	}*/
	return Result;
}

EBTNodeResult::Type UBTTask_PlayCharacterAnimation::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	/*AAIController* const MyController = OwnerComp.GetAIOwner();
	USkeletalMeshComponent* SkelMesh = nullptr;
	ACharacter* const Character = Cast<ACharacter>(MyController->GetPawn());
	if (Character)
	{
		SkelMesh = Character->GetMesh();
		if (Character->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
		{
			if (AnimMontage)
			{
				FAnimMontageInstance* MontageInstance = Character->GetMesh()->GetAnimInstance()->GetActiveInstanceForMontage(AnimMontage);
				if (MontageInstance)
				{
					bool const bEndOfSection = (MontageInstance->GetPlayRate() < 0.f);
					bool const Success = MontageInstance->JumpToSectionName("End", bEndOfSection);
					if (Success)
					{
						TaskResult = EBTNodeResult::Aborted;
						Character->GetMesh()->GetAnimInstance()->Montage_JumpToSection("End", AnimMontage);
					}
					else
					{
						return EBTNodeResult::Aborted;
					}
				}
			}
		}
	}
	else
	{
		return EBTNodeResult::Aborted;
	}*/
	return EBTNodeResult::InProgress;
}

void UBTTask_PlayCharacterAnimation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	/*if (!OwnerComp.IsPaused() && PlayTaskMontage)
	{
		APostapoBaseAIController* PostapoBaseAIController = Cast<APostapoBaseAIController>(OwnerComp.GetAIOwner());
		FAnimalTask AnimalTask = PostapoBaseAIController->GetCurrentActiveAITask();
		FTaskProgress TaskProgress = PostapoBaseAIController->GetTaskProgress(AnimalTask, TaskName);
		if (TaskProgress.TaskName == TaskName)
		{
			TaskProgress.LeftTaskTime -= DeltaSeconds;
			PostapoBaseAIController->UpdateTask(TaskProgress);
		}

		if (!bIsEnding && TaskProgress.LeftTaskTime <= 0)
		{
			USkeletalMeshComponent* SkelMesh = nullptr;
			APostapoBaseAnimal* const PostapoBaseAnimal = Cast<APostapoBaseAnimal>(PostapoBaseAIController->GetPawn());
			if (PostapoBaseAnimal)
			{
				bIsEnding = true;
				if (PostapoBaseAnimal->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
				{
					TaskResult = EBTNodeResult::Succeeded;
					PostapoBaseAnimal->GetMesh()->GetAnimInstance()->Montage_JumpToSection("End", AnimMontage);
				}
				else
				{
					FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
				}
			}
		}
	}*/
}

void UBTTask_PlayCharacterAnimation::OnMontageEnded(class UAnimMontage* AnimInstance, bool bInterrupted)
{
	if (MyOwnerComp)
	{
		if (bInterrupted)
		{
			FinishLatentTask(*MyOwnerComp, EBTNodeResult::Failed);
		}
		else
		{
			FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
		}
	}
}

#if WITH_EDITOR

FName UBTTask_PlayCharacterAnimation::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Task.PlaySound.Icon");
}

#endif	// WITH_EDITOR

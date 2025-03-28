// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_NPWaitTask.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include <AIController.h>
#include "NopeAISystem/Characters/NPAICharacterBase.h"
#include "NopeAISystem/Tasks/NPAITaskBase.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include <Animation/AnimInstance.h>

//----------------------------------------------------------------------//
// UBTTask_PlayAnimation
//----------------------------------------------------------------------//
UBTTask_NPWaitTask::UBTTask_NPWaitTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "BTTask_NPWaitTask";
	// instantiating to be able to use Timers
	bCreateNodeInstance = true;
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_NPWaitTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_NPWaitTask::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* const MyController = OwnerComp.GetAIOwner();
	ACharacter* const Character = Cast<ACharacter>(MyController->GetPawn());
	MyOwnerComp = &OwnerComp;
	if (!Character)
	{
		return EBTNodeResult::Aborted;
	}
	if (Character->GetMesh()->GetAnimInstance() && Character->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
	{
		USkeletalMeshComponent* SkelMesh = Character->GetMesh();
		UAnimInstance* AnimalAnimInstance = Cast<UAnimInstance>(SkelMesh->GetAnimInstance());
		if (!AnimalAnimInstance)
		{
			return EBTNodeResult::Aborted;
		}
		if (UAnimMontage* CurAnimMontage = AnimalAnimInstance->GetCurrentActiveMontage())
		{
			FName CurrentSection = AnimalAnimInstance->Montage_GetCurrentSection(CurAnimMontage);
			const int32 SectionID = CurAnimMontage->GetSectionIndex("End");

			if (!SkelMesh->GetAnimInstance()->OnMontageEnded.IsAlreadyBound(this, &UBTTask_NPWaitTask::OnMontageEnded))
			{
				SkelMesh->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &UBTTask_NPWaitTask::OnMontageEnded);
			}

			if (!CurrentSection.IsEqual("End"))
			{
				if (CurAnimMontage->IsValidSectionIndex(SectionID))
				{
					FAnimMontageInstance* MontageInstance = AnimalAnimInstance->GetActiveInstanceForMontage(CurAnimMontage);
					if (!MontageInstance)
					{
						AnimalAnimInstance->Montage_Play(CurAnimMontage);
						AnimalAnimInstance->Montage_JumpToSection("End", CurAnimMontage);
						AnimalAnimInstance->Montage_SetPlayRate(CurAnimMontage, MontageAbortSpeed);
					}
					else
					{
						AnimalAnimInstance->Montage_SetPlayRate(CurAnimMontage, MontageAbortSpeed);
						AnimalAnimInstance->Montage_JumpToSection("End", CurAnimMontage);
					}
				}
				else
				{
					AnimalAnimInstance->Montage_Stop(0, CurAnimMontage);
				}
			}
		}
		else
		{
			return EBTNodeResult::Aborted;
		}
		return EBTNodeResult::InProgress;
	}
	return EBTNodeResult::Aborted;
}

void UBTTask_NPWaitTask::OnMontageEnded(class UAnimMontage* Montage, bool bInterrupted)
{
	if (MyOwnerComp)
	{
		if (AAIController* PostapoBaseAIController = Cast<AAIController>(MyOwnerComp->GetAIOwner()))
		{
			if(ANPAICharacterBase* const PostapoBaseAnimal = Cast<ANPAICharacterBase>(PostapoBaseAIController->GetPawn()))
			{
				if (PostapoBaseAnimal->GetMesh()->GetAnimInstance()->OnMontageEnded.IsAlreadyBound(this, &UBTTask_NPWaitTask::OnMontageEnded))
				{
					PostapoBaseAnimal->GetMesh()->GetAnimInstance()->OnMontageEnded.RemoveDynamic(this, &UBTTask_NPWaitTask::OnMontageEnded);
				}
				FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
			}
		}
	}
}
// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_PlayAnimationMontage.h"
#include "VisualLogger/VisualLogger.h"
#include "AIController.h"
#include <Animation/AnimInstance.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include "Animation/AnimMontage.h"
#include "NopeAISystem/Tasks/NPAITaskBase.h"
#include "NopeAISystem/Characters/NPAICharacterBase.h"
#include "NopeAISystem/AI/NPAIDefines.h"
#include <Animation/AnimInstance.h>
#include "NopeAISystem/AI/NPAIControllerBase.h"

//----------------------------------------------------------------------//
// UBTTask_PlayAnimation
//----------------------------------------------------------------------//
UBTTask_PlayAnimationMontage::UBTTask_PlayAnimationMontage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "PlayAnimationMontage";
	// instantiating to be able to use Timers
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PlayAnimationMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* const MyController = OwnerComp.GetAIOwner();
	MyOwnerComp = &OwnerComp;
	EBTNodeResult::Type Result = EBTNodeResult::InProgress;
	TaskResult = EBTNodeResult::Succeeded;

	bIsEnding = false;
	bIgnoreAbort = false;
	//AmountOfCall = 0;
	ACharacter* Character = Cast<ACharacter>(MyController->GetPawn());
	USkeletalMeshComponent* SkelMesh = Character->GetMesh();
	if (Character == nullptr)
	{
		return EBTNodeResult::Succeeded;
	}
	if (SkelMesh == nullptr || SkelMesh->GetAnimInstance() == nullptr)
	{
		return EBTNodeResult::Succeeded;
	}
	if (SkelMesh->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
	{
		return EBTNodeResult::Succeeded;
	}
	if (SkelMesh->GetAnimInstance()->IsAnyMontagePlaying())
	{
		return EBTNodeResult::Failed;
	}
	if (MontagesToPlay.Num() == 0)
	{
		return EBTNodeResult::Succeeded;
	}

	if (bUseTimeAnimation)
	{
		LeftAnimationTime = FMath::FRandRange(MinTime, MaxTime);
	}

	int Index = UKismetMathLibrary::RandomIntegerInRange(0, MontagesToPlay.Num() - 1);
	class UAnimMontage* ActiveMontage = MontagesToPlay[Index];
	if (ActiveMontage)
	{
		if (SkelMesh->GetAnimInstance()->IsAnyMontagePlaying())
		{
			SkelMesh->GetAnimInstance()->StopAllMontages(0.0f);
			return EBTNodeResult::Failed;
		}

		if (!SkelMesh->GetAnimInstance()->OnMontageBlendingOut.IsAlreadyBound(this, &UBTTask_PlayAnimationMontage::OnMontageEnded))
		{
			SkelMesh->GetAnimInstance()->OnMontageBlendingOut.AddDynamic(this, &UBTTask_PlayAnimationMontage::OnMontageEnded);
		}
		float Duration = SkelMesh->GetAnimInstance()->Montage_Play(ActiveMontage, MontageSpeed, EMontagePlayReturnType::MontageLength, StartTimeSeconds);
		if (Duration == 0.0f)
		{
			return EBTNodeResult::Succeeded;
		}
	}
	return Result;
}

EBTNodeResult::Type UBTTask_PlayAnimationMontage::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* const MyController = OwnerComp.GetAIOwner();
	ACharacter* const Character = Cast<ACharacter>(MyController->GetPawn());
	if (!Character)
	{
		return EBTNodeResult::Aborted;
	}
	bIsEnding = true;
	USkeletalMeshComponent* SkelMesh = Character->GetMesh();
	if (Character->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
	{
		UAnimInstance* AnimInstance = Cast<UAnimInstance>(Character->GetMesh()->GetAnimInstance());
		if (!AnimInstance)
		{
			return EBTNodeResult::Aborted;
		}
		if (UAnimMontage* CurAnimMontage = AnimInstance->GetCurrentActiveMontage())
		{
			FName CurrentSection = AnimInstance->Montage_GetCurrentSection(CurAnimMontage);
			const int32 SectionID = CurAnimMontage->GetSectionIndex("End");
			if (!CurrentSection.IsEqual("End"))
			{
				if (CurAnimMontage->IsValidSectionIndex(SectionID))
				{
					FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(CurAnimMontage);
					if (!MontageInstance)
					{
						bIgnoreAbort = true;
						AnimInstance->Montage_Play(CurAnimMontage);
						AnimInstance->Montage_JumpToSection("End", CurAnimMontage);
						AnimInstance->Montage_SetPlayRate(CurAnimMontage, AbortMontageSpeed);
					}
					else
					{
						AnimInstance->Montage_SetPlayRate(CurAnimMontage, AbortMontageSpeed);
						AnimInstance->Montage_JumpToSection("End", CurAnimMontage);
					}
				}
				else
				{
					AnimInstance->Montage_Stop(0, CurAnimMontage);
				}
			}
		}
		else
		{
			return EBTNodeResult::Aborted;
		}
		TaskResult = EBTNodeResult::Aborted;
		return EBTNodeResult::InProgress;
	}
	return EBTNodeResult::Aborted;
}

void UBTTask_PlayAnimationMontage::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!OwnerComp.IsPaused() && !bIsEnding)
	{
		ANPAIControllerBase* NPAIControllerBase = Cast<ANPAIControllerBase>(OwnerComp.GetAIOwner());
		if (bUseTimeAnimation)
		{
			LeftAnimationTime -= DeltaSeconds;
			if (LeftAnimationTime <= 0)
			{
				ACharacter* const Character = Cast<ACharacter>(NPAIControllerBase->GetPawn());
				if (Character)
				{
					bIsEnding = true;
					if (Character->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
					{
						if (UAnimMontage* CurAnimMontage = Character->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage())
						{
							TaskResult = EBTNodeResult::Succeeded;
							UAnimInstance* AnimInstance = Cast<UAnimInstance>(Character->GetMesh()->GetAnimInstance());

							const int32 SectionID = CurAnimMontage->GetSectionIndex("End");

							if (CurAnimMontage->IsValidSectionIndex(SectionID))
							{
								FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(CurAnimMontage);
								if (!MontageInstance)
								{
									bIgnoreAbort = true;
									AnimInstance->Montage_Play(CurAnimMontage);
									AnimInstance->Montage_JumpToSection("End", CurAnimMontage);
									AnimInstance->Montage_SetPlayRate(CurAnimMontage, AbortMontageSpeed);
								}
								else
								{
									bIgnoreAbort = true;
									AnimInstance->Montage_SetPlayRate(CurAnimMontage, AbortMontageSpeed);
									AnimInstance->Montage_JumpToSection("End", CurAnimMontage);
								}
							}
							else
							{
								AnimInstance->Montage_Stop(0, CurAnimMontage);
							}
						}
					}
					else
					{
						if (Character->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.IsAlreadyBound(this, &UBTTask_PlayAnimationMontage::OnMontageEnded))
						{
							Character->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.RemoveDynamic(this, &UBTTask_PlayAnimationMontage::OnMontageEnded);
						}
						FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
					}
				}
			}
		}
	}
}

void UBTTask_PlayAnimationMontage::PostLoad()
{
	Super::PostLoad();
}

void UBTTask_PlayAnimationMontage::OnMontageEnded(class UAnimMontage* AnimInstance, bool bInterrupted)
{
	if (MyOwnerComp && !bIgnoreAbort)
	{
		AAIController* AIController = Cast<AAIController>(MyOwnerComp->GetAIOwner());
		if (AIController)
		{
			ACharacter* const Character = Cast<ACharacter>(AIController->GetPawn());
			if (Character)
			{
				if (bInterrupted)
				{
					if (Character->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.IsAlreadyBound(this, &UBTTask_PlayAnimationMontage::OnMontageEnded))
					{
						Character->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.RemoveDynamic(this, &UBTTask_PlayAnimationMontage::OnMontageEnded);
					}
					FinishLatentTask(*MyOwnerComp, EBTNodeResult::Failed);
				}
				else
				{
					if (Character->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.IsAlreadyBound(this, &UBTTask_PlayAnimationMontage::OnMontageEnded))
					{
						Character->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.RemoveDynamic(this, &UBTTask_PlayAnimationMontage::OnMontageEnded);
					}
					FinishLatentTask(*MyOwnerComp, TaskResult);
				}
			}
		}
	}
	else
	{
		bIgnoreAbort = false;
	}
}
#if WITH_EDITOR
FName UBTTask_PlayAnimationMontage::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Task.PlaySound.Icon");
}
#endif	// WITH_EDITOR

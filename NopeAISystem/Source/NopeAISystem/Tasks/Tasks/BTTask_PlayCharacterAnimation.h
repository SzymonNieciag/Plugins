// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PlayCharacterAnimation.generated.h"

/**
 *	Play indicated AnimationAsset on Pawn controlled by BT 
 *	Note that this node is generic and is handing multiple special cases,
 *	If you want a more efficient solution you'll need to implement it yourself (or wait for our BTTask_PlayCharacterAnimation)
 */
UCLASS()
class NOPEAISYSTEM_API UBTTask_PlayCharacterAnimation : public UBTTaskNode
{
	GENERATED_UCLASS_BODY()

	/** Animation asset to play. Note that it needs to match the skeleton of pawn this BT is controlling */
	UPROPERTY(Category = Node, EditAnywhere)
	class UAnimMontage* AnimMontage;

	UPROPERTY(Category = Node, EditAnywhere)
	bool PlayTaskMontage{true};

	UPROPERTY()
	UBehaviorTreeComponent* MyOwnerComp;

	bool bIsEnding = false;

	EBTNodeResult::Type TaskResult = EBTNodeResult::Aborted;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
protected:
	UFUNCTION()
	void OnMontageEnded(class UAnimMontage* Montage, bool bInterrupted);

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR
};

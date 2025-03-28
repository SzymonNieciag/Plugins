// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PlayAnimationMontage.generated.h"

/**
 *	Play indicated AnimationAsset on Pawn controlled by BT 
 *	Note that this node is generic and is handing multiple special cases,
 *	If you want a more efficient solution you'll need to implement it yourself (or wait for our BTTask_PlayCharacterAnimation)
 */
UCLASS()
class NOPEAISYSTEM_API UBTTask_PlayAnimationMontage : public UBTTaskNode
{
	GENERATED_UCLASS_BODY()

	/** Animation asset to play. Note that it needs to match the skeleton of pawn this BT is controlling */
	UPROPERTY(Category = Node, EditAnywhere)
	TArray<class UAnimMontage*> MontagesToPlay;

	UPROPERTY(Category = Node, EditAnywhere)
	float MontageSpeed = 1.0f;

	UPROPERTY(Category = Node, EditAnywhere)
	float StartTimeSeconds = 0.0f;

	UPROPERTY(Category = Node, EditAnywhere)
	float AbortMontageSpeed = 1.0f;

	UPROPERTY(Category = Node, EditAnywhere)
	bool bUseTimeAnimation = false;

	UPROPERTY(Category = Node, EditAnywhere, meta = (EditCondition = "bUseTimeAnimation", EditConditionHides))
	float MinTime = 5;

	UPROPERTY(Category = Node, EditAnywhere, meta = (EditCondition = "bUseTimeAnimation", EditConditionHides))
	float MaxTime = 10;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** called when node instance is added to tree */
	virtual void PostLoad() override;
	
protected:
	UFUNCTION()
	void OnMontageEnded(class UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY()
	UBehaviorTreeComponent* MyOwnerComp;
	UPROPERTY()
	bool bIsEnding = false;
	UPROPERTY()
	bool bIgnoreAbort = false;
	//int AmountOfCall = 0;
	EBTNodeResult::Type TaskResult = EBTNodeResult::Succeeded;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR

private:
	float LeftAnimationTime = 0.0f;
};

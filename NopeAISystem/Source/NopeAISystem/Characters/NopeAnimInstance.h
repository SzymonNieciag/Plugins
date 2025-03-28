// DRAGO Entertainment (c) 2020

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NopeAnimInstance.generated.h"

UCLASS()
class NOPEAISYSTEM_API UNopeAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void SaveAnimInstance(struct FAnimationSaveStruct& OutAnimationSaveStruct);
	virtual void LoadAnimInstance(const struct FAnimationSaveStruct& OutAnimationSaveStruct);

public:
	UFUNCTION(BlueprintCallable)
	void SetStartEndCharacterSnapTransform(const FTransform& InStart, const FTransform& InEnd);
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "NopeAnimInstance")
	FTransform StartCharacterTransform;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "NopeAnimInstance")
	FTransform EndCharacterTransform;
};

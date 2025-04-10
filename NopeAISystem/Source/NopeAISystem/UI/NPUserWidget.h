// Copyright © 2022 DRAGO Entertainment. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NPUserWidget.generated.h"

UCLASS()
class NOPEAISYSTEM_API UNPUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable)
	void SetOwnerObject(class UObject* InOwnerObject);
	UFUNCTION(BlueprintPure)
	UObject* GetOwnerObject() {	return OwnerObject; };

protected:
	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn=true))
	class UObject* OwnerObject = nullptr;
};
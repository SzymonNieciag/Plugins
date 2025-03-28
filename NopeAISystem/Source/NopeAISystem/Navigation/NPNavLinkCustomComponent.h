// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "../../../../../../../Source/Runtime/NavigationSystem/Public/NavLinkCustomComponent.h"
#include "NPNavLinkCustomComponent.generated.h"


UCLASS(MinimalAPI)
class UNPNavLinkCustomComponent : public UNavLinkCustomComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AcceptableRadius = 0;
};


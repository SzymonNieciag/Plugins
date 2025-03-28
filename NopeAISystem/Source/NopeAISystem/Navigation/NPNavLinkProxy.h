// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Navigation/NavLinkProxy.h"
#include "NPNavLinkProxy.generated.h"

UCLASS(Blueprintable, autoCollapseCategories = (SmartLink, Actor), hideCategories = (Input), MinimalAPI)
class ANPNavLinkProxy : public ANavLinkProxy
{
	GENERATED_BODY()

	ANPNavLinkProxy(const FObjectInitializer& ObjectInitializer);

	static FName NPNavLinkProxyName;
};


// Copyright Epic Games, Inc. All Rights Reserved.

#include "NPNavLinkProxy.h"
#include "NPNavLinkCustomComponent.h"

FName ANPNavLinkProxy::NPNavLinkProxyName(TEXT("SmartLinkComp"));


ANPNavLinkProxy::ANPNavLinkProxy(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UNPNavLinkCustomComponent>(ANPNavLinkProxy::NPNavLinkProxyName))
{
	
}

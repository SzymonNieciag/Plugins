// Copyright © 2022 DRAGO Entertainment. All Rights Reserved.

#include "NPWidgetComponentFace.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NPUserWidget.h"
#include "../NPAISubsystem.h"


void UNPWidgetComponentFace::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (GetUserWidgetObject())
	{
		if (APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(this->GetComponentLocation(), PlayerCameraManager->GetTransformComponent()->GetComponentLocation());
			this->SetWorldRotation(Rotator);
		}
	}
}

void UNPWidgetComponentFace::InitWidget()
{
	Super::InitWidget();
	if (UNPUserWidget* UserWidget = Cast<UNPUserWidget>(GetUserWidgetObject()))
	{
		UserWidget->SetOwnerObject(this->GetOwner());
	}
}

void UNPWidgetComponentFace::OnDebugUIEnabled(const bool DebugAIEnaled)
{
	this->SetHiddenInGame(!DebugAIEnaled);
}

void UNPWidgetComponentFace::BeginPlay()
{
	Super::BeginPlay();
	if (UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(this))
	{
		OnDebugUIEnabled(NPAISubsystem->AIDebugSystemEnabled(this));
		NPAISubsystem->OnDebugAIEnabled.AddDynamic(this, &UNPWidgetComponentFace::OnDebugUIEnabled);
	}
}

// Copyright © 2022 DRAGO Entertainment. All Rights Reserved.


#include "AITargetPoint.h"
#include <Components/ArrowComponent.h>
#include "NopeAISystem/NPAISubsystem.h"
#include "NopeAISystem/Characters/NPAICharacterBase.h"
#include <Engine/EngineTypes.h>
#include "Components/NPPathFollowingComponent.h"
#include "NPAIControllerBase.h"

// Sets default values
AAITargetPoint::AAITargetPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	this->SetRootComponent(ArrowComponent);

#if WITH_EDITOR
	ArrowComponent->bHiddenInGame = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
#endif
}

// Called when the game starts or when spawned
void AAITargetPoint::BeginPlay()
{
	Super::BeginPlay();
	OwnerAICharacter = Cast<ANPAICharacterBase>(GetOwner());
	UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(this);
	if (NPAISubsystem)
	{
		if (ArrowComponent->bHiddenInGame == true)
		{
			OnAIDebugChanged(UNPAISubsystem::AIDebugSystemEnabled(this));
			NPAISubsystem->OnDebugAIEnabled.AddDynamic(this, &AAITargetPoint::OnAIDebugChanged);
		}
	}
	if (OwnerAICharacter)
	{
		OwnerAICharacter->OnEndPlay.AddDynamic(this, &AAITargetPoint::EndPlayAITargetPoint);
	}
}

// Called every frame
void AAITargetPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAITargetPoint::SetFollowActor(TSoftObjectPtr<AActor> InFollowActor)
{
	FollowActor = InFollowActor;
	if (FollowActor.LoadSynchronous())
	{
		FName SocketName;
		this->AttachToActor(FollowActor.LoadSynchronous(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false), SocketName);
	}
	else
	{
		this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void AAITargetPoint::SetAllowBackMovement(bool InAllowBackMovement)
{
	bAllowBackMovement = InAllowBackMovement;
}

void AAITargetPoint::SetAcceptableRadius(float InAcceptableRadius)
{
	AcceptableRadius = InAcceptableRadius;
}

void AAITargetPoint::UpdateMovementDirection(ENPAIMovementDirection CurrentMovementDirection, float DeltaTime)
{
	if (MovementDirection != CurrentMovementDirection)
	{
		LeftDirectionCooldown = DirectionCooldown;
		MovementDirection = CurrentMovementDirection;
	}
	else
	{
		LeftDirectionCooldown -= DeltaTime;
	}
}

void AAITargetPoint::ResetMovementDirection()
{
	MovementDirection = ENPAIMovementDirection::Forward;
	LeftDirectionCooldown = 0.0f;
}

const bool AAITargetPoint::AllowChangeMovementDirection()
{
	if (LeftDirectionCooldown <= 0)
		return true;
	return false;
}

bool AAITargetPoint::IsTargetPointReached(float& MissingDistance)
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (ANPAIControllerBase* NPAIControllerBase = Cast<ANPAIControllerBase>(Character->GetController()))
		{
			if (UNPPathFollowingComponent* NPPathFollowingComponent = Cast<UNPPathFollowingComponent>(NPAIControllerBase->GetPathFollowingComponent()))
			{
				if (NPPathFollowingComponent->IsPointReached(this->GetActorLocation(), 0, 0, Character->GetActorLocation(), AcceptableRadius))
					return true;
			}
		}
	}
	return false;
}

void AAITargetPoint::EndPlayAITargetPoint(class AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(this);
	if (NPAISubsystem)
	{
		NPAISubsystem->OnDebugAIEnabled.RemoveDynamic(this, &AAITargetPoint::OnAIDebugChanged);
	}
	this->Destroy();
}

void AAITargetPoint::OnAIDebugChanged(bool AIEnabled)
{
	ArrowComponent->SetHiddenInGame(!AIEnabled);
}

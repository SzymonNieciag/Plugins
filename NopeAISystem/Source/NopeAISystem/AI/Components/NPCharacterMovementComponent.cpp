#include "NPCharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "../AITargetPoint.h"
#include "NopeAISystem/NPAISystemSettings.h"
#include "NopeAISystem/Characters/NPAICharacterBase.h"
#include "../NPAIControllerBase.h"
#include <Curves/CurveVector.h>
#include <Kismet/KismetMathLibrary.h>

UNPCharacterMovementComponent::UNPCharacterMovementComponent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/) : Super(ObjectInitializer)
{
	for (int i = 0; i < (int)ENPAIMovementSpeed::MAX; i++)
	{
		GetSpeedValue[i] = 100.0f;
	}
	if (const UNPAISystemSettings* AISystemSettings = GetDefault<UNPAISystemSettings>())
	{
		AITargetPointSoftClass = AISystemSettings->AITargetPointSoftClass;
	}
	bOrientRotationToMovement = true;
	bUseSeparateBrakingFriction = true;
	BrakingFrictionFactor = 1.0f;
	BrakingFriction = 10.0f;
	RotationRate = FRotator(0, 120, 0);
}

void UNPCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	CreateAITargetPoint();
	SetCharacterMovementSpeed(ENPAIMovementSpeed::Jog);
}

void UNPCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementSettings.MovementSpeed)
	{	
		const float SpeedWeight = GetMovementSpeedWeight();
		this->MaxAcceleration = MovementSettings.MovementSpeed->GetVectorValue(SpeedWeight).X;
		this->BrakingFrictionFactor = MovementSettings.MovementSpeed->GetVectorValue(SpeedWeight).Y;
		this->BrakingFriction = MovementSettings.MovementSpeed->GetVectorValue(SpeedWeight).Z;
	}
}

void UNPCharacterMovementComponent::SetCharacterMovementSpeed(ENPAIMovementSpeed NewAIMovementSpeed)
{
	if (AIMovementSpeed == NewAIMovementSpeed)
	{
		return;
	}
	OnCharacterSpeedChanged.Broadcast(NewAIMovementSpeed);
	AIMovementSpeed = NewAIMovementSpeed;
	uint8 byte = (uint8)NewAIMovementSpeed;
	this->MaxWalkSpeed = GetSpeedValue[byte];
}


void UNPCharacterMovementComponent::SetAIMovementMode(ENPAIMovementMode NewNPAIMovementMode)
{
	AIMovementMode = NewNPAIMovementMode;
}


void UNPCharacterMovementComponent::SetCurrentMovementDirection(ENPAIMovementDirection NewNPAIMovementDirection)
{
	MovementDirection = NewNPAIMovementDirection;
}

FRotator UNPCharacterMovementComponent::ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const
{
	FRotator Rotator = Super::ComputeOrientToMovementRotation(CurrentRotation, DeltaTime, DeltaRotation);
	if (ANPAICharacterBase* AICharacterBase = Cast<ANPAICharacterBase>(GetOwner()))
	{
		if (AICharacterBase->GetNPCharacterMovement()->GetCurrentMovementDirection() == ENPAIMovementDirection::Back)
		{
			Rotator = AITargetPoint->GetActorRotation();
		}
	}
	return Rotator;
}

void UNPCharacterMovementComponent::SetAITargetPoint(AActor* AttachedActor, FTransform GoalTransform, bool AllowBackMovement /*= true*/, bool AllowRotation /*= true*/, bool InRotateDirectToPoint /*= false*/)
{
	if (AITargetPoint) {
		FRotator X = AITargetPoint->GetTransform().Rotator();
		FRotator Y = GoalTransform.Rotator();

		if (!AITargetPoint->GetTransform().Equals(GoalTransform))
		{
			if (AITargetPoint->GetAttachParentActor())
			{
				AITargetPoint->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			}
			if (AttachedActor)
			{
				AITargetPoint->AttachToActor(AttachedActor, FAttachmentTransformRules::KeepWorldTransform);
			}
			AITargetPoint->SetActorTransform(GoalTransform);
			AITargetPoint->SetAllowBackMovement(AllowBackMovement);
			AITargetPoint->AllowRotation = AllowRotation;
			AITargetPoint->RotateDirectToPoint = InRotateDirectToPoint;
			AITargetPoint->ResetMovementDirection();
		}
	}
}

class AAITargetPoint* UNPCharacterMovementComponent::GetAITargetPoint()
{
	return AITargetPoint;
}

void UNPCharacterMovementComponent::CreateAITargetPoint()
{
	TRACE_BOOKMARK(TEXT("CreateAITargetPoint"));

	UWorld* const World = GetWorld();
	UClass* LoadedClass = AITargetPointSoftClass.LoadSynchronous();
	if (World && LoadedClass)
	{
		//spawn the actor
		AITargetPoint = World->SpawnActorDeferred<AAITargetPoint>(
			LoadedClass, FTransform(), GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		// initialize NewActor...
		UGameplayStatics::FinishSpawningActor(AITargetPoint, FTransform());
	}
}

float UNPCharacterMovementComponent::GetMovementSpeedWeight()
{
	float CurSpeed = Velocity.Size();
	if (CurSpeed > 0)
	{
		float PrevMovementSpeed = 0;
		for (int i = 0; i < (int)ENPAIMovementSpeed::MAX; i++)
		{
			float CurMovementSpeed = GetSpeedValue[i];
			if (CurMovementSpeed >= CurSpeed)
			{
				float IndexValue = UKismetMathLibrary::MapRangeClamped(CurSpeed, PrevMovementSpeed, CurMovementSpeed, i, i + 1);
				return IndexValue;
			}
			PrevMovementSpeed = GetSpeedValue[i];
		}
	}
	return 0;
}

//#include "NPCharacterMovementComponent.h"
//#include <NavigationSystem/Public/NavigationSystem.h>
//#include <NavigationSystem/Public/NavigationPath.h>
//#include <Kismet/GameplayStatics.h>
//#include "../AITargetPoint.h"
//#include "NopeAISystem/NPAISystemSettings.h"
//
//UNPCharacterMovementComponent::UNPCharacterMovementComponent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/) : Super(ObjectInitializer)
//{
//	for (int i = 0; i < (int)ENPAIMovementSpeed::MAX; i++)
//	{
//		GetSpeedValue[i] = 100.0f;
//	}
//	if (const UNPAISystemSettings* AISystemSettings = GetDefault<UNPAISystemSettings>())
//	{
//		AITargetPointSoftClass = AISystemSettings->AITargetPointSoftClass;
//	}
//}
//
//void UNPCharacterMovementComponent::BeginPlay()
//{
//	Super::BeginPlay();
//	CreateAITargetPoint();
//}
//
//bool UNPCharacterMovementComponent::DoJump(bool ReplayingMoves)
//{
//	//GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor(0, 200, 20), FString::Printf(TEXT("%s velocity: %s, %f"), *GetNameSafe(this), *Velocity.ToCompactString(), Velocity.Length()), true, FVector2D(1.25f, 1.25f));
//	// 400 run 800 sprint 300 walk
//	if (Velocity.Length() > MinimumVelocityLengthToApplyScaling)
//	{
//		float VDesiredLength = MinimumVelocityLengthToApplyScaling + (Velocity.Length() - MinimumVelocityLengthToApplyScaling) * ExceedingVelocityScaling;
//		FVector VelocityScaled = VDesiredLength * (Velocity / Velocity.Length());
//		Velocity = VelocityScaled;
//	}
//	
//	//GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor(0, 200, 20), FString::Printf(TEXT("%s scaled velocity: %s, %f"), *GetNameSafe(this), *Velocity.ToCompactString(), Velocity.Length()), true, FVector2D(1.25f, 1.25f));
//
//	return Super::DoJump(ReplayingMoves);
//}
//
//FVector UNPCharacterMovementComponent::GetFallingLateralAcceleration(float DeltaTime)
//{
//	// No acceleration in Z
//	FVector FallAcceleration = FVector(FallingLateralAccelerationMultiplier * Acceleration.X, FallingLateralAccelerationMultiplier * Acceleration.Y, 0.f);
//
//	// bound acceleration, falling object has minimal ability to impact acceleration
//	if (!HasAnimRootMotion() && FallAcceleration.SizeSquared2D() > 0.f)
//	{
//		FallAcceleration = GetAirControl(DeltaTime, AirControl, FallAcceleration);
//		FallAcceleration = FallAcceleration.GetClampedToMaxSize(GetMaxAcceleration());
//	}
//
//	return FallAcceleration;
//}
//
//void UNPCharacterMovementComponent::SetCharacterMovementSpeed(ENPAIMovementSpeed NewAIMovementSpeed)
//{
//	if (AIMovementSpeed == NewAIMovementSpeed)
//	{
//		return;
//	}
//	AIMovementSpeed = NewAIMovementSpeed;
//	uint8 byte = (uint8)NewAIMovementSpeed;
//	this->MaxWalkSpeed = GetSpeedValue[byte];
//}
//
//void UNPCharacterMovementComponent::SetAITargetPoint(AActor* AttachedActor, FTransform GoalTransform, bool AllowBackMovement /*= true*/, bool AllowRotation /*= true*/, bool AllowBrake /*= true*/, bool InRotateDirectToPoint /*= false*/)
//{
//	if (AITargetPoint) {
//		FRotator X = AITargetPoint->GetTransform().Rotator();
//		FRotator Y = GoalTransform.Rotator();
//		AITargetPoint->AllowBrake = AllowBrake;
//
//		if (!AITargetPoint->GetTransform().Equals(GoalTransform))
//		{
//			if (AITargetPoint->GetAttachParentActor())
//			{
//				AITargetPoint->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
//			}
//			if (AttachedActor)
//			{
//				AITargetPoint->AttachToActor(AttachedActor, FAttachmentTransformRules::KeepWorldTransform);
//			}
//			AITargetPoint->SetActorTransform(GoalTransform);
//			AITargetPoint->SetAllowBackMovement(AllowBackMovement);
//			AITargetPoint->AllowRotation = AllowRotation;
//			AITargetPoint->RotateDirectToPoint = InRotateDirectToPoint;
//			AITargetPoint->ResetMovementDirection();
//		}
//	}
//}
//
//class AAITargetPoint* UNPCharacterMovementComponent::GetAITargetPoint()
//{
//	return AITargetPoint;
//}
//
//void UNPCharacterMovementComponent::CreateAITargetPoint()
//{
//	UWorld* const World = GetWorld();
//	UClass* LoadedClass = AITargetPointSoftClass.LoadSynchronous();
//	if (World && LoadedClass)
//	{
//		//spawn the actor
//		AITargetPoint = World->SpawnActorDeferred<AAITargetPoint>(
//			LoadedClass, FTransform(), GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
//
//		// initialize NewActor...
//		UGameplayStatics::FinishSpawningActor(AITargetPoint, FTransform());
//	}
//}
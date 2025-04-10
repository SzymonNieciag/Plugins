//Copyright © 2022 DRAGO Entertainment.All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NopeAISystem/AI/NPAIDefines.h"
#include "NPCharacterMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCharacterRotate, class ANPAICharacterBase*, InNPAICharacterBase, const float, Direction_Z, const float, RotateValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterSpeedSignature, const ENPAIMovementSpeed, OutAIMovementSpeed);

USTRUCT(BlueprintType)
struct NOPEAISYSTEM_API FMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory)
	class UCurveVector* MovementSpeed;
};

UCLASS(BlueprintType)
class NOPEAISYSTEM_API UNPCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UNPCharacterMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FCharacterRotate OnCharacterRotated;

	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FCharacterSpeedSignature OnCharacterSpeedChanged;
	
protected:
	ENPAIMovementSpeed AIMovementSpeed = ENPAIMovementSpeed::MAX;
	ENPAIMovementMode AIMovementMode = ENPAIMovementMode::Walking;

	UPROPERTY(EditAnywhere, meta = (ArraySizeEnum = "EAnimalMovementMode"))
	float GetSpeedValue[(int32)ENPAIMovementSpeed::MAX];

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ENPAIMovementDirection MovementDirection = ENPAIMovementDirection::Forward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMovementSettings MovementSettings;

public:
	void SetCharacterMovementSpeed(ENPAIMovementSpeed NewAIMovementSpeed);
	FORCEINLINE ENPAIMovementSpeed GetCharacterMovementSpeed() { return AIMovementSpeed; }

	void SetAIMovementMode(ENPAIMovementMode NewNPAIMovementMode);
	FORCEINLINE ENPAIMovementMode GetAIMovementMode() { return AIMovementMode; }

	void SetCurrentMovementDirection(ENPAIMovementDirection NewNPAIMovementDirection);

	FORCEINLINE ENPAIMovementDirection GetCurrentMovementDirection() { return MovementDirection; }

public:
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RotationSpeed = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float BackWalkReactDist = 800.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RotateToPointReactDist = 300.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float ForwardDirectionWeight = 1.0f;

	UFUNCTION(BlueprintPure)
	float GetMovementSpeedWeight();

#pragma region AITargetPoint

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Controller|TargetPoint")
	TSoftClassPtr<class AAITargetPoint> AITargetPointSoftClass;

	/**
  * Compute a target rotation based on current movement. Used by PhysicsRotation() when bOrientRotationToMovement is true.
  * Default implementation targets a rotation based on Acceleration.
  *
  * @param CurrentRotation	- Current rotation of the Character
  * @param DeltaTime		- Time slice for this movement
  * @param DeltaRotation	- Proposed rotation change based simply on DeltaTime * RotationRate
  *
  * @return The target rotation given current movement.
  */
	virtual FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const;

public:
	void SetAITargetPoint(AActor* AttachedActor, FTransform GoalTransform, bool AllowBackMovement = true, bool AllowRotation = true, bool InRotateDirectToPoint = false);
	class AAITargetPoint* GetAITargetPoint();

private:
	void CreateAITargetPoint();
	UPROPERTY(VisibleAnywhere, Category = "Controller|TargetPoint")
	class AAITargetPoint* AITargetPoint = nullptr;

#pragma endregion AITargetPoint
};



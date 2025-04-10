// Copyright © 2022 DRAGO Entertainment. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPAIDefines.h"
#include "AITargetPoint.generated.h"

UCLASS(BlueprintType)
class NOPEAISYSTEM_API AAITargetPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAITargetPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UArrowComponent* ArrowComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Task")
	class ANPAICharacterBase* OwnerAICharacter = nullptr;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ENPAIMovementDirection MovementDirection = ENPAIMovementDirection::Forward;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float LeftDirectionCooldown = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DirectionCooldown = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float AcceptableRadius = 50.0f;

	UPROPERTY(VisibleAnywhere)
	bool bAllowBackMovement = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<AActor> FollowActor;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool AllowRotation = true;

	// This variable has priority > AllowRotation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool RotateDirectToPoint = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool AllowBrake = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool LerpMovementActive = false;

	UFUNCTION(BlueprintCallable, Category = "Task")
	void SetFollowActor(TSoftObjectPtr<AActor> InFollowActor);
	UFUNCTION(BlueprintPure, Category = "Task")
	TSoftObjectPtr<AActor> GetFollowActor() { return FollowActor; }

	void SetAllowBackMovement(bool InAllowBackMovement);
	bool GetAllowBackMovement() { return bAllowBackMovement; }

	void SetAcceptableRadius(float InAcceptableRadius);
	float GetAcceptableRadius() { return AcceptableRadius; }

	void UpdateMovementDirection(ENPAIMovementDirection CurrentMovementDirection, float DeltaTime);
	void ResetMovementDirection();
	const bool AllowChangeMovementDirection();
	ENPAIMovementDirection GetCurrentMovementDirection() {return MovementDirection; }

	UFUNCTION(BlueprintPure, Category = "Task")
	bool IsTargetPointReached(float& MissingDistance);
	
private:
	UFUNCTION()
	void EndPlayAITargetPoint(class AActor* Actor, EEndPlayReason::Type EndPlayReason);

	UFUNCTION()
	void OnAIDebugChanged(bool AIEnabled);
};

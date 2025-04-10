// Szymon XIII Wielki

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NPAIDefines.h"
#include "NPAIControllerBase.generated.h"

class UBlackboardData;
class UBehaviorTree;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnControllerLogicStart, class ANPAIControllerBase*, AIControllerBase);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCharacterCombatChange, const ECharacterCombatState, CurCombatState, const ECharacterCombatState, PrevCombatState, ACharacter*, AICharacterBase);

UCLASS()
class NOPEAISYSTEM_API ANPAIControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	ANPAIControllerBase(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category = "AIControllerBase")
	FOnControllerLogicStart OnControllerLogicStarted;

	UPROPERTY(BlueprintAssignable, Category = "AIControllerBase")
	FCharacterCombatChange OnCharacterCombatChanged;

	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

#pragma region GenericTeam
public:
	void SetTeam(const ETeamType& NewTeamID);

	UFUNCTION(BlueprintCallable)
	ETeamAttitude::Type IsHostile(const ETeamType& InTeamType);

	void AddTemporaryHostileTeam(const ETeamType& NewTeamID);

	void ForgotTemporaryHostileTeam();

#pragma endregion GenericTeam

protected:
	void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	/** Called on completing current movement request */
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	virtual FAIRequestID RequestMove(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr Path) override;

	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Controller")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, Category = "Controller")
	ANPAICharacterBase* NPAICharacterBase;

protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Controller")
	void InitializeAILogic();
	virtual void InitializeAILogic_Implementation();

#pragma region Blackboard
public:
	UFUNCTION(BlueprintCallable, Category = "Controller")
	void SetNPAIActionState(ENPAIActionState NewAIActionState);
	UFUNCTION(BlueprintPure, Category = "Controller")
	ENPAIActionState GetNPAIActionState();

	UFUNCTION(BlueprintCallable, Category = "Controller")
	void SetCombatState(ECharacterCombatState NewAnimalCombatState, float DeltaSeconds = 0.0f);
	UFUNCTION(BlueprintPure, Category = "Controller")
	ECharacterCombatState GetCombatState();

	void SetCurrentCombatStateTime(float StateTime);
	UFUNCTION(BlueprintPure, Category = "Controller")
	float GetCurrentCombatStateTime();

	void SetLastIdleLocation(FVector LastIdleLocation);
	UFUNCTION(BlueprintPure, Category = "Controller")
	FVector GetLastIdleLocation();

	void SetCombatStateCount(int Amount);
	UFUNCTION(BlueprintPure, Category = "Animals")
	int GetCombatStateCount();

#pragma endregion Blackboard
private:
	UFUNCTION()
	void StartAILogic();

	FTimerHandle StartAILogicTimerHandle;
	UFUNCTION()
	void StartAILogicWithDelay();

#pragma region LogicStart
public:
	bool IsLogicStarted() { return bIsLogicStarted; }

protected:
	UPROPERTY()
	bool bIsLogicStarted = false;

	UFUNCTION()
	void OnMontageEnded(class UAnimMontage* Montage, bool bInterrupted);

#pragma endregion
};

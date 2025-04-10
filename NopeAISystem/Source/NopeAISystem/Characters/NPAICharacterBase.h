// Szymon XIII Wielki

#pragma once

#include "CoreMinimal.h"
#include "../AI/NPAIDefines.h"
#include <GameFramework/Character.h>
#include <Perception/AISightTargetInterface.h>
#include "NPAICharacterBase.generated.h"

class ANPAISpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAITaskStartSignature, class UNPAITaskBase*, CurrentTask);

UCLASS(Abstract)
class NOPEAISYSTEM_API ANPAICharacterBase : public ACharacter, public IAISightTargetInterface
{
	GENERATED_BODY()

public:
	ANPAICharacterBase(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActorSetup")
	class UNPWidgetComponentFace* WidgetCharacterInfo;

	UPROPERTY(BlueprintAssignable, Category = "AIControllerBase")
	FAITaskStartSignature OnAITaskInitialized;

#if WITH_EDITOR
public:
	virtual void PreEditChange(FProperty* PropertyThatWillChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	TArray<struct FNPAITaskStruct> PreAITasksList;
#endif

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//Override for AI Perception "Eye" Location
	void GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const override;

	virtual UAISense_Sight::EVisibilityResult CanBeSeenFrom(const FCanBeSeenFromContext& Context, FVector& OutSeenLocation, int32& OutNumberOfLoSChecksPerformed, int32& OutNumberOfAsyncLosCheckRequested, float& OutSightStrength, int32* UserData = nullptr, const FOnPendingVisibilityQueryProcessedDelegate* Delegate = nullptr) override;

	UPROPERTY(VisibleAnywhere)
	class UAIPerceptionStimuliSourceComponent* AIPerceptionStimuliSourceComponent;

public:
	/** Called when our Controller no longer possesses us. */
	virtual void UnPossessed();
	virtual void PossessedBy(AController* NewController);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPAICharacter|Loading")
	bool bCanBeSaved = true;

	/** Below or equal 0 mean will be never unloaded */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPAICharacter|Loading")
	float DistanceToLoad = 8000.0f;

	/** DistanceToUnload = DistanceToUnload * DistanceToLoad */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPAICharacter|Loading", meta = (ClampMin = "1.0", ClampMax = "10.0", UIMin = "1.0", UIMax = "10.0"))
	float DistanceToUnload = 1.5f;

	/** Returns CharacterMovement subobject **/
	UFUNCTION(BlueprintPure, Category = "NPAICharacter|Settings")
	class UNPCharacterMovementComponent* GetNPCharacterMovement();

	UFUNCTION(BlueprintPure, Category = "NPAICharacter|AI")
	class ANPAIControllerBase* GetNPAIControllerBase();

protected:
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class ANPAIControllerBase* OwnerNPAIController = nullptr;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<class AActor> OwnerActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UNPAITaskBase* CurrentActiveAITask = nullptr;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPAICharacter|AI")
	FNPAIDataCharacter NPAIDataCharacter;

	void SetCurrentActiveAITask(class UNPAITaskBase* NewCurrentActiveAITask);

	UFUNCTION(BlueprintPure, Category = "NPAICharacter|AI")
	class UNPAITaskBase* GetCurrentActiveAITask();

	void SetOwnerActor(TSoftObjectPtr<class AActor> InOwnerActor);
	UFUNCTION(BlueprintPure, Category = "NPAICharacter|AI")
	TSoftObjectPtr<class AActor> GetOwnerActor() {return OwnerActor;}

	UFUNCTION(BlueprintCallable, Category = "NPAICharacter|AI")
	virtual void FinishAITask(const struct FNPTaskFinishReason TaskStopReason = FNPTaskFinishReason());

	UFUNCTION(BlueprintPure, Category = "NPAICharacter|AI")
	FNPAITaskStruct GetNextAITask();
	UFUNCTION(BlueprintCallable, Category = "NPAICharacter|AI")
	void AddAITask(FNPAITaskStruct AITaskStruct, int Index = 0);

	UFUNCTION(BlueprintCallable, Category = "NPAICharacter|AI")
	void RemoveAITask(int Index = 0);

	UFUNCTION(BlueprintCallable, Category = "NPAICharacter|AI")
	FNPAITaskStruct SetNextRandomTask();

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPAICharacter|AI")
	class AActor* InteractableActor = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPAICharacter|AI")
	class UNPSmartObjectComponent* ReservedSmartObjectComponent = nullptr;

public:
	UFUNCTION(BlueprintCallable, Category = "NPAICharacter|AI")
	void SetInteractableActor(class AActor* InInteractableActor);
	UFUNCTION(BlueprintPure, Category = "NPAICharacter|AI")
	class AActor* GetInteractableActor() { return InteractableActor; };

	void SetReservedSmartObject(class UNPSmartObjectComponent* InReservedSmartObjectComponent);
	UFUNCTION(BlueprintPure, Category = "NPAICharacter|AI")
	class UNPSmartObjectComponent* GetReservedSmartObject() { return ReservedSmartObjectComponent; };

#pragma region EnhancedInputs
public:
	UFUNCTION(BlueprintCallable)
	void SetTeam(const ETeamType& NewTeamID);

	UFUNCTION(BlueprintPure)
	ETeamType GetTeam();

	UPROPERTY(EditDefaultsOnly)
	ETeamType OwnTeam;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<ETeamType> HostileTeams;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ETeamType> TemporaryHostileTeams;

#pragma endregion GenericTeam

#pragma endregion 

public:
	void SetCharacterMovementSpeed(ENPAIMovementSpeed NewAIMovementSpeed);
	ENPAIMovementSpeed GetCharacterMovementSpeed();

	UFUNCTION(BlueprintPure)
	AAITargetPoint* GetTargetPoint();
	UFUNCTION(BlueprintCallable)
	virtual void SetTargetPoint(AActor* AttachedActor, FTransform GoalTransform, bool AllowBackMovement = true, bool AllowRotation = true, bool InRotateDirectToPoint = false);
	UFUNCTION(BlueprintPure)
	bool IsTargetPointReached(float &MissingDistance);
};















// Blueprints Studio	

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tickable.h"
#include "../AI/NPAIDefines.h"
#include "NPAITaskBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAITaskProgressUpdate, const FNPAITaskProgress, CurAITaskProgress);

UCLASS(Abstract, Blueprintable)
class NOPEAISYSTEM_API UNPAITaskBase : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "NPAITask")
	class ANPAICharacterBase* OwnerAICharacter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "NPAITask")
	class ANPAIControllerBase* OwnerAIControllerBase = nullptr;

	virtual class UWorld* GetWorld() const override;

	UPROPERTY(BlueprintAssignable, Category = "AINPAITask")
	FAITaskProgressUpdate OnAITaskProgressUpdated;

	UPROPERTY(BlueprintReadWrite, Category = "NPAITask")
	class AActor* TargetActor = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPAITask")
	float AcceptableRadius = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPAITask", meta = (ClampMin = 0, ClampMax = 100, UIMin = 0, UIMax = 100, Units = "Percent"))
	float AcceptableRemainingTaskProgressInPercent = 25.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "NPAITask")
	TArray<struct FNPAITaskProgress> ProgressOfTasks;

	void SetAIMovementSpeed(ENPAIMovementSpeed NewAIMovementSpeed);
	ENPAIMovementSpeed GetAIMovementSpeed() { return AIMovementSpeed; };

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "NPAITask")
	bool bRepeatTask = true;
	
	// If Target point is moveable follow him
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "NPAITask")
	bool bCheckDistanceToPoint = true;

	UPROPERTY()
	float CurTimeToCheckDistance = 0;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "NPAITask")
	float TimeToCheckDistance = 3;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "NPAITask")
	ENPAIMovementSpeed AIMovementSpeed = ENPAIMovementSpeed::Jog;

#pragma region TickableGameObject
public:
	UNPAITaskBase();
	virtual ~UNPAITaskBase();

	// Begin FTickableGameObject Interface.
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	// End FTickableGameObject Interface.

	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual bool IsTickableInEditor() const override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "Tick")
	void SetTickEnabled(bool Enable);

private:
	//Because engine would construct inner object when game load package (before game start), so we need to add a flag to identify which one is construct on game running.
	bool bIsCreateOnRunning = false;
	UPROPERTY()
	bool TickEnabled = true;

#pragma endregion TickableGameObject

#pragma region Task
public:
	virtual void InitializeTask(class ANPAICharacterBase* OwnerCharacter, const struct FNPAITaskStruct& AITaskStruct);
	UFUNCTION(BlueprintImplementableEvent, Category = "NPAITask")
	void OnInitializeTask(class ANPAICharacterBase* OwnerCharacter, class AActor* InTargetActor);

	virtual void StartTask(ENPAIActionState InNPAIActionState);
	UFUNCTION(BlueprintImplementableEvent)
	void EventStartTask(ENPAIActionState InNPAIActionState);

	virtual void TickObject(float DeltaTime, ENPAIActionState AIActionState);
	UFUNCTION(BlueprintImplementableEvent)
	void EventTick(float DeltaTime, ENPAIActionState AIActionState);

protected:
	virtual bool FinishBeginTask(const FNPTaskFinishReason TaskStopReason);
	UFUNCTION(BlueprintImplementableEvent)
	void OnFinishBeginTask(const FNPTaskFinishReason TaskStopReason);

	// This function should only be used in FinishBeginTask
	UFUNCTION(BlueprintCallable, Category = "NPAITask")
	virtual void FinishEndTask();

protected:
	UFUNCTION(BlueprintPure, Category = "NPAITask")
	FNPTaskFinishReason GetReasonToFinishAITask();

	UPROPERTY(BlueprintReadOnly, Category = "NPAITask")
	float CurrentStateTime = 0.0f;

	UFUNCTION(BlueprintPure, Category = "NPAITask")
	bool HasReasonToFinishAITask();

private:
	UPROPERTY()
	FNPTimeStruct BeginValidTime;
	UPROPERTY()
	FNPTimeStruct EndValidTime;

	UFUNCTION()
	void CheckValidTimeForTask(const struct FNPTimeStruct UpdatedTime, struct FNPTimeStruct Dif);

	UPROPERTY()
	bool bFinishTaskBlockerActive = false;
	UPROPERTY()
	TArray<FNPTaskFinishReason> TaskFinishReasons;
	UPROPERTY()
	ENPAIActionState CurrentAIActionState = ENPAIActionState::MAX;

protected:
	bool IsFnishedProgressOfTasks();

	float GetRemainingTaskInPercent(FString ObjectiveName = "MainTask");

	bool HasAcceptableTaskProgressValue(FString ObjectiveName = "MainTask");

public:
	UFUNCTION(BlueprintCallable, Category = "NPAITask")
	void AddReasonToFinishAITask(FNPTaskFinishReason NewTaskFinishReason);

	bool TryCompleteTask();

	void UpdateTimeOfTasks(float DeltaTime, ENPAIActionState AIActionState);

	void SetFinishTaskBlocker(bool InFinishTaskBlockerActive);

	UFUNCTION(BlueprintCallable)
	bool UpdateTaskProgress(FString ObjectiveName = "MainTask", int AddedProgress = 1, float NewTimeTask = -1);

	UFUNCTION(BlueprintCallable)
	bool AddAITaskProgress(struct FNPAITaskProgress AITaskProgress);

	UFUNCTION(BlueprintPure, Category = "NPAITask")
	FNPAITaskProgress GetTaskProgress(FString ObjectiveName = "MainTask");

	void SaveTaskProgress();

	void BeginDestroyTask();
	
#pragma endregion Task

#pragma region Description
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TaskInformation", meta = (MultiLine = true))
	FText TaskDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TaskInformation", meta = (MultiLine = true))
	FText TaskName;

#pragma endregion Description
};

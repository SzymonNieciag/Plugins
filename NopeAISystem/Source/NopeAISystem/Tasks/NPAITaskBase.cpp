// Blueprints Studio


#include "NPAITaskBase.h"
#include <Animation/AnimInstance.h>
#include <Kismet/GameplayStatics.h>
#include <Engine/EngineTypes.h>
#include "../AI/NPAIControllerBase.h"
#include "../Characters/NPAICharacterBase.h"
#include "../AI/Components/NPCharacterMovementComponent.h"
#include "../AI/Components/NPSmartObjectComponent.h"
#include "../AI/NPAILibrary.h"
#include "../NPAISubsystem.h"
#include "NopeAISystem/AI/AITargetPoint.h"



class UWorld* UNPAITaskBase::GetWorld() const
{
	if (OwnerAICharacter)
		return OwnerAICharacter->GetWorld();
	else
		return nullptr;
}

UNPAITaskBase::UNPAITaskBase()
{
	bIsCreateOnRunning = GIsRunning;
	TaskName.FromString("Task Name");
	TaskDescription.FromString("Some Description");
	ProgressOfTasks.Add(FNPAITaskProgress());
}

UNPAITaskBase::~UNPAITaskBase()
{

}

void UNPAITaskBase::Tick(float DeltaTime)
{
	if (OwnerAIControllerBase)
	{
		TickObject(DeltaTime, OwnerAIControllerBase->GetNPAIActionState());
	}
}

bool UNPAITaskBase::IsTickable() const
{
	if (!TickEnabled)
	{
		return false;
	}
	if (IsTemplate(RF_ClassDefaultObject))
	{
		return false;
	}
	if (!GetWorld())
	{
		return false;
	}
	if (!GetWorld()->HasBegunPlay())
	{
		return false;
	}
	return true;
}

TStatId UNPAITaskBase::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UNPAITaskBase, STATGROUP_Tickables);
}

bool UNPAITaskBase::IsTickableInEditor() const
{
	return false;
}

ETickableTickType UNPAITaskBase::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}

UWorld* UNPAITaskBase::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

void UNPAITaskBase::SetTickEnabled(bool Enable)
{
	TickEnabled = Enable;
}


void UNPAITaskBase::InitializeTask(class ANPAICharacterBase* OwnerCharacter, const struct FNPAITaskStruct& AITaskStruct)
{
	if (ensure(OwnerCharacter))
	{
		OwnerAICharacter = OwnerCharacter;
		OwnerAIControllerBase = Cast<ANPAIControllerBase>(OwnerAICharacter->GetController());
		if (!OwnerAIControllerBase)
		{
			return;
		}

		if (AITaskStruct.TasksProgress.Num() != 0)
		{
			TArray<ENPAIActionState> OldRequirmentState = ProgressOfTasks[0].RequirmentState;
			ProgressOfTasks = AITaskStruct.TasksProgress;
			for (FNPAITaskProgress& NPAITaskProgress : ProgressOfTasks)
			{
				NPAITaskProgress.InitTasks();
				NPAITaskProgress.RequirmentState = OldRequirmentState;
			}
		}
		AIMovementSpeed = AITaskStruct.AIMovementSpeed;
		TaskFinishReasons = AITaskStruct.TaskFinishReasons;
		TargetActor = AITaskStruct.TargetActor.LoadSynchronous();

		OwnerAICharacter->SetCurrentActiveAITask(this);
		BeginValidTime = AITaskStruct.BeginValidTime;
		EndValidTime = AITaskStruct.EndValidTime;

		OwnerAICharacter->SetCharacterMovementSpeed(AIMovementSpeed);

		if (UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(GetWorld()))
		{
			NPAISubsystem->OnGameTimeUpdated.AddDynamic(this, &UNPAITaskBase::CheckValidTimeForTask);
		}

		OnInitializeTask(OwnerAICharacter, TargetActor);
		OwnerAIControllerBase->SetNPAIActionState(ENPAIActionState::Move);
	}
}

void UNPAITaskBase::StartTask(ENPAIActionState InNPAIActionState)
{
	if (ensure(OwnerAIControllerBase))
	{
		if (UNPAISubsystem::AIDebugSystemEnabled(this))
		{
			if (InNPAIActionState == ENPAIActionState::Action)
			{
				UE_LOG(LogTemp, Warning, TEXT("Start Action Task %s"), *this->GetName());
			}
		}
		if (InNPAIActionState == ENPAIActionState::Move)
		{
			OwnerAICharacter->GetTargetPoint()->SetAcceptableRadius(AcceptableRadius);
		}
		if (OwnerAIControllerBase->GetCombatState() == ECharacterCombatState::Idle)
		{
			if (TryCompleteTask())
			{

			}
			else
			{
				if (HasAcceptableTaskProgressValue())
				{
					if (ensure(OwnerAICharacter))
					{
						OwnerAICharacter->GetTargetPoint()->SetAllowBackMovement(false);
						OwnerAICharacter->GetTargetPoint()->SetAcceptableRadius(AcceptableRadius);
						EventStartTask(InNPAIActionState);
						if (OwnerAIControllerBase)
						{
							OwnerAIControllerBase->SetNPAIActionState(InNPAIActionState);
						}
					}
				}
				else
				{
					FNPTaskFinishReason NewTaskFinishReason;
					AddReasonToFinishAITask(NewTaskFinishReason);
				}
			}
		}
	}
}

void UNPAITaskBase::TickObject(float DeltaTime, ENPAIActionState AIActionState)
{
	if (OwnerAIControllerBase->GetCombatState() == ECharacterCombatState::Idle)
	{
		UpdateTimeOfTasks(DeltaTime, AIActionState);
		if (CurrentAIActionState != AIActionState)
		{
			CurrentStateTime = 0.0f;
			CurrentAIActionState = AIActionState;
		}

		if (bCheckDistanceToPoint)
		{
			if (CurTimeToCheckDistance <= 0)
			{
				CurTimeToCheckDistance = TimeToCheckDistance;
				float MissingDistance;
				if (!OwnerAICharacter->IsTargetPointReached(MissingDistance))
				{
					if (OwnerAIControllerBase->GetNPAIActionState() != ENPAIActionState::Move)
					{
						StartTask(ENPAIActionState::Move);
					}
				}
			}
			else
			{
				CurTimeToCheckDistance -= DeltaTime;
			}
		}

		CurrentStateTime += DeltaTime;
		if (TryCompleteTask())
		{
			return;
		}
		EventTick(DeltaTime, AIActionState);
		SaveTaskProgress();
	}
	else
	{
		OwnerAICharacter->GetTargetPoint()->SetFollowActor(nullptr);
	}
}

bool UNPAITaskBase::FinishBeginTask(const FNPTaskFinishReason TaskStopReason)
{
	if (TickEnabled == false)
	{
		return false;
	}
	OwnerAIControllerBase->SetNPAIActionState(ENPAIActionState::Wait);
	OnFinishBeginTask(TaskStopReason);
	SetTickEnabled(false);
	SaveTaskProgress();
	return true;
}

bool UNPAITaskBase::HasReasonToFinishAITask()
{
	if (TaskFinishReasons.Num() != 0)
	{
		if (!bFinishTaskBlockerActive)
		{
			return true;
		}
	}
	return false;
}

void UNPAITaskBase::CheckValidTimeForTask(const struct FNPTimeStruct UpdatedTime, struct FNPTimeStruct Dif)
{
	if (!FNPTimeStruct::IsInTimeRange(UpdatedTime, BeginValidTime, EndValidTime))
	{
		FNPTaskFinishReason TaskFinishReason;
		TaskFinishReason.FinishReason = ENPTaskFinish::EndTime;
		TaskFinishReason.TaskResult = ENPTaskResult::TaskFalse;
		AddReasonToFinishAITask(TaskFinishReason);
	}
}

FNPTaskFinishReason UNPAITaskBase::GetReasonToFinishAITask()
{
	FNPTaskFinishReason TaskStopReason;
	if (TaskFinishReasons.Num() != 0)
	{
		return TaskFinishReasons[0];
	}
	return TaskStopReason;
}

void UNPAITaskBase::AddReasonToFinishAITask(FNPTaskFinishReason NewTaskFinishReason)
{
	for (FNPTaskFinishReason& TaskFinishReason : TaskFinishReasons)
	{
		if (TaskFinishReason.FinishReason == NewTaskFinishReason.FinishReason)
		{
			TaskFinishReason = NewTaskFinishReason;
			return;
		}
	}
	TaskFinishReasons.Add(NewTaskFinishReason);
}

bool UNPAITaskBase::TryCompleteTask()
{
	if (HasReasonToFinishAITask())
	{
		FNPTaskFinishReason TaskStopReason = GetReasonToFinishAITask();
		FinishBeginTask(TaskStopReason);
		return true;
	}
	return false;
}

void UNPAITaskBase::FinishEndTask()
{
	if (ensure(OwnerAICharacter))
	{
		if (UNPSmartObjectComponent* NPSmartObjectComponent = OwnerAICharacter->GetReservedSmartObject())
		{
			NPSmartObjectComponent->StopReserveActor(OwnerAICharacter);
		}
		OwnerAICharacter->SetInteractableActor(nullptr);
		OwnerAICharacter->SetCurrentActiveAITask(nullptr);
		OwnerAICharacter->GetTargetPoint()->SetFollowActor(nullptr);

		if (OwnerAICharacter->NPAIDataCharacter.AITasksList.Num() != 0)
		{
			FNPAITaskStruct CurrentAITaskStruct = OwnerAICharacter->NPAIDataCharacter.AITasksList[0];
			OwnerAICharacter->NPAIDataCharacter.AITasksList.RemoveAt(0);
			if (GetReasonToFinishAITask().FinishReason == ENPTaskFinish::Conversation)
			{
				CurrentAITaskStruct.Reset(false);
				OwnerAICharacter->AddAITask(CurrentAITaskStruct, 1);
			}
			else
			{
				if (bRepeatTask)
				{
					CurrentAITaskStruct.Reset(true);
					OwnerAICharacter->AddAITask(CurrentAITaskStruct);
				}
			}
			UNPAILibrary::SpawnAITask(OwnerAICharacter);
		}

		if (UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(GetWorld()))
		{
			NPAISubsystem->OnGameTimeUpdated.RemoveDynamic(this, &UNPAITaskBase::CheckValidTimeForTask);
		}
	}

	OwnerAICharacter = nullptr;
	OwnerAIControllerBase = nullptr;
	TargetActor = nullptr;
	SetTickEnabled(false);
	ConditionalBeginDestroy();
}

bool UNPAITaskBase::IsFnishedProgressOfTasks()
{
	for (FNPAITaskProgress NPAITaskProgress : ProgressOfTasks)
	{
		if (NPAITaskProgress.bIsFinished == false)
		{
			return false;
		}
	}
	return true;
}

float UNPAITaskBase::GetRemainingTaskInPercent(FString ObjectiveName /*= "MainTask"*/)
{
	for (FNPAITaskProgress& TaskProgress : ProgressOfTasks)
	{
		if (TaskProgress.TaskName == ObjectiveName)
		{
			if (TaskProgress.InitLeftTaskTime == 0)
			{
				return 100;
			}
			float AmountInPercent = (TaskProgress.LeftTaskTime / TaskProgress.InitLeftTaskTime) * 100;
			return AmountInPercent;
		}
	}
	return 0;
}

bool UNPAITaskBase::HasAcceptableTaskProgressValue(FString ObjectiveName /*= "MainTask"*/)
{
	float Value = GetRemainingTaskInPercent(ObjectiveName);
	if (Value > AcceptableRemainingTaskProgressInPercent)
	{
		return true;
	}
	return false;
}

void UNPAITaskBase::UpdateTimeOfTasks(float DeltaTime, ENPAIActionState AIActionState)
{
	for (int i = ProgressOfTasks.Num() - 1; i >= 0; i--)
	{
		if (ProgressOfTasks[i].CanUpdateTime(AIActionState))
		{
			if (!ProgressOfTasks[i].bIsFinished)
			{
				ProgressOfTasks[i].LeftTaskTime -= DeltaTime;
				if (ProgressOfTasks[i].LeftTaskTime <= 0)
				{
					ProgressOfTasks[i].LeftRepeatCount--;
					if (ProgressOfTasks[i].LeftRepeatCount <= 0)
					{
						ProgressOfTasks[i].bIsFinished = true;
					}
					else
					{
						ProgressOfTasks[i].LeftTaskTime = ProgressOfTasks[i].InitLeftTaskTime;
					}
					OnAITaskProgressUpdated.Broadcast(ProgressOfTasks[i]);
				}
			}

		}
	}
	if (IsFnishedProgressOfTasks())
	{
		FNPTaskFinishReason TaskFinishReason;
		TaskFinishReason.FinishReason = ENPTaskFinish::EndTime;
		TaskFinishReason.TaskResult = ENPTaskResult::TaskFalse;
		AddReasonToFinishAITask(TaskFinishReason);
	}
}

void UNPAITaskBase::SetFinishTaskBlocker(bool InFinishTaskBlockerActive)
{
	bFinishTaskBlockerActive = InFinishTaskBlockerActive;
}

bool UNPAITaskBase::UpdateTaskProgress(FString ObjectiveName /*= "MainTask"*/, int AddedProgress /*= 1*/, float NewTimeTask /*= -1*/)
{
	for (int i = ProgressOfTasks.Num() - 1; i >= 0; i--)
	{
		if (ProgressOfTasks[i].TaskName == ObjectiveName)
		{
			ProgressOfTasks[i].CurProgress += AddedProgress;
			if (NewTimeTask != -1)
			{
				ProgressOfTasks[i].LeftTaskTime = NewTimeTask;
				ProgressOfTasks[i].InitLeftTaskTime = NewTimeTask;
			}
			if (ProgressOfTasks[i].CurProgress >= ProgressOfTasks[i].MaxProgress)
			{
				FNPTaskFinishReason TaskFinishReason;
				TaskFinishReason.FinishReason = ENPTaskFinish::WorkEnd;
				TaskFinishReason.TaskResult = ENPTaskResult::TaskSuccess;
				AddReasonToFinishAITask(TaskFinishReason);
			}
			OnAITaskProgressUpdated.Broadcast(ProgressOfTasks[i]);
			return true;
		}
	}
	return false;
}

bool UNPAITaskBase::AddAITaskProgress(struct FNPAITaskProgress AITaskProgress)
{
	for (int i = ProgressOfTasks.Num() - 1; i >= 0; i--)
	{
		if (ProgressOfTasks[i].TaskName == AITaskProgress.TaskName)
			return false;
	}
	ProgressOfTasks.Add(AITaskProgress);
	return true;
}

FNPAITaskProgress UNPAITaskBase::GetTaskProgress(FString ObjectiveName /*= "MainTask"*/)
{
	for (FNPAITaskProgress& TaskProgress : ProgressOfTasks)
	{
		if (TaskProgress.TaskName == ObjectiveName)
		{
			return TaskProgress;
		}
	}
	return FNPAITaskProgress(0, 0, 0, 0, 0);
}

void UNPAITaskBase::SaveTaskProgress()
{
	if (OwnerAICharacter && OwnerAICharacter->NPAIDataCharacter.AITasksList.Num() > 0)
	{
		OwnerAICharacter->NPAIDataCharacter.AITasksList[0].TasksProgress = ProgressOfTasks;
		OwnerAICharacter->NPAIDataCharacter.AITasksList[0].TaskFinishReasons = TaskFinishReasons;
	}
}

void UNPAITaskBase::BeginDestroyTask()
{
	OwnerAICharacter->SetCurrentActiveAITask(nullptr);
	SetTickEnabled(false);
	ConditionalBeginDestroy();
}

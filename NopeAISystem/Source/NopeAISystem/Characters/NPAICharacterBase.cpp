// Szymon XIII Wielki

#include "NPAICharacterBase.h"
#include <Kismet/KismetMathLibrary.h>
#include <Components/CapsuleComponent.h>

#include "../NPAISystemSettings.h"
#include "../AI/Components/NPCharacterMovementComponent.h"
#include "../UI/NPWidgetComponentFace.h"
#include "../AI/NPAIControllerBase.h"
#include "../AI/Components/NPSmartObjectComponent.h"
#include "../NPAISubsystem.h"
#include "../NPAIManager.h"
#include "../AI/NPAIDefines.h"
#include "../Tasks/NPAITaskBase.h"
#include <Perception/AISense_Sight.h>
#include <Perception/AISense_Hearing.h>
#include <Perception/AIPerceptionStimuliSourceComponent.h>
#include "../AI/Components/NPAISpawnerComponent.h"
#include "../AI/AITargetPoint.h"
#include "../AI/NPAILibrary.h"

ANPAICharacterBase::ANPAICharacterBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UNPCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	WidgetCharacterInfo = CreateDefaultSubobject<UNPWidgetComponentFace>(TEXT("WidgetInfo"));
	WidgetCharacterInfo->SetupAttachment(RootComponent);
	WidgetCharacterInfo->SetRelativeLocation(FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2));
	WidgetCharacterInfo->SetHiddenInGame(true);

	if (const UNPAISystemSettings* MTAISystemSettings = GetDefault<UNPAISystemSettings>())
	{
		WidgetCharacterInfo->SetWidgetClass(MTAISystemSettings->CharacterInfoWidget);
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		AIPerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));
		AIPerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
		AIPerceptionStimuliSourceComponent->RegisterForSense(UAISense_Hearing::StaticClass());
	}
}

#if WITH_EDITOR
void ANPAICharacterBase::PreEditChange(FProperty* PropertyThatWillChange)
{
	Super::PreEditChange(PropertyThatWillChange);
	PreAITasksList = NPAIDataCharacter.AITasksList;
}

void ANPAICharacterBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::ValueSet)
	{
		return;
	}

	if (PreAITasksList.Num() == 0)
	{
		return;
	}

	int ChangedIndex = -1;
	for (int i = 0; i < NPAIDataCharacter.AITasksList.Num(); i++)
	{
		if (NPAIDataCharacter.AITasksList[i].AITaskBaseClass != PreAITasksList[i].AITaskBaseClass)
		{
			ChangedIndex = i;
			break;
		}
	}

	if (ChangedIndex != -1)
	{
		FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
		FName CheckName = "AITaskBaseClass";

		if (PropertyName == CheckName)
		{
			if (NPAIDataCharacter.AITasksList[ChangedIndex].AITaskBaseClass)
			{
				TSubclassOf<UNPAITaskBase> ClassToCheck = NPAIDataCharacter.AITasksList[ChangedIndex].AITaskBaseClass.LoadSynchronous();
				UNPAITaskBase* DefaultObject = ClassToCheck.GetDefaultObject();
				NPAIDataCharacter.AITasksList[ChangedIndex].TasksProgress = DefaultObject->ProgressOfTasks;
			}
		}
	}
}
#endif

void ANPAICharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (!Cast<ANPAIControllerBase>(GetController()))
	{
		return;
	}
	this->Tags.Remove("RemovedByGoHome");
	this->Tags.Remove("RemovedByDistance");

	if (UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(GetWorld()))
	{
		UNPAISubsystem::GetAISubsystem(GetWorld())->AddAICharacterBase(this);
	}

	if (AActor* OwnerActorLocal = GetOwnerActor().LoadSynchronous())
	{
		if (UNPAISpawnerComponent* NPAISpawnerComponent = Cast<UNPAISpawnerComponent>(OwnerActorLocal->GetComponentByClass(UNPAISpawnerComponent::StaticClass())))
		{
			NPAISpawnerComponent->AddSpawnedCharacter(this);
		}
	}
}

void ANPAICharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANPAICharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Check if this Is Player
	UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(GetWorld());
	if (ReservedSmartObjectComponent)
	{
		ReservedSmartObjectComponent->StopReserveActor(this);
	}

	if (NPAISubsystem)
	{
		UNPAISubsystem::GetAISubsystem(GetWorld())->RemoveAICharacterBase(this);
		ANPAIManager* AIManager = ANPAIManager::GetAIManager(this->GetWorld());
		if (AIManager)
		{
			if (EndPlayReason == EEndPlayReason::Destroyed)
			{
				if (this->ActorHasTag("RemovedByDistance"))
				{
					AIManager->SaveAICharacter(this, true, false);
				}
				else
				{
					AIManager->SaveAICharacter(this, false, false);
				}
			}
			else if (EndPlayReason == EEndPlayReason::RemovedFromWorld)
			{
				AIManager->SaveAICharacter(this, true, false);
			}
		}
	}

	if (GetCurrentActiveAITask())
	{
		GetCurrentActiveAITask()->BeginDestroyTask();
	}

	Super::EndPlay(EndPlayReason);
}

void ANPAICharacterBase::GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const
{
	Location = GetMesh()->GetSocketLocation("Eyes");
	Rotation = UKismetMathLibrary::MakeRotator(0, 0, GetMesh()->GetSocketRotation("Eyes").Yaw);
}

UAISense_Sight::EVisibilityResult ANPAICharacterBase::CanBeSeenFrom(const FCanBeSeenFromContext& Context, FVector& OutSeenLocation, int32& OutNumberOfLoSChecksPerformed, int32& OutNumberOfAsyncLosCheckRequested, float& OutSightStrength, int32* UserData /*= nullptr*/, const FOnPendingVisibilityQueryProcessedDelegate* Delegate /*= nullptr*/)
{
	FVector SightTargetLocation = GetMesh()->GetSocketLocation("head");
	FHitResult HitResult;

	bool HadBlockingHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Context.ObserverLocation,
		SightTargetLocation,
		ECC_Visibility,
		FCollisionQueryParams(FName("Name_AILineOfSight"), false, Context.IgnoreActor)
	);

	AActor* HitActor = Cast<AActor>(HitResult.GetActor());

	if (!HadBlockingHit || (IsValid(HitActor) && HitActor->IsOwnedBy(this)))
	{
		OutSeenLocation = HadBlockingHit ? HitResult.ImpactPoint : SightTargetLocation;
		OutNumberOfLoSChecksPerformed = 1;
		OutNumberOfAsyncLosCheckRequested = 0;
		OutSightStrength = 1;
		return UAISense_Sight::EVisibilityResult::Visible;
	}

	OutNumberOfLoSChecksPerformed = 1;
	OutNumberOfAsyncLosCheckRequested = 0;
	OutSightStrength = 0;
	return UAISense_Sight::EVisibilityResult::NotVisible;
}

void ANPAICharacterBase::UnPossessed()
{
	Super::UnPossessed();

	if (HasAuthority() && AIPerceptionStimuliSourceComponent)
	{
		AIPerceptionStimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
}

void ANPAICharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	OwnerNPAIController = Cast<ANPAIControllerBase>(NewController);

	if (HasAuthority() && AIPerceptionStimuliSourceComponent)
	{
		AIPerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
	}
}

class UNPCharacterMovementComponent* ANPAICharacterBase::GetNPCharacterMovement()
{
	return Cast<UNPCharacterMovementComponent>(GetCharacterMovement());
}

class ANPAIControllerBase* ANPAICharacterBase::GetNPAIControllerBase()
{
	return Cast<ANPAIControllerBase>(GetController());
}

void ANPAICharacterBase::SetCurrentActiveAITask(class UNPAITaskBase* NewCurrentActiveAITask)
{
	CurrentActiveAITask = NewCurrentActiveAITask;
}

class UNPAITaskBase* ANPAICharacterBase::GetCurrentActiveAITask()
{
	return CurrentActiveAITask; 
}

void ANPAICharacterBase::SetOwnerActor(TSoftObjectPtr<class AActor> InOwnerActor)
{
	OwnerActor = InOwnerActor;
}

void ANPAICharacterBase::FinishAITask(const struct FNPTaskFinishReason TaskStopReason /*= FNPTaskFinishReason()*/)
{
	if (IsValid(CurrentActiveAITask))
	{
		CurrentActiveAITask->AddReasonToFinishAITask(FNPTaskFinishReason());
		CurrentActiveAITask->TryCompleteTask();
	}
	else if (OwnerNPAIController && OwnerNPAIController->IsLogicStarted())
	{
		UNPAILibrary::SpawnAITask(this);
	}
	else
	{
		if (GEngine)
		{
			FString Massage = this->GetName();
			GEngine->AddOnScreenDebugMessage(-1, 200, FColor::Green, FString::Printf(TEXT("FinishTask Was Called before LogicStarted %s"), *this->GetName()));
		}
	}
}

FNPAITaskStruct ANPAICharacterBase::GetNextAITask()
{
	if (NPAIDataCharacter.AITasksList.Num() == 1)
	{
		return NPAIDataCharacter.AITasksList[0];
	}
	else if(NPAIDataCharacter.AITasksList.Num() > 1)
	{
		return NPAIDataCharacter.AITasksList[1];
	}
	return FNPAITaskStruct();
}

void ANPAICharacterBase::AddAITask(FNPAITaskStruct AITaskStruct, int Index /*= 0*/)
{
	if (Index > 0 && Index < NPAIDataCharacter.AITasksList.Num())
	{
		NPAIDataCharacter.AITasksList.Insert(AITaskStruct, Index);
	}
	else
	{
		NPAIDataCharacter.AITasksList.Add(AITaskStruct);
	}
}

void ANPAICharacterBase::RemoveAITask(int Index /*= 0*/)
{
	if (NPAIDataCharacter.AITasksList.Num() > 0)
	{
		if (Index >= 0 || Index < NPAIDataCharacter.AITasksList.Num())
		{
			NPAIDataCharacter.AITasksList.RemoveAt(Index);
		}
	}
}

FNPAITaskStruct ANPAICharacterBase::SetNextRandomTask()
{
	FNPAITaskStruct ChoosedTask;
	if (NPAIDataCharacter.AITasksList.Num() > 1)
	{
		int RandomIndex = UKismetMathLibrary::RandomIntegerInRange(0, NPAIDataCharacter.AITasksList.Num() - 1);
		ChoosedTask = NPAIDataCharacter.AITasksList[RandomIndex];
		RemoveAITask(RandomIndex);
		AddAITask(ChoosedTask, 1);
	}
	return ChoosedTask;
}

void ANPAICharacterBase::SetInteractableActor(class AActor* InInteractableActor)
{
	InteractableActor = InInteractableActor;
}

void ANPAICharacterBase::SetReservedSmartObject(class UNPSmartObjectComponent* InReservedSmartObjectComponent)
{
	ReservedSmartObjectComponent = InReservedSmartObjectComponent;
}

void ANPAICharacterBase::SetTeam(const ETeamType& NewTeamID)
{
	OwnTeam = NewTeamID;
}

ETeamType ANPAICharacterBase::GetTeam()
{
	return OwnTeam;
}

void ANPAICharacterBase::SetCharacterMovementSpeed(ENPAIMovementSpeed NewAIMovementSpeed)
{
	GetNPCharacterMovement()->SetCharacterMovementSpeed(NewAIMovementSpeed);
}

ENPAIMovementSpeed ANPAICharacterBase::GetCharacterMovementSpeed()
{
	return GetNPCharacterMovement()->GetCharacterMovementSpeed();
}

AAITargetPoint* ANPAICharacterBase::GetTargetPoint()
{
	return GetNPCharacterMovement()->GetAITargetPoint();
}

void ANPAICharacterBase::SetTargetPoint(AActor* AttachedActor, FTransform GoalTransform, bool AllowBackMovement /*= true*/, bool AllowRotation /*= true*/, bool InRotateDirectToPoint /*= false*/)
{
	GetNPCharacterMovement()->SetAITargetPoint(AttachedActor, GoalTransform, AllowBackMovement, AllowRotation, InRotateDirectToPoint);
}

bool ANPAICharacterBase::IsTargetPointReached(float& MissingDistance)
{
	return GetNPCharacterMovement()->GetAITargetPoint()->IsTargetPointReached(MissingDistance);
}

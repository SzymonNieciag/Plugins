// Copyright Epic Games, Inc. All Rights Reserved.
#include "NPAISpawnerComponent.h"
#include "NopeAISystem/NPAISubsystem.h"
#include <Kismet/KismetMathLibrary.h>
#include "NopeAISystem/NPAIManager.h"
#include "NopeAISystem/Characters/NPAICharacterBase.h"
#include <Kismet/GameplayStatics.h>
#include "NopeAISystem/Interfaces/AISpawnable.h"
#include <Components/CapsuleComponent.h>
#include "NopeAISystem/Characters/NPAICharacterBase.h"
#include "NopeAISystem/NPAISystemSettings.h"
#include "NopeAISystem/Tasks/NPAITaskBase.h"


UNPAISpawnerComponent::UNPAISpawnerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MinRandomSpawnTime = FNPTimeStruct(0, 0, 10);
	MaxRandomSpawnTime = FNPTimeStruct(0, 0, 30);

	BeginValidTime.Days = 0;
	BeginValidTime.Hours = 0;
	BeginValidTime.Minutes = 0;

	EndValidTime.Days = 0;
	EndValidTime.Hours = 23;
	EndValidTime.Minutes = 59;

	if (const UNPAISystemSettings* AISystemSettings = GetDefault<UNPAISystemSettings>())
	{
		FNPAITaskStruct NPAITaskStruct;
		NPAITaskStruct.AITaskBaseClass = AISystemSettings->WaitTask;
		NPAIDataCharacter.AITasksList.Add(NPAITaskStruct);
	}
}

#if WITH_EDITOR
void UNPAISpawnerComponent::PreEditChange(FProperty* PropertyThatWillChange)
{
	Super::PreEditChange(PropertyThatWillChange);
	PreAITasksList = NPAIDataCharacter.AITasksList;
}

void UNPAISpawnerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
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

void UNPAISpawnerComponent::BeginPlay()
{
	Super::BeginPlay();
	if (NPAIDataCharacter.TargetHomes.Num() == 0)
	{
		NPAIDataCharacter.TargetHomes.Add(GetOwner());
	}

	NextCharacterClassToSpawn = RandomCharacterToSpawn();
	UploadSpawnRange();

	if (UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(GetWorld()))
	{
		NPAISubsystem->OnGameTimeUpdated.AddDynamic(this, &UNPAISpawnerComponent::OnTimeUpdated);
		NPAISubsystem->AddAISpawnerComponent(this);
	}

	UNPAISubsystem* AISub = UNPAISubsystem::GetAISubsystem(GetWorld());
	if (AISub && !UNPAISubsystem::GetGameLogicStarted())
	{
		AISub->OnGameLogicStarted.AddDynamic(this, &UNPAISpawnerComponent::LoadAISpawner);
	}
	else
	{
		LoadAISpawner();
	}
}

void UNPAISpawnerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ANPAIManager* AIManager = ANPAIManager::GetAIManager(this->GetWorld());
	if (AIManager)
	{
		if (EndPlayReason == EEndPlayReason::Destroyed)
		{
			AIManager->SaveAISpawner(this, false);
		}
		else if (EndPlayReason == EEndPlayReason::RemovedFromWorld)
		{
			AIManager->SaveAISpawner(this, true);
		}
	}
	if (UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(GetWorld()))
	{
		NPAISubsystem->OnGameTimeUpdated.RemoveDynamic(this, &UNPAISpawnerComponent::OnTimeUpdated);
		NPAISubsystem->RemoveAISpawnerComponent(this);
	}
	Super::EndPlay(EndPlayReason);
}

void UNPAISpawnerComponent::ResetCharacter(ANPAICharacterBase* NPAICharacterBase, bool ByDestroy)
{
	if (ByDestroy)
	{
		NPAICharacterBase->Tags.Add("QuickReset");
		NPAICharacterBase->Destroy();
		LeftTimeToSpawnNPAICharacter = FNPTimeStruct();
	}
}

void UNPAISpawnerComponent::ClearSpawner()
{
	for (int i = SpawnedCharacters.Num() - 1; i >= 0; i--)
	{
		SpawnedCharacters[i]->Destroyed();
	}
	LeftCharacterAmountToSpawn = 0;
	LeftTimeToSpawnNPAICharacter = FNPTimeStruct();
}

ANPAICharacterBase* UNPAISpawnerComponent::SpawnCharacter(FTransform SpawnTransform)
{
	if (ANPAICharacterBase* AICharacterBase = Cast<ANPAICharacterBase>(NextCharacterClassToSpawn->GetDefaultObject(true)))
	{
		float OutRadius;
		float OutHalfHeight;
		AICharacterBase->GetCapsuleComponent()->GetScaledCapsuleSize(OutRadius, OutHalfHeight);
		FVector SpawnLocation = SpawnTransform.GetLocation() + FVector(0, 0, OutHalfHeight);
		SpawnTransform.SetLocation(SpawnLocation);
	}
	ANPAICharacterBase* NPAICharacterBase = ANPAIManager::SpawnAICharacter(GetOwner(), NextCharacterClassToSpawn.Get(), SpawnTransform, NPAIDataCharacter.AITasksList);
	return NPAICharacterBase;
}

bool UNPAISpawnerComponent::CanBeSpawned()
{
	AActor* OwnerActor = GetOwner();
	if (SpawnedCharacters.Num() >= MaxCharacterAmountPerSpawner)
	{
		return false;
	}
	if (LeftCharacterAmountToSpawn < 1)
	{
		return false;
	}
	if (OwnerActor)
	{
		IAISpawnable* AISpawnable = Cast<IAISpawnable>(OwnerActor);
		if (AISpawnable != nullptr)
		{
			//return AISpawnable->Execute_CanBeSpawned(OwnerActor);
			return AISpawnable->Execute_CanBeSpawned(OwnerActor);
		}
	}
	return true;
}

void UNPAISpawnerComponent::RemoveCharacter(class AActor* DestroyedActor)
{
	ANPAICharacterBase* DestroyedCharacter = Cast<ANPAICharacterBase>(DestroyedActor);
	if (DestroyedCharacter)
	{
		SpawnedCharacters.Remove(DestroyedCharacter);
		if (DestroyedCharacter->ActorHasTag("QuickReset") || DestroyedCharacter->ActorHasTag("RemovedByGoHome"))
		{
			LeftCharacterAmountToSpawn++;
		}
	}
}

void UNPAISpawnerComponent::LoadAISpawner()
{
	if (ANPAIManager* AIManager = ANPAIManager::GetAIManager(this))
	{
		AIManager->LoadAISpawner(this);
	}
}

void UNPAISpawnerComponent::OnTimeUpdated(struct FNPTimeStruct UpdatedTime, struct FNPTimeStruct DifTime)
{
	CurrentUpdatedTime = UpdatedTime;

	if (!CanSpawnByTime())
	{
		TimeToGoHome();
		return;
	}

	if (!CanBeSpawned())
	{
		return;
	}

	if (IsValid(NextCharacterClassToSpawn) == false)
	{
		return;
	}

	if (!CanSpawnByRange())
	{
		return;
	}

	LeftTimeToSpawnNPAICharacter.UpdateTime(FNPTimeStruct(0, 0, -1));
	if (CanSpawnByCooldown())
	{
		ResetLeftTimeToSpawn();
		if (UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(this))
		{
			SpawnCharacter(GetOwner()->GetTransform());
			LeftCharacterAmountToSpawn--;
			NextCharacterClassToSpawn = RandomCharacterToSpawn();
			UploadSpawnRange();
		}
	}
}

bool UNPAISpawnerComponent::CanSpawnByRange()
{
	if (SqrDistanceToLoad <= 0)
	{
		return true;
	}
	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Character)
	{
		return true;
	}
	float SqrDistanceToPlayer = UKismetMathLibrary::Vector_DistanceSquared(Character->GetActorLocation(), GetOwner()->GetActorLocation());
	if (SqrDistanceToPlayer < SqrDistanceToLoad)
	{
		return true;
	}
	return false;
}

void UNPAISpawnerComponent::ResetLeftTimeToSpawn()
{
	LeftTimeToSpawnNPAICharacter = FNPTimeStruct::RandomTimeInRange(MinRandomSpawnTime, MaxRandomSpawnTime);
}

bool UNPAISpawnerComponent::CanSpawnByCooldown()
{
	if (LeftTimeToSpawnNPAICharacter.TotalTime <= 0)
	{
		return true;
	}
	return false;
}

TSubclassOf<class ANPAICharacterBase> UNPAISpawnerComponent::RandomCharacterToSpawn()
{
	TSubclassOf<class ANPAICharacterBase> OutNPAICharacterBase;
	if (AICharactersClassToSpawn.Num() != 0)
	{
		int IndexCharacter = UKismetMathLibrary::RandomIntegerInRange(0, AICharactersClassToSpawn.Num() - 1);
		OutNPAICharacterBase = AICharactersClassToSpawn[IndexCharacter].LoadSynchronous();
	}
	return OutNPAICharacterBase;
}

void UNPAISpawnerComponent::TimeToGoHome()
{

}

void UNPAISpawnerComponent::UploadSpawnRange()
{
	if (NextCharacterClassToSpawn)
	{
		ANPAICharacterBase* AICharacterBase = Cast<ANPAICharacterBase>(NextCharacterClassToSpawn->GetDefaultObject(true));
		if (AICharacterBase)
		{
			SqrDistanceToLoad = AICharacterBase->DistanceToLoad * AICharacterBase->DistanceToLoad;
		}
	}
}

bool UNPAISpawnerComponent::CanSpawnByTime()
{
	return FNPTimeStruct::IsInTimeRange(CurrentUpdatedTime, BeginValidTime, EndValidTime);
}

void UNPAISpawnerComponent::AddSpawnedCharacter(class ANPAICharacterBase* SpawnedCharacter)
{
	SpawnedCharacters.Add(SpawnedCharacter);
	SpawnedCharacter->OnDestroyed.AddDynamic(this, &UNPAISpawnerComponent::RemoveCharacter);
}


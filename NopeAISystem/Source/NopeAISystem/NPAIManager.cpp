// Szymon XIII Wielki
#include "NPAIManager.h"
#include "NPAISubsystem.h"
#include "Characters/NPAICharacterBase.h"
#include "AI/NPAIDefines.h"
#include <Kismet/GameplayStatics.h>
#include "Tasks/NPAITaskBase.h"
#include <Animation/AnimMontage.h>
#include <GameFramework/CharacterMovementComponent.h>
#include "Characters/NopeAnimInstance.h"
#include "AI/Components/NPAISpawnerComponent.h"
#include "NPAISystemSettings.h"
#include "AI/NPAIControllerBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogNPCSub, Log, All);

ANPAIManager::ANPAIManager(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.5f;
}

void ANPAIManager::BeginPlay()
{
	Super::BeginPlay();

}

void ANPAIManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ANPAIManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	StreamingCharacters();
}

ANPAIManager* ANPAIManager::GetAIManager(class UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (!World)
	{
		return nullptr;
	}

	UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(World);

	if (ensure(NPAISubsystem))
	{
		return NPAISubsystem->NPAIManager;
	}

	return nullptr;
}

FAIUpdateActorIndex ANPAIManager::SaveAICharacter(class ANPAICharacterBase* NPAICharacterBase, bool Valid, bool LoadedByDistance)
{
	FAIUpdateActorIndex AIUpdateActorIndex;

	if (const UNPAISystemSettings* MTAISystemSettings = GetDefault<UNPAISystemSettings>())
	{
		if (!MTAISystemSettings->bEnableAISave)
		{
			AIUpdateActorIndex.bCanUpdate = false;
			return AIUpdateActorIndex;
		}
	}
	if (!NPAICharacterBase->bCanBeSaved)
	{
		return AIUpdateActorIndex;
	}

	AIUpdateActorIndex = FoundCharacter(NPAICharacterBase);

	// Collect Data
	FSaveDataNPAICharacter SaveDataNPAICharacter;
	SaveDataNPAICharacter.Transform = NPAICharacterBase->GetTransform();

	SaveDataNPAICharacter.Tags = NPAICharacterBase->Tags;

	if (NPAICharacterBase->GetCurrentActiveAITask())
	{
		NPAICharacterBase->GetCurrentActiveAITask()->SaveTaskProgress();
	}

	SaveDataNPAICharacter.NPAIDataCharacter = NPAICharacterBase->NPAIDataCharacter;
	SaveDataNPAICharacter.AICharacterSoft = TSoftObjectPtr<ANPAICharacterBase>(NPAICharacterBase);
	SaveDataNPAICharacter.AICharacterClass = TSoftClassPtr<ANPAICharacterBase>(NPAICharacterBase->GetClass());
	SaveDataNPAICharacter.OwnerActor = NPAICharacterBase->GetOwnerActor();
	SaveDataNPAICharacter.bIsValid = Valid;
	SaveDataNPAICharacter.bIsLoadedByDistance = LoadedByDistance;
	SaveDataNPAICharacter.DistanceToLoad = NPAICharacterBase->DistanceToLoad;
	SaveDataNPAICharacter.DistanceToUnload = NPAICharacterBase->DistanceToUnload;

#pragma region SaveAnimation
	if (UNopeAnimInstance* NopeAnimInstance = Cast<UNopeAnimInstance>(NPAICharacterBase->GetMesh()->GetAnimInstance()))
	{
		NopeAnimInstance->SaveAnimInstance(SaveDataNPAICharacter.AnimationSaveStruct);
	}
#pragma endregion 

	SaveDataNPAICharacter.bCollisionEnabled = NPAICharacterBase->GetActorEnableCollision();
	SaveDataNPAICharacter.MovementMode = NPAICharacterBase->GetCharacterMovement()->GetGroundMovementMode();
	/////////////

	if (AIUpdateActorIndex.ActorSpawnType == EActorSpawnType::Placed)
	{
		if (AIUpdateActorIndex.Index != -1)
		{
			AIData.PlacedAICharacters[AIUpdateActorIndex.Index] = SaveDataNPAICharacter;
		}
		else
		{
			AIData.PlacedAICharacters.Add(SaveDataNPAICharacter);
		}
	}
	else
	{
		if (SaveDataNPAICharacter.bIsValid == true)
		{
			if (AIUpdateActorIndex.Index != -1)
			{
				AIData.SpawnedAICharacters[AIUpdateActorIndex.Index] = SaveDataNPAICharacter;
			}
			else
			{
				AIData.SpawnedAICharacters.Add(SaveDataNPAICharacter);
			}
		}
		else
		{
			// If Animal Is Spawned Remove Him
			if (AIUpdateActorIndex.Index != -1)
			{
				RemoveCharacterFromData(EActorSpawnType::Spawned, AIUpdateActorIndex.Index);
				AIUpdateActorIndex.bCanUpdate = false;
			}
		}
	}
	return AIUpdateActorIndex;
}

FAIUpdateActorIndex ANPAIManager::LoadAICharacter(class ANPAICharacterBase* NPAICharacterBase)
{
	FAIUpdateActorIndex AIUpdateActorIndex;
	if (const UNPAISystemSettings* MTAISystemSettings = GetDefault<UNPAISystemSettings>())
	{
		if (!MTAISystemSettings->bEnableAILoad)
		{
			return AIUpdateActorIndex;
		}
	}

	AIUpdateActorIndex.bCanUpdate = UpdateValidAICharacters(NPAICharacterBase);
	if (AIUpdateActorIndex.bCanUpdate == false)
	{
		return AIUpdateActorIndex;
	}

	AIUpdateActorIndex = FoundCharacter(NPAICharacterBase);
	if (AIUpdateActorIndex.Index != -1)
	{
		FSaveDataNPAICharacter SaveDataNPAICharacter;
		if (AIUpdateActorIndex.ActorSpawnType == EActorSpawnType::Placed)
		{
			LoadAICharacter(NPAICharacterBase, AIData.PlacedAICharacters[AIUpdateActorIndex.Index]);
		}
		else
		{
			LoadAICharacter(NPAICharacterBase, AIData.SpawnedAICharacters[AIUpdateActorIndex.Index]);
		}
	}
	else
	{
		if (NPAICharacterBase->GetOwnerActor())
		{
			UNPAISpawnerComponent* NPAISpawnerComponent = Cast<UNPAISpawnerComponent>(NPAICharacterBase->GetOwnerActor()->GetComponentByClass(UNPAISpawnerComponent::StaticClass()));
			if (NPAISpawnerComponent)
			{
				NPAICharacterBase->NPAIDataCharacter = NPAISpawnerComponent->NPAIDataCharacter;
			}
		}
	}
	return AIUpdateActorIndex;
}

void ANPAIManager::LoadAICharacter(class ANPAICharacterBase* NPAICharacterBase, const FSaveDataNPAICharacter& SaveDataNPAICharacter)
{
	if (SaveDataNPAICharacter.NPAIDataCharacter.AITasksList.Num() != 0)
	{
		NPAICharacterBase->NPAIDataCharacter.AITasksList = SaveDataNPAICharacter.NPAIDataCharacter.AITasksList;
	}
	NPAICharacterBase->Tags = SaveDataNPAICharacter.Tags;
	NPAICharacterBase->SetActorEnableCollision(SaveDataNPAICharacter.bCollisionEnabled);
	NPAICharacterBase->GetCharacterMovement()->SetMovementMode((EMovementMode)SaveDataNPAICharacter.MovementMode);

	if (UNopeAnimInstance* NopeAnimInstance = Cast<UNopeAnimInstance>(NPAICharacterBase->GetMesh()->GetAnimInstance()))
	{
		NopeAnimInstance->LoadAnimInstance(SaveDataNPAICharacter.AnimationSaveStruct);
	}
}

void ANPAIManager::SaveAISpawner(class UNPAISpawnerComponent* NPAISpawnerComponent, bool Valid)
{
	TSoftObjectPtr<UNPAISpawnerComponent> NPAISpawnerComponentSoft = TSoftObjectPtr<UNPAISpawnerComponent>(NPAISpawnerComponent);
	FSaveDataNPAISpawner NewSaveDataNPAISpawner;
	NewSaveDataNPAISpawner.AISpawnerComponentSoft = NPAISpawnerComponentSoft;
	NewSaveDataNPAISpawner.NPAIDataCharacter = NPAISpawnerComponent->NPAIDataCharacter;
	NewSaveDataNPAISpawner.LeftTimeToSpawnNPAICharacter = NPAISpawnerComponent->LeftTimeToSpawnNPAICharacter;
	NewSaveDataNPAISpawner.AICharactersClassToSpawn = NPAISpawnerComponent->AICharactersClassToSpawn;
	NewSaveDataNPAISpawner.LeftCharacterAmountToSpawn = NPAISpawnerComponent->LeftCharacterAmountToSpawn;
	for (int i = 0; i < AIData.SpawnerData.Num() - 1; i++)
	{
		if (AIData.SpawnerData[i].AISpawnerComponentSoft == NPAISpawnerComponentSoft)
		{
			if (Valid)
			{
				AIData.SpawnerData[i] = NewSaveDataNPAISpawner;
			}
			else
			{
				AIData.SpawnerData.RemoveAt(i);
			}
			break;
		}
	}
	AIData.SpawnerData.Add(NewSaveDataNPAISpawner);
}

void ANPAIManager::LoadAISpawner(class UNPAISpawnerComponent* NPAISpawnerComponent)
{
	TSoftObjectPtr<UNPAISpawnerComponent> NPAISpawnerComponentSoft = TSoftObjectPtr<UNPAISpawnerComponent>(NPAISpawnerComponent);
	for (FSaveDataNPAISpawner& SaveDataNPAISpawner : AIData.SpawnerData)
	{
		if (SaveDataNPAISpawner.AISpawnerComponentSoft == NPAISpawnerComponentSoft)
		{
			NPAISpawnerComponent->NPAIDataCharacter = SaveDataNPAISpawner.NPAIDataCharacter;
			NPAISpawnerComponent->LeftTimeToSpawnNPAICharacter = SaveDataNPAISpawner.LeftTimeToSpawnNPAICharacter;
			NPAISpawnerComponent->AICharactersClassToSpawn = SaveDataNPAISpawner.AICharactersClassToSpawn;
			NPAISpawnerComponent->LeftCharacterAmountToSpawn = SaveDataNPAISpawner.LeftCharacterAmountToSpawn;
			break;
		}
	}
}

void ANPAIManager::RemoveCharacterFromData(EActorSpawnType ActorSpawnType, int Index)
{
	if (ActorSpawnType == EActorSpawnType::Spawned)
	{
		if (ensure(AIData.SpawnedAICharacters.Num() > Index))
		{
			AIData.SpawnedAICharacters.RemoveAt(Index);
		}
	}
}

bool ANPAIManager::UpdateValidAICharacters(class ANPAICharacterBase* NPAICharacterBase)
{
	if (NPAICharacterBase->HasAnyFlags(RF_WasLoaded))
	{
		for (FSaveDataNPAICharacter& LoadedCharacter : AIData.PlacedAICharacters)
		{
			if (LoadedCharacter.AICharacterSoft == NPAICharacterBase)
			{
				if (!LoadedCharacter.bIsValid)
				{
					NPAICharacterBase->Destroy();
					return false;
				}
				break;
			}
		}
	}
	return true;
}

void ANPAIManager::StreamingCharacters()
{
	if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		ControlCharactersByDistance(Character->GetActorLocation());
	}
}


ANPAICharacterBase* ANPAIManager::SpawnAICharacter(class UObject* InOwner, TSoftClassPtr<ANPAICharacterBase> NPAICharacterBaseClass, FTransform SpawnTransform, const TArray<FNPAITaskStruct>& InTasksList, bool bEditableModeActive /*= false*/)
{
	ANPAICharacterBase* NPAICharacterBase = nullptr;
	APawn* NewInstigator = nullptr;
	AActor* NewOwner = nullptr;
	if (InOwner && InOwner->GetWorld())
	{
		NewOwner = Cast<AActor>(InOwner);
		if (!NewOwner)
			NewOwner = ANPAIManager::GetAIManager(InOwner);

		NewInstigator = Cast<APawn>(NewOwner);

		TSubclassOf<ANPAICharacterBase> AICharacterClass = NPAICharacterBaseClass.LoadSynchronous();
		//spawn the actor
		NPAICharacterBase = NewOwner->GetWorld()->SpawnActorDeferred<ANPAICharacterBase>(
			AICharacterClass, SpawnTransform, NewOwner, NewInstigator, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (InTasksList.Num() != 0)
		{
			NPAICharacterBase->NPAIDataCharacter.AITasksList = InTasksList;
		}
		NPAICharacterBase->SetOwnerActor(TSoftObjectPtr<AActor>(NewOwner));
		// initialize NewActor...
		UGameplayStatics::FinishSpawningActor(NPAICharacterBase, SpawnTransform);

		if (NPAICharacterBase && NPAICharacterBase->Controller == NULL)
		{	// NOTE: SpawnDefaultController ALSO calls Possess() to possess the pawn (if a controller is successfully spawned).
			NPAICharacterBase->SpawnDefaultController();
		}
	}
	return NPAICharacterBase;
}

ANPAICharacterBase* ANPAIManager::SpawnAICharacterFromSave(class UObject* InOwner, TSoftClassPtr<ANPAICharacterBase> NPAICharacterBaseClass, FTransform SpawnTransform, FSaveDataNPAICharacter& SaveDataNPAICharacter)
{
	ANPAICharacterBase* NPAICharacterBase = nullptr;
	APawn* NewInstigator = nullptr;
	AActor* NewOwner = nullptr;
	if (InOwner && InOwner->GetWorld())
	{
		NewOwner = Cast<AActor>(InOwner);
		if (!NewOwner)
			NewOwner = ANPAIManager::GetAIManager(InOwner);

		NewInstigator = Cast<APawn>(NewOwner);

		TSubclassOf<ANPAICharacterBase> AICharacterClass = NPAICharacterBaseClass.LoadSynchronous();
		//spawn the actor
		NPAICharacterBase = NewOwner->GetWorld()->SpawnActorDeferred<ANPAICharacterBase>(
			AICharacterClass, SpawnTransform, NewOwner, NewInstigator, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		//FromSave
		SaveDataNPAICharacter.AICharacterSoft = TSoftObjectPtr<ANPAICharacterBase>(NPAICharacterBase);
		NPAICharacterBase->SetActorEnableCollision(SaveDataNPAICharacter.bCollisionEnabled);
		NPAICharacterBase->GetCharacterMovement()->SetMovementMode((EMovementMode)SaveDataNPAICharacter.MovementMode);

		NPAICharacterBase->SetOwnerActor(TSoftObjectPtr<AActor>(NewOwner));
		// initialize NewActor...
		UGameplayStatics::FinishSpawningActor(NPAICharacterBase, SpawnTransform);

		if (NPAICharacterBase && NPAICharacterBase->Controller == NULL)
		{	// NOTE: SpawnDefaultController ALSO calls Possess() to possess the pawn (if a controller is successfully spawned).
			NPAICharacterBase->SpawnDefaultController();
		}
	}
	return NPAICharacterBase;
}

void ANPAIManager::SaveAICharacters()
{
	if (UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(this->GetWorld()))
	{
		for (ANPAICharacterBase* NPAICharacterBase : NPAISubsystem->GetAICharacters())
		{
			SaveAICharacter(NPAICharacterBase, true, true);
		}
	}
}

void ANPAIManager::SaveSpawners()
{
	if (UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(this->GetWorld()))
	{
		for (UNPAISpawnerComponent* NPAISpawnerComponent : NPAISubsystem->GetAISpawnerComponents())
		{
			SaveAISpawner(NPAISpawnerComponent, true);
		}
	}
}

void ANPAIManager::SaveAIData(struct FNPAIData& OutAIData)
{
	SaveAICharacters();
	SaveSpawners();
	OutAIData = AIData;
	// Reset to allow spawn them on next load game
	for (FSaveDataNPAICharacter& SaveDataNPAICharacter : OutAIData.SpawnedAICharacters)
	{
		SaveDataNPAICharacter.bIsLoadedByDistance = false;
	}
}

void ANPAIManager::LoadAIData(struct FNPAIData InAIData)
{
	AIData = InAIData;
	StreamingCharacters();
}

void ANPAIManager::ControlCharactersByDistance(FVector PlayerPosition)
{
	for (int i = AIData.SpawnedAICharacters.Num() - 1; i >= 0; i--)
	{
		bool CheckDistance = true;
		if (AIData.SpawnedAICharacters[i].DistanceToLoad <= 0)
		{
			CheckDistance = false;
		}
		if (AIData.SpawnedAICharacters[i].bIsValid)
		{
			float SqrDistToLoad = 0.0f;
			float SqrDistToUnLoad = 0.0f;
			float SqrDistanceToPlayer = 0.0f;
			if (CheckDistance)
			{
				FVector NPCLocation = AIData.SpawnedAICharacters[i].Transform.GetLocation();
				//If Character is Valid get him current position to unload
				if (ANPAICharacterBase* NPAICharacterBase = AIData.SpawnedAICharacters[i].AICharacterSoft.LoadSynchronous())
				{
					NPCLocation = NPAICharacterBase->GetActorLocation();
				}
				SqrDistToLoad = AIData.SpawnedAICharacters[i].DistanceToLoad * AIData.SpawnedAICharacters[i].DistanceToLoad;
				SqrDistanceToPlayer = FVector::DistSquared(PlayerPosition, NPCLocation);
			}
			if (!AIData.SpawnedAICharacters[i].bIsLoadedByDistance && (!CheckDistance || SqrDistanceToPlayer < SqrDistToLoad))
			{
				const TSoftClassPtr<class ANPAICharacterBase> AICharacterClass = AIData.SpawnedAICharacters[i].AICharacterClass;
				AActor* OwnerActor = AIData.SpawnedAICharacters[i].OwnerActor.LoadSynchronous();
				if (OwnerActor)
				{
					UNPAISpawnerComponent* NPAISpawnerComponent = Cast<UNPAISpawnerComponent>(OwnerActor->GetComponentByClass(UNPAISpawnerComponent::StaticClass()));
					if (NPAISpawnerComponent && NPAISpawnerComponent->CanSpawnByTime())
					{
						ANPAICharacterBase* NPAICharacterBase = SpawnAICharacterFromSave(OwnerActor, AICharacterClass, AIData.SpawnedAICharacters[i].Transform, AIData.SpawnedAICharacters[i]);
						// Update Data
						AIData.SpawnedAICharacters[i].bIsLoadedByDistance = true;
						AIData.SpawnedAICharacters[i].bIsValid = true;
					}
					else
					{
						RemoveCharacterFromData(EActorSpawnType::Spawned, i);
						if (NPAISpawnerComponent)
						{
							NPAISpawnerComponent->LeftCharacterAmountToSpawn++;
							continue;
						}
					}
				}
			}
			else if (AIData.SpawnedAICharacters[i].bIsLoadedByDistance && CheckDistance)
			{
				SqrDistToUnLoad = SqrDistToLoad * AIData.SpawnedAICharacters[i].DistanceToUnload * AIData.SpawnedAICharacters[i].DistanceToUnload;
				if (SqrDistanceToPlayer > SqrDistToUnLoad)
				{
					if (ANPAICharacterBase* NPAICharacterBase = AIData.SpawnedAICharacters[i].AICharacterSoft.LoadSynchronous())
					{
						NPAICharacterBase->Tags.Add("RemovedByDistance");
						NPAICharacterBase->Destroy();
					}
					// Update Data
					AIData.SpawnedAICharacters[i].bIsLoadedByDistance = false;
					AIData.SpawnedAICharacters[i].bIsValid = true;
				}
			}
		}
	}
}

FAIUpdateActorIndex ANPAIManager::FoundCharacter(class ANPAICharacterBase* NPAICharacterBase)
{
	FAIUpdateActorIndex AIUpdateActorIndex;
	TSoftObjectPtr<ANPAICharacterBase> CharacterSoft = TSoftObjectPtr<ANPAICharacterBase>(NPAICharacterBase);
	if (NPAICharacterBase->IsNetStartupActor())
	{
		AIUpdateActorIndex.ActorSpawnType = EActorSpawnType::Placed;
		for (int i = 0; i < AIData.PlacedAICharacters.Num(); i++)
		{
			if (AIData.PlacedAICharacters[i].AICharacterSoft == CharacterSoft)
			{
				AIUpdateActorIndex.Index = i;
				return AIUpdateActorIndex;
			}
		}
	}
	else
	{
		AIUpdateActorIndex.ActorSpawnType = EActorSpawnType::Spawned;
		for (int i = 0; i < AIData.SpawnedAICharacters.Num(); i++)
		{
			if (AIData.SpawnedAICharacters[i].AICharacterSoft == CharacterSoft)
			{
				AIUpdateActorIndex.Index = i;
				return AIUpdateActorIndex;
			}
		}
	}
	return AIUpdateActorIndex;
}

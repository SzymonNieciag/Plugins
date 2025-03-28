// Copyright 2018 DRAGO. All Rights Reserved.

#include "NPAILibrary.h"
#include "../Characters/NPAICharacterBase.h"
#include "../Tasks/NPAITaskBase.h"
#include "../NPAISubsystem.h"
#include "Components/NPSmartObjectComponent.h"
#include "../Library/SpawnNPAICharacterAction.h"
#include "NPAIDefines.h"
#include <Engine/HitResult.h>
#include <../../../../../../../Source/Runtime/NavigationSystem/Public/NavigationSystem.h>
#include <../../../../../../../Source/Runtime/AIModule/Classes/AIController.h>

bool UNPAILibrary::IsRunningInEditorPreview(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return false;
	}

	if (auto world = WorldContextObject->GetWorld())
	{
		return world->WorldType == EWorldType::EditorPreview;
	}

	return false;
}

class UNPAITaskBase* UNPAILibrary::SpawnAITask(ANPAICharacterBase* NPAICharacterBase)
{
	if (NPAICharacterBase->NPAIDataCharacter.AITasksList.Num() == 0 || NPAICharacterBase->IsActorBeingDestroyed())
	{
		return nullptr;
	}
	UNPAITaskBase* NewTask = nullptr;
	TSubclassOf<UNPAITaskBase> NewTaskClass = NPAICharacterBase->NPAIDataCharacter.AITasksList[0].AITaskBaseClass.LoadSynchronous();

	if (NewTaskClass == NULL)
	{
		return nullptr;
	}

	NewTask = NewObject<UNPAITaskBase>(NPAICharacterBase, NewTaskClass);
	NewTask->InitializeTask(NPAICharacterBase, NPAICharacterBase->NPAIDataCharacter.AITasksList[0]);
	return NewTask;
}

AActor* UNPAILibrary::GetClosestActorInRange(FVector InstigatorLocation, const TArray<class AActor*>& InActors, const float SearchRange /*= 0.0f*/)
{
	if (InActors.Num() != 0)
	{
		float AveDistance = 999999999999.0f;

		if (SearchRange != 0.0f)
		{
			AveDistance = SearchRange * SearchRange;
		}

		AActor* ClosestActor = nullptr;

		for (int i = 0; i < InActors.Num(); i++)
		{
			if (InActors[i] != nullptr)
			{
				float CurrentDistance = FVector::DistSquared(InstigatorLocation, InActors[i]->GetActorLocation());
				if (CurrentDistance < AveDistance)
				{
					AveDistance = CurrentDistance;
					ClosestActor = InActors[i];
				}
			}
		}
		return ClosestActor;
	}
	return nullptr;
}

const FHitResult UNPAILibrary::GetClosestHitResultInRange(FVector InstigatorLocation, const TArray<struct FHitResult>& InHitResults, const float SearchRange /*= 0.0f*/)
{
	if (InHitResults.Num() != 0)
	{
		float AveDistance = 999999999999.0f;

		if (SearchRange != 0.0f)
		{
			AveDistance = SearchRange * SearchRange;
		}

		FHitResult HitResult;

		for (int i = 0; i < InHitResults.Num(); i++)
		{
			float CurrentDistance = FVector::DistSquared(InstigatorLocation, InHitResults[i].ImpactPoint);
			if (CurrentDistance < AveDistance)
			{
				AveDistance = CurrentDistance;
				HitResult = InHitResults[i];
			}
		}
		return HitResult;
	}
	return FHitResult();
}

void UNPAILibrary::GetObjectsInRange(FVector InstigatorLocation, const TArray<class UObject*>& InObjects, TArray<class UObject*>& OutObjects, const float SearchRange /*= 0.0f*/)
{
	const float SqrRange = SearchRange * SearchRange;
	for (int i = 0; i < InObjects.Num(); i++)
	{
		if (InObjects[i] != nullptr)
		{
			AActor* Actor = Cast<AActor>(InObjects[i]);
			if (!Actor)
			{
				Actor = Cast<AActor>(InObjects[i]->GetOuter());
			}
			if (Actor)
			{
				float SqrDistance = FVector::DistSquared(InstigatorLocation, Actor->GetActorLocation());
				if (SqrDistance < SqrRange)
				{
					OutObjects.Add(InObjects[i]);
				}
			}
		}
	}
}

UNPSmartObjectComponent* UNPAILibrary::FindSmartObjectsByClass(class ACharacter* AICharacterBase, TArray<AActor*>& OutActors, class AActor*& ClosestOutActor, TSoftClassPtr<class AActor> SearchActorClass, const float SearchRange /*= 0.0f*/)
{
	UNPSmartObjectComponent* OutNPSmartObjectComponent = nullptr;
	if (!AICharacterBase || !AICharacterBase->GetWorld())
	{
		return OutNPSmartObjectComponent;
	}
	TSubclassOf<AActor> ActorClass = SearchActorClass.LoadSynchronous();
	UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(AICharacterBase->GetWorld());
	if (NPAISubsystem)
	{
		const float SqrSearchRange = SearchRange * SearchRange;
		for (UNPSmartObjectComponent* NPSmartObject : NPAISubsystem->GetSmartObjectComponents())
		{
			if (NPSmartObject->CanReserveActor(AICharacterBase))
			{
				AActor* OwnerActor = NPSmartObject->GetOwner();
				if (OwnerActor->GetClass()->IsChildOf(ActorClass))
				{
					if (SearchRange <= 0)
					{
						OutActors.Add(OwnerActor);
					}
					else
					{
						float SqrDistance = FVector::DistSquared(OwnerActor->GetActorLocation(), AICharacterBase->GetNavAgentLocation());
						if (SqrDistance < SqrSearchRange)
						{
							OutActors.Add(OwnerActor);
						}
					}
				}
			}
		}
	}

	if (OutActors.Num() != 0)
	{
		ClosestOutActor = OutActors[0];
		if (ClosestOutActor)
		{
			UActorComponent* ActorComponent = ClosestOutActor->GetComponentByClass(UNPSmartObjectComponent::StaticClass());
			if (ActorComponent)
			{
				OutNPSmartObjectComponent = Cast<UNPSmartObjectComponent>(ActorComponent);
			}
		}
	}
	return OutNPSmartObjectComponent;
}

void UNPAILibrary::SpawnAICharacterAsync(UObject* WorldContextObject, TSoftClassPtr<ANPAICharacterBase> InAICharacterBaseClass, FTransform InSpawnTransform, const TArray<FNPAITaskStruct>& InTasksList, bool bEditableModeActive, FLatentActionInfo LatentInfo, ANPAICharacterBase*& OutAICharacterBase)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (World)
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FSpawnNPAICharacterAction* SpawnNPAICharacterAction = LatentActionManager.FindExistingAction<FSpawnNPAICharacterAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		//If not currently running
		if (!SpawnNPAICharacterAction)
		{
			SpawnNPAICharacterAction = new FSpawnNPAICharacterAction(World, InAICharacterBaseClass, InSpawnTransform, InTasksList, bEditableModeActive, LatentInfo, OutAICharacterBase);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, SpawnNPAICharacterAction);
		}
	}

	UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(World);
	if (NPAISubsystem)
	{
		FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
		FStreamableDelegate StreamableDelegate;

		TArray<FSoftObjectPath> ToStream;
		ToStream.Add(InAICharacterBaseClass.ToString());

		StreamableManager.RequestAsyncLoad(ToStream, FStreamableDelegate::CreateUObject(NPAISubsystem, &UNPAISubsystem::OnAssetsLoaded));
	}
}

void UNPAILibrary::ChangePropertyValue(UObject* Object, const FName& PropertyName, float day, float hour, float minute)
{
	if (Object)
	{
		if (UClass* Class = Object->GetClass())
		{
			if (UObject* DefaultObject = Class->GetDefaultObject())
			{
				if (FProperty* Property = Class->FindPropertyByName(PropertyName))
				{
					if (FNPTimeStruct* TimeStruct = Property->ContainerPtrToValuePtr<FNPTimeStruct>(DefaultObject))
					{
						TimeStruct->Days = day;
						TimeStruct->Hours = hour;
						TimeStruct->Minutes = minute;
					}
				}
			}
		}
	}
}

void UNPAILibrary::SetTaskTimeToZero(TArray<struct FNPAITaskStruct>& NPAITaskStructs, TSubclassOf<class UNPAITaskBase> TaskFilter)
{
	for (FNPAITaskStruct& NPAITaskStruct : NPAITaskStructs)
	{
		if (TaskFilter == NPAITaskStruct.AITaskBaseClass->GetClass())
		{
			for (FNPAITaskProgress& InAITaskProgress : NPAITaskStruct.TasksProgress)
			{
				InAITaskProgress.LeftTaskTime = 0;
			}
		}
	}
}


// Copyright Epic Games, Inc. All Rights Reserved.

#include "NPSmartObjectComponent.h"
#include "NopeAISystem/Characters/NPAICharacterBase.h"
#include "NopeAISystem/NPAISubsystem.h"
#include "Engine/World.h"
#include <Kismet/KismetMathLibrary.h>
#include "../NPAIDefines.h"

#include <../Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/SmartObjectDefinition.h>
#include <../Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/Annotations/SmartObjectSlotEntranceAnnotation.h>

UNPSmartObjectComponent::UNPSmartObjectComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsOnUpdateTransform = true;
}

void UNPSmartObjectComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateSmartPoints();
}

void UNPSmartObjectComponent::PostLoad()
{
	Super::PostLoad();
}

void UNPSmartObjectComponent::PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph)
{
	Super::PostLoadSubobjects(OuterInstanceGraph);
}

void UNPSmartObjectComponent::OnRegister()
{
	Super::OnRegister();
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

#if WITH_EDITOR
	// Do not process any component registered to preview world
	if (World->WorldType == EWorldType::EditorPreview)
	{
		return;
	}
#endif
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
	UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(GetWorld());
	if (NPAISubsystem)
	{
		NPAISubsystem->AddSmartObject(this);
	}
}

void UNPSmartObjectComponent::OnUnregister()
{
	Super::OnUnregister();
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

#if WITH_EDITOR
	// Do not process any component registered to preview world
	if (World->WorldType == EWorldType::EditorPreview)
	{
		return;
	}
#endif

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
	UNPAISubsystem* NPAISubsystem = UNPAISubsystem::GetAISubsystem(GetWorld());
	if (NPAISubsystem)
	{
		NPAISubsystem->RemoveSmartObject(this);
	}
}

void UNPSmartObjectComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport /*= ETeleportType::None*/)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
}

#if WITH_EDITOR
void UNPSmartObjectComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

FSmartPointStruct UNPSmartObjectComponent::StartReserveActor(class ANPAICharacterBase* AICharacterBase, bool TryReserveFirstSpot /*= true*/)
{
	FSmartPointStruct OutSmartPoint;
	if (AICharacterBase == nullptr)
	{
		return OutSmartPoint;
	}
	if (!CanReserveActor(AICharacterBase))
	{
		return OutSmartPoint;
	}

	if (TryReserveFirstSpot)
	{
		if (SmartPointsStruct[0].CurrentCharacter == nullptr)
		{
			StopReserveActor(AICharacterBase);
			AICharacterBase->SetReservedSmartObject(this);
			SmartPointsStruct[0].CurrentCharacter = AICharacterBase;
			UpdateSmartPointTransform(SmartPointsStruct[0], 0);
			return SmartPointsStruct[0];
		}
	}

	OutSmartPoint = FindCharacter(AICharacterBase);
	if (OutSmartPoint.CurrentCharacter)
	{
		return OutSmartPoint;
	}

	for (int i = 0; i < SmartPointsStruct.Num(); i++)
	{
		if (SmartPointsStruct[i].CurrentCharacter == nullptr || !bReservePoint)
		{
			SmartPointsStruct[i].CurrentCharacter = AICharacterBase;
			UpdateSmartPointTransform(SmartPointsStruct[i], i);
			AICharacterBase->SetReservedSmartObject(this);
			return SmartPointsStruct[i];
		}
	}

	return OutSmartPoint;
}

FSmartPointStruct UNPSmartObjectComponent::StopReserveActor(class ANPAICharacterBase* AICharacterBase)
{
	for (FSmartPointStruct& SmartPoint : SmartPointsStruct)
	{
		if (SmartPoint.CurrentCharacter == AICharacterBase)
		{
			SmartPoint.CurrentCharacter = nullptr;
			return SmartPoint;
		}
	}
	return FSmartPointStruct();
}

FSmartPointStruct UNPSmartObjectComponent::FindCharacter(class ANPAICharacterBase* AICharacterBase)
{
	for (FSmartPointStruct& SmartPoint : SmartPointsStruct)
	{
		if (SmartPoint.CurrentCharacter == AICharacterBase || !bReservePoint)
		{
			return SmartPoint;
		}
	}
	return FSmartPointStruct();
}

bool UNPSmartObjectComponent::CanReserveActor(ACharacter* User)
{
	for (FSmartPointStruct& SmartPoint : SmartPointsStruct)
	{
		if (SmartPoint.CurrentCharacter == nullptr || SmartPoint.CurrentCharacter == User || !bReservePoint)
		{
			return true;
		}
	}
	return false;
}

void UNPSmartObjectComponent::UpdateSmartPoints()
{
	if (this->GetDefinition())
	{
		SmartPointsStruct.Empty();
		TConstArrayView<FSmartObjectSlotDefinition> DefinitionAssetSlots = this->GetDefinition()->GetSlots();
		for (int i = 0; i < DefinitionAssetSlots.Num(); i++)
		{
			SmartPointsStruct.Add(FSmartPointStruct(nullptr, i));
		}
	}
}

void UNPSmartObjectComponent::UpdateSmartPointTransform(FSmartPointStruct& SmartPointToUpdate, int Index)
{
	FTransform OwnerTransform = this->GetComponentTransform();
	TOptional<FTransform> SlotTransform = this->GetDefinition()->GetSlotWorldTransform(Index, OwnerTransform);
	if (SlotTransform.IsSet())
	{
		FTransform OutTransform = SlotTransform.GetValue();
		SmartPointToUpdate.WorldTransform = OutTransform;

		FSmartObjectSlotDefinition SmartObjectSlotDefinition = this->GetDefinition()->GetSlot(Index);

		for (int32 AnnotationIndex = 0; AnnotationIndex < SmartObjectSlotDefinition.DefinitionData.Num(); AnnotationIndex++)
		{
			FSmartObjectDefinitionDataProxy SmartObjectDefinitionDataProxy = SmartObjectSlotDefinition.DefinitionData[AnnotationIndex];
			if (FSmartObjectSlotEntranceAnnotation* Annotation = SmartObjectDefinitionDataProxy.Data.GetMutablePtr<FSmartObjectSlotEntranceAnnotation>())
			{
				FVector LocationOffset;
				LocationOffset.X = Annotation->Offset.X;
				LocationOffset.Y = Annotation->Offset.Y;
				LocationOffset.Z = Annotation->Offset.Z;

				FRotator RotationOffset;
				RotationOffset.Yaw = Annotation->Rotation.Yaw;
				RotationOffset.Pitch = Annotation->Rotation.Pitch;
				RotationOffset.Roll = Annotation->Rotation.Roll;

				FVector Location = UKismetMathLibrary::TransformLocation(OutTransform, LocationOffset);
				FRotator Rotation = UKismetMathLibrary::TransformRotation(OutTransform, RotationOffset);

				SmartPointToUpdate.SocketSnapTransform.SetRotation(Rotation.Quaternion());
				SmartPointToUpdate.SocketSnapTransform.SetLocation(Location);
			}
		}
	}
}

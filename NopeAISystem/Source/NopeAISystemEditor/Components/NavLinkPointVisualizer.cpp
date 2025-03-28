// Copyright Epic Games, Inc. All Rights Reserved.

#include "NavLinkPointVisualizer.h"
#include "NopeAISystem/Navigation/NPNavLinkCustomComponent.h"
#include "NopeAISystem/Navigation/NPNavLinkProxy.h"
#include "NopeAISystem/AI/Components/NPAISpawnerComponent.h"
#include "NavigationSystem.h"
#include <../../../../../../../Source/Runtime/NavigationSystem/Public/NavigationPath.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>
#include "NopeAISystem/Characters/NPAICharacterBase.h"
#include "NopeAISystem/AI/Components/NPCharacterMovementComponent.h"

void FNavLinkPointVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UNPNavLinkCustomComponent* LevelMetricsComponent = Cast<UNPNavLinkCustomComponent>(Component);
	if (LevelMetricsComponent)
	{
		// Get Owner data
		const AActor* OwnerActor = Component->GetOwner();
		const ANPNavLinkProxy* NPNavLinkProxy = Cast<ANPNavLinkProxy>(OwnerActor);
		const FVector OwnerLocation = OwnerActor->GetActorLocation();
		const float DrawRadius = LevelMetricsComponent->AcceptableRadius;

		TSubclassOf<UNavArea> asd = LevelMetricsComponent->GetObstacleAreaClass();
		const FVector Start = NPNavLinkProxy->GetSmartLinkComp()->GetStartPoint();
		const FVector End = NPNavLinkProxy->GetSmartLinkComp()->GetEndPoint();

		DrawWireCylinder(PDI,
			Start,
			FVector(1, 0, 0),		// The cylinder matrix, here it’s just static
			FVector(0, 1, 0),
			FVector(0, 0, 1),
			FLinearColor::Green,	// Blue color
			DrawRadius, 75, 10,				// Various measurements for the cylinder
			SDPG_Foreground,		// Will render on top of everything
			2.f						// Line width
		);

		DrawWireCylinder(PDI,
			End,
			FVector(1, 0, 0),		// The cylinder matrix, here it’s just static
			FVector(0, 1, 0),
			FVector(0, 0, 1),
			FLinearColor::Green,	// Blue color
			DrawRadius, 75, 10,				// Various measurements for the cylinder
			SDPG_Foreground,		// Will render on top of everything
			2.f						// Line width
		);
	}

	TArray<FNPAITaskStruct> AITasksList;
	if (const UNPAISpawnerComponent* NPAISpawnerComponent = Cast<UNPAISpawnerComponent>(Component))
	{
		AITasksList = NPAISpawnerComponent->NPAIDataCharacter.AITasksList;
	}
	else if (const UNPCharacterMovementComponent* CharacterMovementComponent = Cast<UNPCharacterMovementComponent>(Component))
	{
		if (ANPAICharacterBase* NPAICharacterBase = Cast<ANPAICharacterBase>(CharacterMovementComponent->GetOwner()))
		{
			AITasksList = NPAICharacterBase->NPAIDataCharacter.AITasksList;
		}
	}

	if (AITasksList.Num() > 0)
	{
		FVector StartLocation = Component->GetOwner()->GetActorLocation();
		for (const FNPAITaskStruct& NPAITaskStruct : AITasksList)
		{
			AActor* Actor = NPAITaskStruct.TargetActor.LoadSynchronous();
			if (!Actor)
				break;
			UNavigationPath* NavigationPath = UNavigationSystemV1::FindPathToActorSynchronously(Actor->GetWorld(), StartLocation, Actor);
			if (!NavigationPath->IsValid())
				break;

			for (int i = 1; i < NavigationPath->PathPoints.Num(); i++)
			{
				PDI->DrawLine(NavigationPath->PathPoints[i - 1], NavigationPath->PathPoints[i], FLinearColor::Yellow, 1, 3, 0.0f, true);

				if (i + 1 == NavigationPath->PathPoints.Num())
				{
					const float ArrowLength = 30.0f;
					const float ArrowSize = 20.0f;
					const FColor ArrowColor = FColor::Yellow;

					FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(NavigationPath->PathPoints[i - 1], NavigationPath->PathPoints[i]);
					const FTransform Transform = FTransform(Rotator, NavigationPath->PathPoints[i]);

					//DrawDirectionalArrow(PDI, Transform.ToMatrixNoScale(), ArrowColor, ArrowLength, ArrowSize, SDPG_World, 3);
					DrawConnectedArrow(PDI, Transform.ToMatrixNoScale(), ArrowColor, 20, 20, 1, 1, 3);
				}

			}
			StartLocation = Actor->GetActorLocation();
		}
	}
}


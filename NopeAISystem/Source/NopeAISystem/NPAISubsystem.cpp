// Fill out your copyright notice in the Description page of Project Settings.

#include "NPAISubsystem.h"
#include "NPAISystemSettings.h"
#include "Characters/NPAICharacterBase.h"
#include "NPAIManager.h"
#include "AI/Components/NPSmartObjectComponent.h"
#include <Blueprint/UserWidget.h>
#include "AI/Components/NPAISpawnerComponent.h"

bool UNPAISubsystem::bGameLogicStarted = true;

int32 AIDebugSystem = 0;
FAutoConsoleVariableRef CVarNopeAISystemEnableAI(TEXT("AI.AIDebugSystem"), AIDebugSystem, TEXT("Enable/Disable NopeAISystem."));

UNPAISubsystem::UNPAISubsystem()
{
	// set default class 
	static ConstructorHelpers::FClassFinder<UUserWidget> userWidgetClass(TEXT("/NopeAISystem/WBP_DebugAI"));
	if (userWidgetClass.Class != NULL)
	{
		UserWidgetClass = userWidgetClass.Class;
	}
}

void UNPAISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UWorld* World = GetWorld();
	check(World != nullptr);
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.ObjectFlags = RF_Transient;

		const UNPAISystemSettings* MTAISystemSettings = GetDefault<UNPAISystemSettings>();
		TSubclassOf<class ANPAIManager> AIManagerClass = MTAISystemSettings->AIManagerClass;
		// Store the buoyancy manager we create for future use.
		NPAIManager = World->SpawnActor<ANPAIManager>(AIManagerClass, SpawnInfo);
	}
}

void UNPAISubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UNPAISubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
#if WITH_EDITOR
	// In editor, don't let preview worlds instantiate a water subsystem (except if explicitly allowed by a tool that requested it by setting bAllowWaterSubsystemOnPreviewWorld)
	if (WorldType == EWorldType::EditorPreview)
	{
		return bAllowAISubsystemOnPreviewWorld;
	}
#endif // WITH_EDITOR

	return WorldType == EWorldType::Game || WorldType == EWorldType::Editor || WorldType == EWorldType::PIE;
}

UNPAISubsystem* UNPAISubsystem::GetAISubsystem(const UObject* InWorld)
{
	if (InWorld)
	{
		UWorld* World = InWorld->GetWorld();
		if (World)
		{
			return World->GetSubsystem<UNPAISubsystem>();
		}
	}
	return nullptr;
}

void UNPAISubsystem::SetGameLogicStarted(bool bInValue)
{
	bGameLogicStarted = bInValue;
}

void UNPAISubsystem::SetCurrentGameTime(float NewGameTime)
{
	CurrentGameTime = NewGameTime;
}

float UNPAISubsystem::GetCurrentGameTime()
{
	return CurrentGameTime;
}

void UNPAISubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetWorld())
	{
		return;
	}
	if (!GetWorld()->HasBegunPlay())
	{
		return;
	}

	if (AIDebugSystem != CurrentAIDebugSystem)
	{
		CurrentAIDebugSystem = AIDebugSystem;
		OnDebugAIEnabled.Broadcast((bool)CurrentAIDebugSystem);
		if (CurrentAIDebugSystem)
		{
			if (!UserWidget)
			{
				UserWidget = CreateWidget<UUserWidget>(GetWorld(), UserWidgetClass);
				UserWidget->AddToViewport();
			}
		}
		else if (UserWidget)
		{
			UserWidget->RemoveFromParent();
		}
	}

	if (bOwnTimerEnabled)
	{
		if (CurTickInterval <= 0)
		{
			CurTickInterval = TickInterval;
			FNPTimeStruct NPTimeStruct;
			OnGameTimeUpdated.Broadcast(NPTimeStruct, FNPTimeStruct(0,0,1));
		}
		else
		{
			CurTickInterval -= DeltaTime;
		}
	}
}

TStatId UNPAISubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UNPAISubsystem, STATGROUP_Tickables);
}

void UNPAISubsystem::UseOwnTimer(bool OwnTimerEnabled)
{
	bOwnTimerEnabled = OwnTimerEnabled;
}

void UNPAISubsystem::AddAICharacterBase(class ANPAICharacterBase* InNPAICharacterBase)
{
	AICharacters.AddUnique(InNPAICharacterBase);
	OnAICharacterSpawned.Broadcast(InNPAICharacterBase, true, AICharacters.Num());
}

void UNPAISubsystem::RemoveAICharacterBase(class ANPAICharacterBase* InNPAICharacterBase)
{
	AICharacters.Remove(InNPAICharacterBase);
	OnAICharacterSpawned.Broadcast(InNPAICharacterBase, false, AICharacters.Num());
}

TArray<class ANPAICharacterBase*> UNPAISubsystem::GetAICharacters()
{
	return AICharacters;
}

void UNPAISubsystem::GetAICharacters(TArray<class ANPAICharacterBase*>& OutAICharacters, TSubclassOf<class ANPAICharacterBase> FindClass)
{
	if (!FindClass)
	{
		return;
	}
	for (ANPAICharacterBase* AICharacter : AICharacters)
	{
		if (AICharacter->GetClass()->IsChildOf(FindClass))
		{
			OutAICharacters.Add(AICharacter);
		}
	}
}

TArray<class UNPSmartObjectComponent*> UNPAISubsystem::GetSmartObjectComponents()
{
	return SmartObjectComponents;
}

TArray<class UNPAISpawnerComponent*> UNPAISubsystem::GetAISpawnerComponents(TSubclassOf<class AActor> ClassFilter /*= nullptr*/)
{
	if (!ClassFilter)
		return AISpawnerComponents;

	TArray<UNPAISpawnerComponent*> OutNPSmartObjectComponents;
	if (ClassFilter != NULL)
	{
		for (int i = 0; i < AISpawnerComponents.Num(); i++)
		{
			if (AISpawnerComponents[i])
			{
				if (AISpawnerComponents[i]->GetOwner()->GetClass()->IsChildOf(ClassFilter))
				{
					OutNPSmartObjectComponents.Add(AISpawnerComponents[i]);
				}
			}
		}
	}
	return OutNPSmartObjectComponents;
}

void UNPAISubsystem::AddSmartObject(class UNPSmartObjectComponent* InSmartObjectComponent)
{
	SmartObjectComponents.Add(InSmartObjectComponent);
	OnSmartObjectComponentSpawned.Broadcast(InSmartObjectComponent, true, AISpawnerComponents.Num());
}

void UNPAISubsystem::RemoveSmartObject(class UNPSmartObjectComponent* InSmartObjectComponent)
{
	SmartObjectComponents.Remove(InSmartObjectComponent);
	OnSmartObjectComponentSpawned.Broadcast(InSmartObjectComponent, false, AISpawnerComponents.Num());
}

void UNPAISubsystem::AddAISpawnerComponent(class UNPAISpawnerComponent* AISpawnerComponent)
{
	AISpawnerComponents.Add(AISpawnerComponent);
	OnAISpawnerComponentSpawned.Broadcast(AISpawnerComponent, true, AISpawnerComponents.Num());
}

void UNPAISubsystem::RemoveAISpawnerComponent(class UNPAISpawnerComponent* AISpawnerComponent)
{
	AISpawnerComponents.Remove(AISpawnerComponent);
	OnAISpawnerComponentSpawned.Broadcast(AISpawnerComponent, false, AISpawnerComponents.Num());

}

void UNPAISubsystem::OnAssetsLoaded()
{

}

bool UNPAISubsystem::AIDebugSystemEnabled(UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
		if (World)
		{
			if (AIDebugSystem == 0)
			{
				return false;
			}
			else
			{
				return true;

			}
		}
	}
	return false;
}

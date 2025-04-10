// Szymon XIII Wielki


#include "NPAIControllerBase.h"
#include "Components/NPPathFollowingComponent.h"
#include "../NPAISubsystem.h"
#include "NPAILibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Characters/NPAICharacterBase.h"
#include "../NPAIManager.h"
#include "BehaviorTree/BehaviorTree.h"
#include "NopeAISystem/NPAISystemSettings.h"
#include "GameFramework/Pawn.h"
#include "AITargetPoint.h"
#include <Perception/AIPerceptionComponent.h>


ANPAIControllerBase::ANPAIControllerBase(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UNPPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
	SetGenericTeamId(FGenericTeamId(1));
}

FGenericTeamId ANPAIControllerBase::GetGenericTeamId() const
{
	if (NPAICharacterBase)
	{
		return FGenericTeamId(int(NPAICharacterBase->GetTeam()));
	}
	return FGenericTeamId();
}

void ANPAIControllerBase::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (NPAICharacterBase)
	{
		NPAICharacterBase->SetTeam(ETeamType((int)NewTeamID));
	}
}

void ANPAIControllerBase::BeginPlay()
{
	Super::BeginPlay();
}

void ANPAIControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANPAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	NPAICharacterBase = Cast<ANPAICharacterBase>(this->GetPawn());
	if (NPAICharacterBase)
	{
		UNPAISubsystem* AISub = UNPAISubsystem::GetAISubsystem(GetWorld());
		if (AISub && !UNPAISubsystem::GetGameLogicStarted())
		{
			AISub->OnGameLogicStarted.AddDynamic(this, &ANPAIControllerBase::StartAILogic);
		}
		else
		{
			StartAILogic();
		}
	}
}

void ANPAIControllerBase::OnUnPossess()
{
	Super::OnUnPossess();
}

void ANPAIControllerBase::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
}

FAIRequestID ANPAIControllerBase::RequestMove(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr Path)
{
	return Super::RequestMove(MoveRequest, Path);
}

ETeamAttitude::Type ANPAIControllerBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	Super::GetTeamAttitudeTowards(Other);

	const APawn* EnemyPawn = Cast<APawn>(&Other);
	const AController* Controller = Cast<AController>(EnemyPawn->GetController());

	if (EnemyPawn && NPAICharacterBase)
	{
		if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Controller))
		{
			//Create an alliance with Team with ID 10 and set all the other teams as Hostiles:
			FGenericTeamId OtherTeamID = TeamAgent->GetGenericTeamId();
			FGenericTeamId OwnerTeamID = GetGenericTeamId();
			for (const ETeamType& Team : NPAICharacterBase->HostileTeams)
			{
				if ((int)Team == OtherTeamID.GetId())
				{
					return ETeamAttitude::Hostile;
				}
			}
		}
	}
	return ETeamAttitude::Neutral;
}

void ANPAIControllerBase::InitializeAILogic_Implementation()
{
	bIsLogicStarted = true;
	OnControllerLogicStarted.Broadcast(this);
	if (NPAICharacterBase)
	{
		UBlackboardComponent* BC = GetBlackboardComponent();
		if (BC)
		{
			BC->SetValueAsObject("AITargetPoint", NPAICharacterBase->GetTargetPoint());
		}
		UNPAILibrary::SpawnAITask(NPAICharacterBase);
	}
}

void ANPAIControllerBase::SetNPAIActionState(ENPAIActionState NewAIActionState)
{
	UBlackboardComponent* BC = GetBlackboardComponent();
	return BC->SetValueAsEnum("NPAIActionState", (uint8)NewAIActionState);
}

ENPAIActionState ANPAIControllerBase::GetNPAIActionState()
{
	UBlackboardComponent* BC = GetBlackboardComponent();
	return (ENPAIActionState)BC->GetValueAsEnum("NPAIActionState");
}

void ANPAIControllerBase::SetCombatState(ECharacterCombatState NewAnimalCombatState, float DeltaSeconds)
{
	ECharacterCombatState PrviousAnimalCombatState = GetCombatState();
	uint8 Value = (uint8)NewAnimalCombatState;
	const float CurrentCombatTime = GetCurrentCombatStateTime() + DeltaSeconds;
	SetCurrentCombatStateTime(CurrentCombatTime);
	if (PrviousAnimalCombatState != NewAnimalCombatState) {
		if (PrviousAnimalCombatState != ECharacterCombatState::Idle)
		{
			SetNPAIActionState(ENPAIActionState::Wait);
		}
		GetBlackboardComponent()->SetValueAsEnum("CombatState", Value);
		SetCurrentCombatStateTime(0);
		SetCombatStateCount(0);
		OnCharacterCombatChanged.Broadcast(NewAnimalCombatState, PrviousAnimalCombatState, NPAICharacterBase);
	}
}

void ANPAIControllerBase::AddTemporaryHostileTeam(const ETeamType& NewTeamID)
{
	if (NewTeamID == NPAICharacterBase->GetTeam())
	{
		return;
	}
	// Forget only when you adding new Enemies Type
	if (!NPAICharacterBase->HostileTeams.Contains(NewTeamID))
	{
		NPAICharacterBase->HostileTeams.AddUnique(NewTeamID);
		NPAICharacterBase->TemporaryHostileTeams.AddUnique(NewTeamID);
		PerceptionComponent->ForgetAll();
	}
}

void ANPAIControllerBase::ForgotTemporaryHostileTeam()
{
	if (NPAICharacterBase->TemporaryHostileTeams.Num() != 0)
	{
		for (ETeamType& TeamType : NPAICharacterBase->TemporaryHostileTeams)
		{
			NPAICharacterBase->HostileTeams.Remove(TeamType);
		}
		NPAICharacterBase->TemporaryHostileTeams.Empty();
		PerceptionComponent->ForgetAll();
	}
}

ETeamAttitude::Type ANPAIControllerBase::IsHostile(const ETeamType& InTeamType)
{
	if (NPAICharacterBase->HostileTeams.Contains(InTeamType))
	{
		return ETeamAttitude::Hostile;
	}
	if (InTeamType == NPAICharacterBase->GetTeam())
	{
		return ETeamAttitude::Friendly;
	}
	return ETeamAttitude::Neutral;
}

void ANPAIControllerBase::SetCombatStateCount(int Amount)
{
	UBlackboardComponent* BC = GetBlackboardComponent();
	if (BC)
		BC->SetValueAsInt("CombatStateCount", Amount);
}

int ANPAIControllerBase::GetCombatStateCount()
{
	if (UBlackboardComponent* BC = GetBlackboardComponent())
		return BC->GetValueAsInt("CombatStateCount");
	return 0;
}

void ANPAIControllerBase::SetCurrentCombatStateTime(float StateTime)
{
	UBlackboardComponent* BC = GetBlackboardComponent();
	if (BC)
		BC->SetValueAsFloat("CurrentCombatStateTime", StateTime);
}

float ANPAIControllerBase::GetCurrentCombatStateTime()
{
	UBlackboardComponent* BC = GetBlackboardComponent();
	if (BC)
		return BC->GetValueAsFloat("CurrentCombatStateTime");
	return 0.0f;
}

ECharacterCombatState ANPAIControllerBase::GetCombatState()
{
	UBlackboardComponent* BC = GetBlackboardComponent();
	if (BC)
	{
		uint8 Value = BC->GetValueAsEnum("CombatState");
		return ECharacterCombatState(Value);
	}
	return ECharacterCombatState::Max;
}

void ANPAIControllerBase::StartAILogic()
{
	if (const UNPAISystemSettings* NPAISystemSettings = GetDefault<UNPAISystemSettings>())
	{
		if (NPAISystemSettings->bEnableAILogic)
		{
			if (ANPAIManager* AIManager = ANPAIManager::GetAIManager(this->GetWorld()))
			{
				if (BehaviorTree)
				{
					UBlackboardData* BlackboardData = BehaviorTree->GetBlackboardAsset();
					UBlackboardComponent* BlackboardComp = nullptr;
					if (BlackboardData && (Blackboard == nullptr || Blackboard->IsCompatibleWith(BlackboardData) == false))
					{
						UseBlackboard(BlackboardData, BlackboardComp);
					}
				}

				FAIUpdateActorIndex UpdateActorIndex = AIManager->LoadAICharacter(NPAICharacterBase);
				if (UpdateActorIndex.bCanUpdate)
				{
					GetWorld()->GetTimerManager().SetTimer(StartAILogicTimerHandle, this, &ANPAIControllerBase::StartAILogicWithDelay, 1.0f, false);
				}
				else
				{
					GetWorld()->GetTimerManager().SetTimer(StartAILogicTimerHandle, this, &ANPAIControllerBase::StartAILogicWithDelay, 1.0f, false);
				}
			}
		}
	}
}

void ANPAIControllerBase::StartAILogicWithDelay()
{
	GetWorld()->GetTimerManager().ClearTimer(StartAILogicTimerHandle);
	if (BehaviorTree)
	{
		this->RunBehaviorTree(BehaviorTree);
		InitializeAILogic();
		if (ANPAIManager* AIManager = ANPAIManager::GetAIManager(this->GetWorld()))
		{
			AIManager->SaveAICharacter(NPAICharacterBase, true, true);
		}
		//NPAICharacterBase->AIPerceptionStimuliSourceComponent RegisterWithPerceptionSystem();
	}
}

void ANPAIControllerBase::OnMontageEnded(class UAnimMontage* Montage, bool bInterrupted)
{
	NPAICharacterBase->GetMesh()->GetAnimInstance()->OnMontageEnded.RemoveDynamic(this, &ANPAIControllerBase::OnMontageEnded);
	StartAILogicWithDelay();
}

void ANPAIControllerBase::SetLastIdleLocation(FVector LastIdleLocation)
{
	UBlackboardComponent* BC = GetBlackboardComponent();
	if (LastIdleLocation.Z == 0.0f)
	{
		LastIdleLocation = GetPawn()->GetActorLocation();
	}
	if (BC)
		BC->SetValueAsVector("LastIdleLocation", LastIdleLocation);
}

FVector ANPAIControllerBase::GetLastIdleLocation()
{
	UBlackboardComponent* BC = GetBlackboardComponent();
	if (BC)
		return BC->GetValueAsVector("LastIdleLocation");

	return FVector();
}

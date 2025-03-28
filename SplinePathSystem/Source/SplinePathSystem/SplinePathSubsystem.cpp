// Fill out your copyright notice in the Description page of Project Settings.

#include "SplinePathSubsystem.h"
#include "SPManager.h"
#include "SPSettings.h"
#include "SplinePathActor.h"
#include "Components/PathSplineComponent.h"
#include "Components/PathSplineMetadata.h"
#include "Defines/SPDefines.h"

//bool UNPAISubsystem::bGameLogicStarted = true;
//
//int32 AIDebugSystem = 0;
//FAutoConsoleVariableRef CVarNopeAISystemEnableAI(TEXT("AI.AIDebugSystem"), AIDebugSystem, TEXT("Enable/Disable NopeAISystem."));

USplinePathSubsystem::USplinePathSubsystem()
{

}

void USplinePathSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UWorld* World = GetWorld();
	check(World != nullptr);
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.ObjectFlags = RF_Transient;

		const USPSettings* SPSettings = GetDefault<USPSettings>();
		TSubclassOf<class ASPManager> SPManagerClass = SPSettings->SPManagerClass;
		// Store the buoyancy manager we create for future use.
		SPManager = World->SpawnActor<ASPManager>(SPManagerClass, SpawnInfo);
	}
}

void USplinePathSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool USplinePathSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
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

USplinePathSubsystem* USplinePathSubsystem::GetSplinePathSubsystem(const UObject* InWorld)
{
	if (InWorld)
	{
		UWorld* World = InWorld->GetWorld();
		if (World)
		{
			return World->GetSubsystem<USplinePathSubsystem>();
		}
	}
	return nullptr;
}

void USplinePathSubsystem::GetSplinePathPoints(TArray<class ASplinePathActor*>& OutPathPoints)
{
	OutPathPoints = SplinePathActors;
}

void USplinePathSubsystem::AddSplinePoint(class ASplinePathActor* InSplinePathActor)
{
	SplinePathActors.AddUnique(InSplinePathActor);
}

void USplinePathSubsystem::RemoveSplinePoint(class ASplinePathActor* InSplinePathActor)
{
	SplinePathActors.Remove(InSplinePathActor);
}

void USplinePathSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TStatId USplinePathSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USplinePathSubsystem, STATGROUP_Tickables);
}

TArray<FSplinePathConnection> USplinePathSubsystem::FindShortestPathAStar(const FVector StartLocation, const FVector EndLocation)
{

	TArray<FSplinePathConnection> OutSplineActorConnections;

	// 1. ZnajdŸ najbli¿sze punkty splajnu dla Start i End
	FSplineActorConnection StartConnection = FindClosestSplineActorConnection(StartLocation);
	FSplineActorConnection EndConnection = FindClosestSplineActorConnection(EndLocation);

	if (!StartConnection.SplinePathActor || !EndConnection.SplinePathActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nie znaleziono najbli¿szego punktu splajnu!"));
		return OutSplineActorConnections;
	}

	// 2. Struktury A*
	TMap<FSplineActorConnection, float> GScore;
	TMap<FSplineActorConnection, float> FScore;
	TMap<FSplineActorConnection, FSplineActorConnection> CameFrom;
	TSet<FSplineActorConnection> OpenSet;

	GScore.Add(StartConnection, 0.0f);
	FScore.Add(StartConnection, GetHeuristic(StartConnection, EndConnection));
	OpenSet.Add(StartConnection);

	while (OpenSet.Num() > 0)
	{
		// 3. Znalezienie punktu z najni¿szym FScore - optymalizacja z min-heap
		FSplineActorConnection CurrentConnection;
		TArray<FSplineActorConnection> OpenList = OpenSet.Array();
		OpenList.Sort([&](const FSplineActorConnection& A, const FSplineActorConnection& B)
			{
				return FScore[A] < FScore[B];
			});
		CurrentConnection = OpenList[0];
		OpenSet.Remove(CurrentConnection);

		// 4. Sprawdzenie, czy osi¹gnêliœmy cel
		if (CurrentConnection == EndConnection)
		{
			FSplineActorConnection Node = EndConnection;
			while (CameFrom.Contains(Node))
			{
				FSplineActorConnection PreviousNode = CameFrom[Node];
				OutSplineActorConnections.Insert(FSplinePathConnection(Node.SplinePathActor, Node.IndexPoint), 0);
				Node = PreviousNode;
			}
			OutSplineActorConnections.Insert(FSplinePathConnection(StartConnection.SplinePathActor, StartConnection.IndexPoint), 0);
			return OutSplineActorConnections;
		}

		// 5. Przejrzyj s¹siadów
		for (const FSplineActorConnection& Neighbor : GetNeighbors(CurrentConnection))
		{
			float TentativeGScore = GScore[CurrentConnection] + GetCost(CurrentConnection, Neighbor);

			if (!GScore.Contains(Neighbor) || TentativeGScore < GScore[Neighbor])
			{
				CameFrom.Add(Neighbor, CurrentConnection);

				if (!GScore.Contains(Neighbor))
				{
					GScore.Add(Neighbor, TentativeGScore);
				}
				else
				{
					GScore[Neighbor] = TentativeGScore;
				}

				if (!FScore.Contains(Neighbor))
				{
					FScore.Add(Neighbor, TentativeGScore + GetHeuristic(Neighbor, EndConnection));
				}
				else
				{
					FScore[Neighbor] = TentativeGScore + GetHeuristic(Neighbor, EndConnection);
				}

				OpenSet.Add(Neighbor);
			}
		}
	}

	return OutSplineActorConnections;
}

// Heurystyka A* (odleg³oœæ do celu)
float USplinePathSubsystem::GetHeuristic(FSplineActorConnection A, FSplineActorConnection B)
{
	return FVector::Dist(A.SplinePathActor->SplineComponent->GetLocationAtSplinePoint(A.IndexPoint, ESplineCoordinateSpace::World),
		B.SplinePathActor->SplineComponent->GetLocationAtSplinePoint(B.IndexPoint, ESplineCoordinateSpace::World));
}

FSplineActorConnection USplinePathSubsystem::FindClosestSplineActorConnection(const FVector StartLocation)
{
	FSplineActorConnection ClosestConnection;

	float ClosestDistanceSquared = FLT_MAX;

	for (ASplinePathActor* SplinePathActor : SplinePathActors)
	{
		// ZnajdŸ komponent spline w aktorze
		UPathSplineComponent* SplineComp = SplinePathActor->SplineComponent;
		if (!SplineComp)
		{
			continue;
		}

		// Iteruj przez punkty w spline
		const int32 NumPoints = SplineComp->GetNumberOfSplinePoints();
		for (int32 i = 0; i < NumPoints; i++)
		{
			// Pobierz pozycjê punktu spline
			FTransform PointTransform = SplineComp->GetTransformAtSplinePoint(i, ESplineCoordinateSpace::World);

			// Oblicz odleg³oœæ kwadratow¹
			float DistanceSquared = FVector::DistSquared(PointTransform.GetLocation(), StartLocation);

			// Jeœli jest bli¿szy ni¿ obecny najbli¿szy, zapisz go
			if (DistanceSquared < ClosestDistanceSquared)
			{
				ClosestDistanceSquared = DistanceSquared;
				ClosestConnection.IndexPoint = i;
				ClosestConnection.SplinePathActor = SplinePathActor;
			}
		}
	}
	return ClosestConnection;
}

TArray<FSplineActorConnection> USplinePathSubsystem::GetNeighbors(const FSplineActorConnection& Connection)
{
	TArray<FSplineActorConnection> Connections;

	if (!Connection.SplinePathActor || !Connection.SplinePathActor->SplineComponent)
	{
		return Connections;
	}

	UPathSplineMetadata* PathSplineMetadata = Cast<UPathSplineMetadata>(
		Connection.SplinePathActor->SplineComponent->GetSplinePointsMetadata());

	if (!PathSplineMetadata || !PathSplineMetadata->PointData.IsValidIndex(Connection.IndexPoint))
	{
		return Connections;
	}

	// Iterujemy po po³¹czeniach splajnu
	for (const FSplinePathConnection& SplinePathConnection : PathSplineMetadata->PointData[Connection.IndexPoint].SplinePathConnections)
	{
		if (SplinePathConnection.ConnectedSplinePathActor)
		{
			FSplineActorConnection SplineActorConnection;
			SplineActorConnection.SplinePathActor = SplinePathConnection.ConnectedSplinePathActor;
			SplineActorConnection.IndexPoint = SplinePathConnection.SplineIndex;
			Connections.Add(SplineActorConnection);
		}
	}

	return Connections;
}

float USplinePathSubsystem::GetCost(const FSplineActorConnection& A, const FSplineActorConnection& B)
{
	if (!A.SplinePathActor || !B.SplinePathActor)
	{
		return FLT_MAX; // Zabezpieczenie przed nullami
	}

	// Pobranie metadanych spline z A
	UPathSplineMetadata* Metadata = Cast<UPathSplineMetadata>(A.SplinePathActor->SplineComponent->GetSplinePointsMetadata());
	if (!Metadata || !Metadata->PointData.IsValidIndex(A.IndexPoint))
	{
		return FLT_MAX; // Jeœli brak metadanych lub indeks niepoprawny, zwracamy du¿¹ wartoœæ
	}

	// Szukamy po³¹czenia prowadz¹cego do B
	for (const FSplinePathConnection& Connection : Metadata->PointData[A.IndexPoint].SplinePathConnections)
	{
		if (Connection.ConnectedSplinePathActor == B.SplinePathActor && Connection.SplineIndex == B.IndexPoint)
		{
			return Connection.TotalCost; // U¿ywamy wartoœci TotalCost zamiast liczyæ dystans
		}
	}

	return FLT_MAX; // Jeœli nie znaleziono po³¹czenia, zwracamy du¿¹ wartoœæ
}

UPathSplineComponent* USplinePathSubsystem::StartMovingAlongPath(AActor* Actor, const FVector StartLocation, const FVector EndLocation)
{
	UPathSplineComponent* NewSplineComponent = nullptr;
	USplinePathSubsystem* SplinePathSubsystem = USplinePathSubsystem::GetSplinePathSubsystem(this);
	if (SplinePathSubsystem)
	{
		// ZnajdŸ najkrótsz¹ œcie¿kê
		TArray<FSplinePathConnection> PathConnections = SplinePathSubsystem->FindShortestPathAStar(StartLocation, EndLocation);
		if (PathConnections.Num() > 0)
		{
			// Tworzymy i przypisujemy nowy SplineComponent do SPManager
			NewSplineComponent = SPManager->CreateSplineToManager(Actor, PathConnections);
			// Dodajemy nowy SplineComponent do SPManager, z aktorem jako kluczem
			SPManager->AddSplineComponent(Actor, NewSplineComponent);
		}
	}
	return NewSplineComponent;
}
// Szymon XIII Wielki
#include "SPManager.h"
#include "Defines/SPDefines.h"
#include "SplinePathSubsystem.h"
#include "Components/PathSplineComponent.h"
#include "SplinePathActor.h"

ASPManager::ASPManager(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{

}

void ASPManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASPManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

}

void ASPManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}

void ASPManager::AddSplineComponent(AActor* Actor, UPathSplineComponent* NewSplineComponent)
{
    if (!Actor || !NewSplineComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Actor or SplineComponent passed to AddSplineComponent!"));
        return;
    }

    // Sprawdź, czy aktor ma już spline w TMap
    if (UPathSplineComponent** ExistingSpline = PathSplineComponents.Find(Actor))
    {
        // Jeśli istnieje, niszczymy stary spline przed dodaniem nowego
        if (*ExistingSpline)
        {
            (*ExistingSpline)->DestroyComponent();
        }

        // Usuwamy wpis z mapy
        PathSplineComponents.Remove(Actor);
    }

    // Dodajemy nowy spline do TMap
    PathSplineComponents.Add(Actor, NewSplineComponent);
}

UPathSplineComponent* ASPManager::CreateSplineToManager(AActor* Actor, const TArray<FSplinePathConnection>& PathConnections)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Actor passed to CreateAndAssignSplineToManager!"));
        return nullptr;
    }

    // Tworzymy nowy SplineComponent
    UPathSplineComponent* NewSplineComponent = NewObject<UPathSplineComponent>(Actor, UPathSplineComponent::StaticClass());
    NewSplineComponent->SetupAttachment(Actor->GetRootComponent());
    NewSplineComponent->RegisterComponent();  // Rejestrujemy komponent na aktorze

    // Dodajemy punkty na SplineComponent na podstawie wyników A*
    for (const FSplinePathConnection& Connection : PathConnections)
    {
        // Pobieramy lokalizację punktu na spline w przestrzeni świata
        FVector SplinePointLocation = Connection.ConnectedSplinePathActor->SplineComponent->GetLocationAtSplinePoint(Connection.SplineIndex, ESplineCoordinateSpace::World);
        NewSplineComponent->AddSplinePoint(SplinePointLocation, ESplineCoordinateSpace::World);
    }

    // Uaktualniamy SplineComponent, aby był gotowy do użycia
    NewSplineComponent->UpdateSpline();
    return NewSplineComponent;
}

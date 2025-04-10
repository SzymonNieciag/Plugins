// Szymon XIII Wielki
#include "SplinePathActor.h"
#include "SplinePathSubsystem.h"
#include "Components/PathSplineComponent.h"
#include "Components/PathSplineMetadata.h"

ASplinePathActor::ASplinePathActor(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	USplinePathSubsystem* SplinePathSubsystem = USplinePathSubsystem::GetSplinePathSubsystem(this);
	if (SplinePathSubsystem)
	{
		SplinePathSubsystem->AddSplinePoint(this);
	}

	PrimaryActorTick.bCanEverTick = true;

	// Tworzenie i dodawanie spline component
	SplineComponent = CreateDefaultSubobject<UPathSplineComponent>(TEXT("SplineComponent"));
	SetRootComponent(SplineComponent);
}

void ASplinePathActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASplinePathActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ASplinePathActor::Destroyed()
{
	Super::Destroyed();
	USplinePathSubsystem* SplinePathSubsystem = USplinePathSubsystem::GetSplinePathSubsystem(this);
	if (SplinePathSubsystem)
	{
		SplinePathSubsystem->RemoveSplinePoint(this);
	}
}

void ASplinePathActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SplineComponent->FixupPoints();
}

void ASplinePathActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}

#if WITH_EDITOR
void ASplinePathActor::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Sprawdź, czy zmiana dotyczy SplineComponent
	if (PropertyChangedEvent.Property)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();

		// Sprawdź, czy zmiana dotyczy punktów spline'a
		if (PropertyName == GET_MEMBER_NAME_CHECKED(USplineComponent, SplineCurves))
		{
			// Wykonaj logikę w momencie zmiany punktów
			UE_LOG(LogTemp, Warning, TEXT("Spline points changed! Number of points: %d"), SplineComponent->GetNumberOfSplinePoints());

			// Tutaj możesz dodać własną funkcję lub event
		}
	}
}
#endif


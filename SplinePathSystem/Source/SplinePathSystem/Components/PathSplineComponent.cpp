// UPathSplineComponent.cpp
#include "PathSplineComponent.h"
#include "PathSplineMetadata.h"
#include "SplinePathSystem/SplinePathActor.h"
#include "../Defines/SPDefines.h"

// Konstruktor
UPathSplineComponent::UPathSplineComponent()
{
	PrimaryComponentTick.bCanEverTick = false;  // Jeśli nie chcesz, aby komponent tickował
	MySplineMetadata = CreateDefaultSubobject<UPathSplineMetadata>(TEXT("MySplineMetadata"));
	MySplineMetadata->SetSplineComponent(this);
}

// Funkcja do pobierania metadanych spline
USplineMetadata* UPathSplineComponent::GetSplinePointsMetadata()
{
	if (ASplinePathActor* SplinePathActor = Cast<ASplinePathActor>(GetOwner()))
	{
		return Cast<UPathSplineMetadata>(MySplineMetadata);
	}
	return nullptr;
}

// Funkcja do pobierania metadanych spline (wersja const)
const USplineMetadata* UPathSplineComponent::GetSplinePointsMetadata() const
{
	if (ASplinePathActor* SplinePathActor = Cast<ASplinePathActor>(GetOwner()))
	{
		return Cast<UPathSplineMetadata>(MySplineMetadata);
	}
	return nullptr;
}

// Funkcja do post-load (po załadowaniu komponentu)
void UPathSplineComponent::PostLoad()
{
	Super::PostLoad();
}

// Funkcja duplikująca (w przypadku duplikacji obiektu)
void UPathSplineComponent::PostDuplicate(bool bDuplicateForPie)
{
	Super::PostDuplicate(bDuplicateForPie);
}

// Funkcja do naprawiania punktów spline
void UPathSplineComponent::FixupPoints()
{
	if (UPathSplineMetadata* Metadata = Cast<UPathSplineMetadata>(GetSplinePointsMetadata()))
	{
		Metadata->Fixup(GetNumberOfSplinePoints(), this);
	}
}

#if WITH_EDITOR

// Funkcja do obsługi zmiany właściwości w edytorze
void UPathSplineComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FixupPoints();
}

// Funkcja do obsługi importu w edytorze
void UPathSplineComponent::PostEditImport()
{
	Super::PostEditImport();
}

#endif

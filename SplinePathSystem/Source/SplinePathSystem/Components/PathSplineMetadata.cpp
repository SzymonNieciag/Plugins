// UPathSplineMetadata.cpp
#include "PathSplineMetadata.h"
#include "Components/SplineComponent.h"
#include "../SplinePathActor.h"
#include "PathSplineComponent.h"

UPathSplineMetadata::UPathSplineMetadata()
{
	// Inicjalizacja tablicy Connections z jednym elementem
}

// Wstawienie punktu w danym miejscu (indeks)
void UPathSplineMetadata::InsertPoint(int32 Index, float t, bool bClosedLoop)
{
	// Dodaj nowy punkt do PointData
	if (Index >= 0 && Index <= PointData.Num())
	{

	}
}

// Aktualizacja punktu w danym miejscu (indeks)
void UPathSplineMetadata::UpdatePoint(int32 Index, float t, bool bClosedLoop)
{
	// Zaktualizuj dane punktu w PointData
	if (PointData.IsValidIndex(Index))
	{

	}
}

// Dodanie punktu na końcu
void UPathSplineMetadata::AddPoint(float InputKey)
{

}

// Usunięcie punktu na danym indeksie
void UPathSplineMetadata::RemovePoint(int32 Index)
{

}

// Duplikowanie punktu z jednego miejsca w inne
void UPathSplineMetadata::DuplicatePoint(int32 Index)
{

}

// Kopiowanie punktu z innego zestawu metadanych
void UPathSplineMetadata::CopyPoint(const USplineMetadata* FromSplineMetadata, int32 FromIndex, int32 ToIndex)
{
	if (FromSplineMetadata && FromSplineMetadata->IsA<UPathSplineMetadata>())
	{
		const UPathSplineMetadata* FromPathMetadata = Cast<UPathSplineMetadata>(FromSplineMetadata);
		if (FromPathMetadata && FromPathMetadata->PointData.IsValidIndex(FromIndex))
		{
			const FSplinePathPointData& FromPoint = FromPathMetadata->PointData[FromIndex];

			if (PointData.IsValidIndex(ToIndex))
			{
				FSplinePathPointData& ToPoint = PointData[ToIndex];
				// Skopiuj dane z FromPoint do ToPoint
				ToPoint = FromPoint; // Możesz dodać dodatkową logikę, jeśli to potrzebne
			}
		}
	}
}

// Resetowanie metadanych spline do nowej liczby punktów
void UPathSplineMetadata::Reset(int32 NumPoints)
{
	// Wyczyść istniejące dane w PointData
	PointData.Empty();

	// Inicjalizuj nowe punkty
	for (int32 i = 0; i < NumPoints; ++i)
	{
		FSplinePathPointData NewPointData;
		PointData.Add(NewPointData);
	}
}

// Naprawienie metadanych w zależności od liczby punktów
void UPathSplineMetadata::Fixup(int32 NumPoints, USplineComponent* SplineComp)
{
	if (!SplineComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineComponent jest nullptr!"));
		return;
	}

	AActor* Owner = SplineComp->GetOwner();
	if (!Owner || Owner->HasAnyFlags(RF_Transient)) // Sprawdza, czy aktor jest transient (np. prefab, nie instancja w świecie)
	{
		PointData.Empty();
		return;
	}

	int32 TotalSplinePoints = SplineComp->GetNumberOfSplinePoints();
	if (NumPoints != TotalSplinePoints)
	{
		UE_LOG(LogTemp, Warning, TEXT("Liczba punktów w SplineComp nie zgadza się z NumPoints!"));
		return;
	}

	PointData.SetNum(TotalSplinePoints); // Zaktualizuj rozmiar PointData

	for (int32 i = 0; i < TotalSplinePoints; i++)
	{
		FSplinePathPointData& CurrentPointData = PointData[i];

		// Oblicz odległość i stwórz powiązania dla tego punktu z poprzednim i następnym
		FVector CurrentPointLocation = SplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

		// Tworzymy połączenie z poprzednim punktem, jeśli to nie pierwszy punkt
		if (i > 0)
		{
			GenerateDefaultPoints(SplineComp, i - 1, CurrentPointData);
		}
		// Tworzymy połączenie z następnym punktem, jeśli to nie ostatni punkt
		if (i < TotalSplinePoints - 1)
		{
			GenerateDefaultPoints(SplineComp, i + 1, CurrentPointData);
		}
		UpdatePointsDistance(CurrentPointLocation, CurrentPointData);

	}

	// Jeśli spline jest zamknięty, połącz ostatni punkt z pierwszym
	if (SplineComp->IsClosedLoop())
	{
		FSplinePathPointData& FirstPointData = PointData[0];
		FSplinePathPointData& LastPointData = PointData[TotalSplinePoints - 1];
		GenerateDefaultPoints(SplineComp, TotalSplinePoints - 1, FirstPointData);
		GenerateDefaultPoints(SplineComp, 0, LastPointData);
	}
}

void UPathSplineMetadata::GenerateDefaultPoints(USplineComponent* SplineComp, int32 Index, FSplinePathPointData& InCurrentPointData)
{
	FSplinePathConnection NewConnection;

	NewConnection.ConnectedSplinePathActor = Cast<ASplinePathActor>(SplineComp->GetOwner());
	if (!NewConnection.ConnectedSplinePathActor)
	{
		return;
	}
	NewConnection.SplineIndex = Index;

	FSplinePathConnection* SplinePathConnection = InCurrentPointData.SplinePathConnections.FindByPredicate([NewConnection](const FSplinePathConnection& Element)
		{
			return Element.SplineIndex == NewConnection.SplineIndex && NewConnection.ConnectedSplinePathActor == Element.ConnectedSplinePathActor;
		});
	if (!SplinePathConnection)
	{
		InCurrentPointData.SplinePathConnections.Add(NewConnection);  // Dodajemy do poprzedniego punktu
	}
}

void UPathSplineMetadata::UpdatePointsDistance(const FVector& CurrentPointLocation, FSplinePathPointData& InCurrentPointData)
{
	for (FSplinePathConnection& SplinePathConnection : InCurrentPointData.SplinePathConnections)
	{
		if (SplinePathConnection.ConnectedSplinePathActor)
		{
			UPathSplineComponent* NextPathSplineComponent = SplinePathConnection.ConnectedSplinePathActor->SplineComponent;

			FVector NextPointLocation = NextPathSplineComponent->GetLocationAtSplinePoint(SplinePathConnection.SplineIndex, ESplineCoordinateSpace::World);
			float Distance = FVector::Dist(CurrentPointLocation, NextPointLocation);

			// Obliczanie TotalCost
			if (SplinePathConnection.ConnectedSplinePathActor)
				SplinePathConnection.DefaultCost = SplinePathConnection.ConnectedSplinePathActor->MovementCost;

			SplinePathConnection.TotalCost = Distance * SplinePathConnection.DefaultCost;
			SplinePathConnection.Distance = Distance;
		}
	}
}

void UPathSplineMetadata::SetSplineComponent(USplineComponent* NewSplineComponent)
{
	SplineComponent = NewSplineComponent;
}

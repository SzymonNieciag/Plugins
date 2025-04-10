#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "SplineMetadataDetailsFactory.h"
#include "SplinePathSystem/Defines/SPDefines.h"

/**
 * Klasa szczegółów dla metadanych spline
 */
class SPLINEPATHSYSTEMEDITOR_API FPathSplineMetadataDetails : public ISplineMetadataDetails, public TSharedFromThis<FPathSplineMetadataDetails> 
{
public:
    virtual ~FPathSplineMetadataDetails() {}

    FPathSplineMetadataDetails();

    // Nadpisania metod interfejsu ISplineMetadataDetails
    virtual FName GetName() const override;
    virtual FText GetDisplayName() const override;
    virtual void Update(USplineComponent* InSplineComponent, const TSet<int32>& InSelectedKeys) override;
    virtual void GenerateChildContent(IDetailGroup& DetailGroup) override;

private:
    // Pobiera instancję metadanych spline
    class UPathSplineMetadata* GetPathMetadata() const;

    // Zmienne prywatne
    TArray<FSplinePathConnection> SplinePathConnections;
    TSharedPtr<FStructOnScope> StructOnScope;
    USplineComponent* SplineComp = nullptr;  // Wskaźnik do komponentu spline
	TSet<int32> SelectedKeys;              // Zestaw wybranych kluczy

    // Funkcje pomocnicze do obsługi TArray<FSplinePathConnection>
    UObject* GetConnectedSplinePathActor(int32 Index) const;
    int32 GetSplineIndex(int32 Index) const;

    void OnPropertyValueChanged(const FPropertyChangedEvent& PropertyChangedEvent);
};


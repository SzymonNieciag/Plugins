#include "PathSplineMetadataDetailsFactory.h"
#include "PathSplineMetadataDetails.h"
#include "SplinePathSystem/Components/PathSplineMetadata.h"

TSharedPtr<ISplineMetadataDetails> UPathSplineMetadataDetailsFactory::Create()
{
    // Tworzenie instancji szczegółów i konwertowanie na TSharedPtr<ISplineMetadataDetails>
    return MakeShareable(new FPathSplineMetadataDetails());
}

UClass* UPathSplineMetadataDetailsFactory::GetMetadataClass() const
{
    // Zwracamy klasę UPathSplineMetadata
    return UPathSplineMetadata::StaticClass();
}
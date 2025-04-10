#pragma once

#include "SplineMetadataDetailsFactory.h"
#include "PathSplineMetadataDetailsFactory.generated.h"

/**
 * Fabryka szczegółów dla UPathSplineMetadata
 */
UCLASS()
class SPLINEPATHSYSTEMEDITOR_API UPathSplineMetadataDetailsFactory : public USplineMetadataDetailsFactoryBase
{
    GENERATED_BODY()

public:
    virtual ~UPathSplineMetadataDetailsFactory() {}

    // Nadpisanie metody tworzenia szczegółów
    virtual TSharedPtr<ISplineMetadataDetails> Create() override;

    // Nadpisanie metody pobierania klasy metadanych
    virtual UClass* GetMetadataClass() const override;
};
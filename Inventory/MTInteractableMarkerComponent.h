// Blueprints Studio

#pragma once

#include "CoreMinimal.h"
#include <Components/ActorComponent.h>
#include "InteractableMarkers/Components/InteractableMarkerComponent.h"
#include "MTInteractableMarkerComponent.generated.h"

UCLASS()
class MOUNTAINTRAILS_API UMTInteractableMarkerComponent : public UInteractableMarkerComponent
{
	GENERATED_BODY()

public:
	virtual bool CanInteract();
};


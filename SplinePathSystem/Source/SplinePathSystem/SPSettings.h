
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SPSettings.generated.h"

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "SplinePathSettings"))
class SPLINEPATHSYSTEM_API USPSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USPSettings();

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class ASPManager> SPManagerClass;
};

// Copyright © 2022 DRAGO Entertainment. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NPWidgetComponentFace.generated.h"

/**
 * 
 */

UCLASS(meta = (BlueprintSpawnableComponent))
class NOPEAISYSTEM_API UNPWidgetComponentFace : public UWidgetComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Ensures the user widget is initialized */
	virtual void InitWidget();

private:
	UFUNCTION()
	void OnDebugUIEnabled(const bool DebugAIEnaled);

	virtual void BeginPlay() override;
};
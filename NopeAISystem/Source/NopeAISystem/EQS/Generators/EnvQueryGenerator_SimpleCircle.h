// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_ProjectedPoints.h"
#include "DataProviders/AIDataProvider.h"
#include "EnvQueryGenerator_SimpleCircle.generated.h"

UCLASS(meta = (DisplayName = "Points: Simple Circle"))
class NOPEAISYSTEM_API UEnvQueryGenerator_SimpleCircle : public UEnvQueryGenerator_ProjectedPoints
{
	GENERATED_BODY()

public:
	UEnvQueryGenerator_SimpleCircle(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

protected:
	/** min distance between point and context */
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue InnerRadius;

	/** max distance between point and context */
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue OuterRadius;

	/** number of items to generate for each ring */
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderIntValue DistanceToGeneratePoints;

	/** context */
	UPROPERTY(EditAnywhere, Category = Generator)
	TSubclassOf<class UEnvQueryContext> Center;
};

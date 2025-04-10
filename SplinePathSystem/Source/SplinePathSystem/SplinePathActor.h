// Szymon XIII Wielki

#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Actor.h>
#include "Defines/SPDefines.h"
#include "SplinePathActor.generated.h"


UCLASS()
class SPLINEPATHSYSTEM_API ASplinePathActor : public AActor
{
	GENERATED_BODY()

public:
	ASplinePathActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Komponent spline
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
    class UPathSplineComponent* SplineComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "PathNode")
	float MovementCost = 1;

public:
#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	/** Overridable native event for when play begins for this actor. */
	virtual void BeginPlay();

	/** Overridable function called whenever this actor is being removed from a level */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

	virtual void Destroyed() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void Tick(float DeltaSeconds) override;
};


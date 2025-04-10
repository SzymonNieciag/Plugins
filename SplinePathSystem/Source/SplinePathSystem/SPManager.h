// Szymon XIII Wielki

#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Actor.h>
#include "Defines/SPDefines.h"
#include "SPManager.generated.h"

class UPathSplineComponent;

UCLASS()
class SPLINEPATHSYSTEM_API ASPManager : public AActor
{
	GENERATED_BODY()

public:
	ASPManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // Funkcja do dodawania nowych komponentów Spline do tablicy
    void AddSplineComponent(AActor* Actor, UPathSplineComponent* NewSplineComponent);

    // Zmieniamy na TMap, gdzie kluczem jest aktor, a wartością jest spline
    UPROPERTY(VisibleAnywhere)
    TMap<AActor*, UPathSplineComponent*> PathSplineComponents;


protected:
	/** Overridable native event for when play begins for this actor. */
	virtual void BeginPlay();

	/** Overridable function called whenever this actor is being removed from a level */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
	virtual void Tick(float DeltaSeconds) override;

	// Funkcja tworząca i przypisująca SplineComponent do aktora
	UPathSplineComponent* CreateSplineToManager(AActor* Actor, const TArray<FSplinePathConnection>& PathConnections);
};


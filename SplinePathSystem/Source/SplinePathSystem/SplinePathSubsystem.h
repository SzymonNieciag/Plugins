//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Subsystems/WorldSubsystem.h>
#include "Defines/SPDefines.h"
#include "SplinePathSubsystem.generated.h"

/**
 *
 */

UCLASS(BlueprintType, Transient)
class SPLINEPATHSYSTEM_API USplinePathSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

protected:
	// USubsystem implementation Begin
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// USubsystem implementation End
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

public:

	USplinePathSubsystem();
	// FTickableGameObject implementation Begin
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	// FTickableGameObject implementation End

	UPROPERTY(Transient)
	class ASPManager* SPManager;

	/** Static helper function to get subsystem from a world, returns nullptr if world or subsystem don't exist */
	static USplinePathSubsystem* GetSplinePathSubsystem(const UObject* WorldContextObject);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<class ASplinePathActor*> SplinePathActors;

public:
	UFUNCTION(BlueprintPure)
	void GetSplinePathPoints(TArray<class ASplinePathActor*>& OutPathPoints);

public:
	void AddSplinePoint(class ASplinePathActor* InSplinePathActor);
	void RemoveSplinePoint(class ASplinePathActor* InSplinePathActor);

protected:
#if WITH_EDITOR
	/** By default, there is no water subsystem allowed on preview worlds except when explicitly requested : */
	bool bAllowAISubsystemOnPreviewWorld = false;
#endif // WITH_EDITOR

	int32 DebugPathSubsystem = 0;

#pragma region PathFinding_Star_A

public:
	UFUNCTION(BlueprintCallable)
	TArray<FSplinePathConnection> FindShortestPathAStar(const FVector StartLocation, const FVector EndLocation);

	UFUNCTION(BlueprintCallable)
	class UPathSplineComponent* StartMovingAlongPath(class AActor* Actor, const FVector StartLocation, const FVector EndLocation);

private:
	FSplineActorConnection FindClosestSplineActorConnection(const FVector StartLocation);

	TArray<FSplineActorConnection> GetNeighbors(const FSplineActorConnection& Connection);

	float GetCost(const FSplineActorConnection& A, const FSplineActorConnection& B);

	float GetHeuristic(FSplineActorConnection A, FSplineActorConnection B);

#pragma endregion PathFinding_Star_A

};











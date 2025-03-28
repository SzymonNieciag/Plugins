// Copyright 2018 DRAGO. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NPAIDefines.h"
#include "NPAILibrary.generated.h"

/**
 * 
 */
class ANPAICharacterBase;
class APawn;

UCLASS()
class NOPEAISYSTEM_API UNPAILibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// returns true if the given 'Object' is instantiated and running in Editor Preview (e.g. Persona, AnimBP...)
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "AIManager")
	static bool IsRunningInEditorPreview(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "AI|Task")
	static class UNPAITaskBase* SpawnAITask(ANPAICharacterBase* AICharacter);

	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	static AActor* GetClosestActorInRange(FVector InstigatorLocation, const TArray<class AActor*> &InActors, const float SearchRange = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	static const FHitResult GetClosestHitResultInRange(FVector InstigatorLocation, const TArray<struct FHitResult> &InHitResults, const float SearchRange = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	static void GetObjectsInRange(FVector InstigatorLocation, const TArray<class UObject*>& InObjects, TArray<class UObject*>& OutObjects, const float SearchRange = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	static UNPSmartObjectComponent* FindSmartObjectsByClass(class ACharacter* AICharacterBase, TArray<AActor*>& OutActors, class AActor*& ClosestOutActor, TSoftClassPtr<class AActor> SearchActorClass, const float SearchRange = 0.0f);

	UFUNCTION(BlueprintCallable, meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"), Category = "AIManager")
	static void SpawnAICharacterAsync(UObject* WorldContextObject, TSoftClassPtr<ANPAICharacterBase> InAICharacterBaseClass, FTransform InSpawnTransform, const TArray<FNPAITaskStruct>& InTasksList, bool bEditableModeActive, FLatentActionInfo LatentInfo, ANPAICharacterBase*& OutAICharacterBase);

	UFUNCTION(BlueprintCallable, Category = "AIManager")
	static void ChangePropertyValue(UObject* Object, const FName& PropertyName, float day, float hour, float minute);

	UFUNCTION(BlueprintCallable, Category = "AIManager")
	static void SetTaskTimeToZero(UPARAM(ref) TArray<struct FNPAITaskStruct> &NPAITaskStructs, TSubclassOf<class UNPAITaskBase> TaskFilter);

	//static bool IsPositionReachable(class AAIController* AIController, const FVector PathStart, const FVector Position, struct FPathFindingResult& Result);
};







	



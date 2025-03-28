// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include <Components/ActorComponent.h>
#include <NopeAISystem/AI/NPAIDefines.h>
#include "NPAISpawnerComponent.generated.h"



UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), HideCategories = (Activation, AssetUserData, Collision, Cooking, HLOD, Lighting, LOD, Mobile, Mobility, Navigation, Physics, RayTracing, Rendering, Tags, TextureStreaming))
class NOPEAISYSTEM_API UNPAISpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPAISpawnerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITOR
public:
	virtual void PreEditChange(FProperty* PropertyThatWillChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	TArray<struct FNPAITaskStruct> PreAITasksList;
#endif

	virtual void BeginPlay() override;

	/**
	 * Ends gameplay for this component.
	 * Called from AActor::EndPlay only if bHasBegunPlay is true
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// If character can be spawned he will be on map when time will be equal 0
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPAISpawner")
	struct FNPTimeStruct LeftTimeToSpawnNPAICharacter;

	// F.g if MinRandomSpawnTime = {0, 0, 5} and MaxRandomSpawnTime = = {0, 0, 10}. after begin play character will be spawned after  (5-10) minuts
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPAISpawner")
	struct FNPTimeStruct MinRandomSpawnTime;
	// F.g if MinRandomSpawnTime = {0, 1, 5} and MaxRandomSpawnTime = = {0, 2, 10}. after begin play character will be spawned after  (65-130) minuts
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPAISpawner")
	struct FNPTimeStruct MaxRandomSpawnTime;

	// F.g if BeginValidTime = {0, 10, 13} and EndValidTime = = {0, 20, 0} the character can exis on them map only between 10:13 - 20:00, If Character is exist on the map but time is passed Character is going to Home using taks "GoHome"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPAISpawner")
	struct FNPTimeStruct BeginValidTime;
	// F.g if BeginValidTime = {0, 22, 30} and EndValidTime = = {0, 5, 0} the character can exis on them map only between 22:30 - 5:00,(6.5.h). If Character is exist on the map but time is passed Character is going to Home using taks "GoHome"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPAISpawner")
	struct FNPTimeStruct EndValidTime;

	//Here You can Setup Task
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPAICharacter|AI")
	FNPAIDataCharacter NPAIDataCharacter;

	//Here You can Setup Task for Character, Remember if character is already on the map you have to change task directly in character not in spawner
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPAISpawner")
	TArray<TSoftClassPtr<class ANPAICharacterBase>> AICharactersClassToSpawn;

	// F.g if MaxCharacterAmountPerSpawner = 5, thats mean this spawner can handle in the same time max 5 characters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPAISpawner")
	int MaxCharacterAmountPerSpawner = 1;

	//How many characters this spawner can spawn yet, Every spawn LeftCharacterAmount is decreasing by 1
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPAISpawner")
	int LeftCharacterAmountToSpawn = 1;

	void ResetCharacter(ANPAICharacterBase* NPAICharacterBase, bool ByDestroy);

	void ResetLeftTimeToSpawn();
	void ClearSpawner();

	UPROPERTY()
	TSubclassOf<class ANPAICharacterBase> NextCharacterClassToSpawn = nullptr;

	bool CanSpawnByTime();
	void AddSpawnedCharacter(class ANPAICharacterBase* SpawnedCharacter);

	TArray<ANPAICharacterBase*> GetSpawnedCharacters() {return SpawnedCharacters; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPAISpawner")
	TArray<ANPAICharacterBase*> SpawnedCharacters;

	virtual ANPAICharacterBase* SpawnCharacter(FTransform SpawnTransform);

	virtual TSubclassOf<class ANPAICharacterBase> RandomCharacterToSpawn();

	virtual void TimeToGoHome();

private:
	UFUNCTION()
	void OnTimeUpdated(struct FNPTimeStruct UpdatedTime, struct FNPTimeStruct DifTime);

	bool CanSpawnByRange();

	bool CanBeSpawned();

	bool CanSpawnByCooldown();
	UFUNCTION()
	void LoadAISpawner();
	void UploadSpawnRange();

	UPROPERTY()
	float SqrDistanceToLoad;
	UPROPERTY()
	FNPTimeStruct CurrentUpdatedTime;
	UFUNCTION()
	void RemoveCharacter(class AActor* DestroyedActor);
};

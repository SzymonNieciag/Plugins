// Szymon XIII Wielki

#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Actor.h>
#include "AI/NPAIDefines.h"
#include "NPAIManager.generated.h"

enum class EActorSpawnType :uint8
{
	Spawned = 0,
	Placed = 1,
};

struct NOPEAISYSTEM_API FAIUpdateActorIndex
{
	int Index;
	bool bCanUpdate;
	EActorSpawnType ActorSpawnType;
	FAIUpdateActorIndex() : Index(-1), bCanUpdate(true), ActorSpawnType(EActorSpawnType::Placed){}
};

UCLASS()
class NOPEAISYSTEM_API ANPAIManager : public AActor
{
	GENERATED_BODY()

public:
	ANPAIManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** Overridable native event for when play begins for this actor. */
	virtual void BeginPlay();

	/** Overridable function called whenever this actor is being removed from a level */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, meta = (ArraySizeEnum = "EAISound"))
	FSoundStrenght GetSoundStrenght[(int)ENPAISound::MAX];

	UFUNCTION(BlueprintPure, Category = "NopeAISystem", meta = (WorldContext = "WorldContextObject"), meta = (DisplayName = "GetAIManager"))
	static ANPAIManager* GetAIManager(class UObject* WorldContextObject);


	static ANPAICharacterBase* SpawnAICharacter(class UObject* InOwner, TSoftClassPtr<ANPAICharacterBase> NPAICharacterBaseClass, FTransform SpawnTransform, const TArray<FNPAITaskStruct>& InTasksList, bool bEditableModeActive = false);

	void SaveAIData(struct FNPAIData& OutAIData);

	void LoadAIData(struct FNPAIData InAIData);

	virtual FAIUpdateActorIndex SaveAICharacter(class ANPAICharacterBase* NPAICharacterBase, bool Valid, bool LoadedByDistance);
	virtual FAIUpdateActorIndex LoadAICharacter(class ANPAICharacterBase* NPAICharacterBase);

	virtual void SaveAISpawner(class UNPAISpawnerComponent* NPAISpawnerComponent, bool Valid);
	virtual void LoadAISpawner(class UNPAISpawnerComponent* NPAISpawnerComponent);

	virtual void RemoveCharacterFromData(EActorSpawnType ActorSpawnType, int Index);

private:
	void ControlCharactersByDistance(FVector PlayerPosition);
	FAIUpdateActorIndex FoundCharacter(class ANPAICharacterBase* NPAICharacterBase);
	bool UpdateValidAICharacters(class ANPAICharacterBase* NPAICharacterBase);
	void LoadAICharacter(class ANPAICharacterBase* NPAICharacterBase, const struct FSaveDataNPAICharacter& SaveDataNPAICharacter);

	void SaveAICharacters();
	void SaveSpawners();
	void StreamingCharacters();

	ANPAICharacterBase* SpawnAICharacterFromSave(class UObject* InOwner, TSoftClassPtr<ANPAICharacterBase> NPAICharacterBaseClass, FTransform SpawnTransform, FSaveDataNPAICharacter& SaveDataNPAICharacter);

protected:
	struct FNPAIData AIData;
};


//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Subsystems/WorldSubsystem.h>
#include "AI/NPAIDefines.h"
#include "NPAISubsystem.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNPAICharacterSpawn, class ANPAICharacterBase*, OutNPAICharacterBase, bool, Added, int, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSmartObjectComponentSpawn, const class UNPSmartObjectComponent*, OutSmartObjectComponent, bool, Added, int, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAISpawnerComponentSpawn, const class UNPAISpawnerComponent*, OutAISpawnerComponent, bool, Added, int, Amount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPGameTimeUpdate, const struct FNPTimeStruct, UpdatedTime, const struct FNPTimeStruct, TimeDifference);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDebugAIEnableSignature, const bool, DebugAIEnaled);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNPGameLogicStart);

UCLASS(BlueprintType, Transient)
class NOPEAISYSTEM_API UNPAISubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

protected:
	// USubsystem implementation Begin
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// USubsystem implementation End
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

public:

	UNPAISubsystem();
	// FTickableGameObject implementation Begin
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	// FTickableGameObject implementation End

	UPROPERTY(Transient)
	class ANPAIManager* NPAIManager;

	void UseOwnTimer(bool OwnTimerEnabled);

public:
	UPROPERTY(BlueprintAssignable, Category = "NPAISubsystem")
	FNPAICharacterSpawn OnAICharacterSpawned;

	UPROPERTY(BlueprintAssignable, Category = "NPAISubsystem")
	FSmartObjectComponentSpawn OnSmartObjectComponentSpawned;

	UPROPERTY(BlueprintAssignable, Category = "NPAISubsystem")
	FAISpawnerComponentSpawn OnAISpawnerComponentSpawned;

	UPROPERTY(BlueprintAssignable, Category = "GameState")
	FNPGameTimeUpdate OnGameTimeUpdated;

	UPROPERTY(BlueprintAssignable, Category = "SaveGame")
	FNPGameLogicStart OnGameLogicStarted;

	UPROPERTY(BlueprintAssignable, Category = "SaveGame")
	FDebugAIEnableSignature OnDebugAIEnabled;

	/** Static helper function to get a DragoAISubsystem subsystem from a world, returns nullptr if world or subsystem don't exist */
	static UNPAISubsystem* GetAISubsystem(const UObject* WorldContextObject);

	static void SetGameLogicStarted(bool bInValue);
	static bool GetGameLogicStarted() { return bGameLogicStarted; }

	void SetCurrentGameTime(float NewGameTime);
	float GetCurrentGameTime();

private:
	static bool bGameLogicStarted;
	bool bOwnTimerEnabled = false;
	float TickInterval = 1;
	float CurTickInterval = 1;

	UUserWidget* UserWidget;
	TSubclassOf<UUserWidget> UserWidgetClass;

	float CurrentGameTime = 0.0;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPAISubsystem")
	TArray<class ANPAICharacterBase*> AICharacters;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPAISubsystem")
	TArray<class UNPSmartObjectComponent*> SmartObjectComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPAISubsystem")
	TArray<class UNPAISpawnerComponent*> AISpawnerComponents;

public:
	TArray<class ANPAICharacterBase*> GetAICharacters();

	UFUNCTION(BlueprintPure, Category = "NPAISubsystem")
	void GetAICharacters(TArray<class ANPAICharacterBase*>& OutAICharacters, TSubclassOf<class ANPAICharacterBase> FindClass);

	UFUNCTION(BlueprintPure, Category = "NPAISubsystem")
	TArray<class UNPSmartObjectComponent*> GetSmartObjectComponents();

	UFUNCTION(BlueprintPure, Category = "NPAISubsystem")
	TArray<class UNPAISpawnerComponent*> GetAISpawnerComponents(TSubclassOf<class AActor> ClassFilter = nullptr);

public:
	void AddAICharacterBase(class ANPAICharacterBase* InNPAICharacterBase);
	void RemoveAICharacterBase(class ANPAICharacterBase* InNPAICharacterBase);

	void AddSmartObject(class UNPSmartObjectComponent* InSmartObjectComponent);
	void RemoveSmartObject(class UNPSmartObjectComponent* InSmartObjectComponent);

	void AddAISpawnerComponent(class UNPAISpawnerComponent* AISpawnerComponent);
	void RemoveAISpawnerComponent(class UNPAISpawnerComponent* AISpawnerComponent);

	void OnAssetsLoaded();

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "NPAISubsystem")
	static bool AIDebugSystemEnabled(UObject* WorldContextObject);

protected:
#if WITH_EDITOR
	/** By default, there is no water subsystem allowed on preview worlds except when explicitly requested : */
	bool bAllowAISubsystemOnPreviewWorld = false;
#endif // WITH_EDITOR

	int32 CurrentAIDebugSystem = 0;
};












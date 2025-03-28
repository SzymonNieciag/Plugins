// Copyright 2018 DRAGO. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NPAIDefines.generated.h"

UENUM(BlueprintType)
enum class ENPMapStatusChanged : uint8
{
	Loaded = 0,
	UnLoaded = 1,
	Show = 2,
	Hidden = 3,
};

UENUM(BlueprintType)
enum class ENPAIMovementDirection : uint8
{
	Forward = 0,
	Back,
	Left,
	Right,
	MAX
};

UENUM(BlueprintType)
enum class ETeamType : uint8
{
	Player = 0,
	RomanEmpire,
	OttomanEmpire,
	Barbarian,
	Neutral,
	Animals,
	MAX,
};

UENUM(BlueprintType)
enum class ENPAIMovementSpeed : uint8
{
	Sneak = 0,
	WalkSlow,
	Walk,
	Jog,
	Run,
	Sprint,
	MAX
};

UENUM(BlueprintType)
enum class ENPAIMovementMode : uint8
{
	Walking,
	Swimming,
	Flying,
	Falling,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ENPAISound : uint8
{
	FootStep = 0,
	SprintStep,
	ThrowItem, 
	MAX,
};

USTRUCT(BlueprintType)
struct NOPEAISYSTEM_API FSoundStrenght
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SoundEffectStrenght = 0.0f;
};

UENUM(BlueprintType)
enum class ENPAIActionState :uint8
{
	Action = 0 UMETA(DisplayName = "Action"),
	Move = 1 UMETA(DisplayName = "Move"),
	Wait = 2 UMETA(DisplayName = "Wait"),
	MAX = 3 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ENPTaskResult :uint8
{
	TaskSuccess = 0 UMETA(DisplayName = "TaskSuccess"),
	TaskFalse = 1 UMETA(DisplayName = "TaskFalse"),
	TaskIgnore = 2 UMETA(DisplayName = "TaskIgnore"),
};

UENUM(BlueprintType)
enum class ENPTaskFinish :uint8
{
	WorkEnd = 0,
	Conversation = 1,
	EndTime = 2,
	Unavailable = 3,
};

UENUM(BlueprintType)
enum class ECharacterCombatState : uint8
{
	Idle,
	SearchEnemy,
	SearchNormal,
	Allert,
	Attack,
	RunAway,
	Trapped,
	Death,
	Max UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct NOPEAISYSTEM_API FSmartPointStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|SmartPoint")
	class ANPAICharacterBase* CurrentCharacter;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|SmartPoint")
	FTransform WorldTransform;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|SmartPoint")
	FTransform SocketSnapTransform;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|SmartPoint")
	int DefinitionAssetIndex = -1;

	FSmartPointStruct() : CurrentCharacter(nullptr), DefinitionAssetIndex(-1) {}
	FSmartPointStruct(class ANPAICharacterBase* currentCharacter, int definitionAssetIndex) : CurrentCharacter(currentCharacter), DefinitionAssetIndex(definitionAssetIndex){}
};

USTRUCT(BlueprintType)
struct NOPEAISYSTEM_API FNPTaskFinishReason
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	ENPTaskResult TaskResult;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Task")
	ENPTaskFinish FinishReason;

	FNPTaskFinishReason() : TaskResult(ENPTaskResult::TaskSuccess), FinishReason(ENPTaskFinish::WorkEnd) {}
	FNPTaskFinishReason(ENPTaskResult taskResult, ENPTaskFinish finishReason) : TaskResult(taskResult), FinishReason(finishReason) {}
};

namespace NPAISoundTag
{
	TCHAR const* const FootStep = TEXT("FootStep");
	TCHAR const* const ThrowItem = TEXT("ThrowItem");
}

USTRUCT(BlueprintType)
struct NOPEAISYSTEM_API FNPAITaskProgress
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TaskProgress")
	FString TaskName = "MainTask";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TaskProgress")
	float LeftTaskTime = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TaskProgress")
	float MaxLeftTaskTime = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TaskProgress")
	int CurProgress = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TaskProgress")
	int MaxProgress = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TaskProgress")
	int LeftRepeatCount = 1;

	UPROPERTY()
	float InitLeftTaskTime = 100.0f;
	UPROPERTY()
	int InitCurProgress = 0;
	UPROPERTY()
	int InitLeftRepeatCount = 1;
	UPROPERTY()
	bool bIsInitialized = false;
	UPROPERTY()
	bool bIsFinished = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TaskProgress")
	TArray<ENPAIActionState> RequirmentState;

	FNPAITaskProgress();

	FNPAITaskProgress(float leftTaskTime, float startTaskTime, int curProgress, int maxProgress, int leftRepeatCount) : LeftTaskTime(leftTaskTime), CurProgress(curProgress), MaxProgress(maxProgress), LeftRepeatCount(leftRepeatCount) {}

	FNPAITaskProgress& operator = (const FNPAITaskProgress& other)
	{
		TaskName = other.TaskName;
		LeftTaskTime = other.LeftTaskTime;
		CurProgress = other.CurProgress;
		MaxProgress = other.MaxProgress;
		LeftRepeatCount = other.LeftRepeatCount;
		InitLeftTaskTime = other.InitLeftTaskTime;
		InitCurProgress = other.InitCurProgress;
		InitLeftRepeatCount = other.InitLeftRepeatCount;
		bIsInitialized = other.bIsInitialized;
		bIsFinished = other.bIsFinished;
		return *this;
	}

	void ResetTasks();
	void InitTasks();

	bool CanUpdateTime(ENPAIActionState NPAIActionState);
};

USTRUCT(BlueprintType)
struct NOPEAISYSTEM_API FNPTimeStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
	int Days;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
	int Hours;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
	int Minutes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	float TotalTime;

	FNPTimeStruct() : Days(0), Hours(0), Minutes(0), TotalTime(0.0f) {}
	FNPTimeStruct(float TotalTime)
	{
		UpdateTime(TotalTime);
	} 
	FNPTimeStruct(int days, int hours, int minutes) : Days(days), Hours(hours), Minutes(minutes)
	{
		TotalTime = Days * 1440 + Hours * 60 + Minutes;
	}

	void UpdateTime(FNPTimeStruct TimeToUpdate)
	{
		float AddedTotalTime = TimeToUpdate.Days * 1440 + TimeToUpdate.Hours * 60 + TimeToUpdate.Minutes;
		TotalTime += AddedTotalTime;
		int IntTotalTime = FMath::RoundToInt(TotalTime);
		Minutes = IntTotalTime % 60;
		Hours = (IntTotalTime / 60) % 24;
		Days = (IntTotalTime / 1440);
	}

	void UpdateTime(float DeltaTime)
	{
		TotalTime += DeltaTime;
		int IntTotalTime = FMath::RoundToInt(TotalTime);
		Minutes = IntTotalTime % 60;
		Hours = (IntTotalTime / 60) % 24;
		Days = (IntTotalTime / 1440);
	}

	void UpdateTimeStruct(float InTotalTime);

	void UpdateTotalTime()
	{
		TotalTime = (Days * 3600) + (Hours * 60) + (Minutes * 1);
	}

	static float GetTimeOfDay(const struct FNPTimeStruct& CurrentBeginValidTime)
	{
		return CurrentBeginValidTime.Hours * 60 + CurrentBeginValidTime.Minutes;
	}

	static bool IsInTimeRange(const struct FNPTimeStruct& CurrentBeginValidTime, const struct FNPTimeStruct& OutBeginValidTime, const struct FNPTimeStruct& OutEndValidTime)
	{
		if (FNPTimeStruct::GetTimeOfDay(OutBeginValidTime) < FNPTimeStruct::GetTimeOfDay(OutEndValidTime))
		{
			if (FNPTimeStruct::GetTimeOfDay(CurrentBeginValidTime) >= FNPTimeStruct::GetTimeOfDay(OutBeginValidTime) && FNPTimeStruct::GetTimeOfDay(CurrentBeginValidTime) <= FNPTimeStruct::GetTimeOfDay(OutEndValidTime))
			{
				return true;
			}
		}
		else if (FNPTimeStruct::GetTimeOfDay(CurrentBeginValidTime) >= FNPTimeStruct::GetTimeOfDay(OutBeginValidTime) || FNPTimeStruct::GetTimeOfDay(CurrentBeginValidTime) <= FNPTimeStruct::GetTimeOfDay(OutEndValidTime))
		{
			return true;
		}
		return false;
	}
	FNPTimeStruct& operator =(const FNPTimeStruct& InOther);

	static FNPTimeStruct RandomTimeInRange(const FNPTimeStruct& MinRandomSpawnTime, const FNPTimeStruct& MaxRandomSpawnTime);
};

USTRUCT(BlueprintType)
struct NOPEAISYSTEM_API FNPAITaskStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Task")
	TSoftClassPtr<class UNPAITaskBase> AITaskBaseClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Task")
	TSoftObjectPtr<class AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Task")
	ENPAIMovementSpeed AIMovementSpeed = ENPAIMovementSpeed::Jog;

	//This task can be activated only during this time
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Task")
	FNPTimeStruct BeginValidTime;

	//This task can be activated only during this time
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Task")
	FNPTimeStruct EndValidTime;

	//Here you can setup Time for task
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Task")
	TArray<FNPAITaskProgress> TasksProgress;

	UPROPERTY(BlueprintReadOnly, Category = "Task")
	TArray<FNPTaskFinishReason> TaskFinishReasons;

	FNPAITaskStruct();

	void Reset(bool ResetAll = true)
	{
		if (ResetAll)
		{
			for (FNPAITaskProgress& NPAITaskProgress : TasksProgress)
			{
				NPAITaskProgress.ResetTasks();
			}
		}
		TaskFinishReasons.Empty();
	}
};

USTRUCT(BlueprintType)
struct NOPEAISYSTEM_API FNPAIDataCharacter
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, Category = "NPAICharacter|AI")
	TArray<FNPAITaskStruct> AITasksList;
	// If character has running away points he will be going directly to them whe nhe will be scared, otherwise he will be going in random places far from enemy
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "NPAICharacter|AI")	
	TArray<TSoftObjectPtr<class AActor>> RunAwayPoints;
	// Home is setuping automaticly as SpawnerHome but you can override this and this character will be going to custom Point
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "NPAICharacter|AI")
	TArray<TSoftObjectPtr<class AActor>> TargetHomes;
	UPROPERTY(BlueprintReadOnly, Category = "NPAICharacter|AI")	
	TArray<TSoftObjectPtr<class AActor>> SpawnedActors;
};


USTRUCT(BlueprintType)
struct NOPEAISYSTEM_API FAnimationSaveStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() TSoftObjectPtr<class UAnimMontage> AnimMontageSoft;
	UPROPERTY() float PlayRate;
	UPROPERTY() float StartingPosition;
	UPROPERTY() FName StartingSection;
	UPROPERTY() uint8 VisibilityBasedAnimTickOption = -1;

	UPROPERTY() FTransform StartCharacterTransform;
	UPROPERTY() FTransform EndCharacterTransform;
};

USTRUCT()
struct FSaveDataNPAICharacter
{
	GENERATED_BODY()
	UPROPERTY() TSoftClassPtr<class ANPAICharacterBase> AICharacterClass;
	UPROPERTY() TSoftObjectPtr<class ANPAICharacterBase> AICharacterSoft;
	UPROPERTY() TSoftObjectPtr<class AActor> OwnerActor;
	UPROPERTY() bool bIsValid;
	UPROPERTY() bool bIsLoadedByDistance;
	UPROPERTY() float DistanceToLoad;
	UPROPERTY() float DistanceToUnload;

	UPROPERTY() FTransform Transform;
	UPROPERTY() FNPAIDataCharacter NPAIDataCharacter;

	UPROPERTY() TArray<FName> Tags;
	UPROPERTY() bool bCollisionEnabled = 1;
	UPROPERTY() uint8 MovementMode = -1;

	UPROPERTY() FAnimationSaveStruct AnimationSaveStruct;
};

USTRUCT()
struct FSaveDataNPAISpawner
{
	GENERATED_BODY()
	UPROPERTY() TSoftObjectPtr<class UNPAISpawnerComponent> AISpawnerComponentSoft;
	UPROPERTY() FNPAIDataCharacter NPAIDataCharacter;
	UPROPERTY() FNPTimeStruct LeftTimeToSpawnNPAICharacter;
	UPROPERTY() TArray<TSoftClassPtr<class ANPAICharacterBase>> AICharactersClassToSpawn;
	UPROPERTY() int LeftCharacterAmountToSpawn = 1;
};

USTRUCT()
struct FNPAIData
{
	GENERATED_BODY()
	UPROPERTY() TArray<struct FSaveDataNPAICharacter> SpawnedAICharacters;
	UPROPERTY() TArray<struct FSaveDataNPAICharacter> PlacedAICharacters;
	UPROPERTY() TArray<struct FSaveDataNPAISpawner> SpawnerData;
};


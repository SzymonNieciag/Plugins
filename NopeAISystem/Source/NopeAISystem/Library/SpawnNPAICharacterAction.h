#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "GameFramework/Actor.h"
#include "Engine/LatentActionManager.h"
#include <Engine/StreamableManager.h>
#include <Engine/AssetManager.h>
#include "NopeAISystem/NPAIManager.h"

/**
 * PingPongAction is the class which will handle the interpolation of an Actor between two locations
 */
class NOPEAISYSTEM_API FSpawnNPAICharacterAction : public FPendingLatentAction
{
public:
	bool bLoaded;
	class UObject* InOwner;
	TSoftClassPtr<class ANPAICharacterBase> AICharacterBaseClass;
	FTransform SpawnTransform;
	const TArray<struct FNPAITaskStruct> TasksList;
	bool bEditableModeActive = false;
	/** Function to execute on completion */
	FName ExecutionFunction;

	class ANPAICharacterBase* NewCharacterBase;
	class ANPAICharacterBase*& OutCharacterBase;


	/** Link to fire on completion */
	int32 OutputLink;

	/**
	 * Object to call callback on upon completion
	 */
	FWeakObjectPtr CallbackTarget;

	FSpawnNPAICharacterAction(class UObject* InOwner, TSoftClassPtr<class ANPAICharacterBase> InAICharacterBaseClass, FTransform InSpawnTransform, const TArray<FNPAITaskStruct>& InTasksList, bool InEditableModeActive, const FLatentActionInfo& LatentInfo, class ANPAICharacterBase*& InCharacterBaseClass)
		: bLoaded(false)
		, InOwner(InOwner)
		, AICharacterBaseClass(InAICharacterBaseClass)
		, SpawnTransform(InSpawnTransform)
		, TasksList(InTasksList)
		, bEditableModeActive(InEditableModeActive)
		, ExecutionFunction(LatentInfo.ExecutionFunction)
		, NewCharacterBase(nullptr)
		, OutCharacterBase(InCharacterBaseClass)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)

	{
		
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		UClass* LoadedClass = AICharacterBaseClass.Get();
		if (LoadedClass)
		{
			NewCharacterBase = ANPAIManager::SpawnAICharacter(InOwner, AICharacterBaseClass, SpawnTransform, TasksList, bEditableModeActive);
			OutCharacterBase = NewCharacterBase;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
		else
		{
			Response.FinishAndTriggerIf(false, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};
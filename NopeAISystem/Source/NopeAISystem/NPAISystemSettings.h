// DRAGO Entertainment (c) 2020

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NPAISystemSettings.generated.h"

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "AISettings"))
class NOPEAISYSTEM_API UNPAISystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UNPAISystemSettings();

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UUserWidget> CharacterInfoWidget;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class ANPAIManager> AIManagerClass;

	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category = "DataTable")
	TSoftClassPtr<class AAITargetPoint> AITargetPointSoftClass;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<class UStaticMesh> DefaultStaticMesh;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	bool bEnableAILogic = true;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	bool bShowAITaskWidget = false;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	bool bEnableAISave = true;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	bool bEnableAILoad = true;

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<class UNPAITaskBase> WaitTask;

	//UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	//int SquareDiameter = 50;
	//UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	//int SquareHeight = 400;
	//UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	//float DrawSquareScale = 0.8;
	//UPROPERTY(config, EditAnywhere, BlueprintReadOnly)
	//float HeightSquareSnaping = 200.f;

	//UPROPERTY(config, EditDefaultsOnly, meta = (ArraySizeEnum = "EditableGrid"))
	//FGridNodeTypeArray GetCompatibileNodeType[EGridNodeType::None];

	//UPROPERTY(config, EditDefaultsOnly, meta = (ArraySizeEnum = "EditableGrid"))
	//float GetGridNodeHeight[EGridNodeType::None];

	//UPROPERTY(config, EditDefaultsOnly, meta = (ArraySizeEnum = "EditableGrid"))
	//int GetNodeTypePriority[EGridNodeType::None];
};

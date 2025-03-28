#pragma once

#include "Engine.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "UnrealEd.h"

DECLARE_LOG_CATEGORY_EXTERN(NopeAISystemEditor, All, All)

class FNopeAISystemEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;


private:
	/** StaticClass is not safe on shutdown, so we cache the name, and use this to unregister on shut down */
	TArray<FName> ClassesToUnregisterOnShutdown;
	TArray<FName> VisualizersToUnregisterOnShutdown;
};


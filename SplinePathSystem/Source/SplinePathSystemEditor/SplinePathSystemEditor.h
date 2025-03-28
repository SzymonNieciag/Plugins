#pragma once

#include "Engine.h"
#include "Modules/ModuleInterface.h"
#include "UnrealEd.h"
#include "SplinePathVisualizer.h"

DECLARE_LOG_CATEGORY_EXTERN(SplinePathSystemEditor, All, All)

class FSplinePathSystemEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

private:
	TSharedPtr<FSplinePathVisualizer> SplineVisualizer;
};


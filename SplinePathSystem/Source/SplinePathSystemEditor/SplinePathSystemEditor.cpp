#include "SplinePathSystemEditor.h"
#include "PropertyEditorModule.h"
#include "UnrealEd.h"
#include "SplinePathSystem/Components/PathSplineComponent.h"

IMPLEMENT_GAME_MODULE(FSplinePathSystemEditorModule, SplinePathSystemEditor);


void FSplinePathSystemEditorModule::StartupModule()
{
	SplineVisualizer = MakeShareable(new FSplinePathVisualizer());
	if (GUnrealEd)
	{
		GUnrealEd->RegisterComponentVisualizer(UPathSplineComponent::StaticClass()->GetFName(), SplineVisualizer);
		SplineVisualizer->OnRegister();
	}
}

void FSplinePathSystemEditorModule::ShutdownModule()
{
	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(UPathSplineComponent::StaticClass()->GetFName());
	}
}


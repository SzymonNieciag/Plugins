#include "NopeAISystemEditor.Build.h"
#include "NopeAISystem/Navigation/NPNavLinkCustomComponent.h"
#include "Components/NavLinkPointVisualizer.h"
#include "NopeAISystem/AI/Components/NPAISpawnerComponent.h"
#include "NopeAISystem/AI/Components/NPCharacterMovementComponent.h"

IMPLEMENT_GAME_MODULE(FNopeAISystemEditorModule, NopeAISystemEditor);

DEFINE_LOG_CATEGORY(NopeAISystemEditor)

#define LOCTEXT_NAMESPACE "NopeAISystemEditor"

void FNopeAISystemEditorModule::StartupModule()
{
	UE_LOG(NopeAISystemEditor, Warning, TEXT("NopeAISystemEditor: Log Started"));
	if (GUnrealEd)
	{
		TSharedPtr<FNavLinkPointVisualizer> Visualizer = MakeShared<FNavLinkPointVisualizer>();
		FName Name = UNPNavLinkCustomComponent::StaticClass()->GetFName();
		GUnrealEd->RegisterComponentVisualizer(Name, Visualizer);
		FName SecName = UNPAISpawnerComponent::StaticClass()->GetFName();
		GUnrealEd->RegisterComponentVisualizer(SecName, Visualizer);
		FName ThirdName = UNPCharacterMovementComponent::StaticClass()->GetFName();
		GUnrealEd->RegisterComponentVisualizer(ThirdName, Visualizer);
		// This call should maybe be inside the RegisterComponentVisualizer call above, but since it's not,
		// we'll put it here.
		Visualizer->OnRegister();
		VisualizersToUnregisterOnShutdown.Add(Name);
		VisualizersToUnregisterOnShutdown.Add(SecName);
		VisualizersToUnregisterOnShutdown.Add(ThirdName);
	}
}

void FNopeAISystemEditorModule::ShutdownModule()
{
	UE_LOG(NopeAISystemEditor, Warning, TEXT("NopeAISystemEditor: Log Ended"));

	if (GUnrealEd)
	{
		for (const FName& Name : VisualizersToUnregisterOnShutdown)
		{
			GUnrealEd->UnregisterComponentVisualizer(Name);
		}
	}

	// Unregister customizations
	FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");
	if (PropertyEditorModule)
	{
		for (const FName& ClassName : ClassesToUnregisterOnShutdown)
		{
			PropertyEditorModule->UnregisterCustomClassLayout(ClassName);
		}
	}
}



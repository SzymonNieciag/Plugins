// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NopeAISystemEditor : ModuleRules
{
    public NopeAISystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UnrealEd", "NopeAISystem" });

        PrivateDependencyModuleNames.AddRange(
     new string[]
     {
        "UnrealEd",
        // Other dependencies here
        "ComponentVisualizers",
        "NavigationSystem"

     }
 );
    }
}


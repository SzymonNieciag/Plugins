using UnrealBuildTool;

public class SplinePathSystemEditor : ModuleRules
{
    public SplinePathSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd", 
            "SplinePathSystem", 
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UnrealEd",
            "Slate",
            "SlateCore",
            "InputCore",
            "PropertyEditor",
            "DetailCustomizations",
            "ComponentVisualizers",
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            "SplinePathSystemEditor/Private"
        });
    }
}

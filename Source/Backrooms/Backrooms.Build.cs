using UnrealBuildTool;

public class Backrooms : ModuleRules
{
    public Backrooms(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "AIModule",
            "NavigationSystem",
            "UMG",
            "Slate",
            "SlateCore",
            "GameplayTags",
            "Niagara",
            "PhysicsCore",
            "AnimGraphRuntime"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "RenderCore",
            "RHI"
        });

        // Uncomment if you use YAML or other third-party libs
        // PublicIncludePaths.AddRange(new string[] { });
    }
}

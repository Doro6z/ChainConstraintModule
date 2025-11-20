using UnrealBuildTool;

public class ChainConstraint : ModuleRules
{
    public ChainConstraint(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "PhysicsCore",
            "Chaos",
            "ChaosSolverEngine",
            "PhysicsCore",
            "ProceduralMeshComponent" // optionnel, utile plus tard
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "RenderCore",
            "RHI",
            "Slate",
            "SlateCore"
        });

        PublicIncludePaths.AddRange(new string[]
        {
            "ChainConstraint/Public"
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            "ChainConstraint/Private"
        });
    }
}

// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RopeStressTests : ModuleRules
{
	public RopeStressTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"RopeStressTests",
			"RopeStressTests/Variant_Platforming",
			"RopeStressTests/Variant_Platforming/Animation",
			"RopeStressTests/Variant_Combat",
			"RopeStressTests/Variant_Combat/AI",
			"RopeStressTests/Variant_Combat/Animation",
			"RopeStressTests/Variant_Combat/Gameplay",
			"RopeStressTests/Variant_Combat/Interfaces",
			"RopeStressTests/Variant_Combat/UI",
			"RopeStressTests/Variant_SideScrolling",
			"RopeStressTests/Variant_SideScrolling/AI",
			"RopeStressTests/Variant_SideScrolling/Gameplay",
			"RopeStressTests/Variant_SideScrolling/Interfaces",
			"RopeStressTests/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

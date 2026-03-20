// Copyright (c) 2026 Will Long

using UnrealBuildTool;


public class MultithreadingDemo : ModuleRules
{
	public MultithreadingDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"DeveloperSettings",
			"UMG",
			"EnhancedInput",
			"Slate",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
		
		PublicIncludePaths.AddRange(new string[] {
			"MultithreadingDemo",
			"MultithreadingDemo/Movers",
			"MultithreadingDemo/Settings",
			"MultithreadingDemo/Spawning",
			"MultithreadingDemo/Subsystems",
			"MultithreadingDemo/UI",
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
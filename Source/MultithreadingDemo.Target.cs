// Copyright (c) 2026 Will Long

using UnrealBuildTool;
using System.Collections.Generic;

public class MultithreadingDemoTarget : TargetRules
{
	public MultithreadingDemoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;

		ExtraModuleNames.AddRange( new string[] { "MultithreadingDemo" } );
	}
}

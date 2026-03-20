// Copyright (c) 2026 Will Long

using UnrealBuildTool;
using System.Collections.Generic;

public class MultithreadingDemoEditorTarget : TargetRules
{
	public MultithreadingDemoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;

		ExtraModuleNames.AddRange( new string[] { "MultithreadingDemo" } );
	}
}

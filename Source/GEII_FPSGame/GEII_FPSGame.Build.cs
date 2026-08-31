// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GEII_FPSGame : ModuleRules
{
	public GEII_FPSGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}

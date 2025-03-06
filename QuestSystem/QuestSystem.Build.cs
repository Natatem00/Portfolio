// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class QuestSystem : ModuleRules
{
	public QuestSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayTags", "GameplayMessageRuntime", "GameplayMessageRuntime", "StructUtils" });
		
		PublicIncludePaths.Add("QuestSystem/");
	}
}

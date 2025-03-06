// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class DragoncallNetwork : ModuleRules
{
	public DragoncallNetwork(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "OnlineSubsystem" });
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"HTTP", "Json", "JsonUtilities", "DeveloperSettings", "Sockets", "Networking", "NetCore"
			}
			);
		PublicIncludePaths.AddRange(new string[]{ "DragoncallNetwork/"
		});
	}
}

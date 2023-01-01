// Georgy Treshchev 2023.

using System;
using System.IO;
using UnrealBuildTool;

public class RuntimeArchiver : ModuleRules
{
	
	private string ThirdPartPath
	{
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/")); }
	}
	private string MinizPath
	{
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, ThirdPartPath, "miniz")); }
	}

	public RuntimeArchiver(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[]
			{
				MinizPath
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine"
			}
		);
	}
}
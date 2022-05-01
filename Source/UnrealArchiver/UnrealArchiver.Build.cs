// Georgy Treshchev 2022.

using System;
using System.IO;
using UnrealBuildTool;

public class UnrealArchiver : ModuleRules
{
	private string MinizPath
	{
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/miniz")); }
	}

	public UnrealArchiver(ReadOnlyTargetRules Target) : base(Target)
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
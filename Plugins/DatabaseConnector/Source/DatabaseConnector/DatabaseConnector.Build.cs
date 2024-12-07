// Copyright Pandores Marketplace 2021. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;

public class DatabaseConnector : ModuleRules
{
	/**
	 * If we should link arm libraries for MacOS target platform.
	 **/
	static bool bLinkArmLibsForMacOS = false;

	public DatabaseConnector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Nanodbc throws exceptions, we have to catch'em all.
		bEnableExceptions = true;

		// Path to the ThirtParty directory.
		string ThirdPartyPath		= Path.Combine(PluginDirectory, "Source/ThirdParty/");
		string NanodbcPath			= Path.Combine(ThirdPartyPath,  "nanodbc/");
		string NanodbcRootPath		= Path.Combine(NanodbcPath,     GetPlatformDescriptor(Target));
		string NanodbcCommonPath	= Path.Combine(NanodbcPath,     "common");

		// Windows Libraries
		if (Target.Platform == UnrealTargetPlatform.Win64)
        {
			AddLibrary(Path.Combine(NanodbcRootPath, "lib/nanodbc.lib"));
		}

		// Linux Libraries
		else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
			AddLibrary(Path.Combine(NanodbcRootPath, "lib/libnanodbc.a"));
			AddLibrary(Path.Combine(NanodbcRootPath, "lib/installed/libodbc.a"));
			AddLibrary(Path.Combine(NanodbcRootPath, "lib/installed/libltdl.a"));
		}

		// MacOS Libraries
		else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
			AddLibrary(Path.Combine(NanodbcRootPath, "lib/libnanodbc.a"));
			AddLibrary(Path.Combine(NanodbcRootPath, "lib/installed/libodbc.a"));
			AddLibrary(Path.Combine(NanodbcRootPath, "lib/installed/libodbccr.a"));
			AddLibrary(Path.Combine(NanodbcRootPath, "lib/installed/libodbcinst.a"));
			AddLibrary(Path.Combine(NanodbcRootPath, "lib/installed/libltdl.a"));
			//AddLibrary(Path.Combine(NanodbcRootPath, "lib/installed/libiconv.a"));

			PublicSystemLibraries.Add("iconv");
        }

		// Includes
		PrivateIncludePaths.Add(Path.Combine(NanodbcCommonPath, "include/"));

		// Public Engine's dependencies
		PublicDependencyModuleNames.AddRange(new string[] 
		{
			"Core",
		});
			
		// Private Engine's dependencies
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
		});

		// Plugin files
		PublicIncludePaths .Add(Path.Combine(ModuleDirectory, "Public"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

		// Definitions
		PublicDefinitions.Add("WITH_DATABASE_ODBC_CONNECTOR=1");
	}

	/**
	 *	Gets the platform descriptor (name + arch).
	 **/
	string GetPlatformDescriptor(ReadOnlyTargetRules Target)
    {
		if (Target.Platform == UnrealTargetPlatform.Win64)
        {
			return "windows-x64";
        }

		if (Target.Platform == UnrealTargetPlatform.Linux)
        {
			return "linux-x64";
        }

		if (Target.Platform == UnrealTargetPlatform.Mac)
        {
			return bLinkArmLibsForMacOS ? "darwin-arm64" : "darwin-x64";
        }

		System.Console.Error.WriteLine("Unknown platform: " + Target.Platform.ToString());

		return "unknown";
    }

	/**
	 * Copies a DLL file to the binaries folder.
	 **/
	void CopyDll(string DllPath)
	{
		string DllCleanName = Path.ChangeExtension(Path.GetFileName(DllPath), "");
		RuntimeDependencies.Add("$(BinaryOutputDir)/" + DllCleanName + "dll", DllPath);
		//RuntimeDependencies.Add("$(BinaryOutputDir)/" + DllCleanName + "pdb", Path.ChangeExtension(DllPath, ".pdb")); 
	} 

	void AddLibrary(string Path)
    {
		PublicAdditionalLibraries.Add(Path);
	}
}

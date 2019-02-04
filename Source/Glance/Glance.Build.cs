// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class Glance : ModuleRules
{

    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }


    public Glance(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnableUndefinedIdentifierWarnings = false;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RHI", "RenderCore", "ShaderCore" });

        PrivateDependencyModuleNames.AddRange(new string[] { "Json", "JsonUtilities" });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

        LoadGlance(Target);

    }

    public void LoadGlance(ReadOnlyTargetRules Target)
    {
        string OpenFacePath = Path.Combine(ThirdPartyPath, "OpenFace");

        // Local Libraries Include paths
        string OpenFaceLocalIncludes = Path.Combine(OpenFacePath, "lib", "local");
        PublicIncludePaths.Add(Path.Combine(OpenFaceLocalIncludes, "CppInerop"));

        // Local Libraries Include
        string OpenFaceLibPaths = Path.Combine(OpenFacePath, "x64", "Release");
        PublicAdditionalLibraries.Add(Path.Combine(OpenFaceLibPaths, "GlanceAPI.lib" ));

        // Third Party Libraries Include Paths
        string OpenFaceThirdPartyIncludes = Path.Combine(OpenFacePath, "lib", "3rdParty");
        PublicAdditionalLibraries.Add(Path.Combine(OpenFaceThirdPartyIncludes, "boost", "x64", "lib", "libboost_system-vc140-mt-1_63.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(OpenFaceThirdPartyIncludes, "boost", "x64", "lib", "libboost_filesystem-vc140-mt-1_63.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(OpenFaceThirdPartyIncludes, "dlib", "lib", "x64", "v140", "Release", "dlib.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(OpenFaceThirdPartyIncludes, "OpenBLAS", "lib", "x64", "libopenblas.dll.a"));
        PublicAdditionalLibraries.Add(Path.Combine(OpenFaceThirdPartyIncludes, "OpenCV3.4", "x64", "vc14", "lib", "opencv_world342.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(OpenFaceThirdPartyIncludes, "tbb", "lib", "x64", "v140", "tbb.lib"));

        PublicIncludePaths.Add(Path.Combine(OpenFaceThirdPartyIncludes, "OpenCV3.4", "include"));

        PublicDelayLoadDLLs.Add("libgcc_s_seh-1.dll");
        PublicDelayLoadDLLs.Add("libgfortran-3.dll");
        PublicDelayLoadDLLs.Add("libopenblas.dll");
        PublicDelayLoadDLLs.Add("libquadmath-0.dll");
        PublicDelayLoadDLLs.Add("opencv_ffmpeg342_64.dll");
        PublicDelayLoadDLLs.Add("opencv_img_hash342.dll");
        PublicDelayLoadDLLs.Add("opencv_world342.dll");
        PublicDelayLoadDLLs.Add("tbb.dll");


        RuntimeDependencies.Add(Path.Combine(ModulePath, "libgcc_s_seh-1.dll"));
        RuntimeDependencies.Add(Path.Combine(ModulePath, "libgfortran-3.dll"));
        RuntimeDependencies.Add(Path.Combine(ModulePath, "libopenblas.dll"));
        RuntimeDependencies.Add(Path.Combine(ModulePath, "libquadmath-0.dll"));
        RuntimeDependencies.Add(Path.Combine(ModulePath, "opencv_ffmpeg342_64.dll"));
        RuntimeDependencies.Add(Path.Combine(ModulePath, "opencv_img_hash342.dll"));
        RuntimeDependencies.Add(Path.Combine(ModulePath, "opencv_world342.dll"));
        RuntimeDependencies.Add(Path.Combine(ModulePath, "tbb.dll"));

    }
}

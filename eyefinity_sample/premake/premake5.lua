_AMD_SAMPLE_NAME = "EyefinitySample"

dofile ("../../premake/amd_premake_util.lua")

workspace (_AMD_SAMPLE_NAME)
   configurations { "Debug", "Release" }
   platforms { "x64" }
   location "../build"
   filename (_AMD_SAMPLE_NAME .. _AMD_VS_SUFFIX)
   startproject (_AMD_SAMPLE_NAME)
   systemversion "latest"
   system "Windows"
   architecture "x64"

externalproject "DXUT"
   kind "StaticLib"
   language "C++"
   location "../DXUT/Core"
   uuid "85344B7F-5AA0-4E12-A065-D1333D11F6CA"
   filename ("DXUT" .. _AMD_VS_SUFFIX)

externalproject "DXUTOpt"
   kind "StaticLib"
   language "C++"
   location "../DXUT/Optional"
   uuid "61B333C2-C4F7-4CC1-A9BF-83F6D95588EB"
   filename ("DXUTOpt" .. _AMD_VS_SUFFIX)

project (_AMD_SAMPLE_NAME)
   kind "WindowedApp"
   language "C++"
   characterset "Unicode"
   location "../build"
   filename (_AMD_SAMPLE_NAME .. _AMD_VS_SUFFIX)
   targetdir "../bin"
   objdir "../build/%{_AMD_SAMPLE_DIR_LAYOUT}"
   warnings "Extra"
   floatingpoint "Fast"
   symbols "On"
   entrypoint "wWinMain"

   -- Copy DLLs to the local bin directory
   postbuildcommands { amdAgsSamplePostbuildCommands(true) }
   postbuildmessage "Copying dependencies..."

   files { "../src/**.h", "../src/**.cpp", "../src/**.rc", "../src/**.manifest", "../src/**.hlsl", "../../%{_AMD_AGS_DIRECTORY_NAME}/inc/*.h" }
   includedirs { "../src/ResourceFiles", "../DXUT/Core", "../DXUT/Optional", "../../%{_AMD_AGS_DIRECTORY_NAME}/inc" }
   libdirs { "../../%{_AMD_AGS_DIRECTORY_NAME}/lib" }
   links { "amd_ags_x64", "DXUT", "DXUTOpt", "d3dcompiler", "winmm", "comctl32", "Usp10", "Shlwapi" }

   filter "files:../src/shaders/*.hlsl"
      flags {"ExcludeFromBuild"}

   filter "configurations:Debug"
      defines { "WIN32", "_DEBUG", "DEBUG", "PROFILE", "_WINDOWS", "_WIN32_WINNT=0x0601" }
      flags { "FatalWarnings" }
      targetsuffix ("_Debug" .. _AMD_VS_SUFFIX)

   filter "configurations:Release"
      defines { "WIN32", "NDEBUG", "PROFILE", "_WINDOWS", "_WIN32_WINNT=0x0601" }
      flags { "LinkTimeOptimization", "FatalWarnings" }
      targetsuffix ("_Release" .. _AMD_VS_SUFFIX)
      optimize "On"

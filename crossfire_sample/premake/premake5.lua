_AMD_SAMPLE_NAME = "CrossfireSample"

dofile ("../../premake/amd_premake_util.lua")

workspace (_AMD_SAMPLE_NAME)
   configurations { "Debug", "Release" }
   platforms { "x64" }
   location "../build"
   filename (_AMD_SAMPLE_NAME .. _AMD_VS_SUFFIX)
   startproject (_AMD_SAMPLE_NAME)

   filter "platforms:x64"
      system "Windows"
      architecture "x64"

project (_AMD_SAMPLE_NAME)
   kind "ConsoleApp"
   language "C++"
   location "../build"
   filename (_AMD_SAMPLE_NAME .. _AMD_VS_SUFFIX)
   uuid "4C0AC3AC-57E7-4DCD-8B36-7074CA208B3B"
   targetdir "../bin"
   objdir "../build/%{_AMD_SAMPLE_DIR_LAYOUT}"
   warnings "Extra"
   floatingpoint "Fast"

   -- Specify WindowsTargetPlatformVersion here for VS2015
   windowstarget (_AMD_WIN_SDK_VERSION)

   -- Copy DLLs to the local bin directory
   postbuildcommands { amdAgsSamplePostbuildCommands(true) }
   postbuildmessage "Copying dependencies..."

   files { "../src/**.h", "../src/**.cpp", "../src/**.hlsl", "../../%{_AMD_AGS_DIRECTORY_NAME}/inc/*.h" }
   includedirs { "../../%{_AMD_AGS_DIRECTORY_NAME}/inc" }
   libdirs { "../../%{_AMD_AGS_DIRECTORY_NAME}/lib" }
   links { "amd_ags_x64", "d3dcompiler", "d3d11" }

   filter "configurations:Debug"
      defines { "WIN32", "_DEBUG", "_CONSOLE", "_WIN32_WINNT=0x0601" }
      flags { "Symbols", "FatalWarnings", "Unicode" }
      targetsuffix ("_Debug" .. _AMD_VS_SUFFIX)

   filter "configurations:Release"
      defines { "WIN32", "NDEBUG", "_CONSOLE", "_WIN32_WINNT=0x0601" }
      flags { "LinkTimeOptimization", "Symbols", "FatalWarnings", "Unicode" }
      targetsuffix ("_Release" .. _AMD_VS_SUFFIX)
      optimize "On"

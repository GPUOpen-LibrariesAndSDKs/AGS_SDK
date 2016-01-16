_AMD_SAMPLE_NAME = "AGSSample"

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
   uuid "021ECEC9-6E88-8240-B7C6-33E623706095"
   targetdir "../bin"
   objdir "../build/%{_AMD_SAMPLE_DIR_LAYOUT}"
   warnings "Extra"
   floatingpoint "Fast"

   -- Specify WindowsTargetPlatformVersion here for VS2015
   windowstarget (_AMD_WIN_SDK_VERSION)

   -- Copy DLLs to the local bin directory
   postbuildcommands { amdAgsSamplePostbuildCommands(true) }
   postbuildmessage "Copying dependencies..."

   files { "../src/**.h", "../src/**.cpp", "../../%{_AMD_AGS_DIRECTORY_NAME}/inc/*.h" }
   includedirs { "../../%{_AMD_AGS_DIRECTORY_NAME}/inc" }
   libdirs { "../../%{_AMD_AGS_DIRECTORY_NAME}/lib" }
   links { "amd_ags_x64" }

   filter "configurations:Debug"
      defines { "WIN32", "_DEBUG", "_CONSOLE", "_WIN32_WINNT=0x0601" }
      flags { "Symbols", "FatalWarnings", "Unicode" }
      targetsuffix ("_Debug" .. _AMD_VS_SUFFIX)

   filter "configurations:Release"
      defines { "WIN32", "NDEBUG", "_CONSOLE", "_WIN32_WINNT=0x0601" }
      flags { "LinkTimeOptimization", "Symbols", "FatalWarnings", "Unicode" }
      targetsuffix ("_Release" .. _AMD_VS_SUFFIX)
      optimize "On"

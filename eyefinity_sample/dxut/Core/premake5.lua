dofile ("../../../premake/amd_premake_util.lua")

workspace "DXUT"
   configurations { "Debug", "Release" }
   platforms { "x64" }
   filename ("DXUT" .. _AMD_VS_SUFFIX)
   startproject "DXUT"
   systemversion "latest"
   system "Windows"
   architecture "x64"

project "DXUT"
   kind "StaticLib"
   language "C++"
   characterset "Unicode"
   filename ("DXUT" .. _AMD_VS_SUFFIX)
   uuid "85344B7F-5AA0-4e12-A065-D1333D11F6CA"
   targetdir "Bin/%{_AMD_SAMPLE_DIR_LAYOUT}"
   objdir "Bin/%{_AMD_SAMPLE_DIR_LAYOUT}"
   warnings "Extra"
   floatingpoint "Fast"
   symbols "On"
   pchheader "DXUT.h"
   pchsource "DXUT.cpp"

   files { "*.h", "*.cpp" }

   filter "configurations:Debug"
      defines { "WIN32", "_DEBUG", "DEBUG", "PROFILE", "_WINDOWS", "_LIB", "_WIN32_WINNT=0x0601" }
      flags { "FatalWarnings" }

   filter "configurations:Release"
      defines { "WIN32", "NDEBUG", "_WINDOWS", "_LIB", "_WIN32_WINNT=0x0601" }
      flags { "LinkTimeOptimization", "FatalWarnings" }
      optimize "On"

   filter "action:vs2017"
      -- VS2017 RC puts the log file in a different location than previous versions.
      -- Seems like a bug. Still present in VS2017 release version 15.1 (26403.3).
      -- The workaround is to explicitly specify the log file location.
      buildlog ("$(ProjectDir)Bin\\%{_AMD_SAMPLE_DIR_LAYOUT}\\$(MSBuildProjectName).log")

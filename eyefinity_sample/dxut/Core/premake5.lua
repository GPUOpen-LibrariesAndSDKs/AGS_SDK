dofile ("../../../premake/amd_premake_util.lua")

workspace "DXUT"
   configurations { "Debug", "Release" }
   platforms { "x64" }
   filename ("DXUT" .. _AMD_VS_SUFFIX)
   startproject "DXUT"

   filter "platforms:x64"
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

   -- Specify WindowsTargetPlatformVersion here for VS2015
   systemversion (_AMD_WIN_SDK_VERSION)

   files { "*.h", "*.cpp" }

   filter "configurations:Debug"
      defines { "WIN32", "_DEBUG", "DEBUG", "PROFILE", "_WINDOWS", "_LIB", "_WIN32_WINNT=0x0601" }
      flags { "FatalWarnings" }

   filter "configurations:Release"
      defines { "WIN32", "NDEBUG", "_WINDOWS", "_LIB", "_WIN32_WINNT=0x0601" }
      flags { "LinkTimeOptimization", "FatalWarnings" }
      optimize "On"

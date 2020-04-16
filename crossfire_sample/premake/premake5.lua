_AMD_SAMPLE_NAME = "CrossfireSample"

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

project (_AMD_SAMPLE_NAME)
   kind "ConsoleApp"
   language "C++"
   characterset "Unicode"
   location "../build"
   filename (_AMD_SAMPLE_NAME .. _AMD_VS_SUFFIX)
   uuid "4C0AC3AC-57E7-4DCD-8B36-7074CA208B3B"
   targetdir "../bin"
   objdir "../build/%{_AMD_SAMPLE_DIR_LAYOUT}"
   warnings "Extra"
   floatingpoint "Fast"
   symbols "On"

   -- Copy DLLs to the local bin directory
   postbuildcommands { amdAgsSamplePostbuildCommands(true) }
   postbuildmessage "Copying dependencies..."

   files { "../src/**.h", "../src/**.cpp", "../src/**.hlsl", "../../%{_AMD_AGS_DIRECTORY_NAME}/inc/*.h" }
   includedirs { "../../%{_AMD_AGS_DIRECTORY_NAME}/inc" }
   libdirs { "../../%{_AMD_AGS_DIRECTORY_NAME}/lib" }
   links { "amd_ags_x64", "d3dcompiler", "d3d11" }

   filter { "files:**.hlsl" }
      flags {"ExcludeFromBuild"}

   filter "configurations:Debug"
      defines { "WIN32", "_DEBUG", "_CONSOLE", "_WIN32_WINNT=0x0601" }
      flags { "FatalWarnings" }
      targetsuffix ("_Debug" .. _AMD_VS_SUFFIX)

   filter "configurations:Release"
      defines { "WIN32", "NDEBUG", "_CONSOLE", "_WIN32_WINNT=0x0601" }
      flags { "LinkTimeOptimization", "FatalWarnings" }
      targetsuffix ("_Release" .. _AMD_VS_SUFFIX)
      optimize "On"

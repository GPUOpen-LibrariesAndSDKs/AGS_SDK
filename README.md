# AMD AGS SDK
![AMD AGS SDK](http://gpuopen-librariesandsdks.github.io/media/amd_logo_black.png)

The AMD GPU Services (AGS) library provides software developers with the ability to query AMD GPU software and hardware state information that is not normally available through standard operating systems or graphic APIs. Version 4.0 of the library includes support for querying graphics driver version info, GPU performance, Crossfire&trade; (AMD's multi-GPU rendering technology) configuration info, and Eyefinity (AMD's multi-display rendering technology) configuration info. AGS also exposes the explicit Crossfire API extension, GCN shader extensions, and additional extensions supported in the AMD drivers for DirectX 11 and DirectX 12.

In addition to the library itself, the AGS SDK includes several samples to demonstrate use of the library.

<div>
  <a href="https://github.com/GPUOpen-LibrariesAndSDKs/AGS_SDK/releases/latest/"><img src="http://gpuopen-librariesandsdks.github.io/media/latest-release-button.svg" alt="Latest release" title="Latest release"></a>
</div>

### What's new in AGS
AGS now returns a lot more information from the GPU in addition to exposing the explicit Crossfire API and other extensions for DirectX 11. Version 4.0 also exposes GCN shader extensions for both DirectX 11 and DirectX 12. Highlights include the following:

* The initialization function can now return information about the GPU:
  * Whether the GPU is GCN or not
  * The adapter string and device id
  * The driver version is now rolled into this structure instead of a separate function call
  * Performance metrics such as the number of compute units and clock speeds
* New explicit Crossfire API
  * Provides the ability to control resource transfers between GPUs in Crossfire configuration in DirectX 11
  * Allows improved performance in multi-GPU configurations
* New GCN shader extension support
  * DirectX 11 and DirectX 12
  * Requires Radeon Software Crimson Edition 16.5.2 or later
* AGS provides access to additional extensions available in the AMD driver for DirectX 11:
  * Quad List primitive type
  * UAV overlap
  * Depth bounds test
  * Multi-draw indirect

### Prerequisites
* AMD Radeon&trade; GCN-based GPU (HD 7000 series or newer)
  * Or other DirectX&reg; 11 compatible GPU with Shader Model 5 support<sup>[1](#ags-sdk-footnote1)</sup> 
* 64-bit Windows&reg; 7 (SP1 with the [Platform Update](https://msdn.microsoft.com/en-us/library/windows/desktop/jj863687.aspx)), Windows&reg; 8.1, or Windows&reg; 10
* Visual Studio&reg; 2012, Visual Studio&reg; 2013, or Visual Studio&reg; 2015

### Getting Started
* It is recommended to take a look at the sample source code.
  * There are three samples: ags_sample, crossfire_sample, and eyefinity_sample.
* Visual Studio projects for VS2012, VS2013, and VS2015 can be found in each sample's `build` directory.
* Additional documentation, including API documentation and instructions on how to add AGS support to an existing project, can be found in the `ags_lib\doc` directory.

### Additional Samples
In addition to the three samples included in this repo, there are other samples available on GitHub that use AGS:
* [CrossfireAPI11](https://github.com/GPUOpen-LibrariesAndSDKs/CrossfireAPI11) - a larger example of using the explicit Crossfire API
* [DepthBoundsTest11](https://github.com/GPUOpen-LibrariesAndSDKs/DepthBoundsTest11) - a sample showing how to use the depth bounds test extension
* [Barycentrics12](https://github.com/GPUOpen-LibrariesAndSDKs/Barycentrics12) - a sample showing how to use the GCN shader extensions for DirectX 12

### Premake
The Visual Studio projects in each sample's `build` directory were generated with Premake. To generate the project files yourself (for another version of Visual Studio, for example), open a command prompt in the sample's `premake` directory (where the premake5.lua script for that sample is located, not the top-level directory where the premake5 executable is located) and execute the following command:

* `..\..\premake\premake5.exe [action]`
* For example: `..\..\premake\premake5.exe vs2010`

Alternatively, to regenerate all Visual Studio files for the SDK, execute `ags_update_vs_files.bat` in the top-level `premake` directory.

This version of Premake has been modified from the stock version to use the property sheet technique for the Windows SDK from this [Visual C++ Team blog post](http://blogs.msdn.com/b/vcblog/archive/2012/11/23/using-the-windows-8-sdk-with-visual-studio-2010-configuring-multiple-projects.aspx). The technique was originally described for using the Windows 8.0 SDK with Visual Studio 2010, but it applies more generally to using newer versions of the Windows SDK with older versions of Visual Studio.

The default SDK for a particular version of Visual Studio (for 2012 or higher) is installed as part of Visual Studio installation. This default (Windows 8.0 SDK for Visual Studio 2012 and Windows 8.1 SDK for Visual Studio 2013) will be used if newer SDKs do not exist on the user's machine. However, the projects generated with this version of Premake will use the next higher SDK (Windows 8.1 SDK for Visual Studio 2012 and Windows 10 SDK with Visual Studio 2013), if the newer SDKs exist on the user's machine.

For Visual Studio 2015, this version of Premake adds the `WindowsTargetPlatformVersion` element to the project file to specify which version of the Windows SDK will be used. To change `WindowsTargetPlatformVersion` for Visual Studio 2015, change the value for `_AMD_WIN_SDK_VERSION` in `premake\amd_premake_util.lua` and regenerate the Visual Studio files.

### Third-Party Software
* DXUT is distributed under the terms of the MIT License. See `eyefinity_sample\dxut\MIT.txt`.
* Premake is distributed under the terms of the BSD License. See `premake\LICENSE.txt`.

### Attribution
* AMD, the AMD Arrow logo, Radeon, Crossfire, and combinations thereof are either registered trademarks or trademarks of Advanced Micro Devices, Inc. in the United States and/or other countries.
* Microsoft, DirectX, Visual Studio, and Windows are either registered trademarks or trademarks of Microsoft Corporation in the United States and/or other countries.

### Notes
<a name="ags-sdk-footnote1">1</a>: While the AGS SDK samples will run on non-AMD hardware, they will be of limited usefulness, since the purpose of AGS is to provide convenient access to AMD-specific information and extensions.

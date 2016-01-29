//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

//-----------------------------------------------------------------------------
// File: AGSSample.cpp
//-----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <amd_ags.h>


#pragma warning( disable : 4100 ) // disable unreference formal parameter warnings for /W4 builds

void GetEyefinityInfo( AGSContext* context, int primaryDisplayIndex )
{
    int numDisplaysInfo = 0;

    // Query the number of displays first
    if ( agsGetEyefinityConfigInfo( context, primaryDisplayIndex, nullptr, &numDisplaysInfo, nullptr ) == AGS_SUCCESS && numDisplaysInfo > 0 )
    {
        AGSEyefinityInfo eyefinityInfo = {};
        AGSDisplayInfo* displaysInfo = new AGSDisplayInfo[ numDisplaysInfo ];
        ZeroMemory( displaysInfo, numDisplaysInfo * sizeof( *displaysInfo ) );

        // Find out if this display has an Eyefinity config enabled
        if ( agsGetEyefinityConfigInfo( context, primaryDisplayIndex, &eyefinityInfo, &numDisplaysInfo, displaysInfo ) == AGS_SUCCESS )
        {
            if ( eyefinityInfo.iSLSActive )
            {
                printf( "Eyefinity enabled for display index %d:\n", primaryDisplayIndex );
                printf( "  SLS grid is %d displays wide by %d displays tall\n", eyefinityInfo.iSLSGridWidth, eyefinityInfo.iSLSGridHeight );
                printf( "  SLS resolution is %d x %d pixels\n", eyefinityInfo.iSLSWidth, eyefinityInfo.iSLSHeight );

                if ( eyefinityInfo.iBezelCompensatedDisplay )
                {
                    printf( " SLS is bezel-compensated\n" );
                }

                for ( int i = 0; i < numDisplaysInfo; i++ )
                {
                    printf( "Display %d\n", i );

                    if ( displaysInfo[ i ].iPreferredDisplay )
                    {
                        printf( " Preferred/main monitor\n" );
                    }

                    printf( " SLS grid coord [%d,%d]\n", displaysInfo[i].iGridXCoord, displaysInfo[i].iGridYCoord );
                    printf( " Base coord [%d,%d]\n", displaysInfo[i].displayRect.iXOffset, displaysInfo[i].displayRect.iYOffset );
                    printf( " Dimensions [%d x %d]\n", displaysInfo[i].displayRect.iWidth, displaysInfo[i].displayRect.iHeight );
                    printf( " Visible base coord [%d,%d]\n", displaysInfo[i].displayRectVisible.iXOffset, displaysInfo[i].displayRectVisible.iYOffset );
                    printf( " Visible dimensions [%d x %d]\n", displaysInfo[i].displayRectVisible.iWidth, displaysInfo[i].displayRectVisible.iHeight );
                }
            }
            else
            {
                printf( "Eyefinity not enabled for display index %d\n", primaryDisplayIndex );
            }
        }
        else
        {
            printf( "Eyefinity configuration query failed to get display info for display index %d\n", primaryDisplayIndex );
        }

        delete[] displaysInfo;
    }
    else
    {
        printf( "Eyefinity configuration query failed to get number of displays for display index %d\n", primaryDisplayIndex );
    }
}


int main(int argc, char* argv[])
{
    // Enable run-time memory check for debug builds.
    // (When _DEBUG is not defined, calls to _CrtSetDbgFlag are removed during preprocessing.)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

    AGSContext* agsContext = nullptr;

    int displayIndex = 0;
    int primaryDisplayIndex = 0;
    DISPLAY_DEVICEA displayDevice;
    displayDevice.cb = sizeof( displayDevice );
    while ( EnumDisplayDevicesA( 0, displayIndex, &displayDevice, 0 ) )
    {
        if ( displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE )
        {
            primaryDisplayIndex = displayIndex;
        }

        //printf( "Display Device: %d %s %s\n", displayIndex, displayDevice.DeviceString, displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE ? "(primary)" : "" );

        displayIndex++;
    }

    AGSGPUInfo info;

    if ( agsInit( &agsContext, &info ) == AGS_SUCCESS )
    {
        printf( "AGS Library initialized: v%d.%d.%d\n\n", AMD_AGS_LIB_VERSION_MAJOR, AMD_AGS_LIB_VERSION_MINOR, AMD_AGS_LIB_VERSION_PATCH );
        printf( "-----------------------------------------------------------------\n" );

        printf( "%s, device id: 0x%04X, revision id: 0x%02X\n", info.adapterString ? info.adapterString : "unknown GPU", info.deviceId, info.revisionId );
        printf( "Driver version:            %s\n", info.driverVersion );
        printf( "-----------------------------------------------------------------\n" );

        printf( "Is %sGCN, %d CUs, core clock %d MHz, memory clock %d MHz, %.1f Tflops\n", info.version == AGSGPUInfo::ArchitectureVersion_GCN ? "" : "not ", info.iNumCUs, info.iCoreClock, info.iMemoryClock, info.fTFlops );

        int totalGPUs = 0;
        if ( agsGetTotalGPUCount( agsContext, &totalGPUs ) == AGS_SUCCESS )
        {
            printf( "Total system GPU count = %d\n", totalGPUs );

            for ( int i = 0; i < totalGPUs; i++ )
            {
                long long memory = 0;
                if ( agsGetGPUMemorySize( agsContext, i, &memory ) == AGS_SUCCESS )
                {
                    printf( "GPU %d Memory: %dMBs\n", i, (int)(memory / (1024*1024)) );
                }
                else
                {
                    printf( "Failed to get memory size from GPU %d\n", i );
                }
            }

            printf( "-----------------------------------------------------------------\n" );
        }
        else
        {
            printf( "Failed to get total GPU count\n" );
        }

        int numCrossfireGPUs = 0;
        if ( agsGetCrossfireGPUCount( agsContext, &numCrossfireGPUs ) == AGS_SUCCESS )
        {
            printf( "Crossfire GPU count = %d\n", numCrossfireGPUs );
            printf( "-----------------------------------------------------------------\n" );
        }
        else
        {
            printf( "Failed to get Crossfire GPU count\n" );
        }

        GetEyefinityInfo( agsContext, primaryDisplayIndex );
        printf( "-----------------------------------------------------------------\n" );

        if ( agsDeInit( agsContext ) != AGS_SUCCESS )
        {
            printf( "Failed to cleanup AGS Library\n" );
        }
    }
    else
    {
        printf( "Failed to initialize AGS Library\n" );
    }

    printf( "\ndone\n" );

    _getch();

    return 0;
}


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

//--------------------------------------------------------------------------------------
// File: EyefinitySample.cpp
//--------------------------------------------------------------------------------------

#include "DXUT.h"
#include "DXUTCamera.h"
#include "DXUTGui.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "resource.h"
#include "amd_ags.h"


#pragma warning( disable : 4100 ) // disable unreference formal parameter warnings for /W4 builds


//--------------------------------------------------------------------------------------
// Modified D3DSettingsDlg which will move the dialog to the main display instead of  
// showing the whole screen dialog.
//--------------------------------------------------------------------------------------
class CEF_D3DSettingsDlg: public CD3DSettingsDlg
{
public:
	HRESULT             OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );
};
//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CFirstPersonCamera                  g_Camera;					// A model viewing camera
CDXUTDialogResourceManager          g_DialogResourceManager;	// manager for shared resources of dialogs
CEF_D3DSettingsDlg                  g_D3DSettingsDlg;			// modified device settings dialog which will always shows on the main display
CDXUTDialog                         g_HUD;						// manages the 3D UI
CDXUTDialog                         g_SampleUI;					// dialog for sample specific controls
CDXUTTextHelper*					g_pTxtHelper = NULL;
UINT                                g_iWidth;
UINT                                g_iHeight;

#define NUM_MICROSCOPE_INSTANCES 6

CDXUTSDKMesh                        g_CityMesh;
CDXUTSDKMesh                        g_HeavyMesh;
CDXUTSDKMesh                        g_ColumnMesh;

ID3D11Buffer*						g_pConstantBuffer = NULL;
ID3D11InputLayout*                  g_pVertexLayout = NULL;
ID3D11SamplerState*					g_pSampleLinear = NULL;
// Scene Shaders
ID3D11VertexShader*					g_pSceneVS = NULL;
ID3D11PixelShader*					g_pScenePS = NULL;

bool								g_bMultiCameraMode = false;
AGSContext*                         g_AGSContext = nullptr;
AGSGPUInfo                          g_AGSGPUInfo = {};
bool                                g_EyefinityEnabled = false; // If Eyefinity group has been set and we are in fullscreen
RECT								g_MainDisplayRect;
DirectX::XMMATRIX					g_SingleCameraProjM;
DirectX::XMMATRIX                   g_MultiCameraProjM;
float								g_FovX = 0.0f;

struct LineVertex { float x, y; };
ID3D11InputLayout*                  g_LinesInputLayout = nullptr;
ID3D11VertexShader*                 g_LinesVertexShader = nullptr;
ID3D11PixelShader*                  g_LinesPixelShader = nullptr;
ID3D11Buffer*                       g_LinesVertexBuffer = nullptr;
ID3D11Buffer*                       g_LinesIndexBuffer = nullptr;

//--------------------------------------------------------------------------------------
// Retrieve the main display info and place the dialog on main display 
// whenever the resolution is changed.
//--------------------------------------------------------------------------------------
HRESULT CEF_D3DSettingsDlg::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
    m_Dialog.SetLocation( g_MainDisplayRect.left, g_MainDisplayRect.top );
    m_Dialog.SetSize( g_MainDisplayRect.right-g_MainDisplayRect.left, g_MainDisplayRect.bottom-g_MainDisplayRect.top );	
    m_Dialog.SetBackgroundColors( D3DCOLOR_ARGB( 255, 98, 138, 206 ),
                                  D3DCOLOR_ARGB( 255, 54, 105, 192 ),
                                  D3DCOLOR_ARGB( 255, 54, 105, 192 ),
                                  D3DCOLOR_ARGB( 255, 10,  73, 179 ) );

    m_RevertModeDialog.SetLocation( g_MainDisplayRect.left, g_MainDisplayRect.top );
    m_RevertModeDialog.SetSize( g_MainDisplayRect.right-g_MainDisplayRect.left, g_MainDisplayRect.bottom-g_MainDisplayRect.top );
    m_RevertModeDialog.SetBackgroundColors( D3DCOLOR_ARGB( 255, 98, 138, 206 ),
                                            D3DCOLOR_ARGB( 255, 54, 105, 192 ),
                                            D3DCOLOR_ARGB( 255, 54, 105, 192 ),
                                            D3DCOLOR_ARGB( 255, 10,  73, 179 ) );

    return S_OK;
}
//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_USEMULTICAMERA      5
//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

// Direct3D 11 callbacks
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnD3D11SwapChainResized( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );

void InitApp();
void RenderText();

//--------------------------------------------------------------------------------------
// Helper function to compile an hlsl shader from file, 
// its binary compiled code is returned
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, 
							   LPCSTR szShaderModel, ID3DBlob** ppBlobOut, D3D10_SHADER_MACRO* pDefines )
{
    HRESULT hr = S_OK;

    // find the file
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, szFileName ) );

    // open the file
    HANDLE hFile = CreateFile( str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, NULL );
    if( INVALID_HANDLE_VALUE == hFile )
        return E_FAIL;

    // Get the file size
    LARGE_INTEGER FileSize;
    GetFileSizeEx( hFile, &FileSize );

    // create enough space for the file data
    BYTE* pFileData = new BYTE[ FileSize.LowPart ];
    if( !pFileData )
        return E_OUTOFMEMORY;

    // read the data in
    DWORD BytesRead;
    if( !ReadFile( hFile, pFileData, FileSize.LowPart, &BytesRead, NULL ) )
        return E_FAIL; 

    CloseHandle( hFile );

    // Compile the shader
    char pFilePathName[MAX_PATH];        
    WideCharToMultiByte(CP_ACP, 0, str, -1, pFilePathName, MAX_PATH, NULL, NULL);
    ID3DBlob* pErrorBlob;
    hr = D3DCompile( pFileData, FileSize.LowPart, pFilePathName, pDefines, NULL, szEntryPoint, szShaderModel, D3D10_SHADER_ENABLE_STRICTNESS, 0, ppBlobOut, &pErrorBlob );

    delete []pFileData;

    if( FAILED(hr) )
    {
        OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        SAFE_RELEASE( pErrorBlob );
        return hr;
    }
    SAFE_RELEASE( pErrorBlob );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
   _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Initialise AGS lib
    agsInitialize( AGS_MAKE_VERSION( AMD_AGS_VERSION_MAJOR, AMD_AGS_VERSION_MINOR, AMD_AGS_VERSION_PATCH ), nullptr, &g_AGSContext, &g_AGSGPUInfo );

    // DXUT will create and use the best device 
    // that is available on the system depending on which D3D callbacks are set below
	DXUTSetIsInGammaCorrectMode( false );

    // Set DXUT callbacks
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameMove( OnFrameMove );

    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11SwapChainResized );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );

    InitApp();
    DXUTInit( true, true ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Eyefinity Sample v1.1" );

    // Find out if this display has an Eyefinity config enabled.
    int width = 768;
    int height = 480;
    bool windowed = true;

    if ( g_AGSGPUInfo.numDevices > 0 )
    {
        const AGSDeviceInfo& device = g_AGSGPUInfo.devices[ 0 ];
        if ( device.eyefinityEnabled )
        {
            // Use full screen mode if Eyefinity is enabled.
            width = device.eyefinityResolutionX;
            height = device.eyefinityResolutionY;
            windowed = false;
        }
    }

    DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, windowed, width, height );

    DXUTMainLoop(); // Enter into the DXUT render loop	

    // Clean up AGS lib
    agsDeInitialize( g_AGSContext );

    return DXUTGetExitCode();
}
//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    g_D3DSettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );
	g_SampleUI.GetFont( 0 );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 5, iY, 150, 22 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 5, iY += 24, 150, 22, VK_F2 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 5, iY += 24, 150, 22, VK_F3 );

    iY += 40;
    g_HUD.AddCheckBox( IDC_USEMULTICAMERA, L"Multi-Camera Mode", 5, iY += 24, 150, 22, g_bMultiCameraMode, VK_F5 );	
	
    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
}
//--------------------------------------------------------------------------------------
// Called right before creating a D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    pDeviceSettings->d3d11.SyncInterval = 0;
    g_D3DSettingsDlg.GetDialogControl()->GetComboBox( DXUTSETTINGSDLG_D3D11_PRESENT_INTERVAL )->SetEnabled( false );

    return true;
}
//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );
}
//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}
//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}
//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); 
			break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); 
			break;
        case IDC_CHANGEDEVICE:
            g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() ); 
			break;
   		case IDC_USEMULTICAMERA:
            g_bMultiCameraMode = !g_bMultiCameraMode; 
			break;
    }
}
//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}
//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
    HRESULT hr;

	 // Get device context
    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();

    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );

	 g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );

	ID3DBlob* pBlob = NULL;

	// Main scene VS
	V_RETURN( CompileShaderFromFile( L"..\\src\\Shaders\\EyefinitySample.hlsl", "VSScenemain", "vs_5_0", &pBlob, NULL ) ); 
	V_RETURN( pd3dDevice->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &g_pSceneVS ) );
	// Define our scene vertex data layout
	const D3D11_INPUT_ELEMENT_DESC SceneLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	V_RETURN( pd3dDevice->CreateInputLayout( SceneLayout, ARRAYSIZE( SceneLayout ), pBlob->GetBufferPointer(),
												pBlob->GetBufferSize(), &g_pVertexLayout ) );
	SAFE_RELEASE( pBlob );

	// Main scene PS
	V_RETURN( CompileShaderFromFile( L"..\\src\\Shaders\\EyefinitySample.hlsl", "PSScenemain", "ps_5_0", &pBlob, NULL ) ); 
    V_RETURN( pd3dDevice->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &g_pScenePS ) );
	SAFE_RELEASE( pBlob );

	// Setup constant buffers
    D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;    
    Desc.ByteWidth = sizeof( DirectX::XMMATRIX );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pConstantBuffer ) );

	// Create sampler states 
	D3D11_SAMPLER_DESC SamDesc;
    SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.MipLODBias = 0.0f;
    SamDesc.MaxAnisotropy = 1;
    SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
    SamDesc.MinLOD = 0;
    SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
    V_RETURN( pd3dDevice->CreateSamplerState( &SamDesc, &g_pSampleLinear ) )

    // Load the Meshes
    g_CityMesh.Create( pd3dDevice, L"media\\MicroscopeCity\\occcity.sdkmesh", false );
    g_HeavyMesh.Create( pd3dDevice, L"media\\MicroscopeCity\\scanner.sdkmesh", false );
    g_ColumnMesh.Create( pd3dDevice, L"media\\MicroscopeCity\\column.sdkmesh", false );
    
    // Setup the camera's view parameters
    DirectX::XMVECTOR vecEye = DirectX::XMVectorSet( 0.0f, 0.5f, -1.3f, 0.0f );
    DirectX::XMVECTOR vecAt = DirectX::XMVectorSet( 0.0f, 0.5f,  0.0f, 0.0f );
    g_Camera.SetViewParams( vecEye, vecAt );
	g_Camera.SetRotateButtons(true, false, false);
	g_Camera.SetEnableYAxisMovement( false );

    // Create the line rendering resources
    hr = CompileShaderFromFile( L"..\\src\\Shaders\\Line.hlsl", "LineVS", "vs_4_0", &pBlob, 0 );
    assert( hr == S_OK );
    hr = pd3dDevice->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), 0, &g_LinesVertexShader );
    assert( hr == S_OK );

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &g_LinesInputLayout );
    assert( hr == S_OK );
    SAFE_RELEASE( pBlob );

    hr = CompileShaderFromFile( L"..\\src\\Shaders\\Line.hlsl", "LinePS", "ps_4_0", &pBlob, 0 );
    assert( hr == S_OK );
    hr = pd3dDevice->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), 0, &g_LinesPixelShader );
    assert( hr == S_OK );

    // Create lines vertex buffer
    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof( desc ) );
    desc.ByteWidth = 12 * sizeof( LineVertex );
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pd3dDevice->CreateBuffer( &desc, 0, &g_LinesVertexBuffer );

    const unsigned short indices[] = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 8, 9, 9, 10, 10, 11, 11, 8 };

    desc.ByteWidth = 24 * sizeof( unsigned short );
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = indices;
    data.SysMemPitch = sizeof( indices );
    pd3dDevice->CreateBuffer( &desc, &data, &g_LinesIndexBuffer );

    return S_OK;
}
//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11SwapChainResized( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr = S_OK;   

    g_iWidth = pBackBufferSurfaceDesc->Width;
    g_iHeight = pBackBufferSurfaceDesc->Height;

    const AGSDeviceInfo& device = g_AGSGPUInfo.devices[ 0 ];
    g_EyefinityEnabled = false;
    bool isPortraitEyefinity = false;

    if ( device.eyefinityEnabled )
    {
        BOOL bFullScreen;
        pSwapChain->GetFullscreenState( &bFullScreen, NULL );

        if ( bFullScreen )
        {
            g_EyefinityEnabled = true;
            for ( int i = 0; i< device.numDisplays; i++ )
            {
                if ( device.displays[ i ].eyefinityPreferredDisplay )
                {
                    // Get the view rect of visibleResolution instead of currentResolution so the UI wouldn't be occluded by the bezels.
                    g_MainDisplayRect.left = device.displays[ i ].visibleResolution.offsetX;
                    g_MainDisplayRect.right = device.displays[ i ].visibleResolution.offsetX + device.displays[ i ].visibleResolution.width;
                    g_MainDisplayRect.top = device.displays[ i ].visibleResolution.offsetY;
                    g_MainDisplayRect.bottom = device.displays[ i ].visibleResolution.offsetY + device.displays[ i ].visibleResolution.height;
                }

                isPortraitEyefinity |= device.displays[ i ].eyefinityInPortraitMode;
            }

            if ( device.eyefinityGridWidth == 3 && device.eyefinityGridHeight == 1 )
            {
                D3D11_MAPPED_SUBRESOURCE res;
                DXUTGetD3D11DeviceContext()->Map( g_LinesVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res );
                LineVertex* verts = (LineVertex*)res.pData;
                for ( int i = 0; i< device.numDisplays; i++ )
                {
                    if ( device.displays[ i ].eyefinityInGroup )
                    {
                        const float offset = 2.0f;

                        float x0 = device.displays[ i ].visibleResolution.offsetX + offset;
                        float x1 = device.displays[ i ].visibleResolution.offsetX + device.displays[ i ].visibleResolution.width - 2 * offset;

                        float y0 = device.displays[ i ].visibleResolution.offsetY + offset;
                        float y1 = device.displays[ i ].visibleResolution.offsetY + device.displays[ i ].visibleResolution.height - 2 * offset;

                        x0 = 2.0f * ( x0 / pBackBufferSurfaceDesc->Width ) - 1.0f;
                        x1 = 2.0f * ( x1 / pBackBufferSurfaceDesc->Width ) - 1.0f;

                        y0 = 2.0f * ( y0 / pBackBufferSurfaceDesc->Height ) - 1.0f;
                        y1 = 2.0f * ( y1 / pBackBufferSurfaceDesc->Height ) - 1.0f;

                        verts[ 0 ].x = x0;
                        verts[ 0 ].y = y0;

                        verts[ 1 ].x = x1;
                        verts[ 1 ].y = y0;

                        verts[ 2 ].x = x1;
                        verts[ 2 ].y = y1;

                        verts[ 3 ].x = x0;
                        verts[ 3 ].y = y1;

                        verts += 4;
                    }
                }

                DXUTGetD3D11DeviceContext()->Unmap( g_LinesVertexBuffer, 0 );
            }
        }
    }

    if ( !g_EyefinityEnabled )
    {
        g_MainDisplayRect.left	 = 0;
		g_MainDisplayRect.right  = g_iWidth;
		g_MainDisplayRect.top	 = 0;
		g_MainDisplayRect.bottom = g_iHeight;
    }
	
	V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

	// Using fixed vertical fov and adjust the horizontal fov according to the screen aspect ratio.
	float YFOV = DirectX::XM_PI/4.0f;
    if ( isPortraitEyefinity )
    {
        YFOV *= (float)pBackBufferSurfaceDesc->Width / (float)pBackBufferSurfaceDesc->Height;
    }

	float fAspectRatio;
	if ( g_EyefinityEnabled )
	{
        fAspectRatio = (float)(device.eyefinityResolutionX / device.eyefinityGridWidth ) / (float)( device.eyefinityResolutionY / device.eyefinityGridHeight );
        g_MultiCameraProjM = DirectX::XMMatrixPerspectiveFovLH( YFOV, fAspectRatio, 0.01f, 500.0f );
		
		float xScale = (cosf(YFOV*0.5f)/sinf(YFOV*0.5f)) / fAspectRatio;
		g_FovX = 2.0f * atanf( 1.0f / xScale ); 
    }

	fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	// The projection matrix for the camera of single camera mode.
    g_SingleCameraProjM = DirectX::XMMatrixPerspectiveFovLH( YFOV, fAspectRatio, 0.01f, 500.0f );
    
	// Locate the HUD and UI based on the area of main display.
	g_HUD.SetLocation( g_MainDisplayRect.right - 170, g_MainDisplayRect.top );
	g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( g_MainDisplayRect.right - 170, g_MainDisplayRect.top + 300 );
    g_SampleUI.SetSize( 170, 170 );

    // Only enable multi camera mode in 3x1 Eyefinity mode (non bezel compensated)
    g_HUD.GetCheckBox( IDC_USEMULTICAMERA )->SetEnabled( g_EyefinityEnabled && !device.eyefinityBezelCompensated && device.eyefinityGridWidth == 3 && device.eyefinityGridHeight == 1 );

    return hr;
}
//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK RenderScene( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, DirectX::XMMATRIX& vpm )
{
    DirectX::XMMATRIX mWorldViewProj = vpm;
    
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dImmediateContext->Map( g_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
    DirectX::XMMATRIX* pWVPM = ( DirectX::XMMATRIX* )MappedResource.pData;
	*pWVPM = DirectX::XMMatrixTranspose( mWorldViewProj );
	pd3dImmediateContext->Unmap( g_pConstantBuffer, 0 );
   
    pd3dImmediateContext->IASetInputLayout( g_pVertexLayout );
	pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSampleLinear );
    pd3dImmediateContext->VSSetShader( g_pSceneVS, NULL, 0 );
	pd3dImmediateContext->PSSetShader( g_pScenePS, NULL, 0 );
  
    // Render the city
    g_CityMesh.Render( pd3dImmediateContext, 0 );
    g_ColumnMesh.Render( pd3dImmediateContext, 0 );
	for( int i = 0; i < NUM_MICROSCOPE_INSTANCES; i++ )
    {
        DirectX::XMMATRIX mMatRot = DirectX::XMMatrixRotationY( i * ( DirectX::XM_PI / 3.0f ) );
        DirectX::XMMATRIX mWVP = mMatRot * mWorldViewProj;
		pd3dImmediateContext->Map( g_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
		DirectX::XMMATRIX* pWVPMInstance = ( DirectX::XMMATRIX* )MappedResource.pData;
		*pWVPMInstance = DirectX::XMMatrixTranspose( mWVP );
		pd3dImmediateContext->Unmap( g_pConstantBuffer, 0 );
        g_HeavyMesh.Render( pd3dImmediateContext, 0 );
    }  
}
//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext )
{
    // Clear the render target
    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    float ClearColor[4] = { 0.369f, 0.369f, 0.369f, 0.0f };
    pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.OnRender( fElapsedTime );
        return;
    }

	D3D11_VIEWPORT Viewport;
	DirectX::XMMATRIX ViewM, VPM;

    if ( g_EyefinityEnabled )
	{
        const AGSDeviceInfo& device = g_AGSGPUInfo.devices[ 0 ];
	
		if (g_bMultiCameraMode && device.eyefinityGridWidth == 3 && device.eyefinityGridHeight == 1)
		{
			CModelViewerCamera Camera;

			DirectX::XMVECTOR oldEye = g_Camera.GetEyePt();
			DirectX::XMVECTOR oldAt = g_Camera.GetLookAtPt();
			DirectX::XMVECTOR ViewDir = g_Camera.GetWorldAhead();

			DirectX::XMMATRIX RM;

			for (int i=0; i<device.numDisplays; i++)
			{
                const AGSDisplayInfo& display = device.displays[ i ];

                if ( display.eyefinityInGroup )
                {
                    Viewport.TopLeftX	= (FLOAT)display.currentResolution.offsetX;
				    Viewport.TopLeftY	= (FLOAT)display.currentResolution.offsetY;
				    Viewport.Width		= (FLOAT)display.currentResolution.width;
				    Viewport.Height		= (FLOAT)display.currentResolution.height;
                    Viewport.MinDepth	= 0.0f;
				    Viewport.MaxDepth	= 1.0f;
				    pd3dImmediateContext->RSSetViewports(1, &Viewport);

				    DirectX::XMVECTOR TViewDir;
				    DirectX::XMVECTOR AT;
				    switch (display.eyefinityGridCoordX)
				    {
				    case 0:
					    RM = DirectX::XMMatrixRotationAxis( g_Camera.GetWorldUp(), -g_FovX );
					    TViewDir = DirectX::XMVector3TransformNormal( ViewDir, RM );
					    AT = DirectX::XMVectorAdd( oldEye, TViewDir );
					    ViewM = DirectX::XMMatrixLookAtLH( oldEye, AT, g_Camera.GetWorldUp());
					    VPM = ViewM * g_MultiCameraProjM;
					    break;

				    case 1:
					    ViewM = DirectX::XMMatrixLookAtLH( oldEye, oldAt, g_Camera.GetWorldUp());
					    VPM = ViewM * g_MultiCameraProjM;
					    break;

				    case 2:
					    RM = DirectX::XMMatrixRotationAxis( g_Camera.GetWorldUp(), g_FovX );
					    TViewDir = DirectX::XMVector3TransformNormal( ViewDir, RM );
					    AT = DirectX::XMVectorAdd( oldEye, TViewDir );
					    ViewM = DirectX::XMMatrixLookAtLH( oldEye, AT, g_Camera.GetWorldUp());
					    VPM = ViewM * g_MultiCameraProjM;
					    break;
				    }
				    RenderScene(pd3dDevice, pd3dImmediateContext, VPM);
			    }
            }
			Viewport.TopLeftX	= 0.0f;
			Viewport.TopLeftY	= 0.0f;
            Viewport.Width		= (FLOAT)device.eyefinityResolutionX;
			Viewport.Height		= (FLOAT)device.eyefinityResolutionY;
            Viewport.MinDepth	= 0.0f;
			Viewport.MaxDepth	= 1.0f;
			pd3dImmediateContext->RSSetViewports(1, &Viewport);
		}
		else  //Single camera mode
		{
			Viewport.TopLeftX	= 0.0f;
			Viewport.TopLeftY	= 0.0f;
            Viewport.Width		= (FLOAT)device.eyefinityResolutionY;
			Viewport.Height		= (FLOAT)device.eyefinityResolutionX;
            Viewport.Width		= (FLOAT)device.eyefinityResolutionX;
			Viewport.Height		= (FLOAT)device.eyefinityResolutionY;
            Viewport.MinDepth	= 0.0f;
			Viewport.MaxDepth	= 1.0f;
			pd3dImmediateContext->RSSetViewports(1, &Viewport);
			VPM = g_Camera.GetViewMatrix() * g_SingleCameraProjM;
			RenderScene(pd3dDevice, pd3dImmediateContext, VPM);
		}
	}
	else
	{
		VPM = g_Camera.GetViewMatrix() * g_SingleCameraProjM;
		RenderScene(pd3dDevice, pd3dImmediateContext, VPM);
	}

    if ( g_EyefinityEnabled )
    {
        const AGSDeviceInfo& device = g_AGSGPUInfo.devices[ 0 ];
        if ( device.eyefinityGridWidth == 3 && device.eyefinityGridHeight == 1 )
        {
            Viewport.TopLeftX = 0.0f;
            Viewport.TopLeftY = 0.0f;
            Viewport.Width		= (FLOAT)device.eyefinityResolutionX;
			Viewport.Height		= (FLOAT)device.eyefinityResolutionY;
            Viewport.MinDepth = 0.0f;
            Viewport.MaxDepth = 1.0f;
            pd3dImmediateContext->RSSetViewports( 1, &Viewport );

            pd3dImmediateContext->IASetInputLayout( g_LinesInputLayout );
            pd3dImmediateContext->VSSetShader( g_LinesVertexShader, 0, 0 );
            pd3dImmediateContext->PSSetShader( g_LinesPixelShader, 0, 0 );

            UINT stride = sizeof( LineVertex );
            UINT offset = 0;
            pd3dImmediateContext->IASetVertexBuffers( 0, 1, &g_LinesVertexBuffer, &stride, &offset );
            pd3dImmediateContext->IASetIndexBuffer( g_LinesIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
            pd3dImmediateContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_LINELIST );
            pd3dImmediateContext->DrawIndexed( 8 * 3, 0, 0 );
        }
    }

    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    RenderText();
    g_SampleUI.OnRender( fElapsedTime );
    g_HUD.OnRender( fElapsedTime );
    DXUT_EndPerfEvent();
}
//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
	g_pTxtHelper->Begin();
	g_pTxtHelper->SetInsertionPos( g_MainDisplayRect.left, g_MainDisplayRect.top );
    g_pTxtHelper->SetForegroundColor( DirectX::XMVectorSet( 1.0f, 1.0f, 0.0f, 1.0f ) );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );	
	g_pTxtHelper->DrawTextLine( g_EyefinityEnabled ? L"Eyefinity : ON" : L"Eyefinity : OFF" );
	g_pTxtHelper->End();
}
//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}
//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_D3DSettingsDlg.OnD3D11DestroyDevice();
    CDXUTDirectionWidget::StaticOnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
	
	SAFE_DELETE( g_pTxtHelper );

    SAFE_RELEASE( g_LinesVertexBuffer );
    SAFE_RELEASE( g_LinesIndexBuffer );
    SAFE_RELEASE( g_LinesPixelShader );
    SAFE_RELEASE( g_LinesVertexShader );
    SAFE_RELEASE( g_LinesInputLayout );

    SAFE_RELEASE( g_pVertexLayout );
	SAFE_RELEASE( g_pConstantBuffer );
	SAFE_RELEASE( g_pSceneVS );
	SAFE_RELEASE( g_pScenePS );
	SAFE_RELEASE( g_pSampleLinear );
    
    g_CityMesh.Destroy();
    g_HeavyMesh.Destroy();
    g_ColumnMesh.Destroy();
}
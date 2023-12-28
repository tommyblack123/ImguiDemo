#include "pch.h"
#include "Dx11Hook.h"

#include <iostream>
#include <d3d11.h>
#include <tchar.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "Tool.h"

#pragma comment(lib,"d3d11.lib")

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;

//½»»»Á´ Ðé±íÖ¸Õë
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

static HWND gHwnd = nullptr;

static int64_t* gSwapVTable = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

HRESULT  FakePresent(
	IDXGISwapChain* pSwapChain,
	/* [in] */ UINT SyncInterval,
	/* [in] */ UINT Flags);

using PresentCall = decltype(&FakePresent);
static PresentCall g_OriginPresentCall = nullptr;


void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Windowed = TRUE;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;

	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	auto res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, 
		NULL, NULL, featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
		&g_pSwapChain, NULL, NULL, NULL);


	//D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE,
	//	NULL, NULL,&feature_level, 1, D3D11_SDK_VERSION, &scd, &swapchain, &device, NULL, &context);
	

	if (res != S_OK)
		return false;

	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	
	if (!pBackBuffer)
		return;

	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}



bool InitWork(IDXGISwapChain* pSwapChain)
{
	pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice);

	if (g_pd3dDevice)
	{
		g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);
	}
	else {
		return false;
	}


	ID3D11Texture2D* pBackBuffer = nullptr;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

	if (!pBackBuffer)
		return false;

	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("c:/windows/fonts/msyh.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());


	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(gHwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);


	return true;
}


HRESULT  FakePresent(
	IDXGISwapChain* pSwapChain,
	/* [in] */ UINT SyncInterval,
	/* [in] */ UINT Flags)
{
	static bool bHasInit = false;

	if (!bHasInit)
	{
		InitWork(pSwapChain);
		bHasInit = true;
	}

	// Start the Dear ImGui frame

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("wow1");


	ImGui::End();
	// Rendering
	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	OutputDebugStringEx("[wow1] hooking\r\n");
	return g_OriginPresentCall(pSwapChain, SyncInterval, Flags);
}


int Dx11Hook(void *Param)
{
	HWND hwnd = (HWND)Param;
	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		//::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	gSwapVTable = *(int64_t**)g_pSwapChain;
	g_OriginPresentCall = (PresentCall)gSwapVTable[8];

	HookVtb(gSwapVTable, 8, FakePresent);

	g_pSwapChain->Release();

	return 1;
}



void StartHook(const char* WndClassName,const char *WndTitle)
{
	gHwnd = ::FindWindow(WndClassName, WndTitle);
	if (!gHwnd)
		return;

	CreateConsole();
	std::thread(Dx11Hook, gHwnd).detach();

}
#include "pch.h"
#include "Dx11Hook.h"

#include <iostream>
#include <d3d11.h>
#include <tchar.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "Tool.h"
#include "baidu_font.hpp"
#include "UeEngineTools.h"

#pragma comment(lib,"d3d11.lib")

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;

//交换链 虚表指针
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

static HWND gHwnd = nullptr;
static WNDPROC gHwndOldWndProc = nullptr;


static int64_t* gSwapVTable = nullptr;

static bool g_bHooked = false;

namespace vars
{
	static bool bMenuOpen = true;
}

enum IDXGISwapChainvTable //for dx10 / dx11
{
	QUERY_INTERFACE,
	ADD_REF,
	RELEASE,
	SET_PRIVATE_DATA,
	SET_PRIVATE_DATA_INTERFACE,
	GET_PRIVATE_DATA,
	GET_PARENT,
	GET_DEVICE,
	PRESENT,
	GET_BUFFER,
	SET_FULLSCREEN_STATE,
	GET_FULLSCREEN_STATE,
	GET_DESC,
	RESIZE_BUFFERS,
	RESIZE_TARGET,
	GET_CONTAINING_OUTPUT,
	GET_FRAME_STATISTICS,
	GET_LAST_PRESENT_COUNT
};




bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void SetupWndProcHook();
void UnSetupWndProcHook();

HRESULT  FakePresent(
	IDXGISwapChain* pSwapChain,
	/* [in] */ UINT SyncInterval,
	/* [in] */ UINT Flags);


HRESULT  FakeResizeBuffers(
	IDXGISwapChain* pSwapChain,
	/* [in] */ UINT BufferCount,
	/* [in] */ UINT Width,
	/* [in] */ UINT Height,
	/* [in] */ DXGI_FORMAT NewFormat,
	/* [in] */ UINT SwapChainFlags);


using PresentCall = decltype(&FakePresent);
static PresentCall g_OriginPresentCall = nullptr;

using ResizeBuffersCall = decltype(&FakeResizeBuffers);
static ResizeBuffersCall g_OriginResizeBuffersCall = nullptr;



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


	/*D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE,
		NULL, NULL,&feature_level, 1, D3D11_SDK_VERSION, &scd, &swapchain, &device, NULL, &context);*/
	

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


static bool g_bInit = false;

bool InitWork(IDXGISwapChain* pSwapChain)
{


	if (!g_bInit)
	{
		g_bInit = true;
		SetupWndProcHook();
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
		io.Fonts->AddFontFromFileTTF("c:/windows/fonts/msyh.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		ImFontConfig f_cfg;
		f_cfg.FontDataOwnedByAtlas = false;
		//ImFont* font = io.Fonts->AddFontFromMemoryTTF((void*)baidu_font_data, baidu_font_size, 18.0f, &f_cfg, io.Fonts->GetGlyphRangesChineseFull());
		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(gHwnd);

	};


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

	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);


	return true;
}
void UnSetup();


bool gStartDraw = false;
bool gStartDrawBoneLine = false;
bool gLineTraceSingle = false;


void DrawAllActors(std::string Content, float x,float y)
{
	//开始绘制
	ImGui::GetForegroundDrawList()->AddText({ x,y }, ImColor(255, 255, 0), Content.data());
}
void DrawActorBoneLine(float x1, float y1, float x2, float y2,int r,int g,int b)
{
	//开始绘制
	ImGui::GetForegroundDrawList()->AddLine({ x1,y1 }, { x2,y2 }, ImColor(r, g, b));
}




HRESULT  FakePresent(
	IDXGISwapChain* pSwapChain,
	/* [in] */ UINT SyncInterval,
	/* [in] */ UINT Flags)
{
	if (g_bHooked == false)
	{
		InitWork(pSwapChain);
		g_bHooked = true;
	}

	// Start the Dear ImGui frame

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();



	if (vars::bMenuOpen)
	{
		ImGui::Begin("wow1");

		if (ImGui::Button(u8"结束退出"))
		{

			//CleanupDeviceD3D();
			UnSetupWndProcHook();
			HookVtb(gSwapVTable, IDXGISwapChainvTable::PRESENT, g_OriginPresentCall);
			HookVtb(gSwapVTable, IDXGISwapChainvTable::RESIZE_BUFFERS, g_OriginResizeBuffersCall);
			//UnSetup();

			g_bHooked = false;
			g_bInit = false;
			return 0;
		}


		if (ImGui::Button(u8"绘制actor"))
		{
			gStartDraw = !gStartDraw;
		}

		if (ImGui::Button(u8"绘制骨骼"))
		{
			gStartDrawBoneLine = !gStartDrawBoneLine;
		}


		if (ImGui::Button(u8"绘制障碍骨骼"))
		{
			gLineTraceSingle = !gLineTraceSingle;
		}


		if (gStartDraw)
		{
			UeEngineTools::DrawAllActors(DrawAllActors);
		}

		if (gStartDrawBoneLine)
		{
			UeEngineTools::DrawAllActorsBone(DrawActorBoneLine);
		}

		if (gLineTraceSingle)
		{
			UeEngineTools::DrawTraceSingle(DrawActorBoneLine);
		}

		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	//OutputDebugStringEx("[wow1] hooking\r\n");
	return g_OriginPresentCall(pSwapChain, SyncInterval, Flags);
}



int64_t GetSwapChainObj()
{
	//polygon特征

	//[[[[0x00007FF740D4E748]+0x2698]]+70]

	int64_t nBase = 0x00007FF740D4E748;

	nBase = *(int64_t*)(nBase + 0);
	nBase = *(int64_t*)(nBase + 0x2698);
	nBase = *(int64_t*)(nBase + 0);
	nBase = *(int64_t*)(nBase + 0x70);

	return nBase;
}

int Dx11Hook(void *Param)
{
	HWND hwnd = (HWND)Param;
	// Initialize Direct3D
	//if (!CreateDeviceD3D(hwnd))
	//{
	//	CleanupDeviceD3D();
	//	//::UnregisterClassW(wc.lpszClassName, wc.hInstance);
	//	return 1;
	//}

	g_pSwapChain = (IDXGISwapChain*)GetSwapChainObj();

	gSwapVTable = *(int64_t**)g_pSwapChain;
	g_OriginPresentCall = (PresentCall)gSwapVTable[IDXGISwapChainvTable::PRESENT];

	g_OriginResizeBuffersCall = (ResizeBuffersCall)gSwapVTable[IDXGISwapChainvTable::RESIZE_BUFFERS];

	HookVtb(gSwapVTable, IDXGISwapChainvTable::PRESENT, FakePresent);
	HookVtb(gSwapVTable, IDXGISwapChainvTable::RESIZE_BUFFERS, FakeResizeBuffers);

	//g_pSwapChain->Release();

	return 1;
}



void StartHook(const char* WndClassName,const char *WndTitle)
{
	gHwnd = ::FindWindow(WndClassName, WndTitle);
	if (!gHwnd)
		return;

	//CreateConsole();
	//std::thread(Dx11Hook, gHwnd).detach();
	Dx11Hook(gHwnd);
}



extern
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc_Hooked(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	//////如果按下INS键，就打开或关闭外挂设置界面，如果之前是关闭的就打开，如果是打开的就关闭。
	if (uMsg == WM_KEYDOWN && wParam == VK_INSERT)
	{
		vars::bMenuOpen = !vars::bMenuOpen;
	}

	////如果外挂设置界面是打开状态，则调用ImGui的消息处理
	//if (vars::bMenuOpen && ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
	//{
	//	return TRUE;
	//}

	if (vars::bMenuOpen && ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return TRUE;


	return CallWindowProcW(gHwndOldWndProc, hwnd, uMsg, wParam, lParam);
}

void SetupWndProcHook()
{
	gHwndOldWndProc = (WNDPROC)SetWindowLongPtrW(gHwnd, GWLP_WNDPROC, (LONG_PTR)WndProc_Hooked);
	OutputDebugStringEx(_T("[wow1] the gHwndOldWndProc:0x%llX\r\n"), gHwndOldWndProc);
}

void UnSetupWndProcHook()
{
	SetWindowLongPtr(gHwnd, GWLP_WNDPROC, (LONG_PTR)gHwndOldWndProc);
}




HRESULT  FakeResizeBuffers(
	IDXGISwapChain* pSwapChain,
	/* [in] */ UINT BufferCount,
	/* [in] */ UINT Width,
	/* [in] */ UINT Height,
	/* [in] */ DXGI_FORMAT NewFormat,
	/* [in] */ UINT SwapChainFlags)
{
	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = nullptr;
		g_mainRenderTargetView->Release();
		g_mainRenderTargetView = nullptr;

		ImGui_ImplDX11_Shutdown();
		g_bHooked = false;
		HookVtb(gSwapVTable, IDXGISwapChainvTable::PRESENT, FakePresent);
	}




	return g_OriginResizeBuffersCall(pSwapChain,BufferCount,Width,Height, NewFormat,SwapChainFlags);
 }
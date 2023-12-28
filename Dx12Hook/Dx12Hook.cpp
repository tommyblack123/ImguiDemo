#include "pch.h"
#include "Dx12Hook.h"

#include <iostream>
#include <d3d12.h>
#include <tchar.h>
#include <dxgi1_4.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "Tool.h"

#pragma comment(lib,"d3d12.lib")


//交换链 虚表指针
static HWND gHwnd = nullptr;
static WNDPROC gHwndOldWndProc = nullptr;


static int64_t* gSwapVTable = nullptr;

static bool g_bHooked = false;


#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64                  FenceValue;
};

// Data
static int const                    NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT                         g_frameIndex = 0;

static int const                    NUM_BACK_BUFFERS = 3;
static ID3D12Device* g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
static ID3D12Fence* g_fence = nullptr;
static HANDLE                       g_fenceEvent = nullptr;
static UINT64                       g_fenceLastSignaledValue = 0;
static IDXGISwapChain3* g_pSwapChain = nullptr;
static HANDLE                       g_hSwapChainWaitableObject = nullptr;
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};




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
void CleanupRenderTarget();
void SetupWndProcHook();


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


void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
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

	

	static auto once = []()
	{
		SetupWndProcHook();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.Fonts->AddFontFromFileTTF("c:/windows/fonts/msyh.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(gHwnd);
		return true;
	}();

	ImGui_ImplDX12_Init(g_pd3dDevice, g_pd3dDeviceContext);


	return true;
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

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("wow1");


	ImGui::End();
	// Rendering
	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData());

	OutputDebugStringEx("[wow1] hooking\r\n");
	return g_OriginPresentCall(pSwapChain, SyncInterval, Flags);
}


int Dx12Hook(void *Param)
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
	g_OriginPresentCall = (PresentCall)gSwapVTable[IDXGISwapChainvTable::PRESENT];

	g_OriginResizeBuffersCall = (ResizeBuffersCall)gSwapVTable[IDXGISwapChainvTable::RESIZE_BUFFERS];

	HookVtb(gSwapVTable, IDXGISwapChainvTable::PRESENT, FakePresent);
	HookVtb(gSwapVTable, IDXGISwapChainvTable::RESIZE_BUFFERS, FakeResizeBuffers);

	g_pSwapChain->Release();

	return 1;
}



void StartHook(const char* WndClassName,const char *WndTitle)
{
	gHwnd = ::FindWindow(WndClassName, WndTitle);
	if (!gHwnd)
		return;

	CreateConsole();
	std::thread(Dx12Hook, gHwnd).detach();

}



extern
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc_Hooked(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static auto once = []()
	{
		std::cout << __FUNCTION__ << " first called!" << std::endl;

		return true;
	}();

	//////如果按下INS键，就打开或关闭外挂设置界面，如果之前是关闭的就打开，如果是打开的就关闭。
	//if (uMsg == WM_KEYDOWN && wParam == VK_INSERT)
	//{
	//	vars::bMenuOpen = !vars::bMenuOpen;
	//	return FALSE;
	//}

	////如果外挂设置界面是打开状态，则调用ImGui的消息处理
	//if (vars::bMenuOpen && ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
	//{
	//	return TRUE;
	//}

	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return TRUE;


	return CallWindowProc(gHwndOldWndProc, hwnd, uMsg, wParam, lParam);
}

void SetupWndProcHook()
{
	gHwndOldWndProc = (WNDPROC)SetWindowLongPtr(gHwnd, GWLP_WNDPROC, (LONG_PTR)WndProc_Hooked);
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

		ImGui_ImplDX12_Shutdown();
		g_bHooked = false;
		HookVtb(gSwapVTable, IDXGISwapChainvTable::PRESENT, FakePresent);
	}




	return g_OriginResizeBuffersCall(pSwapChain,BufferCount,Width,Height, NewFormat,SwapChainFlags);
 }


bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC1 sd;
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = NUM_BACK_BUFFERS;
		sd.Width = 0;
		sd.Height = 0;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		sd.Scaling = DXGI_SCALING_STRETCH;
		sd.Stereo = FALSE;
	}

	// [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
	ID3D12Debug* pdx12Debug = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
		pdx12Debug->EnableDebugLayer();
#endif

	// Create device
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
		return false;

	// [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
	if (pdx12Debug != nullptr)
	{
		ID3D12InfoQueue* pInfoQueue = nullptr;
		g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		pInfoQueue->Release();
		pdx12Debug->Release();
	}
#endif

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NumDescriptors = NUM_BACK_BUFFERS;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 1;
		if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
			return false;

		SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
		{
			g_mainRenderTargetDescriptor[i] = rtvHandle;
			rtvHandle.ptr += rtvDescriptorSize;
		}
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
			return false;
	}

	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 1;
		if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
			return false;
	}

	for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
		if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
			return false;

	if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
		g_pd3dCommandList->Close() != S_OK)
		return false;

	if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
		return false;

	g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (g_fenceEvent == nullptr)
		return false;

	{
		IDXGIFactory4* dxgiFactory = nullptr;
		IDXGISwapChain1* swapChain1 = nullptr;
		if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
			return false;
		if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
			return false;
		if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
			return false;
		swapChain1->Release();
		dxgiFactory->Release();
		g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
		g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
	}

	return true;
}
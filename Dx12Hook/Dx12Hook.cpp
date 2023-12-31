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
#include "d3d12hook.h"


int64_t GetGameSwapChain()
{
	//[[[[base+0x355FF80]+0xb08]]+0x18] =  swapchain


	int64_t _base = (int64_t)GetModuleHandle(nullptr) + 0x355FF80;

	_base = *(int64_t*)(_base + 0);
	_base = *(int64_t*)(_base + 0xb08);
	_base = *(int64_t*)(_base + 0x0);
	_base = *(int64_t*)(_base + 0x18);

	return _base;
}



void StartHook(const char* WndClassName, const char* WndTitle) {

		int64_t* pSwapChain = (int64_t*)GetGameSwapChain();
		int64_t nSwapChain = (int64_t)pSwapChain;
		int64_t* vtable1 = (int64_t*)pSwapChain[0];

		int64_t nd3dCommandQueue = *(int64_t*)(nSwapChain + 0x118);
		int64_t* pd3dCommandQueue = (int64_t*)nd3dCommandQueue;
		int64_t* vtable2 = (int64_t*)pd3dCommandQueue[0];

		d3d12hook::oExecuteCommandListsD3D12 = (void(*)(ID3D12CommandQueue*, UINT, ID3D12CommandList*))vtable2[10];
		d3d12hook::oSignalD3D12 = (HRESULT(*)(ID3D12CommandQueue*, ID3D12Fence*, UINT64))vtable2[14];
		d3d12hook::oPresentD3D12 = (d3d12hook::PresentD3D12)vtable1[8];

		HookVtb(vtable2, 10, d3d12hook::hookExecuteCommandListsD3D12);
		HookVtb(vtable2, 14, d3d12hook::hookSignalD3D12);
		HookVtb(vtable1, 8, d3d12hook::hookPresentD3D12);

		//swap +0x118 = g_pd3dCommandList
		OutputDebugStringEx("[wow1] GetGameSwapChain:%p g_pd3dCommandQueue:%p\r\n", pSwapChain, nd3dCommandQueue);

		//kiero::bind(54, (void**)&d3d12hook::oExecuteCommandListsD3D12, d3d12hook::hookExecuteCommandListsD3D12);
		//kiero::bind(58, (void**)&d3d12hook::oSignalD3D12, d3d12hook::hookSignalD3D12);
		//kiero::bind(140, (void**)&d3d12hook::oPresentD3D12, d3d12hook::hookPresentD3D12);
		//kiero::bind(84, (void**)&d3d12hook::oDrawInstancedD3D12, d3d12hook::hookkDrawInstancedD3D12);
		//kiero::bind(85, (void**)&d3d12hook::oDrawIndexedInstancedD3D12, d3d12hook::hookDrawIndexedInstancedD3D12);
		//kiero::bind(145, (void**)&d3d12hook::oResizeBuffersCall, d3d12hook::hookResizeBuffer);
}


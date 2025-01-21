// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include"ImguiInit.h"
#include<process.h>


HMODULE g_hModule = nullptr;

DWORD __stdcall ExitThreadCallBack(void* lParam) {

	FreeLibraryAndExitThread(g_hModule, 0);

	return 1;
}


unsigned int __stdcall ThreadFun(PVOID pM)
{
    XZ::ImguiInitDx11& pImGui = XZ::ImguiInitDx11::get_instance();
    pImGui.CreateImgui();
    while (true)
    {
        if (!pImGui.Render())
        {
            break;
        }
    }

    auto h = ::CreateThread(NULL, 0, ExitThreadCallBack, NULL, 0, NULL);
   ::CloseHandle(h);
    //ExitThreadCallBack(nullptr);
    return 0;
}

struct ProcessWindowData
{
    HWND hWnd;
    unsigned long lProcessId;
};
BOOL CALLBACK EnumWindowCallback(HWND hWnd, LPARAM lParam)
{
    ProcessWindowData& wndData = *(ProcessWindowData*)lParam;
    unsigned long lProcessId = 0;
    ::GetWindowThreadProcessId(hWnd, &lProcessId);
    if ((wndData.lProcessId != lProcessId) || (::GetWindow(hWnd, GW_OWNER) != (HWND)0) || !::IsWindowVisible(hWnd))
        return 1;
    wndData.hWnd = hWnd;
    return 0;
}
HWND GetMainWindowHwnd(unsigned long lProcessId)
{
    ProcessWindowData wndData;
    wndData.hWnd = 0;
    wndData.lProcessId = lProcessId;
    ::EnumWindows(EnumWindowCallback, (LPARAM)&wndData);
    return wndData.hWnd;
}






BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    g_hModule = hModule;

    HANDLE handle = NULL;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        xz_游戏窗口句柄 = GetMainWindowHwnd(GetCurrentProcessId()); //使用
        DisableThreadLibraryCalls(hModule);
        handle = (HANDLE)_beginthreadex(NULL, 0, ThreadFun, NULL, 0, NULL);
        ::CloseHandle(handle);

        break;
    case DLL_PROCESS_DETACH:
    {

    }
  
        break;
    }
    return TRUE;
}


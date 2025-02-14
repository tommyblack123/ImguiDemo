// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "Tool.h"
#include "Dx11Hook.h"


HMODULE g_hModule = nullptr;

DWORD __stdcall ExitThreadCallBack(void* lParam) {

	FreeLibraryAndExitThread(g_hModule, 0);

	return 1;
}


void UnSetup()
{

	auto h = ::CreateThread(NULL, 0, ExitThreadCallBack, NULL, 0, NULL);
	::CloseHandle(h);

}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    g_hModule = hModule;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {

        //UnrealWindow
        OutputDebugStringEx("[wow1] %s:%d\r\n", __FUNCTION__, __LINE__);
        StartHook("UnrealWindow", nullptr);
        //StartHook("GxWindowClass",nullptr); //魔兽
    }
    break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


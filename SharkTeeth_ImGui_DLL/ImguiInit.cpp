#include "ImguiInit.h"
#include <dwmapi.h>
#include"Imgui/baidu_font.hpp"

#include "UeEngineTools.h"

//d3d11.lib; d3dcompiler.lib; dxgi.lib;
#pragma comment(lib,"d3d11.lib")

extern HWND xz_��Ϸ���ھ��;
namespace XZ {


    ImguiInitDx11::ImguiInitDx11()
    {
    }


   



    ImguiInitDx11::~ImguiInitDx11()
    {
        // Cleanup
        //MessageBox(0, "~ImguiInitDx11", 0, 0);
        //ImGui_ImplDX11_Shutdown();
        //ImGui_ImplWin32_Shutdown();
        //ImGui::DestroyContext();

        //CleanupDeviceD3D();
        ////::DestroyWindow(m_�ɰ崰�ھ��);
        //::UnregisterClass(wc.lpszClassName, wc.hInstance);
    }
    
    void ImguiInitDx11::CreateImgui() 
    {
        GetWindowRect(xz_��Ϸ���ھ��, &m_���ھ���);	//��ȡ��Ϸ���ڴ�Сλ��
        wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
        ::RegisterClassEx(&wc);
        m_�ɰ崰�ھ�� = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW ,wc.lpszClassName,_T("Draw ImGui"),WS_POPUP,m_���ھ���.left, m_���ھ���.top, m_���ھ���.��(), m_���ھ���.��(), NULL, NULL, GetModuleHandle(NULL), NULL);

        // Show the window  
        MARGINS n = { m_���ھ���.left, m_���ھ���.top, m_���ھ���.��(), m_���ھ���.��()};
        DwmExtendFrameIntoClientArea(m_�ɰ崰�ھ��, &n);
        SetLayeredWindowAttributes(m_�ɰ崰�ھ��, RGB(0, 0, 0),125, LWA_COLORKEY);
        ::ShowWindow(m_�ɰ崰�ھ��, SW_SHOWDEFAULT);  //SW_SHOW
        ::UpdateWindow(m_�ɰ崰�ھ��);

        // Initialize Direct3D
        if (!CreateDeviceD3D(m_�ɰ崰�ھ��))
        {
            CleanupDeviceD3D();
            ::UnregisterClass(wc.lpszClassName, wc.hInstance);
            return;
        }

        // ���� ImGui ����
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
        io.WantSaveIniSettings = false;
        io.IniFilename = NULL;
        // ��ʼ�����
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();
        //ImGui::StyleColorsLight();


        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        //ImGuiStyle& style = ImGui::GetStyle();
        //if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        //{
        //    style.WindowRounding = 0.0f;
        //    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        //}

        // ��ʼ�� Win32 DX
        ImGui_ImplWin32_Init(m_�ɰ崰�ھ��);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

        //��������
        ImFontConfig f_cfg;
        f_cfg.FontDataOwnedByAtlas = false;
        ImFont* font = io.Fonts->AddFontFromMemoryTTF((void*)baidu_font_data, baidu_font_size, 18.0f, &f_cfg, io.Fonts->GetGlyphRangesChineseFull());

    }

    bool ImguiInitDx11::CreateDeviceD3D(HWND hWnd)
    {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }

    void ImguiInitDx11::CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
        if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    }

    void ImguiInitDx11::CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }

    void ImguiInitDx11::CleanupRenderTarget()
    {
        if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
    }

    LRESULT ImguiInitDx11::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (get_instance().g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
            {
                get_instance().CleanupRenderTarget();
                get_instance().g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                get_instance().CreateRenderTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        case WM_DPICHANGED:
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
            {
                const RECT* suggested_rect = (RECT*)lParam;
                ::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
            }
            break;
        }
        return ::DefWindowProc(hWnd, msg, wParam, lParam);
    }


    bool ImguiInitDx11::Render()
    {
        static bool m_bDone = false;

        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                m_bDone = true;
        }
        if (m_bDone) {

            CleanEnv();
            return false;
        }

        GetWindowRect(xz_��Ϸ���ھ��, &m_���ھ���);	//��ȡ��Ϸ���ڴ�Сλ��
        MoveWindow(m_�ɰ崰�ھ��, m_���ھ���.left, m_���ھ���.top, m_���ھ���.��(), m_���ھ���.��(), true);

        static bool m_imgui���� = true;
        ////����������Ϸ ȡ��ע��
        //if (::GetKeyState(VK_HOME) < 0)
        //{
        //    SetForegroundWindow(m_�ɰ崰�ھ��);
        //    m_imgui���� = true;
        //}
        //if (::GetKeyState(VK_INSERT) < 0)
        //{
        //    m_imgui���� = false;
        //}


        //���Ի�ͼ��
        static int xx = 300;
        if (300 <= xx && xx < m_���ھ���.bottom - m_���ھ���.top)
        {
            xx++;
        }
        else {
            xx = 300;
        }


        StartDraw();

        static int counter = 0;
        if (m_imgui����)
        {
            ImGui::Begin(u8"��������");
            static bool boolbox = true;
            ImGui::Checkbox(u8"��ʾ׼��", &boolbox);
            if (boolbox)
            {
                static float f = 0.0f;
                static int i = 30;
                //ImGui::SliderFloat(u8"�뾶��ֵ", &f, 10.0f, 300.0f);
                //ImGui::SliderInt2("int", &i, 0, 10);
                ImGui::SliderInt(u8"�뾶��ֵ", &i, 10, 100);
                ImGui::Text("This is some useful text. %d ddd", i);
                ImGui::GetForegroundDrawList()->AddCircle(ImVec2(m_���ھ���.���ĵ�x(), m_���ھ���.���ĵ�y()), i, ImColor(84, 250, 250, 250));
            }

            if (ImGui::Button("Button"))
                counter++;

            if (ImGui::Button(u8"�����˳�"))
            {
                ::PostMessage(m_�ɰ崰�ھ��,WM_CLOSE,  0, 0);
            }

			if (ImGui::Button(u8"����Actor��"))
			{
                UeEngineTools::DrawAllActors();
			}


            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        //ImGui::GetForegroundDrawList()->AddLine(ImVec2(m_���ھ���.left, m_���ھ���.top), ImVec2(xx, m_���ھ���.bottom), ImColor(84, 250, 250, 250));
        //ImGui::GetForegroundDrawList()->AddText(ImVec2(m_���ھ���.left + 300, xx), ImColor(30, 250, 250, 250), "fefsefessfess");
        //    
        //ImGui::GetForegroundDrawList()->AddLine(ImVec2(m_���ھ���.left, m_���ھ���.top), ImVec2(m_���ھ���.right, m_���ھ���.bottom), ImColor(84, 250, 250, 250));
        //ImGui::GetForegroundDrawList()->AddLine(ImVec2(m_���ھ���.right, m_���ھ���.top), ImVec2(m_���ھ���.left, m_���ھ���.bottom), ImColor(84, 250, 250, 250));
        

        EndDraw();

        return true;
    }

	void ImguiInitDx11::CleanEnv()
	{
		ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
//::DestroyWindow(m_�ɰ崰�ھ��);
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
	}

	void ImguiInitDx11::StartDraw()
    {
        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void ImguiInitDx11::EndDraw()
    {
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0); // ��ֱͬ��
    }
}

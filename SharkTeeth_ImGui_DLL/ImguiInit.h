#pragma once
#include "Imgui/imconfig.h"
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_dx11.h"
#include "Imgui/imgui_impl_win32.h"
#include "Imgui/imgui_internal.h"
#include "Imgui/imstb_rectpack.h"
#include "Imgui/imstb_textedit.h"
#include "Imgui/imstb_truetype.h"
#include <d3d11.h>
#include <tchar.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace XZ {

	class ImguiInitDx11
	{
	public:
		struct xz_���ھ��� :public RECT {
			int ��() { return right - left; }
			int ��() { return bottom - top; }
			int ���ĵ�x() { return (left + right) / 2; }
			int ���ĵ�y() { return (top + bottom) / 2; }
		};
	private:
		xz_���ھ��� m_���ھ���;
		HWND m_�ɰ崰�ھ��;
		WNDCLASSEX wc;
		
		ID3D11Device* g_pd3dDevice;
		ID3D11DeviceContext* g_pd3dDeviceContext;
		IDXGISwapChain* g_pSwapChain;
		ID3D11RenderTargetView* g_mainRenderTargetView;

	public:
		~ImguiInitDx11();

		/// <summary>
		/// ����ImGui
		/// </summary>
		void CreateImgui();

		/// <summary>
		/// ��Ϣѭ��
		/// </summary>
		bool Render();


		void CleanEnv();


		ImguiInitDx11(const ImguiInitDx11&) = delete;
		ImguiInitDx11& operator=(const ImguiInitDx11&) = delete;
		static ImguiInitDx11& get_instance() {
			static ImguiInitDx11 instance;
			return instance;
	}
	private:
		ImguiInitDx11();

		bool CreateDeviceD3D(HWND hWnd);
		void CleanupDeviceD3D();
		void CreateRenderTarget();
		void CleanupRenderTarget();

		void StartDraw();
		void EndDraw();

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	};
}


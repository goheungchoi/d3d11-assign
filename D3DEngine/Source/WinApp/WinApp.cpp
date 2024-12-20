#include "D3DEngine/WinApp/WinApp.h"

#include <imgui_impl_win32.h>

static WinApp* winApp{ nullptr };

WinApp::WinApp(HINSTANCE _hInstance, const wchar_t* _className) {
	// Register the window class
	this->getWindowClassConfigurationHelper()
		.setClassName(_className)
		.setInstanceHandle(_hInstance)
		.setWindowProcessCallback()
		.setStyle(CS_HREDRAW | CS_VREDRAW)
		.setBackgroundBrushHandle(CreateSolidBrush(RGB(0, 0, 0)))
		.setIconeHandle(LoadIcon(0, IDI_APPLICATION))
		.setCursorHandle(LoadCursor(0, IDC_ARROW))
		.endSetting()
		.registerWindowClass();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WinApp::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(winApp->GetWindow(), uMsg, wParam, lParam))
		return true;
	
	switch (uMsg) {
		// Close the window when user alt-f4s or clicks the X button
	case WM_CLOSE:
		OutputDebugString(L"WinApp: Destroying this window...\n");
		DestroyWindow(GetWindow());
		return 0;
	case WM_DESTROY:
		OutputDebugString(L"WinApp: Post Quit Message...\n");
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(_hwnd, uMsg, wParam, lParam);
}

void WinApp::App_Init(HINSTANCE _hInstance, const wchar_t* _className) {
	if (winApp) return;

	winApp = new WinApp(_hInstance, _className);
}

AppWindow WinApp::App_CreateWindow(int _width, int _height, const wchar_t* _title, DWORD _style)
{
	winApp->getWindowStyleConfigurationHelper()
		.setWindowTitle(_title)
		.setStyle(_style)
		.setWindowSize(_width, _height)
		.setWindowPositionCenter()
		.endSetting()
		.createWindow();

	ShowWindow(winApp->GetWindow(), SW_SHOWNORMAL);
	return winApp->GetWindow();
}

void WinApp::App_Destroy() {
	if (!winApp) return;

	winApp = nullptr;
}

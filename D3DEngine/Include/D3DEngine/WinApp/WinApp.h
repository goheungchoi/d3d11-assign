#pragma once

#include "D3DEngine/WinApp/WinBase.h"

class WinApp : public WinBase<WinApp> {
	WinApp(HINSTANCE _hInstance, const wchar_t* _className);
	WinApp(const WinApp&) = delete;
	WinApp& operator=(const WinApp&) = delete;

public:
	/**
	 * @brief Message handler
	 *
	 * @param uMsg
	 * @param wParam
	 * @param lParam
	 * @return LRESULT
	 */
	LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

public:

	static void App_Init(HINSTANCE _hInstance = GetModuleHandle(NULL), const wchar_t* _className = L"WinClass");
	static AppWindow App_CreateWindow(int _width, int _height, const wchar_t* _title = L"", DWORD _style = WS_OVERLAPPED);
	static void App_Destroy();
};

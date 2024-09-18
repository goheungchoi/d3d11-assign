#ifndef UNICODE
#define UNICODE
#endif

#include "common.h"

#include "demo_app.h"

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow
) {
	// COM initialize
	if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return -1;

	DemoApp app;
	app.Initialize();
	app.Execute();
	app.Shutdown();

	// COM finalize
	CoUninitialize();

	return 0;
}
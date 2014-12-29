#include "windows.h"
#include "../common/D3D10GrabberDefs.hpp"
#include "../common/WinDXUtils.hpp"

LPCWSTR g_szClassName = L"{B0262869-ADE9-4B0A-A4D7-159A80428536}";

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

HWND createWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	//Step 1: Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		return NULL;
	}

	// Step 2: Creating the Window
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		L"",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
		NULL, NULL, hInstance, NULL);

	/*ShowWindow(hwnd, 0);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}*/
	return hwnd;
}

int CALLBACK WinMain(
	HINSTANCE   hInstance,
	HINSTANCE   hPrevInstance,
	LPSTR       lpCmdLine,
	int         nCmdShow
	)
{
	HWND hWnd = createWindow(hInstance);
	if (!hWnd) {
		return 1;
	}

	if (!WinUtils::LoadD3DandDXGI()) {
		return 2;
	}
	UINT d3d9PresentOffset = WinUtils::GetD3D9PresentOffset(hWnd);
	UINT d3d9SCPresentOffset = WinUtils::GetD3D9SCPresentOffset(hWnd);
	UINT dxgiPresentOffset = WinUtils::GetDxgiPresentOffset(hWnd);
	if (!d3d9PresentOffset || !d3d9SCPresentOffset || !dxgiPresentOffset) {
		return 3;
	}

	HANDLE mapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, HOOKSGRABBER_SHARED_MEM_NAME);
	if (NULL == mapping) {
		return 4;
	}

	char *memory = (char *)MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, HOOKSGRABBER_SHARED_MEM_SIZE);
	if (memory == NULL) {
		return 5;
	}

	HOOKSGRABBER_SHARED_MEM_DESC *desc = (HOOKSGRABBER_SHARED_MEM_DESC*)memory;

	desc->frameId = 0x12345678;

	CloseWindow(hWnd);
	DestroyWindow(hWnd);

	return 0;
}
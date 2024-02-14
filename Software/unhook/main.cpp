#include"stdio.h"
#include"shlwapi.h"

#define UINT8 unsigned char

#ifdef HOOKS_SYSWOW64
#define UNLOAD_MODULE_NAME L"prismatik-hooks32.dll"
#else
#define UNLOAD_MODULE_NAME L"prismatik-hooks.dll"
#endif

extern "C" __declspec(dllexport) BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID lpReserved) {
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);

	if (fdwReason == DLL_PROCESS_ATTACH) {
		HMODULE hooksdll = GetModuleHandle(UNLOAD_MODULE_NAME);
		if (hooksdll) {
			FreeLibrary(hooksdll);
		}
		// tell windows to unload us
		return FALSE;
	}

	return TRUE;
}

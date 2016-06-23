#pragma comment(linker, "/entry:AppEntry@0 /subsystem:Windows")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "shell32.lib")

#include <windows.h>
#include <shellapi.h>

void APIENTRY AppEntry()
{
	HMODULE hVcr = LoadLibraryW(L"msvcr120.dll");
	HMODULE hVcp = LoadLibraryW(L"msvcp120.dll");
	if (hVcr == NULL || hVcp == NULL)
		ShellExecuteW(NULL, NULL, L"vcredist_x86.exe", L"/install /passive /norestart", NULL, SW_SHOWNORMAL);
	ExitProcess(0);
}

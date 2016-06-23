#define PSAPI_VERSION 1

#include <unordered_map>
#include <vector>
#include <stdio.h>

#include <Windows.h>
#include <Psapi.h>

#pragma comment(lib, "psapi.lib")

int main(int argc, char* argv[])
{
	HANDLE hOutputPipe = 0;
	if (sscanf(argv[1], "%d", &hOutputPipe) != 1)
		return 1;
    
	std::vector<DWORD> processIds(4096);
	DWORD processCount;

	EnumProcesses(&processIds[0], sizeof(DWORD) * processIds.size(), &processCount);
	processCount /= sizeof(DWORD);

	if (processCount > processIds.size())
		processCount = processIds.size();
	else
		processIds.resize(processCount);

	for (DWORD pid : processIds)
	{
		HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pid);
		if (hProc != NULL)
		{
			std::vector<HMODULE> hModList(1024);
			DWORD modCount;
			bool bModuleFound = true;

			if (EnumProcessModulesEx(hProc, &hModList[0], sizeof(HMODULE) * hModList.size(), &modCount, LIST_MODULES_ALL) != 0)
			{
				if (modCount > hModList.size())
					modCount = hModList.size();

				std::vector<WCHAR> hFileName(MAX_PATH);

				bModuleFound = false;

				for (int i = 0; i < modCount; ++i)
				{
					if (GetModuleFileNameExW(hProc, hModList[i], &hFileName[0], MAX_PATH) > 0)
					{
						for (wchar_t& x : hFileName)
						{
							if (x == 0)
								break;
							if (x >= L'A' && x <= L'Z')
								x = x - L'A' + L'a';
						}
						std::wstring wfn(&hFileName[0]);
						if (wfn.find(L"d3d9.dll") != std::wstring::npos
							|| wfn.find(L"d3d10.dll") != std::wstring::npos
							|| wfn.find(L"d3d10_1.dll") != std::wstring::npos
							|| wfn.find(L"d3d11.dll") != std::wstring::npos
							|| wfn.find(L"dxgi.dll") != std::wstring::npos
							|| wfn.find(L"d3d8.dll") != std::wstring::npos
							|| wfn.find(L"opengl32.dll") != std::wstring::npos)
						{
							bModuleFound = true;
							break;
						}
					}
				}
			}
			CloseHandle(hProc);

			if (bModuleFound == false)
				continue;
		}

		DWORD written;
		WriteFile(hOutputPipe, &pid, sizeof(DWORD), &written, NULL);
	}
	return 0;
}


#include "Tool.h"


int  CreateConsole() {
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
	freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
	freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);
	return 0;
}

void HookVtb(int64_t* vTable, int nIndex, void* NewAddr)
{
	DWORD dwProp = 0;
	VirtualProtect(vTable, 1024, PAGE_EXECUTE_READWRITE, &dwProp);
	
	InterlockedExchange64(&vTable[nIndex], (LONG64)NewAddr);

	VirtualProtect(vTable, 1024, dwProp, &dwProp);

}

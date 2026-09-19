#include "预编译头.h"
#include "引擎/引擎.h"
#include "反调试.h"
#include "断点清理.h"
static VOID NTAPI 第一线程回调(PVOID 参数, BOOLEAN 定时器是否重置)
{
	Engine::初始化();
}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
		反调试::自身模块 = hModule;
		// ★ 抓干净代码基线必须在这里: 越早越好, 晚了基线里就带着 ACE 下的 0xCC 了
		断点清理::锁定基线();
		内核::安全创建线程(WAITORTIMERCALLBACK(第一线程回调), hModule);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
	}
	return TRUE;
}

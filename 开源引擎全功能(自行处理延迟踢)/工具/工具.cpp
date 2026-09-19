#include "工具.h"
#include "../字符串加密.h"
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")
extern "C" SHORT SysCall1(DWORD syscallNum, void* arg1);
std::string gbk转utf8(const std::string& gbk)
{
	int 宽长度 = MultiByteToWideChar(936, 0, gbk.c_str(), -1, nullptr, 0);
	if (宽长度 == 0) return "";
	std::wstring 宽字符串(宽长度, L'\0');
	MultiByteToWideChar(936, 0, gbk.c_str(), -1, &宽字符串[0], 宽长度);
	int utf8长度 = WideCharToMultiByte(CP_UTF8, 0, 宽字符串.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (utf8长度 == 0) return "";
	std::string utf8(utf8长度, '\0');
	WideCharToMultiByte(CP_UTF8, 0, 宽字符串.c_str(), -1, &utf8[0], utf8长度, nullptr, nullptr);
	return utf8.c_str();
}
std::string 宽转utf8(const std::wstring& 宽字符串)
{
	int utf8长度 = WideCharToMultiByte(CP_UTF8, 0, 宽字符串.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (utf8长度 == 0) return "";
	std::string utf8(utf8长度, '\0');
	WideCharToMultiByte(CP_UTF8, 0, 宽字符串.c_str(), -1, &utf8[0], utf8长度, nullptr, nullptr);
	return utf8.c_str();
}
std::string 读配置项(const std::string& 文件名, const std::string& 节名, const std::string& 项名, const std::string& 默认值) {
	char 返回字符串[1024] = { 0 };
	::GetPrivateProfileStringA(节名.c_str(), 项名.c_str(), 默认值.c_str(), 返回字符串, sizeof(返回字符串) - sizeof(char), 文件名.c_str());
	return 返回字符串;
}
SHORT 取按键状态(int 虚拟键) {
	static DWORD 系统调用号 = 0;
	static bool 已解析 = false;
	if (!已解析) {
		已解析 = true;
		BYTE* 函数 = (BYTE*)GetProcAddress(GetModuleHandleA(加密串("user32.dll")), 加密串("GetAsyncKeyState"));
		if (函数 && 函数[0] == 0x4C && 函数[1] == 0x8B && 函数[2] == 0xD1 && 函数[3] == 0xB8) {
			DWORD ssn = *(DWORD*)(函数 + 4);
			if (ssn > 0 && ssn < 0x1000) 系统调用号 = ssn;
		}
	}
	if (系统调用号) {
		return SysCall1(系统调用号, (void*)(intptr_t)虚拟键);
	}
	return GetAsyncKeyState(虚拟键);
}
HANDLE 内核::m_hTimerQueueTimer = nullptr;
void 内核::安全创建线程(WAITORTIMERCALLBACK 回调, PVOID 参数) {
	if (!CreateTimerQueueTimer(&m_hTimerQueueTimer, NULL, 回调, 参数, 0, 0, WT_EXECUTEDEFAULT))
	{
		DeleteTimerQueueTimer(NULL, m_hTimerQueueTimer, NULL);
		m_hTimerQueueTimer = NULL;
		return;
	}
}

#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include "include/detours/detours.h"
typedef std::vector<unsigned char> bytes;
extern std::string gbk转utf8(const std::string& gbk);
extern std::string 宽转utf8(const std::wstring& 宽字符串);
extern std::string 读配置项(const std::string& 文件名, const std::string& 节名, const std::string& 项名, const std::string& 默认值 = "");
extern SHORT 取按键状态(int 虚拟键);
namespace 内核 {
	extern HANDLE m_hTimerQueueTimer;
	extern void 安全创建线程(WAITORTIMERCALLBACK 回调, PVOID 参数);
}

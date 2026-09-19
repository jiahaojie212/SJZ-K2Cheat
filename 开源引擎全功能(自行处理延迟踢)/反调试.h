#pragma once
#include <Windows.h>
#include "内存模块.h"
#include "字符串加密.h"
namespace 反调试 {
    inline HMODULE 自身模块 = nullptr;
    extern "C" NTSTATUS SysCall2(DWORD syscallNr, PVOID a1, PVOID a2);
    inline void* 取终止桩() {
        HMODULE ntdll = 内存模块::取ntdll();
        return ntdll ? 内存模块::查找导出(ntdll, 加密串("NtTerminateProcess")) : nullptr;
    }
    inline void* 取退出函数地址() {
        HMODULE ntdll = 内存模块::取ntdll();
        return ntdll ? 内存模块::查找导出(ntdll, 加密串("RtlExitUserProcess")) : nullptr;
    }
    inline NTSTATUS 紧急结束进程(NTSTATUS 退出码 = 0) noexcept {
        __try {
            auto 桩 = (const BYTE*)取终止桩();
            if (桩 && 桩[0] == 0x4C && 桩[1] == 0x8B && 桩[2] == 0xD1 && 桩[3] == 0xB8) {
                DWORD ssn = *(const DWORD*)(桩 + 4);
                if (ssn > 0 && ssn < 0x1000)
                    return SysCall2(ssn, (PVOID)(uintptr_t)-1, (PVOID)(uintptr_t)退出码);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        __try {
            using 结束函数 = NTSTATUS(NTAPI*)(HANDLE, NTSTATUS);
            if (auto f = (结束函数)取终止桩())
                return f((HANDLE)(uintptr_t)-1, 退出码);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        __try {
            using 退出函数 = void(NTAPI*)(NTSTATUS);
            if (auto f = (退出函数)取退出函数地址())
                f(退出码);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        ExitProcess((UINT)退出码);
        __assume(0);
    }
}

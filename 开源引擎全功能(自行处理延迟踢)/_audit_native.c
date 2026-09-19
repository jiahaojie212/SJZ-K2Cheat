// audit probe v2: guard-page + CONTEXT + NtGetNextThread + VirtualProtect-in-VEH semantics
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winternl.h>
#include <stdio.h>
#include <stddef.h>
#include <intrin.h>
#include <stdlib.h>

#pragma comment(lib, "ntdll.lib")

#ifndef MemoryBasicInformation
#define MemoryBasicInformation 0
#endif
#define MM_FILENAME_INFO 2

typedef struct _TBI {
    NTSTATUS ExitStatus;      // 0x00
    PVOID    TebBaseAddress;  // 0x08
    CLIENT_ID ClientId;       // 0x10
    KAFFINITY AffinityMask;   // 0x20
    KPRIORITY Priority;       // 0x28
    KPRIORITY BasePriority;   // 0x2C
} TBI;

typedef NTSTATUS(NTAPI* PFN_NtGetNextThread)(HANDLE, HANDLE, ACCESS_MASK, ULONG, ULONG, PHANDLE);
typedef NTSTATUS(NTAPI* PFN_NtQueryInformationThread)(HANDLE, ULONG, PVOID, ULONG, PULONG);
typedef NTSTATUS(NTAPI* PFN_NtSetContextThread)(HANDLE, PCONTEXT);
typedef NTSTATUS(NTAPI* PFN_NtGetContextThread)(HANDLE, PCONTEXT);
typedef NTSTATUS(NTAPI* PFN_NtReadVirtualMemory)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
typedef NTSTATUS(NTAPI* PFN_NtQueryVirtualMemory)(HANDLE, PVOID, ULONG, PVOID, SIZE_T, PSIZE_T);

static volatile LONG  g_guardHits = 0;
static volatile LONG  g_lastCode = 0;
static volatile ULONG_PTR g_lastInfo0 = 0;
static volatile ULONG_PTR g_lastInfo1 = 0;
static volatile ULONG_PTR g_vehRip = 0;
static volatile LONG  g_rearm = 1;
static void*  g_guardPage = NULL;
static DWORD  g_guardOrigProt = 0;
static volatile LONG  g_vehCache[8];
static volatile LONG  g_vehCacheN = 0;

static volatile LONG g_fetchCap = 0;   // >0: fetch-fault 计数到上限就自杀, 防死循环
static LONG NTAPI Veh(PEXCEPTION_POINTERS ep) {
    LONG n = InterlockedIncrement(&g_guardHits);
    g_lastCode = ep->ExceptionRecord->ExceptionCode;
    if (ep->ExceptionRecord->NumberParameters > 0) g_lastInfo0 = ep->ExceptionRecord->ExceptionInformation[0];
    if (ep->ExceptionRecord->NumberParameters > 1) g_lastInfo1 = ep->ExceptionRecord->ExceptionInformation[1];
    g_vehRip = ep->ContextRecord ? ep->ContextRecord->Rip : 0;
    if (g_vehCacheN < 8) g_vehCache[InterlockedIncrement(&g_vehCacheN) - 1] = ep->ExceptionRecord->ExceptionCode;
    if (g_rearm) {
        DWORD old = 0;
        BOOL ok = VirtualProtect(g_guardPage, 0x1000, g_guardOrigProt | PAGE_GUARD, &old);
        if (n <= 3) { printf("    [VEH] #%ld re-arm ok=%d old=0x%lX err=%lu\n", n, ok, old, GetLastError()); fflush(stdout); }
    }
    if (g_fetchCap && n >= g_fetchCap) {
        printf("    [VEH] fetch-fault 连续触发 %ld 次 -> 判定死循环, 退出\n", n); fflush(stdout);
        ExitProcess(99);
    }
    return EXCEPTION_CONTINUE_EXECUTION;
}

static volatile LONG g_stop = 0;
static DWORD WINAPI SpinThread(LPVOID unused) { (void)unused; while (!g_stop) Sleep(1); return 0; }

static int RetOne(void) { return 1; }

int main(void) {
    HMODULE nt = GetModuleHandleW(L"ntdll.dll");
    PFN_NtGetNextThread pNext = (PFN_NtGetNextThread)GetProcAddress(nt, "NtGetNextThread");
    PFN_NtQueryInformationThread pQit = (PFN_NtQueryInformationThread)GetProcAddress(nt, "NtQueryInformationThread");
    PFN_NtSetContextThread pSet = (PFN_NtSetContextThread)GetProcAddress(nt, "NtSetContextThread");
    PFN_NtGetContextThread pGet = (PFN_NtGetContextThread)GetProcAddress(nt, "NtGetContextThread");
    PFN_NtReadVirtualMemory pRead = (PFN_NtReadVirtualMemory)GetProcAddress(nt, "NtReadVirtualMemory");
    PFN_NtQueryVirtualMemory pQvm = (PFN_NtQueryVirtualMemory)GetProcAddress(nt, "NtQueryVirtualMemory");

    printf("=== size/const checks ===\n");
    printf("  sizeof(CONTEXT)=%llu sizeof(MBI)=%llu sizeof(TBI)=%llu sizeof(CLIENT_ID)=%llu\n",
        (unsigned long long)sizeof(CONTEXT), (unsigned long long)sizeof(MEMORY_BASIC_INFORMATION),
        (unsigned long long)sizeof(TBI), (unsigned long long)sizeof(CLIENT_ID));
    printf("  CONTEXT_AMD64=0x%08lX CONTEXT_DEBUG_REGISTERS=0x%08lX CONTEXT_CONTROL=0x%08lX CONTEXT_FULL=0x%08lX\n",
        (unsigned long)CONTEXT_AMD64, (unsigned long)CONTEXT_DEBUG_REGISTERS,
        (unsigned long)CONTEXT_CONTROL, (unsigned long)CONTEXT_FULL);
    printf("  off Dr0=%llu Dr7=%llu Dr6=%llu EFlags=%llu Rip=%llu\n",
        (unsigned long long)offsetof(CONTEXT, Dr0), (unsigned long long)offsetof(CONTEXT, Dr7),
        (unsigned long long)offsetof(CONTEXT, Dr6), (unsigned long long)offsetof(CONTEXT, EFlags),
        (unsigned long long)offsetof(CONTEXT, Rip));

    printf("=== [A] free address: MemoryMappedFilenameInformation ===\n");
    {
        BYTE* p = (BYTE*)VirtualAlloc(NULL, 0x800000, MEM_RESERVE, PAGE_READWRITE);
        VirtualFree(p, 0, MEM_RELEASE);
        WCHAR buf[512] = { 0 };
        UNICODE_STRING us = { 0 };
        us.Buffer = buf; us.MaximumLength = (USHORT)(sizeof(buf) - sizeof(WCHAR));
        SIZE_T ret = 0xFFFFFFFF;
        NTSTATUS st = pQvm(GetCurrentProcess(), p + 0x300000, MM_FILENAME_INFO, &us, sizeof(us), &ret);
        printf("  FREE addr + MM_FILENAME_INFO = 0x%08lX retlen=%llu\n", (unsigned long)st, (unsigned long long)ret);
        WCHAR buf2[512] = { 0 };
        UNICODE_STRING us2 = { 0 };
        us2.Buffer = buf2; us2.MaximumLength = (USHORT)(sizeof(buf2) - sizeof(WCHAR));
        SIZE_T ret2 = 0;
        NTSTATUS st2 = pQvm(GetCurrentProcess(), (PVOID)GetModuleHandleW(L"kernel32.dll"), MM_FILENAME_INFO, &us2, sizeof(us2), &ret2);
        printf("  control(mapped) = 0x%08lX retlen=%llu len=%u\n", (unsigned long)st2, (unsigned long long)ret2, us2.Length);
    }

    int skipB = getenv("SKIP_B") != NULL;
    printf("=== [B] guard page on RX code page: fetch fault ===\n"); fflush(stdout);
    if (skipB) goto afterB;
    {
        void* h = AddVectoredExceptionHandler(1, Veh);
        printf("  AddVectoredExceptionHandler(1,..) = %p\n", h); fflush(stdout);
        BYTE* code = (BYTE*)VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        memcpy(code, (void*)RetOne, 64);
        DWORD old = 0;
        printf("  arm RX|GUARD = %d err=%lu\n",
            VirtualProtect(code, 0x1000, PAGE_EXECUTE_READ | PAGE_GUARD, &old), GetLastError()); fflush(stdout);
        g_guardPage = code; g_guardOrigProt = PAGE_EXECUTE_READ; g_rearm = 0; g_guardHits = 0;
        typedef int (*fn_t)(void);
        fn_t f = (fn_t)code;
        int r = f();
        printf("  call#1(no re-arm): ret=%d vehHits=%ld code=0x%08lX info0=%llu (8=fetch) info1=%p rip=%p f==RetOne=%d\n",
            r, g_guardHits, (unsigned long)g_lastCode, (unsigned long long)g_lastInfo0,
            (void*)g_lastInfo1, (void*)g_vehRip, (int)((void*)f == (void*)RetOne)); fflush(stdout);
        g_guardHits = 0;
        r = f();
        printf("  call#2(guard consumed): vehHits=%ld ret=%d\n", g_guardHits, r); fflush(stdout);
        // 现在验证"在取指异常里重挂 guard"会发生什么 —— 用上限保护, 命中死循环就退出
        g_rearm = 1; g_fetchCap = 50000; g_guardHits = 0;
        VirtualProtect(code, 0x1000, PAGE_EXECUTE_READ | PAGE_GUARD, &old);
        printf("  call#3(VEH 里 re-arm, 代码页取指):\n"); fflush(stdout);
        r = f();
        printf("  call#3 居然返回了: ret=%d vehHits=%ld\n", r, g_guardHits); fflush(stdout);
        g_rearm = 0; g_fetchCap = 0;
        RemoveVectoredExceptionHandler(h); fflush(stdout);
    }
afterB:
    printf("=== [C] guard page on RW data page: read/write fault ===\n");
    {
        void* h = AddVectoredExceptionHandler(1, Veh);
        BYTE* d = (BYTE*)VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_READWRITE);
        d[0] = 0x42;
        DWORD old = 0;
        g_guardPage = d; g_guardOrigProt = PAGE_READWRITE; g_rearm = 0;
        VirtualProtect(d, 0x1000, PAGE_READWRITE | PAGE_GUARD, &old);
        g_guardHits = 0;
        volatile BYTE v = d[0];
        printf("  read : vehHits=%ld code=0x%08lX info0=%llu(0=read) info1=%p\n",
            g_guardHits, (unsigned long)g_lastCode, (unsigned long long)g_lastInfo0, (void*)g_lastInfo1);
        VirtualProtect(d, 0x1000, PAGE_READWRITE | PAGE_GUARD, &old);
        g_guardHits = 0;
        d[0] = 0x43;
        printf("  write: vehHits=%ld code=0x%08lX info0=%llu(1=write) info1=%p\n",
            g_guardHits, (unsigned long)g_lastCode, (unsigned long long)g_lastInfo0, (void*)g_lastInfo1);
        RemoveVectoredExceptionHandler(h); fflush(stdout);
    }

    printf("=== [C2] ExceptionInformation[0] detail ===\n"); fflush(stdout);
    {
        void* h = AddVectoredExceptionHandler(1, Veh);
        DWORD old = 0;
        g_rearm = 0;
        // (a) RW 页 guard, 读
        BYTE* d = (BYTE*)VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_READWRITE);
        d[0] = 1; g_guardPage = d; g_guardOrigProt = PAGE_READWRITE;
        VirtualProtect(d, 0x1000, PAGE_READWRITE | PAGE_GUARD, &old); g_guardHits = 0;
        volatile BYTE v = d[0];
        printf("  (a) RW+GUARD read : info0=%llu\n", (unsigned long long)g_lastInfo0); fflush(stdout);
        VirtualProtect(d, 0x1000, PAGE_READWRITE | PAGE_GUARD, &old); g_guardHits = 0;
        d[0] = 2;
        printf("  (b) RW+GUARD write: info0=%llu\n", (unsigned long long)g_lastInfo0); fflush(stdout);
        // (c) RO 页 guard, 写
        BYTE* r = (BYTE*)VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_READONLY);
        g_guardPage = r; g_guardOrigProt = PAGE_READONLY;
        VirtualProtect(r, 0x1000, PAGE_READONLY | PAGE_GUARD, &old); g_guardHits = 0;
        volatile BYTE v2 = r[0];
        printf("  (c) RO+GUARD read : info0=%llu code=0x%08lX\n", (unsigned long long)g_lastInfo0, (unsigned long)g_lastCode); fflush(stdout);
        // (d) AV 对照: 写只读页(无 guard)
        g_guardPage = r; g_rearm = 0;
        VirtualProtect(r, 0x1000, PAGE_READONLY, &old); g_guardHits = 0;
        __try { *(volatile BYTE*)r = 9; } __except (EXCEPTION_EXECUTE_HANDLER) {}
        printf("  (d) RO write AV  : lastCode=0x%08lX info0=%llu\n", (unsigned long)g_lastCode, (unsigned long long)g_lastInfo0); fflush(stdout);
        // (e) 从 guard 页取指(EXECUTE_READ|GUARD) 再确认 info0=8; 以及 NOACCESS+GUARD
        BYTE* x = (BYTE*)VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        memcpy(x, (void*)RetOne, 64);
        g_guardPage = x; g_guardOrigProt = PAGE_EXECUTE_READ;
        VirtualProtect(x, 0x1000, PAGE_EXECUTE_READ | PAGE_GUARD, &old); g_guardHits = 0;
        typedef int (*fn_t)(void); fn_t f = (fn_t)x;
        int rr = f();
        printf("  (e) RX+GUARD exec : info0=%llu ret=%d\n", (unsigned long long)g_lastInfo0, rr); fflush(stdout);
        // (f) PAGE_NOACCESS 上再或 PAGE_GUARD 会怎样
        BYTE* n = (BYTE*)VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_READWRITE);
        BOOL ok = VirtualProtect(n, 0x1000, PAGE_NOACCESS | PAGE_GUARD, &old);
        MEMORY_BASIC_INFORMATION m; VirtualQuery(n, &m, sizeof(m));
        printf("  (f) NOACCESS|GUARD ok=%d 实际Protect=0x%lX\n", ok, m.Protect); fflush(stdout);
        // (g) VEH 句柄返回值形态
        printf("  (g) AddVectoredExceptionHandler 返回 = %p (非NULL即句柄)\n", h); fflush(stdout);
        RemoveVectoredExceptionHandler(h); fflush(stdout);
    }
    printf("=== [D] NtSetContextThread Dr on self ===\n");
    {
        CONTEXT c; memset(&c, 0, sizeof(c));
        c.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        NTSTATUS g = pGet(GetCurrentThread(), &c);
        printf("  Get(DEBUG_REGISTERS) = 0x%08lX flags=0x%lX Dr7=0x%llX\n", (unsigned long)g, c.ContextFlags, c.Dr7);
        c.Dr0 = (DWORD64)(ULONG_PTR)&g_guardHits; c.Dr7 = 0x1;
        c.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        NTSTATUS s = pSet(GetCurrentThread(), &c);
        printf("  Set(Dr0=%p Dr7=0x1) = 0x%08lX\n", (void*)c.Dr0, (unsigned long)s);
        CONTEXT c2; memset(&c2, 0, sizeof(c2)); c2.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        pGet(GetCurrentThread(), &c2);
        printf("  readback Dr0=%p Dr7=0x%llX\n", (void*)c2.Dr0, c2.Dr7);
        c2.Dr0 = c2.Dr1 = c2.Dr2 = c2.Dr3 = 0; c2.Dr7 = 0; c2.Dr6 = 0; c2.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        printf("  Set(clear all) = 0x%08lX\n", (unsigned long)pSet(GetCurrentThread(), &c2));
        CONTEXT c3; memset(&c3, 0, sizeof(c3)); c3.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        pGet(GetCurrentThread(), &c3);
        printf("  after clear Dr0=%p Dr7=0x%llX Dr6=0x%llX\n", (void*)c3.Dr0, c3.Dr7, c3.Dr6);
        CONTEXT c4; memset(&c4, 0, sizeof(c4)); c4.ContextFlags = 0x10;
        printf("  Get(flags=0x10, no AMD64 bit) = 0x%08lX\n", (unsigned long)pGet(GetCurrentThread(), &c4));
        CONTEXT c5; memset(&c5, 0, sizeof(c5)); c5.ContextFlags = CONTEXT_DEBUG_REGISTERS | CONTEXT_CONTROL;
        printf("  Get(DEBUG|CONTROL) = 0x%08lX Dr7=0x%llX EFlags=0x%lX\n",
            (unsigned long)pGet(GetCurrentThread(), &c5), c5.Dr7, c5.EFlags);
    }

    printf("=== [E] NtSetContextThread on other running thread ===\n");
    {
        DWORD tid = 0;
        HANDLE th = CreateThread(NULL, 0, SpinThread, NULL, 0, &tid);
        Sleep(60);
        CONTEXT c; memset(&c, 0, sizeof(c));
        c.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        c.Dr1 = (DWORD64)(ULONG_PTR)&g_guardHits; c.Dr7 = 0x4;
        printf("  Set(other running, Dr1) = 0x%08lX\n", (unsigned long)pSet(th, &c));
        CONTEXT c2; memset(&c2, 0, sizeof(c2)); c2.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        NTSTATUS g = pGet(th, &c2);
        printf("  Get(other) = 0x%08lX Dr1=%p Dr7=0x%llX\n", (unsigned long)g, (void*)c2.Dr1, c2.Dr7);
        c2.Dr1 = 0; c2.Dr7 = 0; c2.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        printf("  Clear(other) = 0x%08lX\n", (unsigned long)pSet(th, &c2));
        InterlockedExchange(&g_stop, 1);
        WaitForSingleObject(th, 2000); CloseHandle(th);
    }

    printf("=== [F] NtGetNextThread ownership ===\n");
    {
        HANDLE cur = NULL, next = NULL;
        for (int i = 0; i < 6; ++i) {
            next = NULL;
            NTSTATUS st = pNext(GetCurrentProcess(), cur, THREAD_QUERY_INFORMATION, 0, 0, &next);
            if (st < 0) { printf("  iter%d: 0x%08lX -> end\n", i, (unsigned long)st); break; }
            TBI tbi; memset(&tbi, 0, sizeof(tbi)); ULONG rl = 0;
            NTSTATUS q = pQit(next, 0, &tbi, sizeof(tbi), &rl);
            printf("  iter%d: st=0x%08lX handle=%p qit=0x%08lX retlen=%lu tid=%llu teb=%p\n", i,
                (unsigned long)st, next, (unsigned long)q, rl,
                (unsigned long long)(ULONG_PTR)tbi.ClientId.UniqueThread, tbi.TebBaseAddress);
            if (cur) NtClose(cur);
            cur = next;
        }
        if (cur) NtClose(cur);
        HANDLE n2 = NULL;
        NTSTATUS st2 = pNext(GetCurrentProcess(), NULL, THREAD_GET_CONTEXT, 0, 0, &n2);
        printf("  access=THREAD_GET_CONTEXT only: 0x%08lX handle=%p\n", (unsigned long)st2, n2);
        if (n2) {
            TBI tbi; memset(&tbi, 0, sizeof(tbi)); ULONG rl = 0;
            NTSTATUS q = pQit(n2, 0, &tbi, sizeof(tbi), &rl);
            printf("    QueryInformationThread: 0x%08lX teb=%p\n", (unsigned long)q, tbi.TebBaseAddress);
            NtClose(n2);
        }
        HANDLE n3 = NULL;
        NTSTATUS st3 = pNext(GetCurrentProcess(), NULL, 0, 0, 0, &n3);
        printf("  access=0: 0x%08lX handle=%p\n", (unsigned long)st3, n3);
        if (n3) NtClose(n3);
    }

    printf("=== [G] NtReadVirtualMemory partial semantics ===\n");
    {
        BYTE* two = (BYTE*)VirtualAlloc(NULL, 0x2000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        memset(two, 0xAA, 0x2000);
        VirtualFree(two + 0x1000, 0x1000, MEM_DECOMMIT);
        BYTE out[0x400];
        SIZE_T got = 0;
        NTSTATUS st = pRead(GetCurrentProcess(), two, out, 0x40, &got);
        printf("  read 0x40 first page        : 0x%08lX got=%llu\n", (unsigned long)st, (unsigned long long)got);
        got = 0;
        st = pRead(GetCurrentProcess(), two + 0x800, out, 0x1800, &got);
        printf("  straddle 0x1800 from +0x800 : 0x%08lX got=%llu\n", (unsigned long)st, (unsigned long long)got);
        got = 0;
        st = pRead(GetCurrentProcess(), two + 0x1000, out, 0x10, &got);
        printf("  read decommitted page       : 0x%08lX got=%llu\n", (unsigned long)st, (unsigned long long)got);
        got = 0;
        st = pRead(GetCurrentProcess(), (PVOID)nt, out, 0x10, &got);
        printf("  read ntdll header           : 0x%08lX got=%llu\n", (unsigned long)st, (unsigned long long)got);
    }

    printf("=== [H] VirtualProtect on already-guarded page ===\n");
    {
        BYTE* d = (BYTE*)VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_READWRITE);
        DWORD old = 0;
        BOOL ok1 = VirtualProtect(d, 0x1000, PAGE_READWRITE | PAGE_GUARD, &old);
        MEMORY_BASIC_INFORMATION m1; VirtualQuery(d, &m1, sizeof(m1));
        BOOL ok2 = VirtualProtect(d, 0x1000, PAGE_READWRITE | PAGE_GUARD, &old);
        MEMORY_BASIC_INFORMATION m2; VirtualQuery(d, &m2, sizeof(m2));
        printf("  arm ok=%d prot=0x%lX ; re-arm ok=%d old=0x%lX prot=0x%lX\n",
            ok1, m1.Protect, ok2, old, m2.Protect);
        VirtualFree(d, 0, MEM_RELEASE);
    }

    printf("=== done ===\n");
    fflush(stdout); return 0;
}

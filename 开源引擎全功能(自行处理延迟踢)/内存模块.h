#pragma once
#include <Windows.h>
#include <winternl.h>
#include <intrin.h>
#include <cstdint>
#include <cstring>
#include <string>
template <std::size_t N>
struct 宽密文 {
	wchar_t 数据[N] = {};
	constexpr 宽密文(const wchar_t(&源)[N], int 密钥) {
		for (std::size_t i = 0; i < N; ++i) 数据[i] = (wchar_t)(源[i] ^ 密钥);
	}
};
class 宽明文RAII {
public:
	std::wstring 串;
	~宽明文RAII() noexcept {
		if (!串.empty()) SecureZeroMemory(串.data(), 串.size() * sizeof(wchar_t));
	}
	operator const wchar_t*() const noexcept { return 串.c_str(); }
	operator const std::wstring&() const noexcept { return 串; }
	const wchar_t* c_str() const noexcept { return 串.c_str(); }
	size_t size() const noexcept { return 串.size(); }
};
template <std::size_t N>
inline 宽明文RAII 宽解密(const 宽密文<N>& 密, int 密钥) {
	宽明文RAII 结果;
	结果.串.resize(N - 1);
	for (std::size_t i = 0; i < N - 1; ++i) 结果.串[i] = (wchar_t)(密.数据[i] ^ 密钥);
	return 结果;
}
#define 宽加密串(s) 宽解密<sizeof(s) / sizeof(wchar_t)>(宽密文<sizeof(s) / sizeof(wchar_t)>(s, 0x5A), 0x5A)
namespace 内存模块 {
    inline PEB* 取PEB() noexcept {
        return (PEB*)__readgsqword(0x60);
    }
    inline HMODULE 取进程基址() noexcept {
        __try {
            PEB* peb = 取PEB();
            if (!peb) return nullptr;
            return *(HMODULE*)((const BYTE*)peb + 0x10);
        } __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    }
    inline HMODULE 取模块基址(const wchar_t* 名称, bool 忽略大小写 = true) noexcept {
        __try {
            PEB* peb = 取PEB();
            if (!peb || !peb->Ldr) return nullptr;
            const LIST_ENTRY* 首项 = &peb->Ldr->InMemoryOrderModuleList;
            size_t 目标长 = 名称 ? wcslen(名称) : 0;
            if (!目标长) return nullptr;
            for (const LIST_ENTRY* 项 = 首项->Flink; 项 != 首项; 项 = 项->Flink) {
                const BYTE* 表项 = (const BYTE*)项 - 0x10;
                const UNICODE_STRING* 名 = (const UNICODE_STRING*)(表项 + 0x58);
                if (!名->Buffer || 名->Length == 0) continue;
                size_t 名长 = 名->Length / 2;
                if (名长 != 目标长) continue;
                bool 匹配 = true;
                for (size_t i = 0; i < 名长; ++i) {
                    wchar_t a = 名->Buffer[i], b = 名称[i];
                    if (忽略大小写) {
                        if (a >= L'a' && a <= L'z') a -= (L'a' - L'A');
                        if (b >= L'a' && b <= L'z') b -= (L'a' - L'A');
                    }
                    if (a != b) { 匹配 = false; break; }
                }
                if (匹配) return *(HMODULE*)(表项 + 0x30);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        return nullptr;
    }
    inline HMODULE 取ntdll() noexcept {
        static const std::wstring 名 = 宽加密串(L"ntdll.dll");
        return 取模块基址(名.c_str());
    }
    inline void* 查找导出(HMODULE 基址, const char* 函数名) noexcept {
        __try {
            if (!基址 || !函数名) return nullptr;
            const auto* dos = (const IMAGE_DOS_HEADER*)基址;
            if (dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;
            const auto* nt = (const IMAGE_NT_HEADERS*)((const BYTE*)基址 + dos->e_lfanew);
            if (nt->Signature != IMAGE_NT_SIGNATURE) return nullptr;
            const IMAGE_DATA_DIRECTORY& 导出 =
                nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
            if (!导出.VirtualAddress || !导出.Size) return nullptr;
            const auto* 表 = (const IMAGE_EXPORT_DIRECTORY*)((const BYTE*)基址 + 导出.VirtualAddress);
            DWORD 名数 = 表->NumberOfNames;
            if (!名数) return nullptr;
            const DWORD* 名表 = (const DWORD*)((const BYTE*)基址 + 表->AddressOfNames);
            const WORD* 序号表 = (const WORD*)((const BYTE*)基址 + 表->AddressOfNameOrdinals);
            const DWORD* 函数表 = (const DWORD*)((const BYTE*)基址 + 表->AddressOfFunctions);
            for (DWORD i = 0; i < 名数; ++i) {
                const char* 名 = (const char*)((const BYTE*)基址 + 名表[i]);
                if (strcmp(名, 函数名) == 0) {
                    DWORD rva = 函数表[序号表[i]];
                    return (BYTE*)基址 + rva;
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        return nullptr;
    }
}

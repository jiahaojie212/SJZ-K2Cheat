#pragma once
#include <Windows.h>
#include <cstdint>
#define 混淆恒真() ((DWORD)(GetCurrentProcessId() | 1) != 0)
#define 混淆恒假() ((DWORD)(GetCurrentProcessId() & 0) != 0)
#define 混淆分支(真语句) \
    do { \
        if (混淆恒真()) { 真语句; } \
        else { ((volatile DWORD*)0x0)[0] = 0; }   \
    } while (0)
#define 混淆花指令() \
    do { \
        volatile DWORD _混淆噪声 = 0; \
        if (混淆恒真()) { _混淆噪声 ^= 0x9E3779B9u; } \
        else {   _混淆噪声 = 0xDEADBEEFu; } \
        if (!_混淆噪声) { _混淆噪声 = 1; } \
    } while (0)
#define 混淆跳变() \
    do { \
        volatile DWORD _跳 = (DWORD)GetCurrentProcessId(); \
        _跳 = (_跳 ^ 0x5A5A5A5Au) + 7u; \
        if (_跳 & 3u) { _跳 -= 3u; } else { _跳 += 11u; } \
        if (_跳 == _跳) { _跳 = _跳; } \
        { volatile DWORD _死 = 0; if (_死) { ((volatile DWORD*)0x0)[0] = _跳; } } \
    } while (0)

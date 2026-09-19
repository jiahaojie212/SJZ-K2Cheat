#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <windows.h>
constexpr uint64_t 字符串编译密钥 = 0x9E3779B97F4A7C15ULL;
namespace 字符串加密 {
    template <std::size_t 长度>
    struct 编译期密文 {
        char 数据[长度];
        constexpr 编译期密文(const char(&原文)[长度], uint64_t 密钥) : 数据{} {
            for (std::size_t i = 0; i < 长度; ++i) {
                unsigned char 字节 = (unsigned char)原文[i];
                unsigned char 密钥字节 = (unsigned char)(密钥 >> ((i & 7) << 3));
                数据[i] = (char)(字节 ^ 密钥字节);
            }
        }
    };
    class 明文 {
    public:
        static constexpr std::size_t 上限 = 384;
        char 缓冲[上限];
        std::size_t 长度 = 0;
        operator const char*() const noexcept { return 缓冲; }
        const char* c_str() const noexcept { return 缓冲; }
        std::size_t size() const noexcept { return 长度; }
        ~明文() noexcept { SecureZeroMemory(缓冲, sizeof(缓冲)); }
    };
    template <std::size_t 长度>
    inline 明文 解密(const 编译期密文<长度>& 密, uint64_t 编译密钥) noexcept {
        明文 结果;
        std::size_t 有效 = 长度 - 1;
        if (有效 > 明文::上限) 有效 = 明文::上限 - 1;
        for (std::size_t i = 0; i < 有效; ++i) {
            unsigned char 密字节 = (unsigned char)密.数据[i];
            unsigned char 编译字节 = (unsigned char)(编译密钥 >> ((i & 7) << 3));
            结果.缓冲[i] = (char)(密字节 ^ 编译字节);
        }
        结果.缓冲[有效] = 0;
        结果.长度 = 有效;
        return 结果;
    }
}
#define 加密串(字面量) \
    [&]() -> ::字符串加密::明文 { \
        constexpr auto 密文 = ::字符串加密::编译期密文<sizeof(字面量)>(字面量, 字符串编译密钥); \
        return ::字符串加密::解密(密文, 字符串编译密钥); \
    }()
#define 静态加密串(字面量) \
    []() -> const char* { \
        static std::string 缓存; \
        if (缓存.empty()) { \
            auto 明文对象 = 加密串(字面量); \
            缓存.assign(明文对象.c_str(), 明文对象.size()); \
        } \
        return 缓存.c_str(); \
    }()

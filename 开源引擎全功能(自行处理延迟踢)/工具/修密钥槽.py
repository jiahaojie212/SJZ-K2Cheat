# -*- coding: utf-8 -*-
# 修复 共用密钥覆盖 BUG: 三个串各自独立密钥槽
p = r'C:/Users/Administrator/Desktop/主/验证.cpp'
src = open(p, 'rb').read().decode('utf-8')

repl = [
    ('static BYTE 全局串密钥[8] = {};',
     'static BYTE 全局串密钥[3][8] = {};   // 三个串各自独立密钥 (共用会互相覆盖)'),
    ('static void 串加密写入(char* 密文缓冲, size_t& 长度出, const std::string& 明文) {',
     'static void 串加密写入(int 槽, char* 密文缓冲, size_t& 长度出, const std::string& 明文) {'),
    ('for (int i = 0; i < 8; ++i) 全局串密钥[i] = (BYTE)(GetTickCount() >> (i * 4)) ^ (BYTE)(uintptr_t)&密文缓冲 ^ 0x5A;',
     'for (int i = 0; i < 8; ++i) 全局串密钥[槽][i] = (BYTE)(GetTickCount() >> (i * 4)) ^ (BYTE)(uintptr_t)&密文缓冲 ^ 0x5A;'),
    ('密文缓冲[i] = 明文[i] ^ 全局串密钥[i & 7];',
     '密文缓冲[i] = 明文[i] ^ 全局串密钥[槽][i & 7];'),
    ('static std::string 串解密读取(const char* 密文缓冲, size_t 上限) {',
     'static std::string 串解密读取(int 槽, const char* 密文缓冲, size_t 上限) {'),
    ('出 += (char)(密文缓冲[i] ^ 全局串密钥[i & 7]);',
     '出 += (char)(密文缓冲[i] ^ 全局串密钥[槽][i & 7]);'),
    ('串加密写入(全局地址密文, 全局地址长度, 地址);',
     '串加密写入(0, 全局地址密文, 全局地址长度, 地址);'),
    ('串加密写入(全局发送密钥密文, 全局发送密钥长度, 发送密钥);',
     '串加密写入(1, 全局发送密钥密文, 全局发送密钥长度, 发送密钥);'),
    ('串加密写入(全局接收密钥密文, 全局接收密钥长度, 接收密钥);',
     '串加密写入(2, 全局接收密钥密文, 全局接收密钥长度, 接收密钥);'),
    ('串解密读取(全局地址密文, 全局地址长度)',
     '串解密读取(0, 全局地址密文, 全局地址长度)'),
    ('串解密读取(全局发送密钥密文, 全局发送密钥长度)',
     '串解密读取(1, 全局发送密钥密文, 全局发送密钥长度)'),
    ('串解密读取(全局接收密钥密文, 全局接收密钥长度)',
     '串解密读取(2, 全局接收密钥密文, 全局接收密钥长度)'),
]
done = 0
for old, new in repl:
    if old in src:
        src = src.replace(old, new)
        done += 1
open(p, 'wb').write(src.encode('utf-8'))
print('applied:', done, '/', len(repl))

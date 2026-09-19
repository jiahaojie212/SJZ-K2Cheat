# -*- coding: utf-8 -*-
# 修复 串解密读取 截断 BUG: 密文字节 XOR 后可能为 0, NUL 扫描提前断 → 用长度
p = r'C:/Users/Administrator/Desktop/主/验证.cpp'
src = open(p, 'rb').read().decode('utf-8')

old = """	for (size_t i = 0; i < 上限 && 密文缓冲[i]; ++i)
		出 += (char)(密文缓冲[i] ^ 全局串密钥[槽][i & 7]);
	return 出;"""
new = """	for (size_t i = 0; i < 上限; ++i)
		出 += (char)(密文缓冲[i] ^ 全局串密钥[槽][i & 7]);
	return 出;"""

assert old in src, "pattern not found"
src = src.replace(old, new)
open(p, 'wb').write(src.encode('utf-8'))
print('fixed truncation bug')

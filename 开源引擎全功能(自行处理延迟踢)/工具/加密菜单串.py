# -*- coding: utf-8 -*-
# 界面.cpp 菜单串批量加密: u8"..." → 静态加密串(u8"...")
import re

p = r'C:/Users/Administrator/Desktop/主/引擎/覆盖层/界面.cpp'
src = open(p, 'rb').read().decode('utf-8')

# 匹配未被 静态加密串( 包着的 u8"..." 字面量 (排除转义引号)
n_before = src.count('u8"')
src2 = re.sub(r'(?<!静态加密串\()u8"((?:[^"\\]|\\.)*)"', r'静态加密串(u8"\1")', src)
open(p, 'wb').write(src2.encode('utf-8'))
print('before u8 count:', n_before)
print('after 静态加密串(u8 count:', src2.count('静态加密串(u8'))
print('leftover bare u8:', len(re.findall(r'(?<!静态加密串\()u8"', src2)))

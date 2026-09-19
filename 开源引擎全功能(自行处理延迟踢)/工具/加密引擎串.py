# -*- coding: utf-8 -*-
# 引擎.cpp 全量 u8 加密: u8"..." → 静态加密串(u8"...")
import re

p = r'C:/Users/Administrator/Desktop/主/引擎/引擎.cpp'
src = open(p, 'rb').read().decode('utf-8')

n_before = len(re.findall(r'(?<!静态加密串\()u8"', src))
src2 = re.sub(r'(?<!静态加密串\()u8"((?:[^"\\]|\\.)*)"', r'静态加密串(u8"\1")', src)
open(p, 'wb').write(src2.encode('utf-8'))
print('before:', n_before)
print('leftover:', len(re.findall(r'(?<!静态加密串\()u8"', src2)))

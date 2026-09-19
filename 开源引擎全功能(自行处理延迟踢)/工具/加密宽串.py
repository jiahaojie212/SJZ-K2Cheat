# -*- coding: utf-8 -*-
# 引擎.cpp 宽字符串加密: L"..." → 宽加密串(L"...")  (排除无特征项)
import re

p = r'C:/Users/Administrator/Desktop/主/引擎/引擎.cpp'
src = open(p, 'rb').read().decode('utf-8')

排除 = {'SimHei', 'zh-cn', '.', '\\'}
count = 0

def rep(m):
    global count
    s = m.group(1)
    if s in 排除:
        return m.group(0)
    count += 1
    return '宽加密串(L"' + s + '")'

src2 = re.sub(r'(?<!宽加密串\()L"((?:[^"\\]|\\.)*)"', rep, src)
open(p, 'wb').write(src2.encode('utf-8'))
print('converted:', count)

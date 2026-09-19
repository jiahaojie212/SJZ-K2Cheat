# -*- coding: utf-8 -*-
# 游戏类.h: 直接调用点的 L"函数名/属性名" 全部包 宽加密串
import re

p = r'C:/Users/Administrator/Desktop/主/引擎/游戏类.h'
src = open(p, 'rb').read().decode('utf-8')

# 转换 取函数(L"X") / 取偏移(L"X") / 查找对象(L"X") / 字符串转名称(L"X") 等直接调用
# (排除已含 宽加密串 的、L#X 宏拼接的)
pat = re.compile(r'(取函数|取偏移|查找对象)\((L"(?:[^"\\]|\\.)*")\)')
count = 0
def rep(m):
    global count
    count += 1
    return m.group(1) + '(宽加密串(' + m.group(2) + '))'
src2 = pat.sub(rep, src)

# 引擎.cpp 用的相同 pattern 也补一遍 (取类属性偏移 等处)
open(p, 'wb').write(src2.encode('utf-8'))
print('converted:', count)

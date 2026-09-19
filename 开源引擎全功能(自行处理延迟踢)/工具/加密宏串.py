# -*- coding: utf-8 -*-
# 游戏类.h: 宏内 L#X 字符串化 全部包 宽加密串 (编译期 XOR)
p = r'C:/Users/Administrator/Desktop/主/引擎/游戏类.h'
src = open(p, 'rb').read().decode('utf-8')
n1 = src.count('L#类名'); n2 = src.count('L#结构体名'); n3 = src.count('L#函数名'); n4 = src.count('L#属性名')
src = src.replace('查找对象(L#类名,1)', '查找对象(宽加密串(L#类名),1)')
src = src.replace('查找对象(L#结构体名,1)', '查找对象(宽加密串(L#结构体名),1)')
src = src.replace('取函数(L#函数名)', '取函数(宽加密串(L#函数名))')
src = src.replace('取偏移(L#属性名)', '取偏移(宽加密串(L#属性名))')
open(p, 'wb').write(src.encode('utf-8'))
print('类名:', n1, '结构体名:', n2, '函数名:', n3, '属性名:', n4)

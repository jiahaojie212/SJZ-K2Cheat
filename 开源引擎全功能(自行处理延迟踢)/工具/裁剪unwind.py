# -*- coding: utf-8 -*-
"""
裁剪unwind.py - 裁剪 DLL 的异常处理元数据 (档位3: 栈隐形的"文件形态"方案)

原理:
    RtlLookupFunctionEntry(PC) 读取模块 PE 头的
    OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION(3)] 定位
    .pdata (RUNTIME_FUNCTION 表)。把该目录的 RVA+Size 清零后, 系统对
    本 DLL 内任意 PC 都查不到 unwind 项 → 回溯展开时 RtlLookupFunctionEntry
    返回 NULL → 回溯方按 leaf 截断 (截断≠计数)。

    本 DLL 运行在 0x7FFF 私有页 (无模块归属): 栈上出现本 DLL 帧时,
    ACE 的 backtrace 查库返回 NULL 直接截断 → 栈隐形。零运行时痕迹:
    不装 DR / 不注册 VEH / 不碰 IAT / 不改函数体 —— 唯一改动发生在
    "进游戏前"的磁盘文件层, ACE 记录的头快照就是这个形态, 完全一致。

注意:
    * 只改 8 字节 (RVA + Size), 不重排节表, 文件大小不变。
    * 代价: 本 DLL 内 __try/__except 与 C++ 异常的展开失效 (RtlVirtualUnwind
      找不到处理者) —— 正常路径不抛异常不受影响, 一旦抛出直接终止无兜底。
    * 加壳顺序: 若 VMProtect 加壳后重建了 EXCEPTION 目录 (节表重排/新增
      .pdata), 需对"加壳后、进游戏前"的最终 DLL 再跑一次本脚本。

用法:
    python 裁剪unwind.py <DLL路径> [--check]
    --check: 只报告当前 Exception 目录状态, 不做修改
"""
import sys
import struct
import os


IMAGE_DIRECTORY_ENTRY_EXCEPTION = 3


def 报告(路径):
    """读取并返回 Exception 目录 (rva, size), 失败返回 None"""
    try:
        with open(路径, 'rb') as f:
            数据 = f.read()
    except OSError as e:
        print('[!] 无法打开文件: %s (%s)' % (路径, e))
        return None
    if len(数据) < 0x40:
        print('[!] 文件过小, 不是有效的 PE')
        return None
    if 数据[:2] != b'MZ':
        print('[!] 缺少 MZ 签名, 不是 PE 文件')
        return None
    e_lfanew = struct.unpack_from('<I', 数据, 0x3C)[0]
    if e_lfanew + 0x18 > len(数据) or 数据[e_lfanew:e_lfanew + 4] != b'PE\0\0':
        print('[!] 缺少 PE 签名')
        return None
    machine, 节数 = struct.unpack_from('<HH', 数据, e_lfanew + 4)
    if machine != 0x8664:
        print('[!] 不是 x64 PE (machine=0x%04X), 不支持' % machine)
        return None
    # PE32+ : OptionalHeader 从 e_lfanew+24 开始, DataDirectory 偏移 112
    异常目录偏移 = e_lfanew + 24 + 112 + IMAGE_DIRECTORY_ENTRY_EXCEPTION * 8
    rva, size = struct.unpack_from('<II', 数据, 异常目录偏移)
    return 异常目录偏移, rva, size, e_lfanew


def 裁剪(路径):
    结果 = 报告(路径)
    if 结果 is None:
        return False
    偏移, rva, size, _ = 结果
    print('[i] %s' % os.path.basename(路径))
    print('[i]   Exception 目录: RVA=0x%08X Size=0x%08X (%s节, %d 项)'
          % (rva, size, '有' if size else '无', size // 12))
    if rva == 0 and size == 0:
        print('[i]   已是裁剪状态, 无需处理')
        return True
    with open(路径, 'r+b') as f:
        f.seek(偏移)
        f.write(struct.pack('<II', 0, 0))
    print('[+]   已清零 -> RVA=0 Size=0 (共写 8 字节)')
    return True


def main():
    if len(sys.argv) < 2:
        print('用法: python 裁剪unwind.py <DLL路径> [--check]')
        return 1
    路径 = sys.argv[1]
    if '--check' in sys.argv:
        结果 = 报告(路径)
        if 结果 is None:
            return 1
        _, rva, size, _ = 结果
        print('Exception 目录: RVA=0x%08X Size=0x%08X (%s)'
              % (rva, size, '有 unwind 元数据' if size else '已裁剪'))
        return 0
    return 0 if 裁剪(路径) else 1


if __name__ == '__main__':
    sys.exit(main())
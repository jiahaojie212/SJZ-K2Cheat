#pragma once
// ============================================================
//  断点全清 —— 反射调用前把"能被 ACE 拿来对账的断点"全收拾干净
//    ① 硬件断点: Dr0-Dr3 + Dr6 + Dr7(x64 的 R/W/LEN 全在里面)
//    ② 软件断点: 代码页里被灌的 0xCC(INT3) —— ACE 就是踩这个进 VEH 看返回地址
//    ③ 内存断点: 代码页上的 PAGE_GUARD —— 同样是"一读就进 VEH"的钩子
//  做法: 尽早抓一份干净代码基线(ACE 下断之前), 之后每次落反射调用前拿基线对,
//        对不上的 0xCC/ICEBP 还原回原始指令, GUARD 位洗掉。
//  纯 Win32 API, 不碰 ntdll、不碰 syscall。
// ============================================================
#include "硬件断点.h"
#include <windows.h>
#include <cstdint>

namespace 断点清理 {

	// ---------------- 开关 ----------------
	inline bool 清硬件断点 = true;   // 走 硬件断点.h 那套
	inline bool 清软件断点 = true;   // 代码页 0xCC / 0xF1 还原
	inline bool 清内存断点 = true;   // 代码页 PAGE_GUARD 洗掉
	// ★★ 性能命门 ★★
	// 跨线程清 Dr 每次调用都要挂起全进程线程(毫秒级), 一帧几十次调用直接把你帧率吃光。
	// 默认关: 只清**当前线程** —— 断点本来就是"哪个线程执行到才触发", 反射调用跑在当前线程,
	// 命门就在自己身上。真要连别人一起清, 打开 跨线程清Dr 并让它按 跨线程清Dr节流毫秒 走。
	inline bool 跨线程清Dr = false;
	inline DWORD 跨线程清Dr节流毫秒 = 500;
	inline DWORD 扫描节流毫秒 = 40;  // 代码页扫描节流(0 = 每次都扫, 会卡)
	inline bool 调试输出 = false;
	// 调用后装回去?  ★ 默认都不装: 装回去等于把 ACE 的陷阱重新摆好。
	// 要"表面无痕"再打开, 代价是调用窗口里依然带着别人下的钩子。
	inline bool 调用后恢复软件断点 = false;
	inline bool 调用后恢复内存断点 = false;

	// 启发式兜底: ACE 要是比你先落地(它先下的断), 基线里就带着 0xCC, 对镜子对不出来。
	// 打开这个就再按"孤立 INT3"抓一遍: 上下各 4 字节没有连片 0xCC 的, 就是被塞进来的断点。
	// 代价: 理论上可能误伤手写的 int3 指令 —— UE 的 .text 段里基本不会有, 敢开就开。
	inline bool 启发式抓孤立INT3 = true;

	// ---------------- 盯"临时钩子" ----------------
	// ACE 另一种玩法: 你扫的时候干净, 你调的时候临时下钩, 完事立马还原。
	// ① 调用后重扫(全模块对镜子, 有节流 —— 每次调用都重扫会卡)
	inline bool 调用后重扫 = true;
	inline DWORD 重扫节流毫秒 = 100;
	// ② 每次落调用前验一遍目标函数入口: 有没有被塞 jmp / 0xCC 内联钩
	inline bool 验目标入口 = true;
	// ③ 每次落调用前验一遍虚表槽: ACE 直接改虚表把你拐走
	inline bool 验虚表槽 = true;
	// ④ 周期性验主模块 IAT: 被换过的导入槽拎出来
	inline bool 验IAT = true;
	inline DWORD 验IAT节流毫秒 = 2000;
	// ⑤ 调用完查自己"被踩中"的痕迹: TF 单步位 / Dr6 命中位
	inline bool 查踩中痕迹 = true;
	// 耗时统计(调试用, 开销就几个 QPC)
	inline bool 统计耗时 = false;

	namespace 内部 {

		constexpr SIZE_T 页大小 = 0x1000;
		constexpr ULONG 页备份上限 = 32;                   // 单次最多记多少个 GUARD 页的保护
		constexpr SIZE_T 基线上限 = 8ull * 1024 * 1024;    // 基线封顶 8MB
		constexpr ULONG_PTR 用户空间顶 = 0x00007FFFFFFF0000ull;

		inline BYTE* 取主模块基址() noexcept {
			__try {
				return (BYTE*)__readgsqword(0x60 + 0x10);   // PEB->ImageBaseAddress
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return nullptr;
			}
		}

		inline bool 摸模块范围(BYTE*& 起, BYTE*& 止) noexcept {
			起 = nullptr; 止 = nullptr;
			BYTE* 基址 = 取主模块基址();
			if (!基址) return false;
			__try {
				const auto* dos = (const IMAGE_DOS_HEADER*)基址;
				if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
				const auto* nt = (const IMAGE_NT_HEADERS*)(基址 + dos->e_lfanew);
				if (nt->Signature != IMAGE_NT_SIGNATURE) return false;
				起 = 基址;
				止 = 基址 + nt->OptionalHeader.SizeOfImage;
				return 止 > 起;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
		}

		// ---------------- 干净代码基线 ----------------
		inline SIZE_T g_基线大小 = 0;
		inline BYTE* g_基线基址 = nullptr;
		inline BYTE* g_基线 = nullptr;
		inline bool g_基线就绪 = false;

		// 抓一份干净代码快照。★ 越早越好, 最好 DllMain 里就抓 —— 那会儿 ACE 还没下断
		inline bool 抓基线() noexcept {
			if (g_基线就绪) return true;
			BYTE* 起 = nullptr; BYTE* 止 = nullptr;
			if (!摸模块范围(起, 止)) return false;
			SIZE_T 想要 = (SIZE_T)(止 - 起);
			if (想要 > 基线上限) 想要 = 基线上限;
			if (!想要) return false;

			static BYTE 静态基线[基线上限];            // 常驻, 不走堆
			VirtualLock(静态基线, 想要);               // 锁住, 别被换页
			SIZE_T 拷到 = 0;
			for (SIZE_T 偏移 = 0; 偏移 < 想要; 偏移 += 页大小) {
				MEMORY_BASIC_INFORMATION 信息{};
				if (!VirtualQuery(起 + 偏移, &信息, sizeof(信息))) continue;
				if (信息.State != MEM_COMMIT) continue;
				const DWORD 保护 = 信息.Protect & 0xFF;
				if (保护 != PAGE_EXECUTE && 保护 != PAGE_EXECUTE_READ && 保护 != PAGE_EXECUTE_READWRITE)
					continue;
				SIZE_T 本页 = 页大小;
				if (想要 - 偏移 < 本页) 本页 = 想要 - 偏移;
				__try {
					memcpy(静态基线 + 偏移, 起 + 偏移, 本页);
					拷到 += 本页;
				} __except (EXCEPTION_EXECUTE_HANDLER) {
				}
			}
			if (!拷到) return false;
			g_基线基址 = 起;
			g_基线 = 静态基线;
			g_基线大小 = 想要;
			g_基线就绪 = true;
			return true;
		}

		// ---------------- 状态 ----------------
		inline struct 页备份项 { BYTE* 地址 = nullptr; DWORD 原保护 = 0; } g_页备份[页备份上限];
		inline ULONG g_页备份数 = 0;
		inline volatile LONG g_修字节 = 0;
		inline volatile LONG g_修页 = 0;
		inline volatile LONG g_上次扫描 = 0;
		inline volatile LONG g_IAT异常 = 0;
		inline volatile LONG g_上次验IAT = 0;
		inline volatile LONG g_踩中次数 = 0;
		inline volatile LONG g_被单步次数 = 0;
		inline volatile LONG g_上次重扫 = 0;
		inline volatile LONG g_上次跨线程清Dr = 0;
		inline volatile LONG g_定点修字节 = 0;   // 目标入口被钩时定点修掉的字节
		inline volatile LONG g_修代次 = 0;       // 每修一次 +1
		inline volatile LONG g_已扫代次 = 0;     // 上一次完整扫描时的代次

		inline bool 该跨线程清Dr了() noexcept {
			const DWORD 现在 = GetTickCount();
			const DWORD 上次 = (DWORD)InterlockedCompareExchange(&g_上次跨线程清Dr, 0, 0);
			if ((DWORD)(现在 - 上次) < 跨线程清Dr节流毫秒) return false;
			InterlockedExchange(&g_上次跨线程清Dr, (LONG)现在);
			return true;
		}

		// ---------------- 耗时统计(找卡点用的) ----------------
		inline volatile LONG g_次数 = 0;         // 护栏跑了几次
		inline volatile LONGLONG g_总微秒 = 0;
		inline volatile LONGLONG g_清Dr微秒 = 0;  // 清硬件断点
		inline volatile LONGLONG g_扫页微秒 = 0;  // 软/内存断点扫描
		inline volatile LONGLONG g_重扫微秒 = 0;  // 调用后重扫
		inline volatile LONGLONG g_验IAT微秒 = 0;

		inline LARGE_INTEGER 频率() noexcept {
			LARGE_INTEGER f{};
			QueryPerformanceFrequency(&f);
			return f;
		}
		inline LONGLONG 取计数() noexcept {
			LARGE_INTEGER c{};
			QueryPerformanceCounter(&c);
			return c.QuadPart;
		}
		inline LONG 转微秒(LONGLONG 差) noexcept {
			const LONGLONG f = 频率().QuadPart;
			if (!f) return 0;
			return (LONG)((差 * 1000000) / f);
		}
		inline void 累加(volatile LONGLONG* 桶, LONGLONG 起) noexcept {
			InterlockedExchangeAdd64(桶, 取计数() - 起);
		}

		// ---------------- 软件断点: 代码页 vs 基线 ----------------
		// 返回 NULL = 不在基线覆盖范围
		inline const BYTE* 基线页(BYTE* 页) noexcept {
			if (!g_基线就绪 || !g_基线基址) return nullptr;
			if (页 < g_基线基址) return nullptr;
			const SIZE_T 偏移 = (SIZE_T)(页 - g_基线基址);
			if (偏移 >= g_基线大小) return nullptr;
			return g_基线 + 偏移;
		}

		// 孤立 INT3? —— 上下各 4 字节没有连片 0xCC, 说明不是对齐填充/数据表, 是被塞的断点
		inline bool 是孤立INT3(BYTE* 页, SIZE_T i, SIZE_T 本页) noexcept {
			BYTE 上下文[9];
			for (int k = 0; k < 9; ++k) {
				const ptrdiff_t 位置 = (ptrdiff_t)i + k - 4;
				上下文[k] = (位置 < 0 || (SIZE_T)位置 >= 本页) ? 0x00 : 页[位置];
			}
			if (上下文[4] != 0xCC) return false;
			int 连片 = 0;
			for (int k = 0; k < 9; ++k) {
				if (k == 4) continue;
				if (上下文[k] == 0xCC) ++连片;
			}
			return 连片 == 0;
		}

		// 整页一次性改保护(逐字节 VirtualProtect 会把你卡死)
		// ★ 但先 memcmp 快查一遍: 一个字节没动就根本别碰保护属性 —— 这才是卡不卡的关键
		inline void 扫一页(BYTE* 页, SIZE_T 本页, const BYTE* 原) noexcept {
			if (原) {
				__try {
					if (memcmp(页, 原, 本页) == 0) return;    // 干净, 直接走
				} __except (EXCEPTION_EXECUTE_HANDLER) {
					return;
				}
			}
			DWORD 旧保护 = 0;
			if (!VirtualProtect(页, 本页, PAGE_EXECUTE_READWRITE, &旧保护)) return;
			__try {
				LONG 本页修 = 0;
				for (SIZE_T i = 0; i < 本页; ++i) {
					if (原 && 页[i] == 原[i]) continue;
					if (页[i] != 0xCC && 页[i] != 0xF1) continue;   // 只认断点指令
					if (!原 && 启发式抓孤立INT3 && !是孤立INT3(页, i, 本页)) continue;
					页[i] = 原 ? 原[i] : 0x90;                      // 没基线就填 NOP
					++本页修;
				}
				if (本页修) {
					InterlockedExchangeAdd(&g_修字节, 本页修);
					InterlockedIncrement(&g_修代次);
				}
			} __except (EXCEPTION_EXECUTE_HANDLER) {
			}
			VirtualProtect(页, 本页, 旧保护, &旧保护);
		}

		inline void 扫软件断点(BYTE* 起, BYTE* 止) noexcept {
			SIZE_T 翻页偏移 = 0;
			for (BYTE* 页 = 起; 页 < 止; 页 += 页大小) {
				if (翻页偏移) { --翻页偏移; continue; }      // 刚跳过的整段, 别重复查
				MEMORY_BASIC_INFORMATION 信息{};
				if (!VirtualQuery(页, &信息, sizeof(信息))) break;
				if (信息.State != MEM_COMMIT) {
					const SIZE_T 整段页 = 信息.RegionSize / 页大小;
					翻页偏移 = 整段页 ? 整段页 - 1 : 0;
					continue;
				}
				if (信息.Protect & PAGE_GUARD) continue;
				const DWORD 保护 = 信息.Protect & 0xFF;
				if (保护 != PAGE_EXECUTE && 保护 != PAGE_EXECUTE_READ && 保护 != PAGE_EXECUTE_READWRITE)
					continue;
				// ★ 一大段可执行页只做一次 VirtualQuery: 按 region 走, 别再逐页问系统
				const SIZE_T 段页 = 信息.RegionSize / 页大小;
				翻页偏移 = 段页 ? 段页 - 1 : 0;

				const BYTE* 原 = 基线页(页);
				SIZE_T 本页 = 页大小;
				if ((SIZE_T)(止 - 页) < 本页) 本页 = (SIZE_T)(止 - 页);
				if (原) {
					if ((SIZE_T)(g_基线 + g_基线大小 - 原) < 本页)
						本页 = (SIZE_T)(g_基线 + g_基线大小 - 原);
					if (!本页) break;
					扫一页(页, 本页, 原);              // 有基线: 严格对镜子
				} else if (启发式抓孤立INT3) {
					扫一页(页, 本页, nullptr);         // 没基线: 启发式揪孤立 INT3
				}
			}
		}

		// 快查: 这一页跟基线是否一个字节不差(不查这个就得每页都动保护属性 —— 那才是卡的原因)
		inline bool 页与基线一致(BYTE* 页, SIZE_T 本页) noexcept {
			const BYTE* 原 = 基线页(页);
			if (!原) return false;
			if ((SIZE_T)(g_基线 + g_基线大小 - 原) < 本页) 本页 = (SIZE_T)(g_基线 + g_基线大小 - 原);
			if (!本页) return true;
			__try {
				return memcmp(页, 原, 本页) == 0;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return true;
			}
		}

		// ---------------- 内存断点: 洗 PAGE_GUARD ----------------
		inline void 洗内存断点(BYTE* 起, BYTE* 止) noexcept {
			SIZE_T 翻页偏移 = 0;
			for (BYTE* 页 = 起; 页 < 止; 页 += 页大小) {
				if (翻页偏移) { --翻页偏移; continue; }
				MEMORY_BASIC_INFORMATION 信息{};
				if (!VirtualQuery(页, &信息, sizeof(信息))) break;
				if (信息.State != MEM_COMMIT) {
					const SIZE_T 整段页 = 信息.RegionSize / 页大小;
					翻页偏移 = 整段页 ? 整段页 - 1 : 0;
					continue;
				}
				if (!(信息.Protect & PAGE_GUARD)) {
					// ★ 没 GUARD 的整段直接跳过(GUARD 本来就是极少数页) —— 别每页都动保护属性
					const SIZE_T 段页 = 信息.RegionSize / 页大小;
					翻页偏移 = 段页 ? 段页 - 1 : 0;
					continue;
				}

				const DWORD 原保护 = 信息.Protect & 0xFF;
				const BOOL 可执行 = (原保护 == PAGE_EXECUTE || 原保护 == PAGE_EXECUTE_READ ||
					原保护 == PAGE_EXECUTE_READWRITE || 原保护 == PAGE_EXECUTE_WRITECOPY);
				DWORD 临时 = 0;
				// GUARD 只对可执行页动手 —— 栈/堆的 GUARD 是系统自己在管的, 别碰
				if (!可执行) continue;
				if (!VirtualProtect(页, 页大小, PAGE_EXECUTE_READWRITE, &临时)) continue;
				if (g_页备份数 < 页备份上限) {
					g_页备份[g_页备份数].地址 = 页;
					g_页备份[g_页备份数].原保护 = 信息.Protect;   // 带 GUARD 位的原值
					++g_页备份数;
				}
				InterlockedIncrement(&g_修页);
				if (调试输出) {
					wchar_t 缓冲[160];
					wsprintfW(缓冲, L"[断点清理] 洗掉 GUARD 页 %p (原 0x%X)\n", (void*)页, 信息.Protect);
					OutputDebugStringW(缓冲);
				}
			}
		}

		inline bool 该扫了() noexcept {
			if (!扫描节流毫秒) return true;
			// ★ 白扫也是扫: 上一趟扫完到现在一字节没被修过, 说明这中间没人动过手脚, 直接免了
			const LONG 代次 = (LONG)InterlockedCompareExchange(&g_修代次, 0, 0);
			if (代次 && 代次 == (LONG)InterlockedCompareExchange(&g_已扫代次, 0, 0)) return false;
			const DWORD 现在 = GetTickCount();
			const DWORD 上次 = (DWORD)InterlockedCompareExchange(&g_上次扫描, 0, 0);
			if ((DWORD)(现在 - 上次) < 扫描节流毫秒) return false;
			InterlockedExchange(&g_上次扫描, (LONG)现在);
			return true;
		}

		inline void 扫一遍(bool 强制) noexcept {
			if (!清软件断点 && !清内存断点) return;
			if (!强制 && !该扫了()) return;
			BYTE* 起 = nullptr; BYTE* 止 = nullptr;
			if (!摸模块范围(起, 止)) return;      // 反射调用和栈回溯都在 EXE 这块, 扫它就够
			if (清内存断点) 洗内存断点(起, 止);
			if (清软件断点) 扫软件断点(起, 止);
			InterlockedExchange(&g_已扫代次, (LONG)InterlockedCompareExchange(&g_修代次, 0, 0));
		}

		inline void 装回内存断点() noexcept {
			for (ULONG i = 0; i < g_页备份数; ++i) {
				if (!g_页备份[i].地址) continue;
				DWORD 临时 = 0;
				VirtualProtect(g_页备份[i].地址, 页大小, g_页备份[i].原保护, &临时);
			}
			g_页备份数 = 0;
		}

		// ---------------- 盯临时钩子 ----------------

		// ① 调用返回后重扫(带节流): 抓"调的时候有、调完就想还原"的那种
		inline bool 该重扫了() noexcept {
			const DWORD 现在 = GetTickCount();
			const DWORD 上次 = (DWORD)InterlockedCompareExchange(&g_上次重扫, 0, 0);
			if ((DWORD)(现在 - 上次) < 重扫节流毫秒) return false;
			InterlockedExchange(&g_上次重扫, (LONG)现在);
			return true;
		}

		inline LONG 调用后重扫一遍() noexcept {
			const LONG 之前 = g_修字节;
			扫一遍(true);
			return g_修字节 - 之前;      // 返回这轮又修掉多少字节
		}

		// ② 目标函数入口验毒: 内联钩的三种经典开头
		inline bool 入口被钩(const BYTE* 函数) noexcept {
			if (!函数) return false;
			__try {
				if (函数[0] == 0xCC || 函数[0] == 0xF1) return true;          // 断点
				if (函数[0] == 0xE9) return true;                             // jmp rel32
				if (函数[0] == 0xEB) return true;                             // jmp rel8
				if (函数[0] == 0xE8) return true;                             // call rel32
				if (函数[0] == 0x68 && 函数[5] == 0xC3) return true;          // push addr; ret
				if (函数[0] == 0xFF && 函数[1] == 0x25) return true;          // jmp [rip+disp]
				if (函数[0] == 0x50 && 函数[1] == 0xE9) return true;          // push rax; jmp
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return true;                                                  // 读不了本身就说明有问题
			}
			return false;
		}

		// ②b 拿一条干净基线来判"是不是刚被塞的钩": 基线里干净, 现在带 jmp/断点 => 实锤被钩
		inline bool 入口已被改(const BYTE* 函数, const BYTE* 干净基线, SIZE_T 长度) noexcept {
			if (!函数 || !干净基线 || !长度) return false;
			__try {
				if (memcmp(函数, 干净基线, 长度) == 0) return false;   // 一个字节没动
				// 变了 —— 只看变了的那几个字节里有没有钩子的形状
				for (SIZE_T i = 0; i < 长度; ++i) {
					if (函数[i] == 干净基线[i]) continue;
					if (函数[i] == 0xCC || 函数[i] == 0xF1) return true;
					if (函数[i] == 0xE9 || 函数[i] == 0xEB || 函数[i] == 0xE8) return true;
					return true;                                      // 入口指令流被动过就认
				}
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return true;
			}
			return false;
		}

		// ②c 给函数入口留一份干净基线(第一次见到就留, 之后拿来判"刚被改")
		struct 入口基线项 { const BYTE* 函数 = nullptr; BYTE 字节[32]{}; };
		constexpr ULONG 入口基线上限 = 16;
		inline 入口基线项 g_入口基线[入口基线上限];
		inline ULONG g_入口基线数 = 0;

		inline const 入口基线项* 留入口基线(const BYTE* 函数) noexcept {
			if (!函数) return nullptr;
			for (ULONG i = 0; i < g_入口基线数; ++i)
				if (g_入口基线[i].函数 == 函数) return &g_入口基线[i];
			if (g_入口基线数 >= 入口基线上限) return nullptr;
			__try {
				// 已经带钩的入口不留基线(留在里面等于认了它的钩)
				if (入口被钩(函数)) return nullptr;
				入口基线项& 项 = g_入口基线[g_入口基线数];
				项.函数 = 函数;
				memcpy(项.字节, 函数, sizeof(项.字节));
				++g_入口基线数;
				return &项;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return nullptr;
			}
		}

		// ②d 定点重扫某一页(比全模块重扫便宜一万倍): 专门用来打"临时钩子"
		inline LONG 定点重扫页(BYTE* 页) noexcept {
			const BYTE* 原 = 基线页(页);
			if (!原) return 0;
			SIZE_T 本页 = 页大小;
			if ((SIZE_T)(g_基线 + g_基线大小 - 原) < 本页)
				本页 = (SIZE_T)(g_基线 + g_基线大小 - 原);
			if (!本页) return 0;
			const LONG 之前 = g_修字节;
			扫一页(页, 本页, 原);
			return g_修字节 - 之前;
		}

		inline bool 入口是否被动态改过(const BYTE* 函数) noexcept {
			const 入口基线项* 项 = 留入口基线(函数);
			if (!项) return 入口被钩(函数);
			return 入口已被改(函数, 项->字节, sizeof(项->字节));
		}

		// ②e 目标入口被动过 -> 当场把那一页对镜子重扫一遍(不等节流)
		inline LONG 入口被动就定点修(const BYTE* 函数) noexcept {
			if (!函数) return 0;
			BYTE* 页 = (BYTE*)((uintptr_t)函数 & ~(uintptr_t)(页大小 - 1));
			return 定点重扫页(页);
		}

		// ③ 虚表槽验毒: 传进来的指针是不是还指着同一条指令流
		inline bool 虚表槽被换(void* 槽地址, void* 期望值) noexcept {
			if (!槽地址) return false;
			__try {
				return *(void**)槽地址 != 期望值;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return true;
			}
		}

		// ④ 主模块 IAT 验毒: 每个导入槽跟它 DLL 里的真身对一遍
		inline void 验一遍IAT() noexcept {
			BYTE* 基址 = 取主模块基址();
			if (!基址) return;
			__try {
				const auto* dos = (const IMAGE_DOS_HEADER*)基址;
				if (dos->e_magic != IMAGE_DOS_SIGNATURE) return;
				const auto* nt = (const IMAGE_NT_HEADERS*)(基址 + dos->e_lfanew);
				if (nt->Signature != IMAGE_NT_SIGNATURE) return;
				const IMAGE_DATA_DIRECTORY& 目录 = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
				if (!目录.VirtualAddress) return;
				const auto* 描述 = (const IMAGE_IMPORT_DESCRIPTOR*)(基址 + 目录.VirtualAddress);
				for (; 描述->Name; ++描述) {
					const char* dll名 = (const char*)(基址 + 描述->Name);
					HMODULE 提供者 = GetModuleHandleA(dll名);
					if (!提供者) continue;
					if (!描述->FirstThunk) continue;
					auto* 槽 = (IMAGE_THUNK_DATA*)(基址 + 描述->FirstThunk);
					const auto* 名表 = 描述->OriginalFirstThunk ? (const IMAGE_THUNK_DATA*)(基址 + 描述->OriginalFirstThunk) : nullptr;
					for (; 槽->u1.Function; ++槽) {
						if (!名表) continue;
						const SIZE_T 序号 = (SIZE_T)(槽 - (IMAGE_THUNK_DATA*)(基址 + 描述->FirstThunk));
						const IMAGE_THUNK_DATA 名字项 = 名表[序号];
						if (!名字项.u1.AddressOfData) continue;
						if (IMAGE_SNAP_BY_ORDINAL(名字项.u1.Ordinal)) continue;
						const auto* 导入名 = (const IMAGE_IMPORT_BY_NAME*)(基址 + 名字项.u1.AddressOfData);
						void* 真身 = (void*)GetProcAddress(提供者, (const char*)导入名->Name);
						if (!真身) continue;
						void* 现槽 = (void*)(uintptr_t)槽->u1.Function;
						if (现槽 == 真身) continue;
						// 被换过了 —— 报出来, 要不要还原自己决定(有些是 Detours 自己的转跳, 别乱动)
						InterlockedIncrement(&g_IAT异常);
						if (调试输出) {
							wchar_t 缓冲[320];
							wsprintfW(缓冲, L"[断点清理] IAT 被换: %hs!%hs 槽=%p 现在=%p 真身=%p\n",
								dll名, (const char*)导入名->Name, (void*)槽, 现槽, 真身);
							OutputDebugStringW(缓冲);
						}
					}
				}
			} __except (EXCEPTION_EXECUTE_HANDLER) {
			}
		}

		inline bool 该验IAT了() noexcept {
			const DWORD 现在 = GetTickCount();
			const DWORD 上次 = (DWORD)InterlockedCompareExchange(&g_上次验IAT, 0, 0);
			if ((DWORD)(现在 - 上次) < 验IAT节流毫秒) return false;
			InterlockedExchange(&g_上次验IAT, (LONG)现在);
			return true;
		}

		// ⑤ 调完查自己有没有被"踩中"的痕迹
		inline bool 查自身痕迹(LONG* 命中位 = nullptr, bool* 单步位 = nullptr) noexcept {
			CONTEXT 上下文{};
			memset(&上下文, 0, sizeof(上下文));
			上下文.ContextFlags = CONTEXT_DEBUG_REGISTERS | CONTEXT_CONTROL;
			if (!GetThreadContext((HANDLE)(intptr_t)-2, &上下文)) return false;
			bool 有 = false;
			if (上下文.Dr6 & 0x0F) {                       // B0-B3: 哪个硬件断点真被踩了
				有 = true;
				if (命中位) *命中位 = (LONG)(上下文.Dr6 & 0x0F);
				上下文.Dr6 &= ~0x0Full;                    // 抹掉命中位, 免得粘连
				SetThreadContext((HANDLE)(intptr_t)-2, &上下文);
				InterlockedIncrement(&g_踩中次数);
			}
			if (上下文.EFlags & 0x100) {                   // TF: 有人在单步你
				有 = true;
				if (单步位) *单步位 = true;
				InterlockedIncrement(&g_被单步次数);
			}
			return 有;
		}

	} // namespace 内部

	// ============ 对外接口 ============

	// ★★ 尽早调(最好 DllMain 里): 抓一份没被下断的干净代码基线
	inline bool 锁定基线() noexcept { return 内部::抓基线(); }

	// 手动清一次, 不节流
	inline void 清理全部断点() noexcept {
		if (清硬件断点) 硬件断点::清理全部线程();
		内部::扫一遍(true);
	}

	inline LONG 已修字节数() noexcept { return 内部::g_修字节; }
	inline LONG 已洗页数() noexcept { return 内部::g_修页; }
	inline bool 基线就绪() noexcept { return 内部::g_基线就绪; }
	inline LONG IAT异常数() noexcept { return 内部::g_IAT异常; }
	inline LONG 被踩中次数() noexcept { return 内部::g_踩中次数; }
	inline LONG 被单步次数() noexcept { return 内部::g_被单步次数; }

	// 目标验毒: 落调用前先看看这条路有没有被人动过手脚
	//   目标函数入口被塞了 jmp/断点(或入口指令流被改过) -> true
	//   判法: 第一次见到留一条干净基线, 之后对基线比 —— 所以正常就是一行裸 jmp 的
	//   ProcessEvent 桩不会被误判成钩子
	inline bool 目标被钩(void* 目标函数) noexcept {
		if (!验目标入口) return false;
		if (!内部::入口是否被动态改过((const BYTE*)目标函数)) return false;
		// 被动过就当场定点重扫那一页(便宜), 顺手把钩子拆了
		InterlockedExchangeAdd(&内部::g_定点修字节, 内部::入口被动就定点修((const BYTE*)目标函数));
		return true;
	}
	inline bool 虚表被动(void* 槽地址, void* 期望值) noexcept {
		if (!验虚表槽) return false;
		return 内部::虚表槽被换(槽地址, 期望值);
	}
	// 纯静态判毒(不看基线): 只要开头是钩子形状就报警。自己拿捏要不要用
	inline bool 入口像钩子(void* 目标函数) noexcept {
		return 内部::入口被钩((const BYTE*)目标函数);
	}
	// 查询用(不节流)
	inline void 手动验IAT() noexcept { if (验IAT) 内部::验一遍IAT(); }

	// 耗时报告: 把统计耗时打开跑一会儿, 调这个拿数字。
	// 缓冲区自己给(建议 256 宽字符), 返回是否成功
	inline bool 耗时报告(wchar_t* 缓冲, int 缓冲字数) noexcept {
		if (!缓冲 || 缓冲字数 < 64) return false;
		const LONG 次数 = 内部::g_次数;
		if (!次数) {
			wsprintfW(缓冲, L"[断点清理] 还没统计数据(先把 统计耗时 打开)");
			return true;
		}
		const LONGLONG f = 内部::频率().QuadPart;
		const LONG 总 = 内部::转微秒(内部::g_总微秒);
		const LONG 清Dr = 内部::转微秒(内部::g_清Dr微秒);
		const LONG 扫页 = 内部::转微秒(内部::g_扫页微秒);
		const LONG 重扫 = 内部::转微秒(内部::g_重扫微秒);
		const LONG 验IAT = 内部::转微秒(内部::g_验IAT微秒);
		wsprintfW(缓冲,
			L"[断点清理] %ld 次调用 | 平均 %ld.%02ld ms | 均摊: 清Dr %ld.%02ld / 扫页 %ld.%02ld / 重扫 %ld.%02ld / 验IAT %ld.%02ld ms",
			次数,
			总 / 次数, (总 % 次数) * 100 / 次数,
			清Dr / 次数, (清Dr % 次数) * 100 / 次数,
			扫页 / 次数, (扫页 % 次数) * 100 / 次数,
			重扫 / 次数, (重扫 % 次数) * 100 / 次数,
			验IAT / 次数, (验IAT % 次数) * 100 / 次数);
		(void)f;
		return true;
	}
	inline void 清零统计() noexcept {
		InterlockedExchange(&内部::g_次数, 0);
		InterlockedExchange64(&内部::g_总微秒, 0);
		InterlockedExchange64(&内部::g_清Dr微秒, 0);
		InterlockedExchange64(&内部::g_扫页微秒, 0);
		InterlockedExchange64(&内部::g_重扫微秒, 0);
		InterlockedExchange64(&内部::g_验IAT微秒, 0);
	}

	// ============================================================
	//  护栏: 落反射调用前把三类断点全清
	//  用法:  { 断点清理::护栏 栏;  反射调用(...); }
	// ============================================================
	struct 护栏 {
		硬件断点::护栏 硬件栏;      // 成员: 析构时它自己把 Dr 装回去
		bool 已处理 = false;
		LONG 调用后补修 = 0;         // 这次调用窗口里又被塞了多少字节(返回后才抓到的)
		bool 被踩中 = false;         // 这次调用踩到硬件断点了
		bool 被单步 = false;         // 这次调用有人在单步
		LONGLONG 进时 = 0;
		LONGLONG 清Dr起 = 0;
		LONGLONG 扫页起 = 0;
		LONG 本次耗时微秒 = 0;

		// ★ 默认只清当前线程: 跨线程挂起全进程是毫秒级开销, 一帧几十次调用会被它吃光
		explicit 护栏(bool 跨线程硬件 = 断点清理::跨线程清Dr, bool 强制跨线程 = false) noexcept
			: 硬件栏(跨线程硬件, 断点清理::清硬件断点) {
			if (统计耗时) 进时 = 内部::取计数();
			if (统计耗时) 清Dr起 = 内部::取计数();
			// ★ 跨线程那趟(枚举+挂起+清)按节流走, 不是每次调用都干
			if (跨线程硬件 && 清硬件断点 && (强制跨线程 || 内部::该跨线程清Dr了())) {
				硬件断点::清理全部线程();
			} else if (清硬件断点) {
				硬件断点::清理当前线程();       // 快路: 两次 syscall, 微秒级
			}
			if (统计耗时) 内部::累加(&内部::g_清Dr微秒, 清Dr起);
			if (统计耗时) 扫页起 = 内部::取计数();
			内部::扫一遍(false);
			if (统计耗时) 内部::累加(&内部::g_扫页微秒, 扫页起);
			if (验IAT && 内部::该验IAT了()) {
				const LONGLONG 起 = 内部::取计数();
				内部::验一遍IAT();
				if (统计耗时) 内部::累加(&内部::g_验IAT微秒, 起);
			}
			已处理 = true;
		}

		~护栏() noexcept {
			// ★ 盯着临时钩子: 调用窗口里被塞过东西, 返回重扫抓回来(带节流)
			if (调用后重扫 && 内部::该重扫了()) {
				const LONGLONG 起 = 内部::取计数();
				调用后补修 = 内部::调用后重扫一遍();
				if (统计耗时) 内部::累加(&内部::g_重扫微秒, 起);
			}
			if (查踩中痕迹) {
				LONG 命中 = 0;
				bool 单步 = false;
				内部::查自身痕迹(&命中, &单步);
				被踩中 = (命中 != 0);
				被单步 = 单步;
			}
			if (调用后恢复内存断点) 内部::装回内存断点();
			// 硬件断点的装回由 硬件栏 自己的析构负责(详见 硬件断点.h)
			if (统计耗时 && 进时) {
				本次耗时微秒 = 内部::转微秒(内部::取计数() - 进时);
				InterlockedIncrement(&内部::g_次数);
				InterlockedExchangeAdd64(&内部::g_总微秒, 内部::取计数() - 进时);
			}
		}

		护栏(const 护栏&) = delete;
		护栏& operator=(const 护栏&) = delete;
	};

} // namespace 断点清理

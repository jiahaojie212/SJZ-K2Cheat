#pragma once
// ============================================================
//  硬件断点清理 —— 反射调用专用
//  反射调用(UObject::处理事件 -> 虚表 ProcessEvent)前先把 Dr0-Dr3 + Dr7 全清了,
//  免得反作弊/调试器架在 GObjects / GNames / ProcessEvent 上的硬件断点被踩到。
//  调完原样装回去, 表面无痕。
//  只动调试寄存器, 不碰任何其它东西。
// ============================================================
#include <windows.h>
#include <tlhelp32.h>
#include <intrin.h>
#include <cstdint>

namespace 硬件断点 {

	// 要不要连别的线程一起清。断点是"哪个线程执行到那个地址"才触发,
	// 反射调用跑在当前线程, 自己线程才是命门。false = 只清自己(微秒级)。
	inline bool 清理所有线程 = true;
	// 调完把断点原样装回去(反作弊回头查 Dr 还是原值)
	inline bool 调用后恢复 = true;

	namespace 内部 {

		constexpr ULONG 线程上限 = 256;

		inline DWORD 当前线程号() noexcept {
			__try {
				return (DWORD)__readgsqword(0x48);   // TEB->ClientId.UniqueThread
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return 0;
			}
		}

		inline bool 有断点(const CONTEXT& 上下文) noexcept {
			return (上下文.Dr0 | 上下文.Dr1 | 上下文.Dr2 | 上下文.Dr3 | (上下文.Dr7 & 0xFF)) != 0;
		}
		inline void 抹掉(CONTEXT& 上下文) noexcept {
			上下文.Dr0 = 上下文.Dr1 = 上下文.Dr2 = 上下文.Dr3 = 0;
			上下文.Dr6 = 0;    // 陈旧命中位一起抹
			上下文.Dr7 = 0;    // 全量清(LEN/RW/LE/GE 都在里面)
		}

		inline bool 取上下文(HANDLE 线程, CONTEXT& 上下文) noexcept {
			memset(&上下文, 0, sizeof(上下文));
			上下文.ContextFlags = CONTEXT_DEBUG_REGISTERS;
			return GetThreadContext(线程, &上下文) != FALSE;
		}
		inline bool 设上下文(HANDLE 线程, const CONTEXT& 上下文) noexcept {
			CONTEXT 副本 = 上下文;
			副本.ContextFlags = CONTEXT_DEBUG_REGISTERS;
			return SetThreadContext(线程, &副本) != FALSE;
		}

		struct 备份项 { HANDLE 线程 = nullptr; CONTEXT 上下文{}; };
		inline 备份项 g_备份[线程上限];
		inline ULONG g_备份数 = 0;
		// 当前线程自己的断点备份(反射调用就跑在这个线程上)
		inline CONTEXT g_自身备份{};
		inline bool g_自身有断点 = false;
		inline volatile LONG g_锁 = 0;

		inline void 上锁() noexcept { while (InterlockedExchange(&g_锁, 1) != 0) YieldProcessor(); }
		inline void 解锁() noexcept { InterlockedExchange(&g_锁, 0); }

		// 备下自己的断点并清掉
		inline void 记清自身() noexcept {
			g_自身有断点 = false;
			if (!取上下文((HANDLE)(intptr_t)-2, g_自身备份)) return;
			if (!有断点(g_自身备份)) return;
			g_自身有断点 = true;
			CONTEXT 清空 = g_自身备份;
			抹掉(清空);
			设上下文((HANDLE)(intptr_t)-2, 清空);
		}

		// 装回自己的断点
		inline void 装回自身() noexcept {
			if (!g_自身有断点) return;
			CONTEXT 现在{};
			if (!取上下文((HANDLE)(intptr_t)-2, 现在)) return;
			// 只在当前 Dr 还是净的时候装回, 别覆盖调用期间新架的断点
			if (!有断点(现在)) 设上下文((HANDLE)(intptr_t)-2, g_自身备份);
		}

	} // namespace 内部

	// 清当前线程的硬件断点(快路, 不挂任何线程)
	inline void 清理当前线程() noexcept {
		CONTEXT 上下文{};
		if (!内部::取上下文((HANDLE)(intptr_t)-2, 上下文)) return;   // NtCurrentThread
		if (!内部::有断点(上下文)) return;
		内部::抹掉(上下文);
		内部::设上下文((HANDLE)(intptr_t)-2, 上下文);
	}

	// 清全进程所有线程的硬件断点: 逐个挂起 -> 清 -> 立刻放行
	inline void 清理全部线程() noexcept {
		const DWORD 自身 = 内部::当前线程号();
		HANDLE 快照 = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (快照 == INVALID_HANDLE_VALUE) { 清理当前线程(); return; }

		THREADENTRY32 项{};
		项.dwSize = sizeof(项);
		if (Thread32First(快照, &项)) {
			do {
				if (项.dwSize < FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(DWORD)) continue;
				if (项.th32OwnerProcessID != GetCurrentProcessId()) continue;
				if (自身 && 项.th32ThreadID == 自身) continue;   // 自己走快路
				HANDLE 线程 = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_QUERY_INFORMATION,
					FALSE, 项.th32ThreadID);
				if (!线程) continue;
				if (SuspendThread(线程) != (DWORD)-1) {
					CONTEXT 上下文{};
					if (内部::取上下文(线程, 上下文) && 内部::有断点(上下文)) {
						内部::抹掉(上下文);
						内部::设上下文(线程, 上下文);
					}
					ResumeThread(线程);
				}
				CloseHandle(线程);
			} while (Thread32Next(快照, &项));
		}
		CloseHandle(快照);
		清理当前线程();
	}

	// ============================================================
	//  护栏: 反射调用前清断点, 调用后原样装回
	//  用法:  { 硬件断点::护栏 栏;  反射调用(...); }
	// ============================================================
	struct 护栏 {
		ULONG 挂起数 = 0;
		bool 已处理 = false;
		bool 启用 = true;

		explicit 护栏(bool 全部线程 = 硬件断点::清理所有线程, bool 启用 = true) noexcept {
			this->启用 = 启用;
			if (!启用) { 已处理 = true; return; }
			内部::上锁();
			const DWORD 自身 = 内部::当前线程号();
			if (全部线程) {
				HANDLE 快照 = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
				if (快照 != INVALID_HANDLE_VALUE) {
					THREADENTRY32 项{};
					项.dwSize = sizeof(项);
					if (Thread32First(快照, &项)) {
						do {
							if (项.dwSize < FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(DWORD)) continue;
							if (项.th32OwnerProcessID != GetCurrentProcessId()) continue;
							if (自身 && 项.th32ThreadID == 自身) continue;
							if (内部::g_备份数 >= 内部::线程上限) break;
							HANDLE 线程 = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_QUERY_INFORMATION,
								FALSE, 项.th32ThreadID);
							if (!线程) continue;
							if (SuspendThread(线程) == (DWORD)-1) { CloseHandle(线程); continue; }
							CONTEXT 上下文{};
							if (!内部::取上下文(线程, 上下文)) { ResumeThread(线程); CloseHandle(线程); continue; }
							if (内部::有断点(上下文)) {
								const ULONG 槽 = 内部::g_备份数;
								内部::g_备份[槽].线程 = 线程;
								内部::g_备份[槽].上下文 = 上下文;
								++内部::g_备份数;
								内部::抹掉(上下文);
								内部::设上下文(线程, 上下文);
							} else {
								// 本来就没断点, 不用留备份, 直接放行
								ResumeThread(线程);
								CloseHandle(线程);
							}
						} while (Thread32Next(快照, &项));
					}
					CloseHandle(快照);
				}
			}
			挂起数 = 内部::g_备份数;
			内部::记清自身();     // 自己最后清, 清完立刻进调用
			已处理 = true;
		}

		~护栏() noexcept {
			if (!启用) return;
			装回();
			内部::解锁();
		}

		护栏(const 护栏&) = delete;
		护栏& operator=(const 护栏&) = delete;

		void 装回() noexcept {
			if (已处理) return;
			已处理 = true;
			if (硬件断点::调用后恢复) 内部::装回自身();
			for (ULONG i = 0; i < 挂起数; ++i) {
				HANDLE 线程 = 内部::g_备份[i].线程;
				if (!线程) continue;
				if (硬件断点::调用后恢复) 内部::设上下文(线程, 内部::g_备份[i].上下文);
				ResumeThread(线程);
				CloseHandle(线程);
				内部::g_备份[i].线程 = nullptr;
			}
			内部::g_备份数 = 0;
			挂起数 = 0;
		}
	};

} // namespace 硬件断点

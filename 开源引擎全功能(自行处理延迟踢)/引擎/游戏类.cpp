#include "预编译头.h"
#include "游戏类.h"
#include "../断点清理.h"
#include <intrin.h>
#include <type_traits>
#pragma intrinsic(_AddressOfReturnAddress)
void UE::FName::追加字符串(const FName* 名称, wchar_t* 输出缓冲) {
	if (追加字符串函数 == nullptr || !名称 || !输出缓冲) {
		if (追加字符串函数 == nullptr) 初始化内部();
		if (追加字符串函数 && 名称 && 输出缓冲)
			reinterpret_cast<void(*)(const FName*, wchar_t*)>(追加字符串函数)(名称, 输出缓冲);
		return;
	}
	reinterpret_cast<void(*)(const FName*, wchar_t*)>(追加字符串函数)(名称, 输出缓冲);
}
bool UE::UObject::是否类(UClass* 类型类) {
	return 取类()->是否子类(类型类);
}
UE::UObject* UE::UObject::静态查找对象(UClass* 类, UObject* 外部, const wchar_t* 名称, bool 精确类) {
	return 快速查找对象(名称, 类, 精确类);
}
struct 处理事件锁_t {
	CRITICAL_SECTION cs;
	处理事件锁_t() { InitializeCriticalSection(&cs); }
	~处理事件锁_t() { DeleteCriticalSection(&cs); }
};
static 处理事件锁_t 处理事件锁;
// 护栏必须活在 __try 之外(SEH 函数里带析构对象直接 C2712 编译不过),
// 所以真正落调用单独关一个小函数里
static void 执行反射调用(UE::UObject* 对象, UE::UFunction* 函数, void* 参数) {
	void* 虚表 = 内存辅助::读字段<void*>(对象, 0);
	const int32 PE索引 = 偏移::ProcessEvent::取PEIndex();
	void* 槽地址 = 虚表 ? 内存辅助::取字段指针<void*>(虚表, PE索引 * sizeof(void*)) : nullptr;
	void* 处理事件函数 = 槽地址 ? *(void**)槽地址 : nullptr;
	if (!处理事件函数) return;
	// ★ 落调用前: 硬件断点(Dr0-Dr3/Dr6/Dr7) + 软件断点(代码页 0xCC) + 内存断点(PAGE_GUARD) 全清
	//   ACE 靠 0xCC/GUARD 踩进 VEH 读返回地址对账的路, 到这儿就断了
	断点清理::护栏 断点护栏;
	// ★ 有人想"临时下钩、完事还原"? 落调用前当场验一遍, 被钩就不往里跳
	if (断点清理::目标被钩(处理事件函数)) return;
	if (断点清理::虚表被动(槽地址, 处理事件函数)) return;
	reinterpret_cast<void(*)(UE::UObject*, UE::UFunction*, void*)>(处理事件函数)(对象, 函数, 参数);
}
static void 处理事件_内部(UE::UObject* 对象, UE::UFunction* 函数, void* 参数) {
	__try {
		auto* 标志指针 = 函数->取函数标志地址();
		if (!标志指针) return;
		if (!UE::UObject::GObjects自洽(对象)) return;
		auto& 标志 = *标志指针;
		auto 旧标志 = 标志;
		标志 |= 0x400;
		执行反射调用(对象, 函数, 参数);
		标志 = 旧标志;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
}
struct 锁助手 {
	CRITICAL_SECTION* cs;
	锁助手(CRITICAL_SECTION* 锁) : cs(锁) { EnterCriticalSection(cs); }
	~锁助手() { LeaveCriticalSection(cs); }
};
void UE::UObject::处理事件(UFunction* 函数, void* 参数) {
	if (!函数 || !this) return;
	锁助手 持锁(&处理事件锁.cs);
	处理事件_内部(this, 函数, 参数);
}

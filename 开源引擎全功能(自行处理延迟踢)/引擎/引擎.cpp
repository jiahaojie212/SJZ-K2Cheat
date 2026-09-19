#include "引擎.h"
#include "明文投影.h"
#include "../反调试.h"
#include "../轻量混淆.h"
#include "../字符串加密.h"
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include "../obf/obf.h"
#include <cstdio>
#include "工具/include/detours/detours.h"
全局数据 游戏全局数据;
自瞄系统 自瞄;
static bool 战斗模式开关 = false;
void 全局数据::初始化骨骼名() {
	this->骨骼数据.头 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"Head")));
	this->骨骼数据.脖子 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"Neck")));
	this->骨骼数据.胯 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"Hips")));
	this->骨骼数据.右肩 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"RightShoulder")));
	this->骨骼数据.右臂 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"RightArm")));
	this->骨骼数据.右前臂 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"RightForeArm")));
	this->骨骼数据.右手 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"RightHand")));
	this->骨骼数据.右大腿 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"RightUpLeg")));
	this->骨骼数据.右小腿 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"RightLeg")));
	this->骨骼数据.右脚 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"RightFoot")));
	this->骨骼数据.左肩 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"LeftShoulder")));
	this->骨骼数据.左臂 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"LeftArm")));
	this->骨骼数据.左前臂 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"LeftForeArm")));
	this->骨骼数据.左手 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"LeftHand")));
	this->骨骼数据.左大腿 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"LeftUpLeg")));
	this->骨骼数据.左小腿 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"LeftLeg")));
	this->骨骼数据.左脚 = UE::UKismetStringLibrary::字符串转名称(UE::FString(宽加密串(L"LeftFoot")));
	this->骨骼数据.骨架[骨骼编号::头] = this->骨骼数据.头;
	this->骨骼数据.骨架[骨骼编号::脖子] = this->骨骼数据.脖子;
	this->骨骼数据.骨架[骨骼编号::胯] = this->骨骼数据.胯;
	this->骨骼数据.骨架[骨骼编号::右肩] = this->骨骼数据.右肩;
	this->骨骼数据.骨架[骨骼编号::右臂] = this->骨骼数据.右臂;
	this->骨骼数据.骨架[骨骼编号::右前臂] = this->骨骼数据.右前臂;
	this->骨骼数据.骨架[骨骼编号::右手] = this->骨骼数据.右手;
	this->骨骼数据.骨架[骨骼编号::右大腿] = this->骨骼数据.右大腿;
	this->骨骼数据.骨架[骨骼编号::右小腿] = this->骨骼数据.右小腿;
	this->骨骼数据.骨架[骨骼编号::右脚] = this->骨骼数据.右脚;
	this->骨骼数据.骨架[骨骼编号::左肩] = this->骨骼数据.左肩;
	this->骨骼数据.骨架[骨骼编号::左臂] = this->骨骼数据.左臂;
	this->骨骼数据.骨架[骨骼编号::左前臂] = this->骨骼数据.左前臂;
	this->骨骼数据.骨架[骨骼编号::左手] = this->骨骼数据.左手;
	this->骨骼数据.骨架[骨骼编号::左大腿] = this->骨骼数据.左大腿;
	this->骨骼数据.骨架[骨骼编号::左小腿] = this->骨骼数据.左小腿;
	this->骨骼数据.骨架[骨骼编号::左脚] = this->骨骼数据.左脚;
}
bool 全局数据::世界坐标转屏幕点(const FVector& 位置, FVector2D* 坐标) {
	if (明文投影::S.可用) {
		if (明文投影::世界转屏幕(位置, 坐标)) return true;
		return false;
	}
	return 玩家控制器->世界坐标转屏幕(位置, 坐标[0]);
}
bool 全局数据::世界坐标转屏幕矩形(const FVector& 位置, FVector2D* 坐标) {
	if (明文投影::S.可用) {
		if (世界坐标转屏幕点(位置 + FVector(0, 0, 100.0f), &坐标[0]) &&
			世界坐标转屏幕点(位置 + FVector(0, 0, -100.0f), &坐标[1])) {
			float 宽度 = (坐标[1].Y - 坐标[0].Y) / 2.0f;
			坐标[0].X -= 宽度 / 2.0f;
			坐标[1].X = 坐标[0].X + 宽度;
			return true;
		}
		return false;
	}
	if (玩家控制器->世界坐标转屏幕(位置 + FVector(0, 0, 100.0f), 坐标[0]) && 玩家控制器->世界坐标转屏幕(位置 + FVector(0, 0, -100.0f), 坐标[1])) {
		float 宽度 = (坐标[1].Y - 坐标[0].Y) / 2.0f;
		坐标[0].X -= 宽度 / 2.0f;
		坐标[1].X = 坐标[0].X + 宽度;
		return true;
	}
	return false;
}
std::string 全局数据::取单位池名(ADFMCharacter* 角色) {
	auto 池名 = 角色->取PoolName();
	uint64_t 池名索引 = *(uint64_t*)&池名;
	std::string 池名文本;
	if (池名索引 == 0) {
		if (角色->是否类(ADFMRangeTargetCharacter::取静态类())) {
			return 静态加密串(u8"靶场假人");
		}
		return 静态加密串(u8"None");
	}
	auto 缓存项 = 单位池名缓存.find(池名索引);
	if (缓存项 != 单位池名缓存.end()) {
		return 缓存项->second;
	}
	FString 池名缓冲 = 角色->GetPoolChosenName();
	if (池名缓冲.取C字符串() == nullptr) {
		池名文本 = 静态加密串(u8"None");
	}
	else
	{
		池名文本 = 宽转utf8(池名缓冲.取C字符串());
	}
	单位池名缓存.insert({ 池名索引,池名文本 });
	return 池名文本;
}
std::string 全局数据::取英雄名(APlayerState* 玩家状态) {
	auto 英雄编号 = 玩家状态->转换<ADFMPlayerState>()->取HeroId();
	switch ((unsigned char)英雄编号)
	{
	case 25: return 静态加密串(u8"威龙");
	case 26: return 静态加密串(u8"骇爪");
	case 27: return 静态加密串(u8"峰医");
	case 28: return 静态加密串(u8"露娜");
	case 29: return 静态加密串(u8"牧羊人");
	case 30: return 静态加密串(u8"红狼");
	case 35: return 静态加密串(u8"乌鲁鲁");
	case 36: return 静态加密串(u8"蛊");
	case 37: return 静态加密串(u8"深蓝");
	case 38: return 静态加密串(u8"无名");
	case 39: return 静态加密串(u8"疾风");
	case 40: return 静态加密串(u8"银翼");
	case 41: return 静态加密串(u8"比特");
	case 42: return 静态加密串(u8"赛伊德");
	case 43: return 静态加密串(u8"老赛小弟");
	case 44: return 静态加密串(u8"老赛小弟");
	case 45: return 静态加密串(u8"蝶");
	case 46: return 静态加密串(u8"回响");
	case 47: return 静态加密串(u8"液氮");
	case 48: return 静态加密串(u8"老太");
	case 49: return 静态加密串(u8"老太小弟");
	case 50: return 静态加密串(u8"老太小弟");
	case 51: return 静态加密串(u8"旅人");
	}
	return std::to_string((unsigned char)英雄编号);
}
std::string 全局数据::取拾取物名(AInventoryPickup* 拾取物) {
	auto 物品编号名 = 拾取物->取InventoryIdName();
	uint64_t 物品编号索引 = *(uint64_t*)&物品编号名;
	if (物品编号索引 == 0) {
		return std::string();
	}
	auto 缓存项 = 拾取名缓存.find(物品编号索引);
	if (缓存项 != 拾取名缓存.end()) {
		return 缓存项->second;
	}
	std::string 物品名;
	FString 显示字符串 = UKismetTextLibrary::文本转字符串(拾取物->GetItemName());
	if (显示字符串.取C字符串()) {
		物品名 = 宽转utf8(显示字符串.取C字符串());
	}
	拾取名缓存.insert({ 物品编号索引,物品名 });
	return 物品名;
}
void 全局数据::清缓存() {
	if (单位池名缓存.size())单位池名缓存.clear();
	if (拾取名缓存.size())拾取名缓存.clear();
}
bool 全局数据::取骨骼网格(ADFMCharacter* 角色, USkeletalMeshComponent* 骨骼网格组件, 骨骼信息* 输出骨骼信息) {
	bool 可见 = false;
	{
		void* 虚表 = 内存辅助::读字段<void*>(骨骼网格组件, 0);
		if (!内存辅助::有效用户地址(虚表)) return false;
	}
	for (size_t i = 0; i < sizeof(骨骼数据.骨架) / sizeof(骨骼数据.骨架[0]); i++)
	{
		输出骨骼信息[i].有效 = false;
		输出骨骼信息[i].位置 = 骨骼网格组件->按名取骨骼位置(骨骼数据.骨架[i]);
		if (输出骨骼信息[i].位置.是否为零()) continue;
		if (输出骨骼信息[i].位置.取模() > 1000000.0f) continue;
		bool 投影成功;
		if (明文投影::S.可用)
			投影成功 = 明文投影::世界转屏幕(输出骨骼信息[i].位置, &输出骨骼信息[i].屏幕);
		else
			投影成功 = 玩家控制器->世界坐标转屏幕(输出骨骼信息[i].位置, 输出骨骼信息[i].屏幕);
		if (!投影成功) continue;
		输出骨骼信息[i].有效 = true;
		if (是否在屏幕内(输出骨骼信息[i].屏幕)) {
			输出骨骼信息[i].可见 = !UKismetSystemLibrary::单线追踪(全局世界, 本地角色, 角色, 相机位置, 输出骨骼信息[i].位置);
			if (输出骨骼信息[i].可见) 可见 = true;
		}
		else {
			输出骨骼信息[i].可见 = false;
		}
	}
	return 可见;
}
bool 全局数据::是否在屏幕内(const FVector2D 世界转屏幕) {
	const float 屏幕边距 = SCALE(100.0f);
	const float 最小X = -屏幕边距;
	const float 最小Y = -屏幕边距;
	const float 最大X = this->屏幕宽 + 屏幕边距;
	const float 最大Y = this->屏幕高 + 屏幕边距;
	return (世界转屏幕.X >= 最小X) &&
		(世界转屏幕.X <= 最大X) &&
		(世界转屏幕.Y >= 最小Y) &&
		(世界转屏幕.Y <= 最大Y);
}
uint32_t 全局数据::编号取色(int 编号) {
	switch (编号 % 12) {
	case 0:  return 颜色32(236, 94, 135, 230);
	case 1:  return 颜色32(249, 136, 126, 230);
	case 2:  return 颜色32(181, 71, 135, 230);
	case 3:  return 颜色32(102, 187, 106, 230);
	case 4:  return 颜色32(79, 193, 233, 230);
	case 5:  return 颜色32(103, 128, 159, 230);
	case 6:  return 颜色32(171, 130, 255, 230);
	case 7:  return 颜色32(232, 121, 249, 230);
	case 8:  return 颜色32(131, 96, 195, 230);
	case 9:  return 颜色32(220, 148, 111, 230);
	case 10: return 颜色32(102, 153, 153, 230);
	case 11: return 颜色32(143, 143, 159, 230);
	default: return 颜色32(143, 143, 159, 230);
	}
}
uint32_t 全局数据::等级取色(int 等级) {
	switch (等级) {
	case 1:
		return 颜色32(245, 245, 245, 255);
	case 2:
		return 颜色32(76, 217, 100, 255);
	case 3:
		return 颜色32(85, 172, 238, 255);
	case 4:
		return 颜色32(214, 137, 255, 255);
	case 5:
		return 颜色32(255, 214, 70, 255);
	case 6:
		return 颜色32(255, 105, 97, 255);
	case 7:
		return 颜色32(255, 40, 40, 255);
	default:
		return 颜色32(220, 220, 220, 230);
	}
}
void 全局数据::画线(const FVector2D& 起点, const FVector2D& 终点, uint32_t 颜色值, float 粗细) {
	原生绘制::画线段(起点.X, 起点.Y, 终点.X, 终点.Y, 颜色值, 粗细);
}
void 全局数据::排队文本(float 字号, const std::string& 文本, FVector2D 位置, uint32_t 颜色, bool 居中) {
	原生绘制::文本(字号, 位置.X, 位置.Y, 颜色, 文本.c_str(), 居中, true);
}
void 全局数据::渲染骨骼网格(const 骨骼信息* 骨骼信息表, uint32_t 可见颜色, uint32_t 不可见颜色, float 粗细) {
	float 段长上限 = 0;
	{
		float 最小X = 1e9f, 最小Y = 1e9f, 最大X = -1e9f, 最大Y = -1e9f;
		int 有效数 = 0;
		for (int i = 0; i < 17; ++i) {
			if (!骨骼信息表[i].有效) continue;
			++有效数;
			if (骨骼信息表[i].屏幕.X < 最小X) 最小X = 骨骼信息表[i].屏幕.X;
			if (骨骼信息表[i].屏幕.Y < 最小Y) 最小Y = 骨骼信息表[i].屏幕.Y;
			if (骨骼信息表[i].屏幕.X > 最大X) 最大X = 骨骼信息表[i].屏幕.X;
			if (骨骼信息表[i].屏幕.Y > 最大Y) 最大Y = 骨骼信息表[i].屏幕.Y;
		}
		if (有效数 >= 2) {
			float 宽 = 最大X - 最小X, 高 = 最大Y - 最小Y;
			段长上限 = sqrtf(宽 * 宽 + 高 * 高) * 2.5f;
		}
	}
	auto 画段 = [&](const 骨骼信息& 起点, const 骨骼信息& 终点) -> void {
		if (!起点.有效 || !终点.有效) return;
		if (段长上限 > 0) {
			float 段宽 = 终点.屏幕.X - 起点.屏幕.X;
			float 段高 = 终点.屏幕.Y - 起点.屏幕.Y;
			if (sqrtf(段宽 * 段宽 + 段高 * 段高) > 段长上限) return;
		}
		画线(起点.屏幕, 终点.屏幕, 起点.可见 ? 可见颜色 : 不可见颜色, 粗细);
	};
	画段(骨骼信息表[骨骼编号::脖子], 骨骼信息表[骨骼编号::胯]);
	画段(骨骼信息表[骨骼编号::脖子], 骨骼信息表[骨骼编号::右肩]);
	画段(骨骼信息表[骨骼编号::右肩], 骨骼信息表[骨骼编号::右臂]);
	画段(骨骼信息表[骨骼编号::右臂], 骨骼信息表[骨骼编号::右前臂]);
	画段(骨骼信息表[骨骼编号::右前臂], 骨骼信息表[骨骼编号::右手]);
	画段(骨骼信息表[骨骼编号::胯], 骨骼信息表[骨骼编号::右大腿]);
	画段(骨骼信息表[骨骼编号::右大腿], 骨骼信息表[骨骼编号::右小腿]);
	画段(骨骼信息表[骨骼编号::右小腿], 骨骼信息表[骨骼编号::右脚]);
	画段(骨骼信息表[骨骼编号::脖子], 骨骼信息表[骨骼编号::左肩]);
	画段(骨骼信息表[骨骼编号::左肩], 骨骼信息表[骨骼编号::左臂]);
	画段(骨骼信息表[骨骼编号::左臂], 骨骼信息表[骨骼编号::左前臂]);
	画段(骨骼信息表[骨骼编号::左前臂], 骨骼信息表[骨骼编号::左手]);
	画段(骨骼信息表[骨骼编号::胯], 骨骼信息表[骨骼编号::左大腿]);
	画段(骨骼信息表[骨骼编号::左大腿], 骨骼信息表[骨骼编号::左小腿]);
	画段(骨骼信息表[骨骼编号::左小腿], 骨骼信息表[骨骼编号::左脚]);
}
文本堆叠管理 物资文本管理器;
float 文本堆叠管理::计算屏幕距离(const FVector2D& a, const FVector2D& b) const {
	float dx = a.X - b.X;
	float dy = a.Y - b.Y;
	return sqrt(dx * dx + dy * dy);
}
void 文本堆叠管理::添加文本(int 字号, const std::string& 文本, const FVector2D& 位置, const uint32_t& 颜色, float 距离) {
	文本列表.push_back({ 字号,文本, 位置, 颜色, 距离 });
}
void 文本堆叠管理::显示堆叠文本(float 最大屏幕范围, float 最大距离) {
	std::vector<std::vector<文本信息>> 聚类表;
	for (const auto& 文本信息 : 文本列表) {
		bool 已加入 = false;
		for (auto& 聚类 : 聚类表) {
			const auto& 参考 = 聚类[0];
			float 屏幕距离 = 计算屏幕距离(文本信息.位置, 参考.位置);
			if (屏幕距离 < 最大屏幕范围 &&
				fabsf(文本信息.距离 - 参考.距离) < 最大距离) {
				聚类.push_back(文本信息);
				已加入 = true;
				break;
			}
		}
		if (!已加入) {
			聚类表.push_back({ 文本信息 });
		}
	}
	for (const auto& 聚类 : 聚类表) {
		if (聚类.empty()) continue;
		FVector2D 平均位置(0, 0);
		for (const auto& 文本信息 : 聚类) {
			平均位置.X += 文本信息.位置.X;
			平均位置.Y += 文本信息.位置.Y;
		}
		平均位置.X /= 聚类.size();
		平均位置.Y /= 聚类.size();
		const float 字号 = 聚类[0].字号;
		float 垂直偏移 = 0;
		for (const auto& 文本信息 : 聚类) {
			游戏全局数据.排队文本(字号, 文本信息.文本.c_str(),
				FVector2D(平均位置.X, 平均位置.Y + 垂直偏移),
				文本信息.颜色, false);
			垂直偏移 += 字号;
		}
	}
	清空();
}
void 文本堆叠管理::清空() {
	文本列表.clear();
}
void Engine::初始化() {
	混淆跳变();
	混淆花指令();
	while (!(游戏窗口 = Engine::查找游戏窗口())) {
		Sleep(500);
	}
	Sleep(4000);
	while (!(游戏全局数据.全局引擎 = UMTAPI_UEngine::GetEngine())) {
		Sleep(100);
	}
	while (!(游戏全局数据.游戏视口 = 游戏全局数据.全局引擎->取GameViewport())) {
		Sleep(100);
	}
	游戏全局数据.初始化骨骼名();
	安装渲染后钩子();
}
HWND Engine::游戏窗口 = nullptr;
HWND Engine::查找游戏窗口() {
	HWND 窗口 = nullptr;
	if (窗口 == nullptr) {
		DWORD 前台进程编号 = 0;
		DWORD 自身进程编号 = GetCurrentProcessId();
		do
		{
			窗口 = ::FindWindowExA(nullptr, 窗口, 静态加密串(u8"UnrealWindow"), nullptr);
			if (窗口 && GetWindowThreadProcessId(窗口, &前台进程编号) && 前台进程编号 == 自身进程编号)break;
		} while (窗口);
	}
	return 窗口;
}
void* Engine::fnPostRender = nullptr;
extern "C" void*& Engine_fnPostRender = Engine::fnPostRender;
extern "C" void (&Engine_IndirectPostRender)(DWORD_PTR) = Engine::IndirectPostRender;
extern "C" void Engine_DetourPostRender();
void Engine::安装渲染后钩子() {
	if (fnPostRender == nullptr) {
		fnPostRender = 游戏全局数据.游戏视口->取虚表函数(偏移::UCanvas::PostRender);
		if (!fnPostRender) return;
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&fnPostRender, &Engine_DetourPostRender);
		if (DetourTransactionCommit() != NO_ERROR) {
			fnPostRender = nullptr;
		}
	}
}
void Engine::IndirectPostRender(DWORD_PTR Rsp) {
	if (fnPostRender == nullptr) {
		安装渲染后钩子();
		return;
	}
	游戏全局数据.游戏视口 = *(UGameViewportClient**)(Rsp - 4 * 8);
	游戏全局数据.游戏画布 = *(UCanvas**)(Rsp - 5 * 8);
	if (!内存辅助::有效用户地址(游戏全局数据.游戏视口) ||
		!内存辅助::有效用户地址(游戏全局数据.游戏画布)) {
		return;
	}
	{
		void* 视口虚表 = 内存辅助::读字段<void*>(游戏全局数据.游戏视口, 0);
		void* 画布虚表 = 内存辅助::读字段<void*>(游戏全局数据.游戏画布, 0);
		if (!内存辅助::有效用户地址(视口虚表) || !内存辅助::有效用户地址(画布虚表)) {
			return;
		}
	}
	原生绘制::设置画布(游戏全局数据.游戏画布);
	原生绘制::初始化();
	明文投影::每帧刷新(游戏全局数据.游戏画布);
	{
		const int 画布宽 = 游戏全局数据.游戏画布->取SizeX();
		const int 画布高 = 游戏全局数据.游戏画布->取SizeY();
		if (画布宽 > 0 && 画布高 > 0) 原生绘制::每帧开始(画布宽, 画布高);
	}
	渲染后回调(游戏全局数据.游戏视口, 游戏全局数据.游戏画布);
	{
		static bool 战斗按键状态 = false;
		bool 战斗按下 = (取按键状态(VK_OEM_3) & 0x8000) != 0;
		if (战斗按下 && !战斗按键状态) {
			战斗模式开关 = !战斗模式开关;
		}
		战斗按键状态 = 战斗按下;
	}
	{
		const char* 模式文本 = 战斗模式开关
			? 静态加密串(u8"战斗模式:开") : 静态加密串(u8"战斗模式:关");
		uint32_t 模式颜色 = 战斗模式开关 ? 颜色32(0, 255, 0, 255) : 颜色32(255, 255, 255, 255);
		原生绘制::文本(SCALE(16.0f), SCALE(12.0f), SCALE(12.0f), 模式颜色, 模式文本, false, true);
	}
}
void Engine::渲染后回调(UGameViewportClient* 视口客户端, UCanvas* 画布) {
	bool 更新成功 = false;
	更新成功 = 更新物体();
	static UWorld* 上帧世界 = nullptr;
	static UCanvas* 上帧壳 = nullptr;
	static DWORD64 切图节拍 = 0;
	if (画布 != 上帧壳 || 游戏全局数据.全局世界 != 上帧世界) {
		if (上帧壳 != nullptr) {
			切图节拍 = GetTickCount64();
			游戏全局数据.切图抑制截止 = 切图节拍 + 5000;
		}
		上帧壳 = 画布;
		上帧世界 = 游戏全局数据.全局世界;
	}
	const bool 切图抑制 = 切图节拍 && GetTickCount64() - 切图节拍 < 800;
	if (更新成功 && !切图抑制) {
		缓存物体();
		渲染单位();
		if (!战斗模式开关) {
			渲染拾取物();
		}
	}
}
namespace 人性化移动 {
	static float 截断(float 值, float 最小, float 最大) {
		return 值 < 最小 ? 最小 : (值 > 最大 ? 最大 : 值);
	}
	static float 平滑五阶(float 进度) {
		进度 = 截断(进度, 0.0f, 1.0f);
		return 进度 * 进度 * 进度 * (进度 * (进度 * 6.0f - 15.0f) + 10.0f);
	}
	struct 运动状态 {
		float 速度X = 0.0f;
		float 速度Y = 0.0f;
		float 累积X = 0.0f;
		float 累积Y = 0.0f;
		float 阶段 = 1.0f;
		float 弧线偏移 = 0.0f;
		float 弧线方向 = 1.0f;
		float 上次误差X = 0.0f;
		float 上次误差Y = 0.0f;
		float 抖动X = 0.0f;
		float 抖动Y = 0.0f;
		float 剩余偏差X = 0.0f;
		float 剩余偏差Y = 0.0f;
		uint64_t 残余节拍 = 0;
		uint32_t 脱帧率 = 0;
		uint64_t 脱帧节拍 = 0;
		float 弧线强度 = 0.0f;
		float 加速度强度 = 1.0f;
		float 抖动强度 = 0.0f;
		uint64_t 上次节拍 = 0;
		uint64_t 抖动节拍 = 0;
		uint64_t 锁定时长 = 0;
		uint64_t 疲劳节拍 = 0;
		float 疲劳系数 = 1.0f;
		float 微过冲X = 0.0f;
		float 微过冲Y = 0.0f;
		float 过冲方向 = 1.0f;
		uint64_t 种子 = 0;
		void 确保种子() {
			if (!种子) {
				种子 = 0x9E3779B97F4A7C15ull ^ GetTickCount64() ^ (reinterpret_cast<uint64_t>(this) << 7);
			}
		}
		uint32_t 下一个随机数() {
			确保种子();
			种子 = 种子 * 6364136223846793005ull + 1442695040888963407ull;
			return static_cast<uint32_t>(种子 >> 32);
		}
		float 随机单位() {
			return (static_cast<float>(下一个随机数() & 0xFFFFu) / 32767.5f) - 1.0f;
		}
		float 随机范围(float 最小, float 最大) {
			return 最小 + (随机单位() + 1.0f) * 0.5f * (最大 - 最小);
		}
		void 开始弧线() {
			阶段 = 0.0f;
			弧线偏移 = 0.0f;
			弧线方向 = (随机单位() < 0.0f) ? -1.0f : 1.0f;
			弧线强度 = 随机范围(0.18f, 0.48f);
			加速度强度 = 随机范围(0.52f, 0.84f);
			抖动强度 = 随机范围(0.08f, 0.32f);
		}
		void 重置() {
			速度X = 0.0f;
			速度Y = 0.0f;
			累积X = 0.0f;
			累积Y = 0.0f;
			阶段 = 1.0f;
			弧线偏移 = 0.0f;
			上次误差X = 0.0f;
			上次误差Y = 0.0f;
			抖动X = 0.0f;
			抖动Y = 0.0f;
			剩余偏差X = 0.0f;
			剩余偏差Y = 0.0f;
			残余节拍 = 0;
			脱帧率 = 0;
			脱帧节拍 = 0;
			上次节拍 = 0;
			抖动节拍 = 0;
			锁定时长 = 0;
			疲劳节拍 = 0;
			疲劳系数 = 1.0f;
			微过冲X = 0.0f;
			微过冲Y = 0.0f;
		}
	};
	static 运动状态 状态实例;
}
void Engine::鼠标移动(int x, int y) {
	if (x == 0 && y == 0) {
		人性化移动::状态实例.重置();
		return;
	}
	人性化移动::运动状态& 运动 = 人性化移动::状态实例;
	const uint64_t 当前节拍 = GetTickCount64();
	uint64_t 间隔毫秒 = (运动.上次节拍 == 0 || 当前节拍 < 运动.上次节拍) ? 16 : (当前节拍 - 运动.上次节拍);
	if (间隔毫秒 == 0) 间隔毫秒 = 1;
	if (间隔毫秒 > 250) 间隔毫秒 = 250;
	const float 原始X = (float)x;
	const float 原始Y = (float)y;
	const float 误差距离 = sqrtf(原始X * 原始X + 原始Y * 原始Y);
	if (误差距离 < 0.35f) {
		运动.重置();
		return;
	}
	const bool 冷启动 = (运动.上次节拍 == 0 || 间隔毫秒 > 140);
	const float 上次距离 = sqrtf(运动.上次误差X * 运动.上次误差X + 运动.上次误差Y * 运动.上次误差Y);
	const float 点积 = 原始X * 运动.上次误差X + 原始Y * 运动.上次误差Y;
	const bool 目标跳变 =
		!冷启动 &&
		上次距离 > 1.0f &&
		((fabsf(原始X - 运动.上次误差X) + fabsf(原始Y - 运动.上次误差Y)) > 140.0f ||
			点积 < -0.20f * 误差距离 * 上次距离);
	if (冷启动 || 目标跳变) {
		运动.速度X *= 目标跳变 ? 0.20f : 0.0f;
		运动.速度Y *= 目标跳变 ? 0.20f : 0.0f;
		运动.累积X = 0.0f;
		运动.累积Y = 0.0f;
		运动.抖动X = 0.0f;
		运动.抖动Y = 0.0f;
		运动.抖动节拍 = 当前节拍;
		运动.开始弧线();
		运动.锁定时长 = 0;
		运动.疲劳系数 = 1.0f;
		运动.微过冲X = 0.0f;
		运动.微过冲Y = 0.0f;
		运动.过冲方向 = (运动.随机单位() < 0.0f) ? -1.0f : 1.0f;
	}
	运动.上次节拍 = 当前节拍;
	const float 间隔缩放 = 人性化移动::截断((float)间隔毫秒 / 16.6f, 0.25f, 2.0f);
	const float 上次阶段 = 运动.阶段;
	运动.锁定时长 += 间隔毫秒;
	{
		const uint64_t 疲劳间隔 = 1000ull;
		if (运动.疲劳节拍 == 0) 运动.疲劳节拍 = 当前节拍;
		if (当前节拍 - 运动.疲劳节拍 >= 疲劳间隔) {
			运动.疲劳节拍 = 当前节拍;
			运动.疲劳系数 = 人性化移动::截断(运动.疲劳系数 + 0.006f, 1.0f, 1.18f);
		}
	}
	const float 平滑X = 人性化移动::截断(0.95f * (1.0f + (运动.疲劳系数 - 1.0f) * 0.5f), 0.05f, 0.95f);
	const float 平滑Y = 平滑X;
	const float 进度步长 = 人性化移动::截断((0.030f + 误差距离 * 0.00040f) * 间隔缩放 / 运动.疲劳系数 * 1.4f, 0.020f, 0.180f);
	运动.阶段 = 人性化移动::截断(运动.阶段 + 进度步长, 0.0f, 1.0f);
	const float 缓动 = 人性化移动::平滑五阶(运动.阶段);
	const float 速度缓动 = 0.24f + 0.54f * 缓动;
	float 目标X = 原始X * 平滑X * 速度缓动 * 间隔缩放;
	float 目标Y = 原始Y * 平滑Y * 速度缓动 * 间隔缩放;
	const float 弧线强度 = 运动.弧线强度;
	if (弧线强度 > 0.01f && 误差距离 > 8.0f) {
		const float 距离倒数 = 1.0f / 误差距离;
		const float 垂直X = -原始Y * 距离倒数;
		const float 垂直Y = 原始X * 距离倒数;
		const float 幅度 = 人性化移动::截断(误差距离 * (0.003f + 0.006f * 弧线强度), 0.0f, 4.5f * 弧线强度);
		const float 当前弧线 = 运动.弧线方向 * 幅度 * 4.0f * 运动.阶段 * (1.0f - 运动.阶段);
		const float 弧线增量 = 当前弧线 - 运动.弧线偏移;
		运动.弧线偏移 = 当前弧线;
		目标X += 垂直X * 弧线增量;
		目标Y += 垂直Y * 弧线增量;
	}
	else {
		运动.弧线偏移 = 0.0f;
	}
	const float 抖动强度 = 运动.抖动强度;
	if (抖动强度 > 0.01f && 误差距离 > 5.0f) {
		const uint64_t 抖动间隔 = 28ull + (运动.下一个随机数() % 45u);
		if (运动.抖动节拍 == 0 || 当前节拍 - 运动.抖动节拍 >= 抖动间隔) {
			运动.抖动节拍 = 当前节拍;
			运动.抖动X = 运动.抖动X * 0.45f + 运动.随机单位() * 0.55f;
			运动.抖动Y = 运动.抖动Y * 0.45f + 运动.随机单位() * 0.55f;
		}
		const float 抖动幅度 = 抖动强度 * 人性化移动::截断(误差距离 / 90.0f, 0.15f, 1.0f) * 0.10f;
		目标X += 运动.抖动X * 抖动幅度;
		目标Y += 运动.抖动Y * 抖动幅度;
	}
	{
		const float 快到位下限 = 3.0f, 快到位上限 = 14.0f;
		if (误差距离 >= 快到位下限 && 误差距离 <= 快到位上限) {
			const float 过冲量 = 运动.过冲方向 * (0.2f + 运动.随机范围(0.0f, 0.4f));
			运动.微过冲X = (原始X / 误差距离) * 过冲量;
			运动.微过冲Y = (原始Y / 误差距离) * 过冲量;
		}
		else {
			运动.微过冲X *= 0.6f;
			运动.微过冲Y *= 0.6f;
		}
		目标X += 运动.微过冲X;
		目标Y += 运动.微过冲Y;
	}
	float 期望距离 = sqrtf(目标X * 目标X + 目标Y * 目标Y);
	const float 平均平滑系数 = (平滑X + 平滑Y) * 0.5f;
	float 最大像素 = (
		2.5f +
		26.0f * 平均平滑系数 * 速度缓动 +
		人性化移动::截断(误差距离 * 0.018f, 0.0f, 6.5f)
		) * 间隔缩放 * 1.6f;
	if (期望距离 > 最大像素) {
		const float 缩放 = 最大像素 / 期望距离;
		目标X *= 缩放;
		目标Y *= 缩放;
		期望距离 = 最大像素;
	}
	const float 加速度强度 = 运动.加速度强度;
	float 速度差X = 目标X - 运动.速度X;
	float 速度差Y = 目标Y - 运动.速度Y;
	const float 速度差距离 = sqrtf(速度差X * 速度差X + 速度差Y * 速度差Y);
	const float 最大速度差 = (0.80f + 9.0f * 加速度强度) * 间隔缩放;
	if (速度差距离 > 最大速度差 && 速度差距离 > 0.001f) {
		const float 缩放 = 最大速度差 / 速度差距离;
		速度差X *= 缩放;
		速度差Y *= 缩放;
	}
	运动.速度X += 速度差X;
	运动.速度Y += 速度差Y;
	const float 速度距离 = sqrtf(运动.速度X * 运动.速度X + 运动.速度Y * 运动.速度Y);
	if (速度距离 > 最大像素 && 速度距离 > 0.001f) {
		const float 缩放 = 最大像素 / 速度距离;
		运动.速度X *= 缩放;
		运动.速度Y *= 缩放;
	}
	if (误差距离 < 6.0f) {
		运动.速度X *= 0.72f;
		运动.速度Y *= 0.72f;
	}
	float 最终X = 运动.速度X + 运动.累积X;
	float 最终Y = 运动.速度Y + 运动.累积Y;
	if (运动.残余节拍 == 0 || 当前节拍 - 运动.残余节拍 >= 70) {
		运动.残余节拍 = 当前节拍;
		const float 残余幅度 = 0.25f + 0.6f * 平均平滑系数;
		运动.剩余偏差X = 运动.剩余偏差X * 0.55f + 运动.随机单位() * 残余幅度;
		运动.剩余偏差Y = 运动.剩余偏差Y * 0.55f + 运动.随机单位() * 残余幅度;
		if (运动.剩余偏差X > 1.2f) 运动.剩余偏差X = 1.2f;
		else if (运动.剩余偏差X < -1.2f) 运动.剩余偏差X = -1.2f;
		if (运动.剩余偏差Y > 1.2f) 运动.剩余偏差Y = 1.2f;
		else if (运动.剩余偏差Y < -1.2f) 运动.剩余偏差Y = -1.2f;
	}
	最终X += 运动.剩余偏差X;
	最终Y += 运动.剩余偏差Y;
	int 整数X = (int)最终X;
	int 整数Y = (int)最终Y;
	运动.累积X = 最终X - (float)整数X;
	运动.累积Y = 最终Y - (float)整数Y;
	if (fabsf(运动.累积X) > 2.0f) 运动.累积X = 0.0f;
	if (fabsf(运动.累积Y) > 2.0f) 运动.累积Y = 0.0f;
	运动.上次误差X = 原始X;
	运动.上次误差Y = 原始Y;
	if (上次阶段 >= 1.0f && 误差距离 > 80.0f && 期望距离 < 1.0f) {
		运动.开始弧线();
	}
	if (整数X != 0 || 整数Y != 0) {
		if (游戏全局数据.玩家控制器 && 游戏全局数据.相机FOV > 1.0f && 游戏全局数据.屏幕宽 > 0) {
			const float 像素转角度 = 游戏全局数据.相机FOV / (float)游戏全局数据.屏幕宽;
			void* 控制器虚表 = 内存辅助::读字段<void*>(游戏全局数据.玩家控制器, 0);
			if (内存辅助::有效用户地址(控制器虚表)) {
				游戏全局数据.玩家控制器->加偏航输入((float)整数X * 像素转角度);
				游戏全局数据.玩家控制器->加俯仰输入((float)整数Y * 像素转角度);
			}
		}
	}
}
void Engine::重置自瞄运动() {
	人性化移动::状态实例.重置();
}
static bool 更新物体_内部() {
	if (!游戏全局数据.游戏视口) {
		if (!(游戏全局数据.全局引擎 = UMTAPI_UEngine::GetEngine())) return false;
		游戏全局数据.游戏视口 = 游戏全局数据.全局引擎->取GameViewport();
		if (!游戏全局数据.游戏视口) return false;
	}
	if (!(游戏全局数据.全局世界 = 游戏全局数据.游戏视口->取World())) return false;
	if (!(游戏全局数据.相机管理器 = UGPWeaponBlueprintLibrary::取玩家相机管理器(游戏全局数据.全局世界))) return false;
	if (!(游戏全局数据.玩家控制器 = UGPWeaponBlueprintLibrary::取玩家控制器(游戏全局数据.全局世界))) return false;
	{
		void* 相机虚表 = 内存辅助::读字段<void*>(游戏全局数据.相机管理器, 0);
		if (!内存辅助::有效用户地址(相机虚表)) return false;
		void* 控制器虚表 = 内存辅助::读字段<void*>(游戏全局数据.玩家控制器, 0);
		if (!内存辅助::有效用户地址(控制器虚表)) return false;
	}
	if (!(游戏全局数据.本地角色 = UGameplayBlueprintHelper::取本地角色(游戏全局数据.全局世界))) return false;
	if (!(游戏全局数据.本地队伍组件 = 游戏全局数据.本地角色->取队伍组件())) return false;
	if (游戏全局数据.游戏画布) {
		void* 画布虚表 = 内存辅助::读字段<void*>(游戏全局数据.游戏画布, 0);
		if (内存辅助::有效用户地址(画布虚表)) {
			游戏全局数据.屏幕宽 = 游戏全局数据.游戏画布->取SizeX();
			游戏全局数据.屏幕高 = 游戏全局数据.游戏画布->取SizeY();
		}
	}
	游戏全局数据.相机旋转 = 游戏全局数据.相机管理器->取相机旋转();
	游戏全局数据.控制旋转 = 游戏全局数据.玩家控制器->取ControlRotation();
	游戏全局数据.相机位置 = 游戏全局数据.相机管理器->取相机位置();
	游戏全局数据.相机FOV = 游戏全局数据.相机管理器->取FOV();
	return true;
}
uint64_t Engine::会话丢失节拍 = 0;
bool Engine::更新物体() {
	const bool 有会话 = 游戏全局数据.全局世界 && 游戏全局数据.本地角色 && 游戏全局数据.相机管理器;
	if (有会话) {
		const bool ok = 更新物体_内部();
		if (!ok) 会话丢失节拍 = GetTickCount64();
		return ok;
	}
	if (会话丢失节拍 && GetTickCount64() - 会话丢失节拍 < 10000) {
		游戏全局数据.清缓存();
		return false;
	}
	if (!游戏全局数据.游戏视口) {
		游戏全局数据.游戏视口 = 游戏全局数据.全局引擎->取GameViewport();
	}
	if (!(游戏全局数据.全局世界 = 游戏全局数据.游戏视口->取World())) {
		游戏全局数据.清缓存();
		return false;
	}
	static UObject* 稳定世界 = nullptr;
	static int 稳定计数 = 0;
	if ((UObject*)游戏全局数据.全局世界 == 稳定世界) {
		if (稳定计数 < 1000) ++稳定计数;
	}
	else {
		稳定世界 = (UObject*)游戏全局数据.全局世界;
		稳定计数 = 0;
	}
	if (稳定计数 < 60) {
		游戏全局数据.清缓存();
		return false;
	}
	EMainFlowState 主流程状态 = UGPScalabilityBlueprintTools::GetMainFlowState();
	if (主流程状态 != EMainFlowState::SafeHouse && 主流程状态 != EMainFlowState::InGame) {
		游戏全局数据.清缓存();
		return false;
	}
	const bool ok = 更新物体_内部();
	if (!ok) 会话丢失节拍 = GetTickCount64();
	return ok;
}
void Engine::缓存物体() {
	static DWORD64 上次时间 = 0;
	auto 当前时间 = GetTickCount64();
	if (上次时间 + 500 > 当前时间)return;
	上次时间 = 当前时间;
	游戏全局数据.单位列表.clear();
	游戏全局数据.拾取列表.clear();
	{
		TArray<AActor*> 单位表;
		UGameplayStatics::取所有同类的参与者(游戏全局数据.全局世界, ADFMCharacter::取静态类(), &单位表);
		for (int i = 0; i < 单位表.数量(); i++)
		{
			if (!单位表.是否有效下标(i)) continue;
			auto 角色 = 单位表[i];
			if (!内存辅助::有效用户地址(角色)) continue;
			{
				void* 虚表 = 内存辅助::读字段<void*>(角色, 0);
				if (!内存辅助::有效用户地址(虚表)) continue;
			}
			if (!UE::UObject::类匹配(角色, ADFMCharacter::取静态类())) continue;
			游戏全局数据.单位列表.push_back(reinterpret_cast<ADFMCharacter*>(角色));
		}
	}
	{
		TArray<AActor*> 互动表;
		UGameplayStatics::取所有同类的参与者(游戏全局数据.全局世界, AInteractorBase::取静态类(), &互动表);
		for (int i = 0; i < 互动表.数量(); i++)
		{
			if (!互动表.是否有效下标(i)) continue;
			auto 参与者 = 互动表[i];
			if (!内存辅助::有效用户地址(参与者)) continue;
			{
				void* 虚表 = 内存辅助::读字段<void*>(参与者, 0);
				if (!内存辅助::有效用户地址(虚表)) continue;
			}
			AInteractorBase* 互动物 = reinterpret_cast<AInteractorBase*>(参与者);
			EMarkingItemType 标记类型 = 互动物->取MarkingItemType();
			if (标记类型 == EMarkingItemType::LootingItem) {
				if (互动物->是否类(AInventoryPickup::取静态类())) {
					游戏全局数据.拾取列表.push_back(互动物->转换<AInventoryPickup>());
				}
			}
			else if (标记类型 == EMarkingItemType::DeadBody) {
				if (互动物->是否类(AInventoryPickup_DeadBody::取静态类())) {
					游戏全局数据.拾取列表.push_back(互动物->转换<AInventoryPickup>());
				}
			}
		}
	}
}
static bool 手持狙击武器(uint64_t 武器编号) {
	return (武器编号 >= 18050000000ULL && 武器编号 <= 18059999999ULL) ||
		(武器编号 >= 18060000000ULL && 武器编号 <= 18069999999ULL);
}
static bool 手持武器禁止自瞄() {
	AGPCharacter* 本地角色 = 游戏全局数据.本地角色;
	if (!本地角色) return false;
	AWeaponBase* 当前武器 = 本地角色->取CacheCurWeapon();
	if (!当前武器) return false;
	uint64_t 武器编号 = 当前武器->取WeaponID();
	switch (武器编号) {
	case 18130000001: case 18130000002:
	case 18150000001:
	case 18080000003: case 18080000005: case 18080000007:
	case 18080000009:
	case 18080000010: case 18080000011: case 18080000012:
	case 18080000013: case 18080000014: case 18080000019:
	case 18080000020:
	case 18080000008:
	case 21010000001: case 21010000003: case 21010000005:
	case 21010000006: case 21010000012: case 21010000017:
	case 21010000018: case 21010000022:
	case 21020300006: case 21020300007: case 21020300009:
	case 21020300010: case 21020300011: case 21020300012:
	case 21020300013: case 21020300017: case 21020300018:
	case 18990000003: case 18990000007: case 18990000008:
		return true;
	default:
		break;
	}
	if (武器编号 >= 18100000000ULL && 武器编号 <= 18100000031ULL) return true;
	return false;
}
void Engine::渲染单位() {
	骨骼信息 骨骼信息表[17];
	自瞄.清空();
	bool 武器禁止自瞄 = 手持武器禁止自瞄();
	static struct 名字缓存项 {
		ADFMCharacter* 角色 = nullptr;
		DWORD          时间戳 = 0;
		std::string    池名, 英雄名;
	};
	static std::vector<名字缓存项> 名字缓存;
	static DWORD 名字缓存清理节拍 = 0;
	DWORD 缓存时钟 = GetTickCount();
	const int 本地队伍编号 = 游戏全局数据.本地队伍组件 ? 游戏全局数据.本地队伍组件->取队伍编号() : 0;
	static struct 组件缓存项 {
		ADFMCharacter*          角色 = nullptr;
		USceneComponent*        根组件 = nullptr;
		UGPTeamComponent*       队伍组件 = nullptr;
		UGPHealthDataComponent* 生命组件 = nullptr;
		USkeletalMeshComponent* 网格组件 = nullptr;
		bool                    AI标志 = false;
		bool                    AI已取 = false;
		DWORD                   时间戳 = 0;
		int                     头盔等级 = 0;
		int                     胸甲等级 = 0;
		DWORD                   装备时间戳 = 0;
	};
	static std::vector<组件缓存项> 组件缓存;
	for (size_t i = 0; i < 游戏全局数据.单位列表.size(); i++)
	{
		ADFMCharacter* 角色 = 游戏全局数据.单位列表[i];
		if (!内存辅助::有效用户地址(角色)) continue;
		{
			void* 虚表 = 内存辅助::读字段<void*>(角色, 0);
			if (!内存辅助::有效用户地址(虚表)) continue;
		}
		if (!UE::UObject::类匹配(角色, ADFMCharacter::取静态类())) continue;
		if (角色 == nullptr || 角色 == 游戏全局数据.本地角色 || 角色->是否死亡())continue;
		组件缓存项* 组件项 = nullptr;
		for (auto& 候选 : 组件缓存) {
			if (候选.角色 == 角色) { 组件项 = &候选; break; }
		}
		if (!组件项) {
			if (组件缓存.size() > 128) 组件缓存.clear();
			组件缓存.push_back(组件缓存项());
			组件项 = &组件缓存.back();
			组件项->角色 = 角色;
		}
		if (组件项->时间戳 == 0 || 缓存时钟 - 组件项->时间戳 >= 500) {
			组件项->根组件 = nullptr;
			组件项->队伍组件 = nullptr;
			组件项->生命组件 = nullptr;
			组件项->网格组件 = nullptr;
			组件项->AI已取 = false;
			组件项->时间戳 = 缓存时钟;
		}
		if (UE::UObject::GObjects自洽(角色)) {
			if (组件项->根组件 && !UE::UObject::GObjects自洽(组件项->根组件)) 组件项->根组件 = nullptr;
			if (组件项->队伍组件 && !UE::UObject::GObjects自洽(组件项->队伍组件)) 组件项->队伍组件 = nullptr;
			if (组件项->生命组件 && !UE::UObject::GObjects自洽(组件项->生命组件)) 组件项->生命组件 = nullptr;
			if (组件项->网格组件 && !UE::UObject::GObjects自洽(组件项->网格组件)) 组件项->网格组件 = nullptr;
		}
		FVector2D 坐标[2]; bool 是否部分在屏外 = false;
		USceneComponent* 场景组件 = nullptr;
		FVector 位置;
		{
			if (!组件项->根组件) 组件项->根组件 = 角色->取根组件();
			if (!(场景组件 = 组件项->根组件))continue;
			if (!UE::UObject::类匹配(场景组件, USceneComponent::取静态类())) {
				组件项->根组件 = nullptr;
				continue;
			}
			位置 = 场景组件->K2_GetComponentLocation();
			if (位置.是否为零())continue;
			if (!游戏全局数据.世界坐标转屏幕矩形(位置, 坐标))continue;
			if (!(是否部分在屏外 = 游戏全局数据.是否在屏幕内(坐标[0]) || 游戏全局数据.是否在屏幕内(坐标[1])))continue;
		}
		UGPTeamComponent* 队伍组件 = nullptr;
		UGPHealthDataComponent* 生命组件 = nullptr;
		USkeletalMeshComponent* 骨骼网格组件 = nullptr;
		APlayerState* 玩家状态 = nullptr;
		int 队伍编号 = 0;
		if (!组件项->队伍组件) 组件项->队伍组件 = 角色->取队伍组件();
		if (!(队伍组件 = 组件项->队伍组件))continue;
		if (!UE::UObject::类匹配(队伍组件, UGPTeamComponent::取静态类())) { 组件项->队伍组件 = nullptr; continue; }
		if (!组件项->生命组件) 组件项->生命组件 = 角色->取生命组件();
		if (!(生命组件 = 组件项->生命组件))continue;
		if (!UE::UObject::类匹配(生命组件, UGPHealthDataComponent::取静态类())) { 组件项->生命组件 = nullptr; continue; }
		队伍编号 = 队伍组件->取队伍编号();
		玩家状态 = 角色->取PlayerState();
		if (玩家状态) {
			void* 状态虚表 = 内存辅助::读字段<void*>(玩家状态, 0);
			if (!内存辅助::有效用户地址(状态虚表)) 玩家状态 = nullptr;
		}
		bool 是否AI;
		if (!玩家状态) {
			是否AI = true;
		}
		else {
			if (!组件项->AI已取) {
				组件项->AI已取 = true;
				组件项->AI标志 = 角色->是否AI();
			}
			是否AI = 组件项->AI标志;
		}
		if (是否AI && 角色->是否类(ADFMAIAnimalCharacter::取静态类())) continue;
		if (队伍编号 == 本地队伍编号) continue;
		if (玩家状态 && 玩家状态->转换<ADFMPlayerState>()->取ExitState() == EExitState::Escaped) continue;
		float 距离 = 游戏全局数据.相机位置.取距离(位置) / 100.0f;
		const bool 是人机 = 是否AI;
		if (战斗模式开关 && 是人机) continue;
		const float 单位最大距离 = 是人机 ? 简化配置::人机透视距离 : 简化配置::玩家透视距离;
		if (距离 > 单位最大距离) continue;
		std::string 显示名;
		{
			bool 命中 = false;
			for (auto& 项 : 名字缓存) {
				if (项.角色 == 角色 && 缓存时钟 - 项.时间戳 < 3000) {
					显示名 = 是人机 ? 项.池名 : 项.英雄名;
					项.时间戳 = 缓存时钟;
					命中 = true;
					break;
				}
			}
			if (!命中) {
				名字缓存项 项;
				项.角色 = 角色; 项.时间戳 = 缓存时钟;
				if (是否AI) { 项.池名 = 游戏全局数据.取单位池名(角色); 项.英雄名 = ""; }
				else { 项.池名 = ""; 项.英雄名 = 游戏全局数据.取英雄名(玩家状态); }
				显示名 = 是人机 ? 项.池名 : 项.英雄名;
				名字缓存.push_back(项);
				if (名字缓存.size() > 128 || 缓存时钟 - 名字缓存清理节拍 > 5000) {
					名字缓存.clear();
					名字缓存清理节拍 = 缓存时钟;
				}
			}
		}
		if (是人机) {
			const bool 是BOSS = 显示名.size() >= 2 && 显示名[0] == '[' && 显示名.find(']') != std::string::npos;
			if (!是BOSS) continue;
		}
		if (!组件项->网格组件) 组件项->网格组件 = UMTAPI_ACharacter::取网格(角色);
		if (!(骨骼网格组件 = 组件项->网格组件))continue;
		if (!UE::UObject::类匹配(骨骼网格组件, USkeletalMeshComponent::取静态类())) { 组件项->网格组件 = nullptr; continue; }
		{
			bool 可见 = 游戏全局数据.取骨骼网格(角色, 骨骼网格组件, 骨骼信息表);
			if (可见 && !武器禁止自瞄) 自瞄.添加目标(角色, 骨骼信息表);
			float 字号 = SCALE(是人机 ? 简化配置::人机字号 : 简化配置::人物字号);
			FVector2D 顶部位置(坐标[0].X + (坐标[1].X - 坐标[0].X) / 2.0f, 坐标[0].Y);
			FVector2D 底部位置(坐标[0].X + (坐标[1].X - 坐标[0].X) / 2.0f, 坐标[1].Y);
			const float 框宽 = 坐标[1].X - 坐标[0].X;
			const float 框高 = 坐标[1].Y - 坐标[0].Y;
			游戏全局数据.渲染骨骼网格(骨骼信息表, 简化配置::骨骼可见色, 简化配置::骨骼不可见色, SCALE(1.0f));
			if (骨骼信息表[头].有效 && 骨骼信息表[脖子].有效) {
				FVector2D 头部屏幕 = 骨骼信息表[头].屏幕;
				FVector2D 脖子屏幕 = 骨骼信息表[脖子].屏幕;
				float 头颈距离 = sqrtf((头部屏幕.X - 脖子屏幕.X) * (头部屏幕.X - 脖子屏幕.X) + (头部屏幕.Y - 脖子屏幕.Y) * (头部屏幕.Y - 脖子屏幕.Y));
				float 圆心X = 头部屏幕.X;
				float 圆心Y = 头部屏幕.Y - 头颈距离 * 0.35f;
				float 半径 = 头颈距离 * 1.1f;
				const float 框半宽 = 框宽 * 0.5f;
				if (半径 > 框半宽) 半径 = 框半宽;
				if (圆心Y - 半径 < 坐标[0].Y - SCALE(2.0f)) {
					圆心Y = 坐标[0].Y - SCALE(2.0f) + 半径;
				}
				if (半径 < SCALE(2.0f)) 半径 = SCALE(2.0f);
				uint32_t 圆圈颜色 = 可见 ? 简化配置::骨骼可见色 : 简化配置::骨骼不可见色;
				原生绘制::画圆形(圆心X, 圆心Y, 半径, 圆圈颜色, SCALE(1.0f), 24);
				{
					const float 连X = 脖子屏幕.X - 圆心X;
					const float 连Y = 脖子屏幕.Y - 圆心Y;
					const float 连长 = sqrtf(连X * 连X + 连Y * 连Y);
					if (连长 > 1.0f) {
						原生绘制::画线段(圆心X + 连X / 连长 * 半径, 圆心Y + 连Y / 连长 * 半径,
							脖子屏幕.X, 脖子屏幕.Y, 圆圈颜色, SCALE(1.0f));
					}
				}
			}
			if (!是人机) {
				void* 生命虚表 = 内存辅助::读字段<void*>(生命组件, 0);
				float 血量比例 = 1.0f;
				if (内存辅助::有效用户地址(生命虚表))
					血量比例 = 生命组件->取生命值() / 生命组件->取最大生命值();
				if (血量比例 < 0.0f) 血量比例 = 0.0f;
				if (血量比例 > 1.0f) 血量比例 = 1.0f;
				float 条高 = 框高 * 0.85f;
				float 条Y = 坐标[0].Y + (框高 - 条高) * 0.5f;
				float 条宽 = SCALE(4.0f);
				float 条X = 坐标[1].X + SCALE(5.0f);
				uint32_t 血条颜色;
				if (血量比例 > 0.6f) 血条颜色 = 颜色32(50, 230, 140, 255);
				else if (血量比例 > 0.3f) 血条颜色 = 颜色32(255, 210, 60, 255);
				else 血条颜色 = 颜色32(255, 80, 80, 255);
				原生绘制::画矩形填充(条X, 条Y, 条X + 条宽, 条Y + 条高, 颜色32(15, 18, 28, 200));
				const int 最大分段数 = 10;
				const float 间隔 = SCALE(1.0f);
				int 分段数 = 最大分段数;
				float 分段高 = (条高 - 间隔 * (分段数 - 1)) / 分段数;
				while (分段高 < 1.0f && 分段数 > 1) {
					分段数--;
					分段高 = (条高 - 间隔 * (分段数 - 1)) / 分段数;
				}
				if (分段高 < 1.0f) 分段高 = 1.0f;
				int 已填充 = (int)(血量比例 * 分段数 + 0.999f);
				if (已填充 > 分段数) 已填充 = 分段数;
				for (int s = 0; s < 已填充; s++) {
					float 分段顶 = 条Y + 条高 - (s + 1) * 分段高 - s * 间隔;
					原生绘制::画矩形填充(条X, 分段顶, 条X + 条宽, 分段顶 + 分段高, 血条颜色);
				}
			}
			if (!显示名.empty()) {
				std::string 行文本 = 是人机 ? 显示名 : ("[" + 显示名 + "]");
				顶部位置.Y -= 字号;
				游戏全局数据.排队文本(字号, 行文本, 顶部位置, 是人机 ? 简化配置::人机名色 : 简化配置::玩家名色, true);
			}
			if (!是人机) {
				std::string 队伍编号文本 = std::to_string(队伍编号);
				float 文本宽 = 原生绘制::文本宽度(字号, 队伍编号文本.c_str());
				float 矩形宽 = 文本宽 + SCALE(8.0f);
				float 矩形高 = 字号 * 0.8f + SCALE(6.0f);
				float 矩形中心X = 顶部位置.X;
				float 矩形中心Y = 顶部位置.Y - 矩形高 * 0.5f - SCALE(3.0f);
				uint32_t 队伍颜色 = 游戏全局数据.编号取色(队伍编号);
				原生绘制::画矩形填充(矩形中心X - 矩形宽 * 0.5f, 矩形中心Y - 矩形高 * 0.5f, 矩形中心X + 矩形宽 * 0.5f, 矩形中心Y + 矩形高 * 0.5f, (队伍颜色 & 0x00FFFFFF) | 0x60000000);
				原生绘制::文本(字号 * 0.8f, 矩形中心X, 矩形中心Y, 0xFFFFFFFF, 队伍编号文本.c_str(), true, false, true);
				顶部位置.Y = 矩形中心Y - 矩形高 * 0.5f - SCALE(4.0f);
				if (缓存时钟 - 组件项->装备时间戳 >= 500) {
					组件项->装备时间戳 = 缓存时钟;
					组件项->头盔等级 = 0;
					组件项->胸甲等级 = 0;
					UCharacterEquipComponent* 装备组件 = UCharacterEquipComponent::获取(角色);
					if (装备组件) {
						void* 装备虚表 = 内存辅助::读字段<void*>(装备组件, 0);
						if (内存辅助::有效用户地址(装备虚表)) {
							FEquipmentInfo 头盔信息 = 装备组件->按类型取装备信息(EEquipmentType::Helmet);
							FEquipmentInfo 胸甲信息 = 装备组件->按类型取装备信息(EEquipmentType::BreastPlate);
							if (头盔信息.取ItemID()) 组件项->头盔等级 = (头盔信息.取ItemID() % 1000000) / 1000;
							if (胸甲信息.取ItemID()) 组件项->胸甲等级 = (胸甲信息.取ItemID() % 1000000) / 1000;
						}
					}
				}
				if (组件项->头盔等级 || 组件项->胸甲等级) {
					float 物品宽 = SCALE(24.0f);
					float 基准Y = 坐标[1].Y + SCALE(2.0f);
					int 数量 = (组件项->头盔等级 ? 1 : 0) + (组件项->胸甲等级 ? 1 : 0);
					float 起始X = 坐标[0].X + (坐标[1].X - 坐标[0].X - 物品宽 * 数量 - SCALE(2.0f) * (数量 - 1)) * 0.5f;
					float 光标X = 起始X;
					if (组件项->头盔等级 >= 1) {
						char 文本缓冲[16];
						sprintf_s(文本缓冲, 静态加密串(u8"头%d"), 组件项->头盔等级);
						原生绘制::文本(字号, 光标X + 物品宽 * 0.5f, 基准Y, 游戏全局数据.等级取色(组件项->头盔等级), 文本缓冲, true, true);
						光标X += 物品宽 + SCALE(2.0f);
					}
					if (组件项->胸甲等级 >= 1) {
						char 文本缓冲[16];
						sprintf_s(文本缓冲, 静态加密串(u8"胸%d"), 组件项->胸甲等级);
						原生绘制::文本(字号, 光标X + 物品宽 * 0.5f, 基准Y, 游戏全局数据.等级取色(组件项->胸甲等级), 文本缓冲, true, true);
					}
					底部位置.Y = 基准Y + 字号 + SCALE(1.0f);
				}
				游戏全局数据.排队文本(字号, std::to_string((int)距离) + 静态加密串(u8"m"), 底部位置, 简化配置::距离色, true);
			}
		}
	}
	自瞄.保存();
	自瞄.绘制();
}
void Engine::渲染拾取物() {
	for (int i = 0; i < 游戏全局数据.拾取列表.size(); i++)
	{
		USceneComponent* 场景组件 = nullptr;
		AInventoryPickup* 拾取物 = nullptr;
		EMarkingItemType 标记类型 = EMarkingItemType::None;
		拾取物 = reinterpret_cast<AInventoryPickup*>(游戏全局数据.拾取列表[i]);
		if (!内存辅助::有效用户地址(拾取物)) continue;
		{
			void* 虚表 = 内存辅助::读字段<void*>(拾取物, 0);
			if (!内存辅助::有效用户地址(虚表)) continue;
		}
		if (!(场景组件 = 拾取物->取根组件()))continue;
		标记类型 = 拾取物->取MarkingItemType();
		if (标记类型 == EMarkingItemType::LootingItem) {
			FVector 位置 = 场景组件->K2_GetComponentLocation();
			if (位置.是否为零())continue;
			float 距离 = 游戏全局数据.相机位置.取距离(位置) / 100.0f;
			if (简化配置::物资距离 < 距离)continue;
			FInventoryItemInfo* 拾取物品信息 = 拾取物->取PickupItemInfo地址();
			if (!拾取物品信息) continue;
			auto 物品行 = 拾取物品信息->取物品行();
			if (物品行 == nullptr)continue;
			if (!内存辅助::有效用户地址(物品行)) continue;
			const int 品质 = 物品行->取Quality();
			if (品质 < 5) continue;
			std::string 物品名 = 游戏全局数据.取拾取物名(拾取物);
			if (物品名.empty())continue; FVector2D 坐标;
			if (游戏全局数据.世界坐标转屏幕点(位置, &坐标) && 游戏全局数据.是否在屏幕内(坐标)) {
				uint32_t 颜色 = 游戏全局数据.等级取色(品质);
				char 显示文本[128];
				sprintf_s(显示文本, sizeof(显示文本), 静态加密串(u8"%s %.0fm"),
					物品名.c_str(),
					距离);
				物资文本管理器.添加文本(SCALE(简化配置::物资字号 + 4.0f), 显示文本, 坐标, 颜色, 距离);
			}
		}
		else if (标记类型 == EMarkingItemType::DeadBody) {
			auto 尸体 = 拾取物->转换<AInventoryPickup_DeadBody>();
			if (!尸体->取LootingCharacterOwner() || !尸体->取OwnerPlayerState()) continue;
			if (尸体->取bLooted()) continue;
			FVector 位置 = 场景组件->K2_GetComponentLocation();
			if (位置.是否为零())continue;
			float 距离 = 游戏全局数据.相机位置.取距离(位置) / 100.0f;
			if (简化配置::盒子距离 < 距离)continue;
			FVector2D 坐标;
			if (游戏全局数据.世界坐标转屏幕点(位置, &坐标) && 游戏全局数据.是否在屏幕内(坐标)) {
				char 显示文本[128];
				sprintf_s(显示文本, sizeof(显示文本), 静态加密串(u8"%s %.0fm"), 静态加密串(u8"玩家盒子"), 距离);
				游戏全局数据.排队文本(SCALE(简化配置::物资字号), 显示文本, 坐标, 简化配置::盒子色, true);
			}
		}
	}
	物资文本管理器.显示堆叠文本(50.0f, SCALE(20.0f));
}
void 自瞄系统::添加目标(AGPCharacter* 角色, 骨骼信息* 骨骼信息表) {
	if (简化配置::过滤倒地 && 角色->取生命组件()->取生命值() <= 0.0f)return;
	FVector2D 屏幕中心(游戏全局数据.屏幕宽 / 2.0f, 游戏全局数据.屏幕高 / 2.0f);
	float 屏幕距离 = SCALE(简化配置::自瞄范围);
	float 最大自瞄距离 = 简化配置::自瞄最大距离 * 100.f;
	bool 锁定到目标 = false;
	int 锁定骨骼编号 = 0;
	float 最小屏幕距离 = 99999999.0f;
	for (size_t i = 0; i < 17; i++)
	{
		if (骨骼信息表[i].有效 && 骨骼信息表[i].可见) {
			if (游戏全局数据.相机位置.取距离(骨骼信息表[i].位置) < 最大自瞄距离) {
				float 当前屏幕距离 = 屏幕中心.取距离(骨骼信息表[i].屏幕);
				if (当前屏幕距离 <= 屏幕距离 && 当前屏幕距离 < 最小屏幕距离) {
					锁定骨骼编号 = i;
					锁定到目标 = true;
					最小屏幕距离 = 当前屏幕距离;
				}
			}
		}
	}
	if (锁定到目标) {
		if (this->锁定角色 && this->锁定角色 == 角色) {
			this->保存时间 = GetTickCount64();
			this->锁定骨骼编号 = 锁定骨骼编号;
			this->锁定屏幕距离 = 最小屏幕距离;
			memcpy(this->骨骼信息表, this->锁定中的骨骼信息表, sizeof(this->骨骼信息表));
		}
		if (最小屏幕距离 < this->最小屏幕距离) {
			this->最小屏幕距离 = 最小屏幕距离;
			this->锁定中的角色 = 角色;
			this->锁定中的骨骼编号 = 锁定骨骼编号;
			this->锁定中的屏幕距离 = 最小屏幕距离;
			memcpy(this->锁定中的骨骼信息表, 骨骼信息表, sizeof(this->锁定中的骨骼信息表));
		}
	}
}
void 自瞄系统::清空() {
	this->最小屏幕距离 = 999999999.0f;
	this->锁定中的角色 = nullptr;
}
void 自瞄系统::保存() {
	if (this->锁定中的角色) {
		if (this->锁定角色 == nullptr || this->锁定角色 == this->锁定中的角色) {
			this->保存时间 = GetTickCount64();
			this->锁定角色 = this->锁定中的角色;
			this->锁定骨骼编号 = this->锁定中的骨骼编号;
			memcpy(this->骨骼信息表, this->锁定中的骨骼信息表, sizeof(this->骨骼信息表));
		}
		else if (abs(this->锁定屏幕距离 - this->锁定中的屏幕距离) > SCALE(5))
		{
			this->保存时间 = GetTickCount64();
			this->锁定角色 = this->锁定中的角色;
			this->锁定骨骼编号 = this->锁定中的骨骼编号;
			memcpy(this->骨骼信息表, this->锁定中的骨骼信息表, sizeof(this->骨骼信息表));
		}
		else if (GetTickCount64() - this->保存时间 > 300)
		{
			this->保存时间 = GetTickCount64();
			this->锁定角色 = this->锁定中的角色;
			this->锁定骨骼编号 = this->锁定中的骨骼编号;
			memcpy(this->骨骼信息表, this->锁定中的骨骼信息表, sizeof(this->骨骼信息表));
		}
	}
}
void 自瞄系统::绘制() {
	FVector2D 屏幕位置;
	if (this->锁定角色 && GetTickCount64() - this->保存时间 < 200) {
		if (!内存辅助::有效用户地址(this->锁定角色)) {
			this->锁定角色 = nullptr;
			return;
		}
		{
			void* 锁定虚表 = 内存辅助::读字段<void*>(this->锁定角色, 0);
			if (!内存辅助::有效用户地址(锁定虚表)) {
				this->锁定角色 = nullptr;
				return;
			}
		}
		static AGPCharacter* 松手状态目标 = nullptr;
		static DWORD 松手截止 = 0;
		{
			const bool 目标已死 = this->锁定角色->是否死亡() || this->锁定角色->取生命组件()->取生命值() <= 0.0f;
			if (目标已死) {
				if (松手状态目标 != this->锁定角色) {
					松手状态目标 = this->锁定角色;
					松手截止 = GetTickCount() + 300 + (GetTickCount() % 601);
					Engine::重置自瞄运动();
				}
				if (GetTickCount() < 松手截止) {
					return;
				}
			}
			else {
				松手状态目标 = nullptr;
				松手截止 = 0;
			}
		}
		if (游戏全局数据.世界坐标转屏幕点(this->骨骼信息表[0].位置, &屏幕位置)) {
			if (游戏全局数据.本地角色 == nullptr || 游戏全局数据.本地角色->是否死亡() || 游戏全局数据.本地角色->取生命组件()->取生命值() <= 0.0f) {
				return;
			}
			auto 缓存武器 = 游戏全局数据.本地角色->取CacheCurWeapon();
			if (缓存武器 == nullptr || 缓存武器->取bIsSupportWeapon())return;
			auto 武器编号 = 缓存武器->取WeaponID();
			int 目标骨骼 = -1;
			bool 自瞄开启 = (取按键状态(VK_LBUTTON) & 0x8000) != 0;
			if (手持狙击武器(武器编号) && (取按键状态(VK_RBUTTON) & 0x8000) != 0) {
				自瞄开启 = true;
			}
			if (!自瞄开启) {
				Engine::重置自瞄运动();
				return;
			}
			if (取按键状态(VK_SHIFT) & 0x8000) {
				const 骨骼编号 头优先表[] = { 头,脖子,右肩,左肩,右臂,左臂,右前臂,左前臂,右手,左手,胯,右大腿,左大腿,右小腿,左小腿,右脚,左脚 };
				for (int 优先骨骼 : 头优先表) {
					auto& 信息 = this->骨骼信息表[优先骨骼];
					if (信息.有效 && 信息.可见) {
						目标骨骼 = 优先骨骼;
						break;
					}
				}
			}
			else {
				目标骨骼 = this->锁定骨骼编号;
			}
			if (目标骨骼 == -1)return;
			FVector 瞄准点 = this->骨骼信息表[目标骨骼].位置;
			{
				// 目标速度实测: 锁定角色根组件位置差分 + EMA 平滑(反射数据, 无手调参数)
				static AGPCharacter* 速度角色 = nullptr;
				static FVector 上帧根位置;
				static FVector 平滑速度;
				static DWORD64 速度节拍 = 0;
				if (速度角色 != this->锁定角色) {
					速度角色 = this->锁定角色;
					速度节拍 = 0;
					平滑速度 = FVector();
				}
				if (UE::UObject::GObjects自洽(this->锁定角色)) {
					USceneComponent* 速度根 = this->锁定角色->取根组件();
					if (速度根 && UE::UObject::类匹配(速度根, USceneComponent::取静态类())) {
						FVector 当前根位置 = 速度根->K2_GetComponentLocation();
						if (!当前根位置.是否为零()) {
							const DWORD64 现在节拍 = GetTickCount64();
							if (速度节拍) {
								const float 差秒 = (float)(现在节拍 - 速度节拍) / 1000.0f;
								if (差秒 > 0.004f && 差秒 < 0.25f) {
									FVector 瞬时速度 = (当前根位置 - 上帧根位置) / 差秒;
									if (瞬时速度.取模() < 3000.0f) {
										平滑速度 = 平滑速度 * 0.5f + 瞬时速度 * 0.5f;
									}
								}
							}
							上帧根位置 = 当前根位置;
							速度节拍 = 现在节拍;
						}
					}
				}
				// 弹道: 飞行时间 = 距离/弹速(按武器段估算, cm/s), 下坠 = 0.5*g*t²(UE 标准重力 980)
				float 弹速 = 72000.0f;
				if (手持狙击武器(武器编号)) 弹速 = 85000.0f;
				else if (武器编号 >= 18040000000ULL && 武器编号 <= 18049999999ULL) 弹速 = 80000.0f;
				else if (武器编号 >= 18020000000ULL && 武器编号 <= 18029999999ULL) 弹速 = 42000.0f;
				else if (武器编号 >= 18030000000ULL && 武器编号 <= 18039999999ULL) 弹速 = 38000.0f;
				else if (武器编号 >= 18070000000ULL && 武器编号 <= 18079999999ULL) 弹速 = 37000.0f;
				const float 距离厘米 = 游戏全局数据.相机位置.取距离(瞄准点);
				float 飞行秒 = 距离厘米 / 弹速;
				if (飞行秒 > 1.2f) 飞行秒 = 1.2f;
				if (飞行秒 > 0.001f) {
					瞄准点.X += 平滑速度.X * 飞行秒;
					瞄准点.Y += 平滑速度.Y * 飞行秒;
					瞄准点.Z += 平滑速度.Z * 飞行秒 + 490.0f * 飞行秒 * 飞行秒;
				}
			}
			if (游戏全局数据.世界坐标转屏幕点(瞄准点, &屏幕位置)) {
				static AGPCharacter* 偏移目标 = nullptr;
				static float 偏移X = 0.0f, 偏移Y = 0.0f;
				static DWORD64 偏移种子 = GetTickCount64();
				if (偏移目标 != this->锁定角色) {
					偏移目标 = this->锁定角色;
					偏移种子 = 偏移种子 * 6364136223846793005ull + 1442695040888963407ull;
					偏移X = (float)((int)((偏移种子 >> 33) % 5) - 2);
					偏移Y = (float)((int)((偏移种子 >> 59) % 3) - 1);
				}
				屏幕位置.X += SCALE(偏移X);
				屏幕位置.Y += SCALE(偏移Y);
				{
					// 稳态误差积分: 持续同向偏差(后坐力飘/系统滞后)逐帧加强修正, 误差归零后自动衰减
					static FVector2D 误差积分(0, 0);
					static AGPCharacter* 积分角色 = nullptr;
					static DWORD64 积分节拍 = 0;
					const float 残差X = 屏幕位置.X - 游戏全局数据.屏幕宽 / 2.0f;
					const float 残差Y = 屏幕位置.Y - 游戏全局数据.屏幕高 / 2.0f;
					if (积分角色 != this->锁定角色) {
						积分角色 = this->锁定角色;
						误差积分 = FVector2D();
						积分节拍 = GetTickCount64();
					}
					const DWORD64 积分现在 = GetTickCount64();
					float 帧秒 = (float)(积分现在 - 积分节拍) / 1000.0f;
					积分节拍 = 积分现在;
					if (帧秒 <= 0.0f || 帧秒 > 0.1f) 帧秒 = 0.016f;
					if (fabsf(残差X) > 0.5f) {
						误差积分.X += 残差X * 帧秒 * 4.0f;
						if (误差积分.X > 40.0f) 误差积分.X = 40.0f;
						else if (误差积分.X < -40.0f) 误差积分.X = -40.0f;
					}
					else {
						误差积分.X *= 0.90f;
					}
					if (fabsf(残差Y) > 0.5f) {
						误差积分.Y += 残差Y * 帧秒 * 4.0f;
						if (误差积分.Y > 40.0f) 误差积分.Y = 40.0f;
						else if (误差积分.Y < -40.0f) 误差积分.Y = -40.0f;
					}
					else {
						误差积分.Y *= 0.90f;
					}
					屏幕位置.X += 误差积分.X;
					屏幕位置.Y += 误差积分.Y;
				}
				Engine::鼠标移动(屏幕位置.X - 游戏全局数据.屏幕宽 / 2.0f, 屏幕位置.Y - 游戏全局数据.屏幕高 / 2.0f);
			}
		}
	}
}

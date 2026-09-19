#pragma once
#include "预编译头.h"
#include "游戏类.h"
#include "覆盖层/原生绘制.h"
#include <map>
using namespace UE;

// ===== 全部功能默认值(无菜单无配置, 改这里) =====
namespace 简化配置 {
	constexpr float 玩家透视距离 = 600.0f;   // 米
	constexpr float 人机透视距离 = 70.0f;    // 米
	constexpr float 自瞄最大距离 = 500.0f;   // 米
	constexpr float 自瞄范围 = 500.0f;       // 屏幕像素(锁定半径)
	constexpr float 物资距离 = 100.0f;       // 米, 只显示金(6)/红(7)级
	constexpr float 盒子距离 = 50.0f;        // 米, 只显示未开的玩家盒
	constexpr bool 过滤倒地 = true;
	constexpr float 人物字号 = 19.0f;
	constexpr float 人机字号 = 19.0f;
	constexpr float 物资字号 = 18.0f;
	// 颜色
	constexpr uint32_t 骨骼可见色 = 颜色32(255, 0, 0, 255);
	constexpr uint32_t 骨骼不可见色 = 颜色32(255, 255, 255, 255);
	constexpr uint32_t 玩家名色 = 颜色32(0, 255, 0, 255);   // 干员 [名字]
	constexpr uint32_t 人机名色 = 颜色32(255, 255, 255, 255);
	constexpr uint32_t 距离色 = 颜色32(255, 255, 0, 255);
	constexpr uint32_t 盒子色 = 颜色32(255, 255, 255, 255);
}
enum 骨骼编号
{
	头,
	脖子,
	胯,
	右肩,
	右臂,
	右前臂,
	右手,
	右大腿,
	右小腿,
	右脚,
	左肩,
	左臂,
	左前臂,
	左手,
	左大腿,
	左小腿,
	左脚,
};
struct 骨骼信息
{
	FVector 位置;
	FVector2D 屏幕;
	bool 可见;
	bool 有效;
};
class 全局数据 {
public:
	UEngine* 全局引擎 = nullptr;
	UWorld* 全局世界 = nullptr;
	UCanvas* 游戏画布 = nullptr;
	UGameViewportClient* 游戏视口 = nullptr;
	APlayerCameraManager* 相机管理器 = nullptr;
	APlayerController* 玩家控制器 = nullptr;
	AGPCharacter* 本地角色 = nullptr;
	UGPTeamComponent* 本地队伍组件 = nullptr;
	int 屏幕宽 = 0;
	int 屏幕高 = 0;
	FRotator 相机旋转;
	FRotator 控制旋转;
	FVector 相机位置;
	float 相机FOV = 90.0f;
public:
	std::vector<ADFMCharacter*> 单位列表;
	std::vector<AInventoryPickup*> 拾取列表;
public:
	struct 骨骼数据表
	{
		FName 头;
		FName 脖子;
		FName 胯;
		FName 右肩;
		FName 右臂;
		FName 右前臂;
		FName 右手;
		FName 右大腿;
		FName 右小腿;
		FName 右脚;
		FName 左肩;
		FName 左臂;
		FName 左前臂;
		FName 左手;
		FName 左大腿;
		FName 左小腿;
		FName 左脚;
		FName 骨架[17];
	}骨骼数据;
	void 初始化骨骼名();
public:
	bool 世界坐标转屏幕点(const FVector& 位置, FVector2D* 坐标);
	bool 世界坐标转屏幕矩形(const FVector& 位置, FVector2D* 坐标);
private:
	std::map<uint64_t, std::string> 单位池名缓存;
	std::map<uint64_t, std::string> 拾取名缓存;
public:
	void 清缓存();
public:
	std::string 取单位池名(ADFMCharacter* 角色);
	std::string 取英雄名(APlayerState* 玩家状态);
	std::string 取拾取物名(AInventoryPickup* 拾取物);
public:
	bool 取骨骼网格(ADFMCharacter* 角色, USkeletalMeshComponent* 骨骼网格组件, 骨骼信息* 输出骨骼信息);
public:
	volatile DWORD64 切图抑制截止 = 0;
public:
	bool 是否在屏幕内(const FVector2D 世界转屏幕);
public:
	uint32_t 编号取色(int 队伍编号);
	uint32_t 等级取色(int 等级);
public:
	void 画线(const FVector2D& 起点, const FVector2D& 终点, uint32_t 颜色值, float 粗细 = 1.0f);
	void 排队文本(float 字号, const std::string& 文本, FVector2D 位置, uint32_t 颜色, bool 居中 = true);
	void 渲染骨骼网格(const 骨骼信息* 骨骼信息表, uint32_t 可见颜色, uint32_t 不可见颜色, float 粗细 = 1.0f);
};
class 文本堆叠管理 {
private:
	struct 文本信息 {
		int 字号;
		std::string 文本;
		FVector2D 位置;
		uint32_t 颜色;
		float 距离;
	};
	std::vector<文本信息> 文本列表;
	float 计算屏幕距离(const FVector2D& a, const FVector2D& b) const;
public:
	文本堆叠管理() = default;
	~文本堆叠管理() = default;
	文本堆叠管理(const 文本堆叠管理&) = delete;
	文本堆叠管理& operator=(const 文本堆叠管理&) = delete;
	void 添加文本(int 字号, const std::string& 文本, const FVector2D& 位置, const uint32_t& 颜色, float 距离);
	void 显示堆叠文本(float 最大屏幕范围, float 最大距离);
	void 清空();
};
namespace Engine {
	extern void 初始化();
	extern HWND 游戏窗口;
	extern HWND 查找游戏窗口();
	extern void* fnPostRender;
	extern void 安装渲染后钩子();
	extern void IndirectPostRender(DWORD_PTR Rsp);
	extern void 渲染后回调(UGameViewportClient* 视口客户端, UCanvas* 画布);
	extern void 鼠标移动(int x, int y);
	extern void 重置自瞄运动();
	extern bool 更新物体();
	extern uint64_t 会话丢失节拍;
	extern void 缓存物体();
	extern void 渲染单位();
	extern void 渲染拾取物();
}
class 自瞄系统 {
private:
	float 最小屏幕距离 = 999999999.0f;
	int 锁定中的骨骼编号 = 0;
	骨骼信息 锁定中的骨骼信息表[17];
	AGPCharacter* 锁定中的角色 = nullptr;
	float 锁定中的屏幕距离 = 999999999.0f;
public:
	DWORD64 保存时间 = 0;
	int 锁定骨骼编号 = 0;
	骨骼信息 骨骼信息表[17];
	AGPCharacter* 锁定角色 = nullptr;
	float 锁定屏幕距离 = 999999999.0f;
public:
	void 添加目标(AGPCharacter* 角色, 骨骼信息* 骨骼信息表);
	void 清空();
	void 保存();
	void 绘制();
};

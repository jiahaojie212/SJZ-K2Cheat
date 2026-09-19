#pragma once
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <cmath>
#include <cwchar>
#include <stdexcept>
#include <Windows.h>
#include "偏移.h"
#include "../内存模块.h"
#include "../字符串加密.h"
typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
enum class EClassCastFlags : uint64 {
	None = 0x0000000000000000,
	Function = 0x0000000000080000,
};
enum class EMainFlowState : uint8 {
	Default = 0,
	Lobby = 1,
	Loading = 2,
	SafeHouse = 3,
	InGame = 4,
	LobbyBHD = 5,
	EMainFlowState_MAX = 6,
};
enum class EExitState : uint8_t {
	Normal = 1,
	WaitingToEscape = 2,
	Escaped = 3,
	ReadyToStartEscape = 4,
	InEscaping = 5,
	EExitState_MAX = 6,
};
enum class EEquipmentType : uint8 {
	None = 0,
	Helmet = 1,
	Headset = 2,
	FaceMask = 3,
	Armband = 4,
	BreastPlate = 5,
	Glasses = 6,
	ChestHanging = 7,
	Bag = 8,
	SafeBox = 9,
	Tool = 10,
	Shoes = 11,
	KeyChain = 12,
	Max = 13,
};
enum class ETraceTypeQuery : uint8 {
	TraceTypeQuery1 = 0,
	TraceTypeQuery2 = 1,
	TraceTypeQuery3 = 2,
	TraceTypeQuery4 = 3,
	TraceTypeQuery5 = 4,
	TraceTypeQuery6 = 5,
	TraceTypeQuery7 = 6,
	TraceTypeQuery8 = 7,
	TraceTypeQuery9 = 8,
	TraceTypeQuery10 = 9,
	TraceTypeQuery11 = 10,
	TraceTypeQuery12 = 11,
	TraceTypeQuery13 = 12,
	TraceTypeQuery14 = 13,
	TraceTypeQuery15 = 14,
	TraceTypeQuery16 = 15,
	TraceTypeQuery17 = 16,
	TraceTypeQuery18 = 17,
	TraceTypeQuery19 = 18,
	TraceTypeQuery20 = 19,
	TraceTypeQuery21 = 20,
	TraceTypeQuery22 = 21,
	TraceTypeQuery23 = 22,
	TraceTypeQuery24 = 23,
	TraceTypeQuery25 = 24,
	TraceTypeQuery26 = 25,
	TraceTypeQuery27 = 26,
	TraceTypeQuery28 = 27,
	TraceTypeQuery29 = 28,
	TraceTypeQuery30 = 29,
	TraceTypeQuery31 = 30,
	TraceTypeQuery32 = 31,
	TraceTypeQuery_MAX = 32,
	ETraceTypeQuery_MAX = 33,
};
enum class EDrawDebugTrace : uint8 {
	None = 0,
	ForOneFrame = 1,
	ForDuration = 2,
	Persistent = 3,
	EDrawDebugTrace_MAX = 4,
};
enum class EMarkingItemType : uint8_t {
	None = 0,
	CommonItem = 1,
	LootingItem = 2,
	Container = 3,
	DyingTeammate = 4,
	DeadBody = 5,
	Weapon = 6,
	EngineerSonicTrap = 7,
	Vehicle = 8,
	TacticalEquip = 9,
	PendingDeathCharacter = 10,
	SectorAnchor = 11,
	MandelBrickInteractor = 12,
	MandelBrickDecipherDevice = 13,
	COUNT = 14,
	EMarkingItemType_MAX = 15,
};
namespace 内存辅助 {
	inline bool 有效用户地址(const void* 指针) noexcept {
		const uintptr_t 地址 = reinterpret_cast<uintptr_t>(指针);
		return 地址 >= 0x10000 && 地址 <= 0x7FFFFFF00000;
	}
	template<class 字段类型>
	字段类型* 取字段指针(void* 基址, int32_t 偏移) noexcept {
		if (!基址 || !有效用户地址(基址)) {
			return nullptr;
		}
		if (偏移 == 0) {
			reinterpret_cast<字段类型*>(基址);
		}
		const uintptr_t 目标地址 =
			reinterpret_cast<uintptr_t>(基址) + static_cast<uintptr_t>(偏移);
		if (!有效用户地址(reinterpret_cast<void*>(目标地址))) {
			return nullptr;
		}
		return reinterpret_cast<字段类型*>(目标地址);
	}
	template<class 字段类型>
	字段类型 读字段(const void* 基址, int32_t 偏移) noexcept {
		const 字段类型* 字段指针 = 取字段指针<字段类型>(
			const_cast<void*>(基址), 偏移
		);
		if (字段指针) {
			return *字段指针;
		}
		return 字段类型();
	}
}
#define 定义静态类方法(类名) \
public: \
	static UClass* 取静态类() \
	{ \
		static UClass* 静态类指针 = nullptr; \
		if (静态类指针 == nullptr) \
		{ \
			静态类指针 = reinterpret_cast<UClass*>(查找对象(宽加密串(L#类名),1)); \
		} \
		return 静态类指针; \
	} \
		static 类名* 取默认对象() \
	{ \
		  \
		auto* 静态类 = 取静态类(); \
		return 静态类 ? reinterpret_cast<类名*>(静态类->取默认对象()) : nullptr; \
	}
	#define 定义静态函数取(返回类型, 函数名) \
static 返回类型 函数名() \
{ \
	static UFunction* 函数 = nullptr; \
	if (函数 == nullptr) \
	{ \
		auto* 静态类 = 取静态类(); \
		函数 = 静态类 ? 静态类->取函数(宽加密串(L#函数名)) : nullptr; \
	} \
	struct \
	{ \
		返回类型 返回值; \
	} 参数{}; \
	if (!函数 || !函数->参数区装得下(sizeof(参数))) { 返回类型 零值{}; return 零值; } \
	auto* 默认对象 = 取默认对象(); \
	if (默认对象) 默认对象->处理事件(函数, &参数); \
	return 参数.返回值; \
}
#define 定义函数取(返回类型, 函数名) \
返回类型 函数名() \
{ \
	static UFunction* 函数 = nullptr; \
	if (函数 == nullptr) \
	{ \
		auto* 静态类 = 取静态类(); \
		函数 = 静态类 ? 静态类->取函数(宽加密串(L#函数名)) : nullptr; \
	} \
	struct \
	{ \
		返回类型 返回值; \
	} 参数{}; \
	if (!函数 || !函数->参数区装得下(sizeof(参数))) { 返回类型 零值{}; return 零值; } \
	处理事件(函数, &参数); \
	return 参数.返回值; \
}
#define 定义属性取(返回类型, 属性名) \
	返回类型 取##属性名() \
	{ \
		static int32 偏移 = -2;   \
		if (偏移 == -2) \
		{ \
			auto* 静态类 = 取静态类(); \
			偏移 = 静态类 ? 静态类->取偏移(宽加密串(L#属性名)) : -1; \
		} \
		return 内存辅助::读字段<返回类型>(this, 偏移); \
	}
#define 定义属性指针(返回类型, 属性名) \
	返回类型* 取##属性名##地址() \
	{ \
		static int32 偏移 = -2;   \
		if (偏移 == -2) \
		{ \
			auto* 静态类 = 取静态类(); \
			偏移 = 静态类 ? 静态类->取偏移(宽加密串(L#属性名)) : -1; \
		} \
		return 内存辅助::取字段指针<返回类型>(this, 偏移); \
	}
struct FVector2D final {
public:
	using 底层类型 = float;
	float X;
	float Y;
public:
	constexpr FVector2D(底层类型 X = 0, 底层类型 Y = 0)
		: X(X), Y(Y)
	{
	}
	constexpr FVector2D(const FVector2D& 其他)
		: X(其他.X), Y(其他.Y)
	{
	}
	FVector2D& operator*=(底层类型 标量)
	{
		*this = *this * 标量;
		return *this;
	}
	FVector2D& operator*=(const FVector2D& 其他)
	{
		*this = *this * 其他;
		return *this;
	}
	FVector2D& operator+=(const FVector2D& 其他)
	{
		*this = *this + 其他;
		return *this;
	}
	FVector2D& operator-=(const FVector2D& 其他)
	{
		*this = *this - 其他;
		return *this;
	}
	FVector2D& operator/=(底层类型 标量)
	{
		*this = *this / 标量;
		return *this;
	}
	FVector2D& operator/=(const FVector2D& 其他)
	{
		*this = *this / 其他;
		return *this;
	}
	FVector2D& operator=(const FVector2D& 其他)
	{
		X = 其他.X;
		Y = 其他.Y;
		return *this;
	}
	底层类型 取距离(const FVector2D& 其他) const
	{
		FVector2D 差向量 = 其他 - *this;
		return 差向量.取模();
	}
	bool 是否为零() const
	{
		return X == 0 && Y == 0;
	}
	底层类型 取模() const
	{
		return std::sqrt((X * X) + (Y * Y));
	}
	bool operator!=(const FVector2D& 其他) const
	{
		return X != 其他.X || Y != 其他.Y;
	}
	FVector2D operator*(底层类型 标量) const
	{
		return { X * 标量, Y * 标量 };
	}
	FVector2D operator*(const FVector2D& 其他) const
	{
		return { X * 其他.X, Y * 其他.Y };
	}
	FVector2D operator+(const FVector2D& 其他) const
	{
		return { X + 其他.X, Y + 其他.Y };
	}
	FVector2D operator-(const FVector2D& 其他) const
	{
		return { X - 其他.X, Y - 其他.Y };
	}
	FVector2D operator/(底层类型 标量) const
	{
		if (标量 == 0)
			return *this;
		return { X / 标量, Y / 标量 };
	}
	FVector2D operator/(const FVector2D& 其他) const
	{
		if (其他.X == 0 || 其他.Y == 0)
			return *this;
		return { X / 其他.X, Y / 其他.Y };
	}
	bool operator==(const FVector2D& 其他) const
	{
		return X == 其他.X && Y == 其他.Y;
	}
};
struct FVector {
public:
	using 底层类型 = float;
	float X;
	float Y;
	float Z;
public:
	constexpr FVector(底层类型 X = 0, 底层类型 Y = 0, 底层类型 Z = 0)
		: X(X), Y(Y), Z(Z)
	{
	}
	constexpr FVector(const FVector& 其他)
		: X(其他.X), Y(其他.Y), Z(其他.Z)
	{
	}
	FVector& operator*=(底层类型 标量)
	{
		*this = *this * 标量;
		return *this;
	}
	FVector& operator*=(const FVector& 其他)
	{
		*this = *this * 其他;
		return *this;
	}
	FVector& operator+=(const FVector& 其他)
	{
		*this = *this + 其他;
		return *this;
	}
	FVector& operator-=(const FVector& 其他)
	{
		*this = *this - 其他;
		return *this;
	}
	FVector& operator/=(底层类型 标量)
	{
		*this = *this / 标量;
		return *this;
	}
	FVector& operator/=(const FVector& 其他)
	{
		*this = *this / 其他;
		return *this;
	}
	FVector& operator=(const FVector& 其他)
	{
		X = 其他.X;
		Y = 其他.Y;
		Z = 其他.Z;
		return *this;
	}
	底层类型 取距离(const FVector& 其他) const
	{
		FVector 差向量 = 其他 - *this;
		return 差向量.取模();
	}
	bool 是否为零() const
	{
		return X == 0 && Y == 0 && Z == 0;
	}
	底层类型 取模() const
	{
		return std::sqrt((X * X) + (Y * Y) + (Z * Z));
	}
	bool operator!=(const FVector& 其他) const
	{
		return X != 其他.X || Y != 其他.Y || Z != 其他.Z;
	}
	FVector operator*(底层类型 标量) const
	{
		return { X * 标量, Y * 标量, Z * 标量 };
	}
	FVector operator*(const FVector& 其他) const
	{
		return { X * 其他.X, Y * 其他.Y, Z * 其他.Z };
	}
	FVector operator+(const FVector& 其他) const
	{
		return { X + 其他.X, Y + 其他.Y, Z + 其他.Z };
	}
	FVector operator-(const FVector& 其他) const
	{
		return { X - 其他.X, Y - 其他.Y, Z - 其他.Z };
	}
	FVector operator/(底层类型 标量) const
	{
		if (标量 == 0)
			return *this;
		return { X / 标量, Y / 标量, Z / 标量 };
	}
	FVector operator/(const FVector& 其他) const
	{
		if (其他.X == 0 || 其他.Y == 0 || 其他.Z == 0)
			return *this;
		return { X / 其他.X, Y / 其他.Y, Z / 其他.Z };
	}
	bool operator==(const FVector& 其他) const
	{
		return X == 其他.X && Y == 其他.Y && Z == 其他.Z;
	}
};
struct FRotator final {
public:
	using 底层类型 = float;
	float Pitch;
	float Yaw;
	float Roll;
public:
	constexpr FRotator(底层类型 Pitch = 0, 底层类型 Yaw = 0, 底层类型 Roll = 0)
		: Pitch(Pitch), Yaw(Yaw), Roll(Roll)
	{
	}
	constexpr FRotator(const FRotator& 其他)
		: Pitch(其他.Pitch), Yaw(其他.Yaw), Roll(其他.Roll)
	{
	}
	FRotator& operator*=(底层类型 标量)
	{
		*this = *this * 标量;
		return *this;
	}
	FRotator& operator*=(const FRotator& 其他)
	{
		*this = *this * 其他;
		return *this;
	}
	FRotator& operator+=(const FRotator& 其他)
	{
		*this = *this + 其他;
		return *this;
	}
	FRotator& operator-=(const FRotator& 其他)
	{
		*this = *this - 其他;
		return *this;
	}
	FRotator& operator/=(底层类型 标量)
	{
		*this = *this / 标量;
		return *this;
	}
	FRotator& operator/=(const FRotator& 其他)
	{
		*this = *this / 其他;
		return *this;
	}
	FRotator& operator=(const FRotator& 其他)
	{
		Pitch = 其他.Pitch;
		Yaw = 其他.Yaw;
		Roll = 其他.Roll;
		return *this;
	}
	bool operator!=(const FRotator& 其他) const
	{
		return Pitch != 其他.Pitch || Yaw != 其他.Yaw || Roll != 其他.Roll;
	}
	FRotator operator*(底层类型 标量) const
	{
		return { Pitch * 标量, Yaw * 标量, Roll * 标量 };
	}
	FRotator operator*(const FRotator& 其他) const
	{
		return { Pitch * 其他.Pitch, Yaw * 其他.Yaw, Roll * 其他.Roll };
	}
	FRotator operator+(const FRotator& 其他) const
	{
		return { Pitch + 其他.Pitch, Yaw + 其他.Yaw, Roll + 其他.Roll };
	}
	FRotator operator-(const FRotator& 其他) const
	{
		return { Pitch - 其他.Pitch, Yaw - 其他.Yaw, Roll - 其他.Roll };
	}
	FRotator operator/(底层类型 标量) const
	{
		if (标量 == 0)
			return *this;
		return { Pitch / 标量, Yaw / 标量, Roll / 标量 };
	}
	FRotator operator/(const FRotator& 其他) const
	{
		if (其他.Pitch == 0 || 其他.Yaw == 0 || 其他.Roll == 0)
			return *this;
		return { Pitch / 其他.Pitch, Yaw / 其他.Yaw, Roll / 其他.Roll };
	}
	bool operator==(const FRotator& 其他) const
	{
		return Pitch == 其他.Pitch && Yaw == 其他.Yaw && Roll == 其他.Roll;
	}
};
class FLinearColor {
public:
	float R;
	float G;
	float B;
	float A;
	FLinearColor()
		: R(0), G(0), B(0), A(0)
	{
	}
	FLinearColor(float r, float g, float b, float a)
		: R(r),
		G(g),
		B(b),
		A(a)
	{
	}
	FLinearColor(unsigned int hex)
		:R((hex & 0xFF) / 255.0f),
		G(((hex >> 8) & 0xFF) / 255.0f),
		B(((hex >> 16) & 0xFF) / 255.0f),
		A(((hex >> 24) & 0xFF) / 255.0f)
	{
		if (A == 0.0f) A = 1.0f;
	}
	FLinearColor(uintptr_t r, uintptr_t g, uintptr_t b, uintptr_t a)
		: R((float)r / 255.f),
		G((float)g / 255.f),
		B((float)b / 255.f),
		A((float)a / 255.f)
	{
	}
	FLinearColor(uintptr_t r, uintptr_t g, uintptr_t b)
		: R((float)r / 255.f),
		G((float)g / 255.f),
		B((float)b / 255.f),
		A(1.0f)
	{
	}
};
namespace UE {
	template<typename 数组元素类型>
	class TArray;
	class FString;
	class FText;
	class UObject;
	class UClass;
	class UFunction;
	class UEngine;
	class UWorld;
	class UCanvas;
	class UGameViewportClient;
	class APlayerState;
	class AActor;
	class APlayerCameraManager;
	class AWeaponBase;
	template<typename 数组元素类型>
	class TArray {
	protected:
		static constexpr uint64 元素对齐 = alignof(数组元素类型);
		static constexpr uint64 元素大小 = sizeof(数组元素类型);
	public:
		数组元素类型* 数据;
		int32 元素数量;
		int32 最大元素;
	public:
		TArray()
			: 数据(nullptr), 元素数量(0), 最大元素(0)
		{
		}
		TArray(const TArray&) = default;
		TArray(TArray&&) = default;
	public:
		TArray& operator=(TArray&&) = default;
		TArray& operator=(const TArray&) = default;
	private:
		inline int32 取余量() const { return 最大元素 - 元素数量; }
		inline void 校验下标(int32 下标) const { if (!是否有效下标(下标)) throw std::out_of_range(加密串("Index was out of range!")); }
	public:
		inline bool 添加(const 数组元素类型& 元素)
		{
			if (取余量() <= 0)
				return false;
			数据[元素数量] = 元素;
			元素数量++;
			return true;
		}
	public:
		inline int32 数量() const { return 元素数量; }
		inline int32 最大() const { return 最大元素; }
		inline const 数组元素类型* 取数据指针() const { return 数据; }
		inline bool 是否有效下标(int32 下标) const { return 数据 && 下标 >= 0 && 下标 < 元素数量; }
	public:
		inline 数组元素类型& operator[](int32 下标) { 校验下标(下标); return 数据[下标]; }
		inline const 数组元素类型& operator[](int32 下标) const { 校验下标(下标); return 数据[下标]; }
	};
	class FString : public TArray<wchar_t> {
	public:
		using TArray::TArray;
		FString(const wchar_t* 文本)
		{
			const uint32 终止长度 = static_cast<uint32>(wcslen(文本) + 0x1);
			数据 = const_cast<wchar_t*>(文本);
			元素数量 = 终止长度;
			最大元素 = 终止长度;
		}
		FString(wchar_t* 文本, int32 数量, int32 最大)
		{
			数据 = 文本;
			元素数量 = 数量;
			最大元素 = 最大;
		}
	public:
		inline wchar_t* 取C字符串() { return 数据; }
		inline const wchar_t* 取C字符串() const { return 数据; }
	};
	class FText final {
	public:
		void* 文本数据;
		uint8 Pad_8[0x10];
	public:
	};
	class FName {
	private:
		static inline void (*追加字符串函数)(const FName*, wchar_t&) = nullptr;
	public:
		int32 比较索引;
		int32 编号;
	public:
		static void 追加字符串(const FName* 名称, wchar_t* 输出缓冲);
	private:
		static void 初始化内部() {
			追加字符串函数 = reinterpret_cast<decltype(追加字符串函数)>((DWORD_PTR)内存模块::取进程基址() + 偏移::Name::取AppendNameToString());
		}
		std::wstring 取原始宽字符串() {
			wchar_t 临时缓冲[1024] = { 0 };
			追加字符串(this, 临时缓冲);
			临时缓冲[1023] = 0;
			return std::wstring(临时缓冲);
		}
	public:
		std::wstring 取宽字符串() {
			return 取原始宽字符串();
		}
	};
	struct FUObjectItem {
		UObject* 对象;
		uint8 Pad[0x10];
	};
	class TUObjectArray {
	public:
		static constexpr int32 每块元素 = 0x10000;
		uint8 Pad_0[0x10];
		int32 最大元素;
		int32 元素数量;
		int32 块数量;
		int32 最大块;
		FUObjectItem** 对象表;
		int32 数量() const { return 元素数量; }
		UObject* 按下标取(int32 下标) const {
			int32 块下标 = 下标 / 每块元素;
			int32 块内下标 = 下标 % 每块元素;
			if (下标 < 0 || 块下标 >= 块数量 || 下标 >= 元素数量)
				return nullptr;
			FUObjectItem* 块 = 对象表[块下标];
			if (!块) return nullptr;
			return 块[块内下标].对象;
		}
	};
	class UObject {
	private:
		static inline UObject* (*静态查找函数)(UClass*, UObject*, const wchar_t*, bool) = nullptr;
		static inline TUObjectArray* 对象表 = nullptr;
		static void 初始化内部() {
			对象表 = reinterpret_cast<TUObjectArray*>((DWORD_PTR)内存模块::取进程基址() + 偏移::UObject::取GObjects());
		}
		static TUObjectArray* 取对象表() {
			if (!对象表) 初始化内部();
			return 对象表;
		}
		static UObject* 快速查找对象_扫描(TUObjectArray* 对象数组, const wchar_t* 名称, UClass* 必需类, bool 精确类) {
			for (int32 i = 0; i < 对象数组->数量(); i++) {
				UObject* 对象 = 对象数组->按下标取(i);
				if (!对象) continue;
				if (精确类) {
					if (对象->取类() != 必需类) continue;
				}
				else if (必需类) {
					if (!对象->是否类(必需类)) continue;
				}
				if (对象->取名字().取宽字符串() == 名称)
					return 对象;
			}
			return nullptr;
		}
		static UObject* 快速查找对象(const wchar_t* 名称, UClass* 必需类 = nullptr, bool 精确类 = false) {
			auto* 对象数组 = 取对象表();
			if (!对象数组) return nullptr;
			UObject* 结果 = nullptr;
			__try {
				结果 = 快速查找对象_扫描(对象数组, 名称, 必需类, 精确类);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				结果 = nullptr;
			}
			return 结果;
		}
	public:
		static UObject* 静态查找对象(UClass* 类, UObject* 外部, const wchar_t* 名称, bool 精确类 = false);
	public:
		static bool GObjects自洽(const void* 对象) {
			if (!对象) return false;
			auto* 表 = 取对象表();
			if (!表) return false;
			const int32 自身下标 = 内存辅助::读字段<int32>(对象, 偏移::UObject::Index);
			if (自身下标 < 0 || 自身下标 >= 表->数量()) return false;
			const int32 块下标 = 自身下标 / TUObjectArray::每块元素;
			const int32 块内 = 自身下标 % TUObjectArray::每块元素;
			if (块下标 >= 表->块数量) return false;
			FUObjectItem* 块 = 表->对象表[块下标];
			if (!块) return false;
			if (块[块内].对象 != 对象) return false;
			const int32 标志值 = 内存辅助::读字段<int32>(&块[块内], 8);
			if (标志值 & 0x09000000) return false;
			const int32 对象标志 = 内存辅助::读字段<int32>(对象, 偏移::UObject::Flags);
			if (对象标志 & 0x00030000) return false;
			return true;
		}
		static bool 类匹配(const void* 对象, UClass* 基类) {
			if (!对象 || !基类) return false;
			UClass* 类 = 内存辅助::读字段<UClass*>(对象, 偏移::UObject::Class);
			if (!类) return false;
			void* 结构 = 类;
			while (结构) {
				if (结构 == (void*)基类) return true;
				结构 = 内存辅助::读字段<void*>(结构, 偏移::UStruct::SuperStruct);
			}
			return false;
		}
	public:
		static int 按类收集(UClass* 类, UObject** 输出, int 上限) {
			auto* 表 = 取对象表();
			if (!表 || !类 || !输出 || 上限 <= 0) return 0;
			int 已收 = 0;
			__try {
				for (int32 i = 0; i < 表->数量() && 已收 < 上限; i++) {
					UObject* 对象 = 表->按下标取(i);
					if (!对象) continue;
					if (对象->取类() == 类) 输出[已收++] = 对象;
				}
			} __except (EXCEPTION_EXECUTE_HANDLER) {}
			return 已收;
		}
	public:
		template<class T = UClass>
		static inline T* 查找对象(UObject* 外部, const wchar_t* 名称, bool 精确类 = false)
		{
			return 静态查找对象(T::取静态类(), 外部, 名称, 精确类)->转换<T>();
		}
		template<typename 虚幻类型 = UObject>
		static inline 虚幻类型* 查找对象(const wchar_t* 名称, int 偏移 = 0)
		{
			return 静态查找对象(nullptr, reinterpret_cast<UObject*>(-1), 名称 + 偏移, false)->转换<虚幻类型>();
		}
	public:
		std::wstring 取对象名() {
			return 取名字().取宽字符串();
		}
	public:
		template<typename 目标类型>
		目标类型* 转换() { return static_cast<目标类型*>(this); }
		void* 取虚表函数(int 下标) {
			void* 虚表 = 内存辅助::读字段<void*>(this, 0);
			if (虚表) {
				void* 函数 = 内存辅助::读字段<void*>(虚表, 下标 * sizeof(void*));
				if (函数) {
					return 函数;
				}
			}
			return nullptr;
		}
	public:
		UClass* 取类() {
			return 内存辅助::读字段<UClass*>(this, 偏移::UObject::Class);
		}
		FName 取名字() {
			return 内存辅助::读字段<FName>(this, 偏移::UObject::Name);
		}
	public:
		bool 是否类(UClass* 类型类);
	public:
		void 处理事件(UFunction* 函数, void* 参数);
	};
	class FField {
	public:
		FField* 取下一个() {
			return 内存辅助::读字段<FField*>(this, 偏移::FField::Next);
		}
		UClass* 取类() {
			return 内存辅助::读字段<UClass*>(this, 偏移::FField::Class);
		}
		FName 取名字() {
			return 内存辅助::读字段<FName>(this, 偏移::FField::Name);
		}
	};
	class FProperty : public FField {
	public:
		int32 取偏移() {
			return 内存辅助::读字段<int32>(this, 偏移::Property::Offset_Internal);
		}
	};
	class UField : public UObject {
	public:
		UField* 取下一个() {
			return 内存辅助::读字段<UField*>(this, 偏移::UField::Next);
		}
	};
	class UStruct : public UField {
	public:
		UStruct* 取父结构() {
			return 内存辅助::读字段<UStruct*>(this, 偏移::UStruct::SuperStruct);
		}
		UField* 取子字段() {
			return 内存辅助::读字段<UField*>(this, 偏移::UStruct::Children);
		}
		FField* 取子属性() {
			return 内存辅助::读字段<FField*>(this, 偏移::UStruct::ChildProperties);
		}
		bool 是否子类(const UStruct* 基类) {
			if (!基类)return false;
			for (UStruct* 结构 = this; 结构; 结构 = 结构->取父结构())
			{
				if (结构 == 基类)
					return true;
			}
			return false;
		}
		int32 取偏移(const std::wstring& 属性名) {
			for (UStruct* 当前 = this; 当前; 当前 = 当前->取父结构())
			{
				for (FProperty* 属性 = (FProperty*)当前->取子属性(); 属性; 属性 = (FProperty*)属性->取下一个())
				{
					auto 偏移 = 属性->取偏移();
					if (偏移 < 0xFFFF && 属性->取名字().取宽字符串() == 属性名) {
						return 偏移;
					}
				}
			}
			return -1;
		}
	};
	class UFunction : public UStruct {
	public:
		uint32_t* 取函数标志地址() {
			return 内存辅助::取字段指针<uint32_t>(this, 偏移::UFunction::FunctionFlags);
		}
		int32 取参数大小() {
			return (int32)内存辅助::读字段<uint16_t>(this, 偏移::UFunction::ParmsSize);
		}
		bool 参数区装得下(size_t 我方大小) {
			const int32 引擎大小 = 取参数大小();
			if (引擎大小 <= 0 || 引擎大小 > 0x1000) return true;
			return 我方大小 >= (size_t)引擎大小;
		}
	};
	class UClass : public UStruct {
	public:
		EClassCastFlags 取转换标志() {
			return 内存辅助::读字段<EClassCastFlags>(this, 偏移::UClass::CastFlags);
		}
		UObject* 取默认对象() {
			return 内存辅助::读字段<UObject*>(this, 偏移::UClass::ClassDefaultObject);
		}
	public:
		UFunction* 取函数(const std::wstring& 函数名) {
			for (UField* 字段 = this->取子字段(); 字段; 字段 = 字段->取下一个())
			{
				if (((uint64)字段->取类()->取转换标志() & (uint64)EClassCastFlags::Function) && 字段->取对象名() == 函数名)
					return static_cast<class UFunction*>(字段);
			}
			return nullptr;
		}
		int32 取偏移(const std::wstring& 属性名) {
			for (FProperty* 属性 = (FProperty*)this->取子属性(); 属性; 属性 = (FProperty*)属性->取下一个())
			{
				auto 偏移 = 属性->取偏移();
				if (偏移 > 0 && 偏移 < 0xFFFF && 属性->取名字().取宽字符串() == 属性名) {
					return 偏移;
				}
			}
			return NULL;
		}
	};
	struct FHitResult final {
		uint8 Pad[0x88];
	};
	struct alignas(0x08) FEquipmentInfo final {
		uint64 取ItemID() {
			static int32 偏移 = -2;
			if (偏移 == -2) {
				UStruct* 结构 = (UStruct*)UObject::查找对象(宽加密串(L"FEquipmentInfo"), 1);
				偏移 = 结构 ? 结构->取偏移(宽加密串(L"ItemID")) : -1;
			}
			return 内存辅助::读字段<uint64>(this, 偏移);
		}
		uint8 Pad[0x30];
	};
	struct FDFMCommonItemRow final {
		int32 取Quality() {
			static int32 偏移 = -2;
			if (偏移 == -2) {
				UStruct* 结构 = (UStruct*)UObject::查找对象(宽加密串(L"FDFMCommonItemRow"), 1);
				偏移 = 结构 ? 结构->取偏移(宽加密串(L"Quality")) : -1;
			}
			return 内存辅助::读字段<int32>(this, 偏移);
		}
	};
	struct FInventoryItemInfo final {
		FDFMCommonItemRow* 取物品行() {
			static int32 锚点偏移 = -2;
			if (锚点偏移 == -2) {
				UStruct* 结构 = (UStruct*)UObject::查找对象(宽加密串(L"FInventoryItemInfo"), 1);
				锚点偏移 = 结构 ? 结构->取偏移(宽加密串(L"OwnerPlayerId")) : -1;
			}
			if (锚点偏移 < 0) return nullptr;
			return 内存辅助::读字段<FDFMCommonItemRow*>(this, 锚点偏移 + 8);
		}
	};
class UFont;
class UTexture2D;
class UEngine : public UObject {
	定义静态类方法(UEngine);
	定义属性取(UGameViewportClient*, GameViewport);
	定义属性取(UFont*, MediumFont);
};
	class UWorld : public UObject {
		定义静态类方法(UWorld);
	};
class UCanvas : public UObject {
	定义静态类方法(UCanvas);
	定义属性取(int, SizeX);
	定义属性取(int, SizeY);
	UTexture2D* 取DefaultTexture() {
		static int32 偏移 = -2;
		if (偏移 == -2) {
			auto* 静态类 = 取静态类();
			偏移 = 静态类 ? 静态类->取偏移(宽加密串(L"DefaultTexture")) : -1;
		}
		return 偏移 >= 0 ? 内存辅助::读字段<UTexture2D*>(this, 偏移) : nullptr;
	}
private:
	enum class K2槽 { 画线 = 0, 画纹理, 画文本, 槽上限 };
	bool K2发起(K2槽 槽位, const wchar_t* 函数名, void* 参数, size_t 参数大小) {
		static UFunction* 缓存[(int)K2槽::槽上限] = {};
		static bool 禁用[(int)K2槽::槽上限] = {};
		const int 槽 = (int)槽位;
		if (槽 < 0 || 槽 >= (int)K2槽::槽上限 || 禁用[槽]) return false;
		if (!缓存[槽]) {
			UClass* 类 = 取静态类();
			UFunction* 函数 = 类 ? 类->取函数(std::wstring(函数名)) : nullptr;
			if (!函数) {
				函数 = (UFunction*)静态查找对象(nullptr, (UObject*)-1, 函数名, false);
			}
			缓存[槽] = 函数;
			if (!缓存[槽]) { 禁用[槽] = true; return false; }
		}
		{
			const int32 实参大小 = 缓存[槽]->取参数大小();
			if (实参大小 > 0 && 实参大小 <= 0x1000 && (int32)参数大小 < 实参大小) {
				禁用[槽] = true;
				return false;
			}
		}
		处理事件(缓存[槽], 参数);
		return true;
	}
	public:
		void K2画线段(FVector2D A, FVector2D B, float 粗细, FLinearColor 颜色) {
			struct 参数结构 {
				FVector2D A, B; float T; FLinearColor C;
			} 参数{ A, B, 粗细, 颜色 };
			K2发起(K2槽::画线, 宽加密串(L"K2_DrawLine"), &参数, sizeof(参数));
		}
		void K2画实心矩形(FVector2D 位置, FVector2D 大小, FLinearColor 颜色) {
		UTexture2D* 白纹理 = 取DefaultTexture();
		if (!白纹理) return;
		struct 参数结构 {
			UTexture2D* 纹理; FVector2D 位置, 大小, 坐标位置, 坐标大小;
			FLinearColor 颜色; uint8 混合模式; uint8 填充[3]; float 旋转; FVector2D 轴心;
		} 参数{};
		参数.纹理 = 白纹理;
		参数.位置 = 位置; 参数.大小 = 大小;
		参数.坐标位置 = FVector2D(0, 0); 参数.坐标大小 = FVector2D(1, 1);
		参数.颜色 = 颜色;
		参数.混合模式 = 2  ;
		参数.旋转 = 0.0f; 参数.轴心 = FVector2D(0.5f, 0.5f);
		K2发起(K2槽::画纹理, 宽加密串(L"K2_DrawTexture"), &参数, sizeof(参数));
	}
	void K2画文本(UFont* 字体, const wchar_t* 文本, FVector2D 位置, FVector2D 缩放,
		FLinearColor 颜色, bool 水平居中, bool 垂直居中, bool 描边) {
		if (!字体 || !文本) return;
		struct 参数结构 {
			UFont* 字体槽; UE::FString 文本槽; FVector2D 位置槽, 缩放槽; FLinearColor 颜色槽;
			float 字距; FLinearColor 阴影颜色; FVector2D 阴影偏移;
			bool 中X, 中Y, 轮廓; uint8 填充[1]; FLinearColor 轮廓颜色;
		} 参数{};
		参数.字体槽 = 字体;
		参数.文本槽 = UE::FString{ (wchar_t*)文本 };
		参数.位置槽 = 位置; 参数.缩放槽 = 缩放;
		参数.颜色槽 = 颜色;
		参数.字距 = 0.0f;
		参数.阴影颜色 = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
		参数.阴影偏移 = FVector2D(1.0f, 1.0f);
		参数.中X = 水平居中; 参数.中Y = 垂直居中; 参数.轮廓 = 描边;
		参数.轮廓颜色 = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
		K2发起(K2槽::画文本, 宽加密串(L"K2_DrawText"), &参数, sizeof(参数));
	}
};
	class UGameViewportClient : public UObject {
		定义静态类方法(UGameViewportClient);
		定义属性取(UWorld*, World);
	};
	class UActorComponent : public UObject {
		定义静态类方法(UActorComponent);
	};
	class USceneComponent : public UActorComponent {
		定义静态类方法(USceneComponent);
		定义函数取(FVector, K2_GetComponentLocation);
	};
	class UGPTeamComponent : public UActorComponent {
		定义静态类方法(UGPTeamComponent);
		int32_t 取队伍编号() {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"GetTeamID"));
			struct {
				int32_t 返回值;
			}参数{};
			if (函数 && 函数->参数区装得下(sizeof(参数))) this->处理事件(函数, &参数);
			return 参数.返回值;
		}
	};
	class UGPAttributeBaseComponent : public UActorComponent {
		定义静态类方法(UGPAttributeBaseComponent);
	};
	class UGPHealthDataComponent : public UGPAttributeBaseComponent {
		定义静态类方法(UGPHealthDataComponent);
		float 取生命值() {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) {
				函数 = 取静态类()->取函数(宽加密串(L"GetHealth"));
			}
			struct 参数结构 {
				float 返回值;
			}参数{};
			if (函数 && 函数->参数区装得下(sizeof(参数))) 处理事件(函数, &参数);
			return 参数.返回值;
		}
		float 取最大生命值() {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) {
				函数 = 取静态类()->取函数(宽加密串(L"GetHealthMax"));
			}
			struct 参数结构 {
				float 返回值;
			}参数{};
			if (函数 && 函数->参数区装得下(sizeof(参数))) 处理事件(函数, &参数);
			return 参数.返回值;
		}
	};
	class UPrimitiveComponent : public USceneComponent {
		定义静态类方法(UPrimitiveComponent);
	};
	class UMeshComponent : public UPrimitiveComponent {
		定义静态类方法(UMeshComponent);
	};
	class USkinnedMeshComponent : public UMeshComponent {
		定义静态类方法(USkinnedMeshComponent);
		void 骨骼空间变换(const FName& 骨骼名, const FVector& 输入位置, const FRotator& 输入旋转, FVector* 输出位置, FRotator* 输出旋转) {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr)函数 = 取静态类()->取函数(宽加密串(L"TransformFromBoneSpace"));
			struct {
				FName BoneName;
				FVector InPosition;
				FRotator InRotation;
				FVector OutPosition;
				FRotator OutRotation;
			} 参数{ 骨骼名 ,输入位置 ,输入旋转 };
			if (!函数 || !函数->参数区装得下(sizeof(参数))) {
				if (输出位置) *输出位置 = FVector();
				if (输出旋转) *输出旋转 = FRotator();
				return;
			}
			if (函数 && 函数->参数区装得下(sizeof(参数))) this->处理事件(函数, &参数);
			if (输出位置 != nullptr)*输出位置 = 参数.OutPosition;
			if (输出旋转 != nullptr)*输出旋转 = 参数.OutRotation;
		}
		FVector 按名取骨骼位置(const FName& 骨骼名) {
			FVector 输出位置;
			骨骼空间变换(骨骼名, FVector(1.0f, 1.0f, 1.0f), FRotator(1.0f, 1.0f, 1.0f), &输出位置, nullptr);
			return 输出位置;
		}
	};
	class USkeletalMeshComponent : public USkinnedMeshComponent {
		定义静态类方法(USkeletalMeshComponent);
	};
	class UCharacterEquipComponent : public UActorComponent {
		定义静态类方法(UCharacterEquipComponent);
		static UCharacterEquipComponent* 获取(AActor* 所有者) {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) {
				函数 = 取静态类()->取函数(宽加密串(L"Get"));
			} struct 参数结构 {
				AActor* Owner;
				UCharacterEquipComponent* ReturnValue;
			}; 参数结构 参数{ 所有者 };
			if (函数 && 函数->参数区装得下(sizeof(参数))) 取默认对象()->处理事件(函数, &参数);
			return 参数.ReturnValue;
		}
		FEquipmentInfo 按类型取装备信息(EEquipmentType 类型) {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) {
				函数 = 取静态类()->取函数(宽加密串(L"GetEquipmentInfoByType"));
			} struct 参数结构 {
				EEquipmentType Type;
				uint8 填充[7];
				FEquipmentInfo ReturnValue;
			}; 参数结构 参数{};
			参数.Type = 类型;
			if (!函数 || !函数->参数区装得下(sizeof(参数结构))) return FEquipmentInfo{};
			if (函数 && 函数->参数区装得下(sizeof(参数))) 处理事件(函数, &参数);
			return 参数.ReturnValue;
		}
	};
	class AActor : public UObject {
		定义静态类方法(AActor);
		USceneComponent* 取根组件() {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"K2_GetRootComponent"));
			struct {
				USceneComponent* 返回值;
				uint8 填充[16];
			}参数{};
			if (函数 && 函数->参数区装得下(sizeof(参数))) 处理事件(函数, &参数);
			return 参数.返回值;
		}
	};
	class AController : public AActor {
		定义静态类方法(AController);
		定义属性取(FRotator, ControlRotation);
	};
	class APlayerController : public AController {
		定义静态类方法(APlayerController);
		定义属性取(APlayerCameraManager*, PlayerCameraManager);
		void 加偏航输入(float 数值) {
			static UFunction* 函数 = nullptr;
			if (!函数) 函数 = 取静态类()->取函数(宽加密串(L"AddYawInput"));
			struct { float 数值; } 参数{ 数值 };
			if (函数 && 函数->参数区装得下(sizeof(参数))) 处理事件(函数, &参数);
		}
		void 加俯仰输入(float 数值) {
			static UFunction* 函数 = nullptr;
			if (!函数) 函数 = 取静态类()->取函数(宽加密串(L"AddPitchInput"));
			struct { float 数值; } 参数{ 数值 };
			if (函数 && 函数->参数区装得下(sizeof(参数))) 处理事件(函数, &参数);
		}
	bool 世界坐标转屏幕(FVector 世界位置, FVector2D& 屏幕位置, bool 相对视口 = false) {
		static UFunction* 函数 = nullptr;
		if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"ProjectWorldLocationToScreen"));
		struct {
			FVector WorldLocation;
			FVector2D ScreenLocation;
			bool bPlayerViewportRelative;
			bool ReturnValue;
		} 参数{ 世界位置 ,FVector2D(),相对视口 };
		if (函数 && 函数->参数区装得下(sizeof(参数))) 处理事件(函数, &参数);
		屏幕位置 = 参数.ScreenLocation;
		return 参数.ReturnValue;
	}
	};
	class APlayerCameraManager : public AActor {
		定义静态类方法(APlayerCameraManager);
		FRotator 取相机旋转() {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"GetCameraRotation"));
			struct {
				FRotator 返回值;
			}参数{};
			if (函数 && 函数->参数区装得下(sizeof(参数))) this->处理事件(函数, &参数);
			return 参数.返回值;
		}
		FVector 取相机位置() {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"GetCameraLocation"));
			struct {
				FVector 返回值;
			}参数{};
			if (函数 && 函数->参数区装得下(sizeof(参数))) this->处理事件(函数, &参数);
			return 参数.返回值;
		}
		float 取FOV() {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"GetFOVAngle"));
			struct {
				float 返回值;
			}参数{};
			if (函数 && 函数->参数区装得下(sizeof(参数))) this->处理事件(函数, &参数);
			return 参数.返回值;
		}
	};
	class APawn : public AActor {
		定义静态类方法(APawn);
		定义属性取(APlayerState*, PlayerState);
	};
	class ACHARACTER : public APawn {
		定义静态类方法(ACHARACTER);
	};
	class AIntCharacter : public ACHARACTER {
		定义静态类方法(AIntCharacter);
		bool 是否死亡() {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr)函数 = 取静态类()->取函数(宽加密串(L"IsDead"));
			struct {
				bool 返回值;
				uint8 填充[15];
			} 参数{ true, {} };
			if (函数 && 函数->参数区装得下(sizeof(参数))) this->处理事件(函数, &参数);
			return 参数.返回值;
		}
	};
	class ACharacterBase : public AIntCharacter {
		定义静态类方法(ACharacterBase);
	};
	class AGPCharacterBase : public ACharacterBase {
		定义静态类方法(AGPCharacterBase);
		bool 是否AI() {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"IsAI"));
			struct {
				bool 返回值;
				uint8 填充[15];
			}参数{};
			if (函数 && 函数->参数区装得下(sizeof(参数))) this->处理事件(函数, &参数);
			return 参数.返回值;
		}
		UGPTeamComponent* 取队伍组件() {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"GetTeamComp"));
			struct {
				UGPTeamComponent* 返回值;
				uint8 填充[8];
			}参数{};
			if (函数 && 函数->参数区装得下(sizeof(参数))) this->处理事件(函数, &参数);
			return 参数.返回值;
		}
		UGPHealthDataComponent* 取生命组件() {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"GetHealthComp"));
			struct {
				UGPHealthDataComponent* 返回值;
				uint8 填充[8];
			}参数{};
			if (函数 && 函数->参数区装得下(sizeof(参数))) this->处理事件(函数, &参数);
			return 参数.返回值;
		}
		定义属性取(AWeaponBase*, CacheCurWeapon);
	};
	class AGPCharacter : public AGPCharacterBase {
		定义静态类方法(AGPCharacter);
	};
	class ADFMCharacter : public AGPCharacter {
		定义静态类方法(ADFMCharacter);
		定义属性取(FName, PoolName);
		定义函数取(FString, GetPoolChosenName);
	};
	class ADFMPlayerCharacter : public ADFMCharacter {
		定义静态类方法(ADFMPlayerCharacter);
	};
	class ADFMRangeTargetCharacter : public ADFMPlayerCharacter {
		定义静态类方法(ADFMRangeTargetCharacter);
	};
	class ADFMAICharacter : public ADFMCharacter {
		定义静态类方法(ADFMAICharacter);
	};
	class ADFMAIAnimalCharacter : public ADFMAICharacter {
		定义静态类方法(ADFMAIAnimalCharacter);
	};
	class AWeaponBase : public AActor {
		定义静态类方法(AWeaponBase);
		定义属性取(uint64_t, WeaponID);
		定义属性取(bool, bIsSupportWeapon);
	};
	class AInfo : public AActor {
		定义静态类方法(AInfo);
	};
	class APlayerState : public AInfo {
		定义静态类方法(APlayerState);
		定义属性取(int32_t, PlayerId);
		定义属性取(FString, PlayerNamePrivate);
	};
	class ADFMPlayerState : public APlayerState {
		定义静态类方法(ADFMPlayerState);
		定义属性取(int64_t, HeroId);
		定义属性取(EExitState, ExitState);
	};
	class AGPInteractorBase : public AActor {
		定义静态类方法(AGPInteractorBase);
	};
	class AInteractorBase : public AGPInteractorBase {
		定义静态类方法(AInteractorBase);
		定义属性取(EMarkingItemType, MarkingItemType);
	};
	class APickupBase : public AInteractorBase {
		定义静态类方法(APickupBase);
		定义属性取(FName, InventoryIdName);
		定义函数取(FText, GetItemName);
	};
	class AInventoryPickup : public APickupBase {
		定义静态类方法(AInventoryPickup);
		定义属性指针(FInventoryItemInfo, PickupItemInfo);
	};
	class AInventoryPickup_Container : public AInventoryPickup {
		定义静态类方法(AInventoryPickup_Container);
	};
	class AInventoryPickup_DeadBody : public AInventoryPickup_Container {
		定义静态类方法(AInventoryPickup_DeadBody);
		定义属性取(AGPCharacterBase*, LootingCharacterOwner);
		定义属性取(APlayerState*, OwnerPlayerState);
		定义属性取(bool, bLooted);
		定义函数取(FString, GetPlayerName);
	};
	class UBlueprintFunctionLibrary : public UObject {
		定义静态类方法(UBlueprintFunctionLibrary);
	};
	class UGPScalabilityBlueprintTools : public UBlueprintFunctionLibrary {
		定义静态类方法(UGPScalabilityBlueprintTools);
		定义静态函数取(EMainFlowState, GetMainFlowState);
	};
	class UGPWeaponBlueprintLibrary : public UBlueprintFunctionLibrary {
		定义静态类方法(UGPWeaponBlueprintLibrary);
		static APlayerController* 取玩家控制器(UObject* 世界上下文) {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"GetPlayerController"));
			struct {
				UObject* WorldContext;
				APlayerController* ReturnValue;
			}参数{ 世界上下文 };
			if (函数 && 函数->参数区装得下(sizeof(参数))) 取默认对象()->处理事件(函数, &参数);
			return 参数.ReturnValue;
		}
		static APlayerCameraManager* 取玩家相机管理器(UObject* 世界上下文) {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"GetPlayerCameraManager"));
			struct {
				UObject* WorldContext;
				APlayerCameraManager* ReturnValue;
			}参数{ 世界上下文 };
			if (函数 && 函数->参数区装得下(sizeof(参数))) 取默认对象()->处理事件(函数, &参数);
			return 参数.ReturnValue;
		}
	};
	class UGameplayBlueprintHelper : public UBlueprintFunctionLibrary {
		定义静态类方法(UGameplayBlueprintHelper);
		static AGPCharacter* 取本地角色(class UObject* 世界上下文) {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"GetLocalGPCharacter"));
			struct {
				UObject* WorldContext;
				AGPCharacter* ReturnValue;
			}参数{ 世界上下文 };
			if (函数 && 函数->参数区装得下(sizeof(参数))) 取默认对象()->处理事件(函数, &参数);
			return 参数.ReturnValue;
		}
	};
	class UGameplayStatics : public UBlueprintFunctionLibrary {
		定义静态类方法(UGameplayStatics);
		static void 取所有同类的参与者(UObject* 世界上下文对象, UClass* 参与者类, TArray<AActor*>* 输出参与者) {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"GetAllActorsOfClass"));
			struct {
				UObject* WorldContextObject;
				UClass* ActorClass;
				TArray<AActor*> OutActors;
			}参数{ 世界上下文对象,参与者类 };
			if (!函数 || !函数->参数区装得下(sizeof(参数))) return;
			if (函数 && 函数->参数区装得下(sizeof(参数))) 取默认对象()->处理事件(函数, &参数);
			if (输出参与者 != nullptr)
				*输出参与者 = std::move(参数.OutActors);
		}
	};
	class UKismetTextLibrary : public UBlueprintFunctionLibrary {
		定义静态类方法(UKismetTextLibrary);
		static FString 文本转字符串(const FText& 输入文本) {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) 函数 = 取静态类()->取函数(宽加密串(L"Conv_TextToString"));
			struct {
				FText InText;
				FString ReturnValue;
			}参数{ 输入文本 };
			if (!函数 || !函数->参数区装得下(sizeof(参数))) return FString();
			if (函数 && 函数->参数区装得下(sizeof(参数))) 取默认对象()->处理事件(函数, &参数);
			return 参数.ReturnValue;
		}
	};
	class UKismetStringLibrary : public UObject {
		定义静态类方法(UKismetStringLibrary);
		static FName 字符串转名称(const FString& 输入字符串) {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr)函数 = 取静态类()->取函数(宽加密串(L"Conv_StringToName"));
			struct {
				FString InString;
				FName ReturnValue;
			} 参数 = { 输入字符串 };
			if (!函数 || !函数->参数区装得下(sizeof(参数))) return FName();
			if (函数 && 函数->参数区装得下(sizeof(参数))) 取默认对象()->处理事件(函数, &参数);
			return 参数.ReturnValue;
		}
	};
	class UKismetSystemLibrary : public UBlueprintFunctionLibrary {
		定义静态类方法(UKismetSystemLibrary);
		static bool 单线追踪(UObject* 世界上下文对象, const FVector& 起点, const FVector& 终点, ETraceTypeQuery 追踪通道, bool 追踪复杂, const TArray<AActor*>& 忽略参与者, EDrawDebugTrace 调试绘制, FHitResult* 输出命中, bool 忽略自身, const FLinearColor& 追踪颜色, const FLinearColor& 命中颜色, float 绘制时间) {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr)函数 = 取静态类()->取函数(宽加密串(L"LineTraceSingle"));
			struct {
				UObject* WorldContextObject;
				FVector Start;
				FVector End;
				ETraceTypeQuery TraceChannel;
				bool bTraceComplex;
				unsigned char UnknownData_H2FF[0x2];
				TArray<AActor*> ActorsToIgnore;
				EDrawDebugTrace DrawDebugType;
				unsigned char UnknownData_DZIR[0x3];
				FHitResult OutHit;
				bool bIgnoreSelf;
				unsigned char UnknownData_ACTN[0x3];
				FLinearColor TraceColor;
				FLinearColor TraceHitColor;
				float DrawTime;
				bool ReturnValue;
			}参数{};
			if (!函数 || !函数->参数区装得下(sizeof(参数))) return false;
			参数.WorldContextObject = 世界上下文对象;
			参数.Start = 起点;
			参数.End = 终点;
			参数.TraceChannel = 追踪通道;
			参数.bTraceComplex = 追踪复杂;
			参数.ActorsToIgnore = 忽略参与者;
			参数.DrawDebugType = 调试绘制;
			参数.bIgnoreSelf = 忽略自身;
			参数.TraceColor = 追踪颜色;
			参数.TraceHitColor = 命中颜色;
			参数.DrawTime = 绘制时间;
			if (函数 && 函数->参数区装得下(sizeof(参数))) 取默认对象()->处理事件(函数, &参数);
			if (输出命中 != nullptr)*输出命中 = 参数.OutHit;
			return 参数.ReturnValue;
		}
		static bool 单线追踪(UObject* 世界上下文对象, AActor* 本地角色, AActor* 忽略参与者, const FVector& 追踪起点, const FVector& 追踪终点, FHitResult& 命中结果) {
			static TArray<AActor*> 忽略列表;
			if (忽略列表.取数据指针() == nullptr) {
				忽略列表.数据 = new AActor * [2];
				忽略列表.最大元素 = 2;
			}
			忽略列表.元素数量 = 0;
			忽略列表.添加(本地角色);
			忽略列表.添加(忽略参与者);
			return 单线追踪(世界上下文对象, 追踪起点, 追踪终点, ETraceTypeQuery::TraceTypeQuery1, true, 忽略列表, EDrawDebugTrace::None, &命中结果, true, FLinearColor(1.f, 0.f, 0.f, 1.f), FLinearColor(0.f, 1.f, 0.f, 1.f), 5.0f);
		}
		static bool 单线追踪(UObject* 世界上下文对象, AActor* 本地角色, AActor* 忽略参与者, const FVector& 追踪起点, const FVector& 追踪终点) {
			FHitResult 命中结果;
			memset(&命中结果, 0, sizeof(命中结果));
			return 单线追踪(世界上下文对象, 本地角色, 忽略参与者, 追踪起点, 追踪终点, 命中结果);
		}
	};
	class UMTAPI_UEngine : public UObject {
		定义静态类方法(UMTAPI_UEngine);
		定义静态函数取(UEngine*, GetEngine);
	};
	class UMTAPI_ACharacter : public UObject {
		定义静态类方法(UMTAPI_ACharacter);
		static USkeletalMeshComponent* 取网格(ACHARACTER* 自身) {
			static UFunction* 函数 = nullptr;
			if (函数 == nullptr) {
				函数 = 取静态类()->取函数(宽加密串(L"GetMesh"));
			}
			struct {
				ACHARACTER* Self;
				USkeletalMeshComponent* ReturnValue;
			} 参数{ 自身 };
			if (函数 && 函数->参数区装得下(sizeof(参数))) 取默认对象()->处理事件(函数, &参数);
			return 参数.ReturnValue;
		};
	};
}


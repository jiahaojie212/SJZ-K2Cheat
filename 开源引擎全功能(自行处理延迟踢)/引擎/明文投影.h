#pragma once
#include <cmath>
extern 全局数据 游戏全局数据;
namespace 明文投影 {
	struct 状态 {
		bool 可用 = false;
	};
	inline 状态 S;
	constexpr float PI = 3.14159265358979323846f;
	inline void 每帧刷新(UCanvas* 画布) {
		if (!画布) { S.可用 = false; return; }
		void* 虚表 = 内存辅助::读字段<void*>(画布, 0);
		S.可用 = 内存辅助::有效用户地址(虚表);
	}
	inline bool 世界转屏幕(const FVector& 世界位置, FVector2D* 屏幕) {
		if (!S.可用 || !屏幕) return false;
		const int 屏宽 = 游戏全局数据.屏幕宽;
		const int 屏高 = 游戏全局数据.屏幕高;
		if (屏宽 < 8 || 屏高 < 8) return false;
		float 视场角 = 游戏全局数据.相机FOV;
		if (!(视场角 > 1.0f && 视场角 < 170.0f)) 视场角 = 90.0f;
		const FVector& 相机 = 游戏全局数据.相机位置;
		const FRotator& 旋转 = 游戏全局数据.相机旋转;
		const float 俯仰 = 旋转.Pitch * PI / 180.0f;
		const float 偏航 = 旋转.Yaw * PI / 180.0f;
		const float 滚转 = 旋转.Roll * PI / 180.0f;
		const float sinP = std::sin(俯仰), cosP = std::cos(俯仰);
		const float sinY = std::sin(偏航), cosY = std::cos(偏航);
		const float sinR = std::sin(滚转), cosR = std::cos(滚转);
		const float 前X = cosP * cosY,            前Y = cosP * sinY,            前Z = sinP;
		const float 右X = sinR * sinP * cosY - cosR * sinY;
		const float 右Y = sinR * sinP * sinY + cosR * cosY;
		const float 右Z = -sinR * cosP;
		const float 上X = -(cosR * sinP * cosY + sinR * sinY);
		const float 上Y = cosY * sinR - cosR * sinP * sinY;
		const float 上Z = cosR * cosP;
		const float 差X = 世界位置.X - 相机.X;
		const float 差Y = 世界位置.Y - 相机.Y;
		const float 差Z = 世界位置.Z - 相机.Z;
		const float 视深 = 差X * 前X + 差Y * 前Y + 差Z * 前Z;
		if (视深 < 1.0f) return false;
		const float 视右 = 差X * 右X + 差Y * 右Y + 差Z * 右Z;
		const float 视上 = 差X * 上X + 差Y * 上Y + 差Z * 上Z;
		const float 缩放 = (屏宽 * 0.5f) / std::tan(视场角 * 0.5f * PI / 180.0f);
		屏幕->X = 屏宽 * 0.5f + (视右 / 视深) * 缩放;
		屏幕->Y = 屏高 * 0.5f - (视上 / 视深) * 缩放;
		return true;
	}
}

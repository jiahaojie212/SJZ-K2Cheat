#pragma once
#include <cstdint>
inline constexpr uint32_t 打包颜色(float r, float g, float b, float a) {
	uint32_t R = (uint32_t)(r * 255.0f); R = R > 255 ? 255 : R;
	uint32_t G = (uint32_t)(g * 255.0f); G = G > 255 ? 255 : G;
	uint32_t B = (uint32_t)(b * 255.0f); B = B > 255 ? 255 : B;
	uint32_t A = (uint32_t)(a * 255.0f); A = A > 255 ? 255 : A;
	return (A << 24) | (B << 16) | (G << 8) | R;
}
#define 颜色32(r, g, b, a) 打包颜色((r) / 255.0f, (g) / 255.0f, (b) / 255.0f, (a) / 255.0f)
namespace 原生绘制 { extern float 全局缩放; }
#define SCALE(v) ((v) * 原生绘制::全局缩放)
namespace 原生绘制 {
	bool 初始化();
	void 设置画布(void* 画布指针);
	void 每帧开始(int 宽度, int 高度);
	void 画线段(float x1, float y1, float x2, float y2, uint32_t 颜色值, float 粗细);
	void 画矩形填充(float x1, float y1, float x2, float y2, uint32_t 颜色值);
	void 画圆形(float cx, float cy, float 半径, uint32_t 颜色值, float 粗细, int 分段 = 24);
	float 文本宽度(float 字号, const char* 文本);
	void 文本(float 字号, float x, float y, uint32_t 颜色值, const char* 文本, bool 居中, bool 描边 = true, bool 垂直居中 = false);
}

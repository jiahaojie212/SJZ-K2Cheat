#include "预编译头.h"
#include "原生绘制.h"
#include "../游戏类.h"
#include "../引擎.h"
extern 全局数据 游戏全局数据;
namespace 原生绘制 {
	float 全局缩放 = 1.0f;
	static UE::UCanvas* 画布 = nullptr;
	static UE::UCanvas* 画布壳 = nullptr;
	static UE::UFont* 字体 = nullptr;
	static int 屏宽 = 0;
	static int 屏高 = 0;
	static void 选择呈现画布() {
		static UE::UObject* 实例[8] = {};
		static DWORD 下次扫 = 0;
		static UE::UCanvas* 上帧壳 = nullptr;
		const DWORD 现在 = GetTickCount();
		if (GetTickCount64() < 游戏全局数据.切图抑制截止) {
			画布 = 画布壳;
			return;
		}
		if (画布壳 != 上帧壳) {
			上帧壳 = 画布壳;
			下次扫 = 0;
			for (int i = 0; i < 8; i++) 实例[i] = nullptr;
			画布 = 画布壳;
			return;
		}
		if (现在 >= 下次扫) {
			下次扫 = 现在 + 1000;
			UE::UClass* 画布类 = UE::UCanvas::取静态类();
			if (画布类) {
				int 数 = UE::UObject::按类收集(画布类, 实例, 8);
				for (int i = 数; i < 8; i++) 实例[i] = nullptr;
			}
		}
		if (!画布壳) return;
		void* 壳外部 = 内存辅助::读字段<void*>(画布壳, 偏移::UObject::Outer);
		const int 壳宽 = 画布壳->取SizeX();
		const int 壳高 = 画布壳->取SizeY();
		UE::UClass* 画布类 = UE::UCanvas::取静态类();
		for (int i = 0; i < 8; i++) {
			UE::UObject* 候 = 实例[i];
			if (!候) break;
			if ((UE::UCanvas*)候 == 画布壳) continue;
			if (画布类 && 候->取类() != 画布类) continue;
			if (内存辅助::读字段<void*>(候, 偏移::UObject::Outer) != 壳外部) continue;
			UE::UCanvas* 候画 = (UE::UCanvas*)候;
			if (候画->取SizeX() != 壳宽 || 候画->取SizeY() != 壳高) continue;
			画布 = 候画;
			return;
		}
	}
	static void 选CJK字体() {
		UE::UObject* 当前 = (UE::UObject*)字体;
		UE::UClass* 字体类 = 当前->取类();
		if (!字体类) return;
		UE::UObject* 候选[64] = {};
		const int 数 = UE::UObject::按类收集(字体类, 候选, 64);
		const std::wstring 固定序[] = {
			宽加密串(L"FZLTZHJW_ZH_Font"),
			宽加密串(L"FZLTHJW_Font"),
			宽加密串(L"FZLanTingHei"),
			宽加密串(L"SimHei"), 宽加密串(L"msyh"), 宽加密串(L"YaHei"),
		};
		for (const std::wstring& 固定名 : 固定序) {
			for (int i = 0; i < 数; i++) {
				UE::UObject* f = 候选[i];
				if (!f || f == 当前) continue;
				const std::wstring 名 = f->取对象名();
				if (名 == 固定名) {
					字体 = (UE::UFont*)f;
					return;
				}
			}
		}
		const std::wstring 关键词[] = {
			宽加密串(L"FZL"), 宽加密串(L"Hei"), 宽加密串(L"hei"), 宽加密串(L"YaHei"), 宽加密串(L"SimHei"), 宽加密串(L"Chinese"), 宽加密串(L"CHS"),
			宽加密串(L"CJK"), 宽加密串(L"HanYi"), 宽加密串(L"SourceHan"), 宽加密串(L"Noto"), 宽加密串(L"yahei"), 宽加密串(L"msyh"), 宽加密串(L"Zh"), 宽加密串(L"zh"),
		};
		for (int i = 0; i < 数; i++) {
			UE::UObject* f = 候选[i];
			if (!f || f == 当前) continue;
			const std::wstring 名 = f->取对象名();
			for (const std::wstring& k : 关键词) {
				if (名.find(k) != std::wstring::npos) {
					字体 = (UE::UFont*)f;
					return;
				}
			}
		}
	}
	bool 初始化() {
		if (字体) return true;
		UE::UEngine* 引擎 = UE::UMTAPI_UEngine::GetEngine();
		if (!引擎) return false;
		字体 = 引擎->取MediumFont();
		if (!字体) {
			字体 = (UE::UFont*)UE::UObject::静态查找对象(
				nullptr, (UE::UObject*)-1, 宽加密串(L"FZLTZHJW_ZH_Font"), false);
		}
		if (!字体) {
			字体 = (UE::UFont*)UE::UObject::静态查找对象(
				nullptr, (UE::UObject*)-1, 宽加密串(L"FZLTHJW_Font"), false);
		}
		if (字体) {
			选CJK字体();
		}
		return 字体 != nullptr;
	}
	void 设置画布(void* 画布指针) {
		画布壳 = (UE::UCanvas*)画布指针;
	}
	void 每帧开始(int 宽度, int 高度) {
		选择呈现画布();
		if (宽度 > 0) {
			float s = (float)宽度 / 1920.0f;
			if (s < 0.4f) s = 0.4f;
			if (s > 1.7f) s = 1.7f;
			全局缩放 = s;
		}
		屏宽 = 宽度;
		屏高 = 高度;
	}
	static inline FLinearColor 解包颜色(uint32_t v) {
		return FLinearColor((float)(v & 0xFF) / 255.0f,
						 (float)((v >> 8) & 0xFF) / 255.0f,
						 (float)((v >> 16) & 0xFF) / 255.0f,
						 (float)((v >> 24) & 0xFF) / 255.0f);
	}
	void 画线段(float x1, float y1, float x2, float y2, uint32_t 颜色值, float 粗细) {
		if (!画布) return;
		画布->K2画线段(FVector2D(x1, y1), FVector2D(x2, y2), 粗细, 解包颜色(颜色值));
	}
	void 画矩形填充(float x1, float y1, float x2, float y2, uint32_t 颜色值) {
		if (!画布) return;
		if (x2 < x1) { float t = x1; x1 = x2; x2 = t; }
		if (y2 < y1) { float t = y1; y1 = y2; y2 = t; }
		画布->K2画实心矩形(FVector2D(x1, y1), FVector2D(x2 - x1, y2 - y1), 解包颜色(颜色值));
	}
	void 画圆形(float cx, float cy, float 半径, uint32_t 颜色值, float 粗细, int 分段) {
		if (!画布 || 分段 < 3) return;
		const float 步长 = 3.14159265f * 2.0f / (float)分段;
		const FLinearColor c = 解包颜色(颜色值);
		float 前X = cx + 半径, 前Y = cy;
		for (int i = 1; i <= 分段; i++) {
			const float a = 步长 * (float)i;
			const float x = cx + cosf(a) * 半径;
			const float y = cy + sinf(a) * 半径;
			画布->K2画线段(FVector2D(前X, 前Y), FVector2D(x, y), 粗细, c);
			前X = x; 前Y = y;
		}
	}
	float 文本宽度(float 字号, const char* 文本) {
		if (!文本) return 0.0f;
		float w = 0.0f;
		for (const unsigned char* p = (const unsigned char*)文本; *p; p++) {
			if (*p < 0x80) { w += 0.52f * 字号; }
			else if ((*p & 0xE0) == 0xC0) { w += 0.62f * 字号; p += 1; }
			else if ((*p & 0xF0) == 0xE0) { w += 1.00f * 字号; p += 2; }
			else if ((*p & 0xF8) == 0xF0) { w += 1.30f * 字号; p += 3; }
		}
		return w;
	}
	void 文本(float 字号, float x, float y, uint32_t 颜色值, const char* 文本, bool 居中, bool 描边, bool 垂直居中) {
		if (!画布 || !文本) return;
		if (!字体) { 初始化(); if (!字体) return; }
		wchar_t 宽缓冲[512];
		const int 长度 = MultiByteToWideChar(CP_UTF8, 0, 文本, -1, 宽缓冲, 512);
		if (长度 <= 0 || 长度 > 511) return;
		const FVector2D 缩放(字号 / 16.0f, 字号 / 16.0f);
		画布->K2画文本(字体, 宽缓冲, FVector2D(x, y), 缩放, 解包颜色(颜色值), 居中, 垂直居中, 描边);
	}
}

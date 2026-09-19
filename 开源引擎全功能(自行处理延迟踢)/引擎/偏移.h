#pragma once
#include <cstdint>
typedef int int32;
namespace 偏移 {
	namespace UObject {
		inline int32 Class = 0x08;
		inline int32 Outer = 0x10;
		inline int32 Flags = 0x18;
		inline int32 Name = 0x1C;
		inline int32 Index = 0x24;
		int32 取GObjects();
	}
	namespace FField {
		inline int32 Next = 0x18;
		inline int32 Class = 0x20;
		inline int32 Name = 0x28;
	}
	namespace Property {
		inline int32 Offset_Internal = 0x54;
	}
	namespace UField {
		inline int32 Next = 0x28;
	}
	namespace UStruct {
		inline int32 SuperStruct = 0x48;
		inline int32 Children = 0x58;
		inline int32 ChildProperties = 0x70;
	}
	namespace UFunction {
		inline int32 ParmsSize = 0xBA;
		inline int32 FunctionFlags = 0xC2;
	}
	namespace UClass {
		inline int32 CastFlags = 0xE0;
		inline int32 ClassDefaultObject = 0x140;
	}
	namespace Name {
		int32 取AppendNameToString();
	}
	namespace ProcessEvent {
		int32 取PEIndex();
	}
	namespace UCanvas {
		inline int32 PostRender = 0x60;
	}
}

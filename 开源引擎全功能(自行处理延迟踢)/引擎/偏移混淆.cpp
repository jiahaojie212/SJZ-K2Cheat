#include <windows.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include "../obf/obf.h"
#include "偏移.h"
using namespace ithare::obf;
namespace 偏移 {
	namespace UObject {
		int32 取GObjects() { return OBF5I(0x1E34DE88); }
	}
	namespace Name {
		int32 取AppendNameToString() { return OBF5I(0x10B77BF0); }
	}
	namespace ProcessEvent {
		int32 取PEIndex() { return OBF5I(0x44); }
	}
}

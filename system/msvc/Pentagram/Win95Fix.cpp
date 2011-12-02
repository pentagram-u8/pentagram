#include "pent_include.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Win95Fix {
	static BOOL WINAPI IsDebuggerPresent() {
		static HMODULE kernel32 = 0;
		static BOOL (WINAPI * fpIsDebuggerPresent)() = 0;
		if (!kernel32) {
			kernel32 = GetModuleHandle("KERNEL32");
			*(FARPROC*)& fpIsDebuggerPresent = GetProcAddress(kernel32,"IsDebuggerPresent");
		}

		return fpIsDebuggerPresent?fpIsDebuggerPresent():0;
	}

	__declspec(naked) static LONG WINAPI InterlockedCompareExchange ( __inout LONG volatile *Destination, __in    LONG Exchange, __in    LONG Comperand )
	{
		__asm {
			MOV				ecx, [esp+4]
			MOV				edx, [esp+8];
			MOV				eax, [esp+12];
			LOCK CMPXCHG	[ecx], edx
			ret 12
		}
	}

#pragma warning (disable: 4483)
	extern "C" {
		BOOL (WINAPI * __identifier("_imp__IsDebuggerPresent@0"))() = &IsDebuggerPresent;
		LONG (WINAPI * __identifier("_imp__InterlockedCompareExchange@12")) ( __inout LONG volatile *Destination, __in    LONG Exchange, __in    LONG Comperand ) = &InterlockedCompareExchange;
	}

}


/*
Copyright (C) 2002-2004 The Pentagram Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//
// memset_n is a set of optimized functions for filling buffers of 
// 16 and 32 bit integers
// 

#ifndef MEMSET_N_H_INCLUDED
#define MEMSET_N_H_INCLUDED

namespace Pentagram {

#if defined(__GNUC__) && defined(i386)

//
// GNU i386 memset_32
//
// asm version for the memset32 for gcc/x86
//
// borrowed from SDL's src/video/SDL_memops.h (SDL_memset4)
//
// buf should be DWORD aligned
//
inline void memset_32_aligned(void *buf, uint32 val, uint32 dwords)
{
	int u0, u1, u2;
    __asm__ __volatile__ (                                  \
			"cld\n\t"                                       \
            "rep ; stosl\n\t"                               \
            : "=&D" (u0), "=&a" (u1), "=&c" (u2)            \
            : "0" (buf), "1" (val), "2" (dwords)            \
            : "memory" );
}

#elif defined(_MSC_VER) && defined(_M_IX86) && (_M_IX86 >= 300)

#ifdef USE_MMX_ASM	// Disabled by default, no obvious speed up

//
// MSVC MMX memset_32
//
// asm version for the memset32 for MSVC MMX x86 
//
// buf should be DWORD aligned
//
inline void memset_32_aligned(void *buf, uint32 val, uint32 dwords)
{
	// Qword align
	if ((uint32)(buf) & 4) 
	{
		*(reinterpret_cast<uint32*>(buf)) = val;
		buf = (reinterpret_cast<uint32*>(buf))+1;
		dwords--;
	}

	if (dwords > 1) 
	{
		__asm {
			cld
			mov   edi,  buf
			mov   ecx,  dwords
			shr	  ecx,  1
			mov   eax,  val
			movd  mm0,  eax
			movd  mm1,  eax
			psllq mm1,  32
			por   mm0,  mm1
			align 16
	repeat:
			movq [edi], mm0
			add   edi,  8
			loop  repeat
			emms
		};
	}

	// Final dword
	if (dwords & 1) *(reinterpret_cast<uint32*>(buf)) = val;
}

#else // USE_MMX_ASM

//
// MSVC i386 memset_32
//
// asm version for the memset32 for MSVC x86 
//
// buf should be DWORD aligned
//
inline void memset_32_aligned(void *buf, uint32 val, uint32 dwords)
{
	__asm {
		cld
		mov edi, buf
		mov eax, val
		mov ecx, dwords
		repne stosd
	};
}

#endif // USE_MMX_ASM

#else

//
// Generic memset_32
//
// Can be used by all
//
inline void memset_32_aligned(void *buf, uint32 val, uint32 dwords)
{
	do
	{ 
		*reinterpret_cast<uint32*>(buf) = val; 
		buf = (reinterpret_cast<uint32*>(buf))+1;
	} 
	while (--dwords);
}

#endif

//
// memset_32
//
// Can be used by all
//
inline void memset_32(void *buf, uint32 val, uint32 dwords)
{
	// Fisrly we should dword Align it
	int align = 0;
	if (reinterpret_cast<uintptr>(buf) & 3) 
	{
		align = 4;
		dwords--;

		// Ok, shift along by 1 byte
		if ((reinterpret_cast<uintptr>(buf) & 1))
		{
			*reinterpret_cast<uint8*>(buf) = static_cast<uint8>(val&0xFF); 
			buf = (reinterpret_cast<uint8*>(buf))+1;
			val = ((val& 0xFF) << 24) || ((val& 0xFFFFFF00) >> 8);
			align --;
		}

		// Ok, shift along by 2 bytes
		if ((reinterpret_cast<uintptr>(buf) & 2))
		{
			*reinterpret_cast<uint16*>(buf) = static_cast<uint16>(val&0xFFFF); 
			buf = (reinterpret_cast<uint16*>(buf))+1;
			val = ((val& 0xFFFF) << 16) || ((val& 0xFFFF0000) >> 16);
			align-=2;
		}
	}

	// Fill Aligned
	if (dwords) memset_32_aligned(buf,val,dwords);

	// Do the unaligned data
	if (align)
	{
		// Ok, shift along by 1 byte
		if (align == 1)
		{
			*reinterpret_cast<uint8*>(buf) = static_cast<uint8>(val&0xFF); 
		}
		// Ok, shift along by 2 bytes
		else
		{
			*reinterpret_cast<uint16*>(buf) = static_cast<uint16>(val&0xFFFF); 

			// Ok, shift along by another byte
			if (align & 1) *(reinterpret_cast<uint8*>(buf)+2) = static_cast<uint8>((val>>16)&0xFF); 
		}
	}
}

//
// memset_16
//
// Can be used by all
//
inline void memset_16(void *buf, sint32 val, uint32 words)
{
	// Use memset_32
	if (words > 1) memset_32(buf,val|val<<16,words>>1);

	// Final word
	if (words & 1) *(reinterpret_cast<uint16*>(buf)) = static_cast<uint16>(val&0xFFFF);
}

}

#endif //MEMSET_N_H_INCLUDED

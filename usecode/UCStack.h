/*
Copyright (C) 2002 The Pentagram team

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

#ifndef UCSTACK_H
#define UCSTACK_H

#include "IDataSource.h"

// A little-endian stack for use with usecode

//! IBufferDataSource is almost surely not the right base class
class UCStack : protected IBufferDataSource
{
public:
	UCStack(unsigned int len=0x1000) :
		IBufferDataSource(new uint8[len],len)
	{
		// stack grows downward, so start at the end of the buffer
		buf_ptr = buf + len;
	}

	~UCStack() {
		uint8 *delptr = const_cast<uint8*>(buf);
		delete[] delptr;
	}

	inline uint32 stacksize() const {
		return buf+size-buf_ptr;
	}

	uint32 getSize() {
		return IBufferDataSource::getSize();
	}

	inline void addSP(const sint32 offset) {
		skip(offset);
	}

	inline unsigned int getSP() {
		return getPos();
	}

	inline void moveSP(unsigned int pos) {
		seek(pos);
	}

	//
	// Push values to the stack
	//

	inline void push1(uint8 val) {
		buf_ptr--;
		const_cast<uint8*>(buf_ptr)[0] = val;
	}
	
	inline void push2(uint16 val) {
		buf_ptr-=2;
		const_cast<uint8*>(buf_ptr)[0] = static_cast<uint8>( val     & 0xFF);
		const_cast<uint8*>(buf_ptr)[1] = static_cast<uint8>((val>>8) & 0xFF);
	}
	inline void push4(uint32 val) {
		buf_ptr-=4;
		const_cast<uint8*>(buf_ptr)[0] = static_cast<uint8>( val      & 0xFF);
		const_cast<uint8*>(buf_ptr)[1] = static_cast<uint8>((val>>8)  & 0xFF);
		const_cast<uint8*>(buf_ptr)[2] = static_cast<uint8>((val>>16) & 0xFF);
		const_cast<uint8*>(buf_ptr)[3] = static_cast<uint8>((val>>24) & 0xFF);
	}
	// Push an arbitrary number of bytes of 0
	inline void push0(const uint32 size) { 
		buf_ptr -= size;
		std::memset (const_cast<uint8*>(buf_ptr), 0, size);
	}
	// Push an arbitrary number of bytes
	inline void push(const uint8 *in, const uint32 size) { 
		buf_ptr -= size;
		std::memcpy (const_cast<uint8*>(buf_ptr), in, size);
	}

	//
	// Pop values from the stack
	//

	inline uint16 pop2() {
		return read2();
	}
	inline uint32 pop4() {
		return read4();
	}
	inline void pop(uint8 *out, const uint32 size) {
		read(out, size);
	}

	//
	// Access a value from a location in the stacck
	//

	inline uint8 access1(const uint32 offset) const {
		return buf[offset];
	}
	inline uint16 access2(const uint32 offset) const {
		return (buf[offset] | (buf[offset+1] << 8));
	}
	inline uint32 access4(const uint32 offset) const {
		return buf[offset] | (buf[offset+1]<<8) |
			(buf[offset+2]<<16) | (buf[offset+3]<<24);
	}
	inline const uint8* access(const uint32 offset) const {		
		return buf+offset;
	}
	inline const uint8* access() const {
		return buf_ptr;
	}

	//
	// Assign a value to a location in the stack
	//

	inline void assign1(const uint32 offset, const uint8 val) {
		const_cast<uint8*>(buf)[offset]   = static_cast<uint8>( val     & 0xFF);
	}
	inline void assign2(const uint32 offset, const uint16 val) {
		const_cast<uint8*>(buf)[offset]   = static_cast<uint8>( val     & 0xFF);
		const_cast<uint8*>(buf)[offset+1] = static_cast<uint8>((val>>8) & 0xFF);
	}
	inline void assign4(const uint32 offset, const uint32 val) {
		const_cast<uint8*>(buf)[offset]   = static_cast<uint8>( val      & 0xFF);
		const_cast<uint8*>(buf)[offset+1] = static_cast<uint8>((val>>8)  & 0xFF);
		const_cast<uint8*>(buf)[offset+2] = static_cast<uint8>((val>>16) & 0xFF);
		const_cast<uint8*>(buf)[offset+3] = static_cast<uint8>((val>>24) & 0xFF);
	}
	inline void assign(const uint32 offset, const uint8 *in, const uint32 len)
	{
		std::memcpy (const_cast<uint8*>(buf)+offset, in, len);
	}
};


#endif

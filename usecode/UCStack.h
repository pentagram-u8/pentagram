/*
Copyright (C) 2002,2003 The Pentagram team

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

class IDataSource;
class ODataSource;

// A little-endian stack for use with usecode
class UCStack
{
protected:
	uint8* buf;
	uint8* buf_ptr;
	uint32 size;
public:
	UCStack(uint32 len=0x1000) : size(len)
	{
		buf = new uint8[size];
		// stack grows downward, so start at the end of the buffer
		buf_ptr = buf + size;
	}

	~UCStack() {
		delete[] buf;
	}

	inline uint32 getSize() const {
		return size;
	}

	inline uint32 stacksize() const {
		return size - (buf_ptr - buf);
	}

	inline void addSP(const sint32 offset) {
		buf_ptr += offset;
	}

	inline unsigned int getSP() const {
		return (buf_ptr - buf);
	}

	inline void moveSP(unsigned int pos) {
		buf_ptr = buf + pos;
	}

	//
	// Push values to the stack
	//

	inline void push1(uint8 val) {
		buf_ptr--;
		buf_ptr[0] = val;
	}
	
	inline void push2(uint16 val) {
		buf_ptr-=2;
		buf_ptr[0] = static_cast<uint8>( val     & 0xFF);
		buf_ptr[1] = static_cast<uint8>((val>>8) & 0xFF);
	}
	inline void push4(uint32 val) {
		buf_ptr-=4;
		buf_ptr[0] = static_cast<uint8>( val      & 0xFF);
		buf_ptr[1] = static_cast<uint8>((val>>8)  & 0xFF);
		buf_ptr[2] = static_cast<uint8>((val>>16) & 0xFF);
		buf_ptr[3] = static_cast<uint8>((val>>24) & 0xFF);
	}
	// Push an arbitrary number of bytes of 0
	inline void push0(const uint32 size) { 
		buf_ptr -= size;
		std::memset (buf_ptr, 0, size);
	}
	// Push an arbitrary number of bytes
	inline void push(const uint8 *in, const uint32 size) { 
		buf_ptr -= size;
		std::memcpy (buf_ptr, in, size);
	}

	//
	// Pop values from the stack
	//

	inline uint16 pop2() {
		uint8 b0, b1;
		b0 = *buf_ptr++;
		b1 = *buf_ptr++;
		return (b0 | (b1 << 8));
	}
	inline uint32 pop4() {
		uint8 b0, b1, b2, b3;
		b0 = *buf_ptr++;
		b1 = *buf_ptr++;
		b2 = *buf_ptr++;
		b3 = *buf_ptr++;
		return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	}
	inline void pop(uint8 *out, const uint32 size) {
		std::memcpy(out, buf_ptr, size);
		buf_ptr += size;
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
	inline uint8* access(const uint32 offset) const {		
		return buf+offset;
	}
	inline uint8* access() const {
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

	void save(ODataSource* ods);
	bool load(IDataSource* ids);
};

#endif

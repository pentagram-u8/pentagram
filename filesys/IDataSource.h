/*
 *	IDataSource.h - DataSource type for loading data, only needs read only access
 *
 *  Copyright (C) 2002 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef IDATASOURCE_H
#define IDATASOURCE_H

#include "pent_include.h"
#include <fstream>

class IDataSource
{
	public:
		IDataSource() {};
		virtual ~IDataSource() {};

		virtual uint32 read1()=0;
		virtual uint16 read2()=0;
		virtual uint16 read2high()=0;
		virtual uint32 read3()=0;
		virtual uint32 read4()=0;
		virtual uint32 read4high()=0;
		virtual void read(void *str, sint32 num_bytes)=0;

		uint32 readX(uint32 num_bytes)
		{
			assert(num_bytes > 0 && num_bytes <= 4);
			if (num_bytes == 1) return read1();
			else if (num_bytes == 2) return read2();
			else if (num_bytes == 3) return read3();
			else return read4();
		}
		
		virtual void seek(uint32 pos)=0;
		virtual void skip(sint32 delta)=0;
		virtual uint32 getSize()=0;
		virtual uint32 getPos()=0;
};

/*//	Read a 2-byte value from a buffer.
inline uint16 Read2
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	return (b0 | (b1 << 8));
	}

//	Read a 2-byte value from a buffer.
inline uint16 Read2high
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	return ((b0 << 8) | b1);
	}

//	Read a 4-byte value from a buffer.
inline uint32 Read4
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	uint8 b2 = *in++;
	uint8 b3 = *in++;
	return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	}

//	Read a 4-byte value from a buffer.
inline uint32 Read4high
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	uint8 b2 = *in++;
	uint8 b3 = *in++;
	return ((b0<<24) | (b1<<16) | (b2<<8) | b3);
	}*/


class IFileDataSource: public IDataSource
{
	private:
		std::ifstream *in;
	
	public:
	IFileDataSource(std::ifstream *data_stream)
	{
		in = data_stream;
	};

	virtual ~IFileDataSource()
	{
		FORGET_OBJECT(in);
	};

	bool good() const { return in->good(); };
	
	//	Read a byte value
	virtual uint32 read1()
	{
		return static_cast<uint8>(in->get());
	};

	//	Read a 2-byte value, lsb first.
	virtual uint16 read2()
	{
		uint16 val = 0;
		val |= static_cast<uint16>(in->get());
		val |= static_cast<uint16>(in->get()<<8);
		return val;
	};

	//	Read a 2-byte value, hsb first.
	virtual uint16 read2high()
	{
		uint16 val = 0;
		val |= static_cast<uint16>(in->get()<<8);
		val |= static_cast<uint16>(in->get());
		return val;
	};

	//	Read a 3-byte value, lsb first.
	virtual uint32 read3()
	{
		uint32 val = 0;
		val |= static_cast<uint32>(in->get());
		val |= static_cast<uint32>(in->get()<<8);
		val |= static_cast<uint32>(in->get()<<16);
		return val;
	};

	//	Read a 4-byte long value, lsb first.
	virtual uint32 read4()
	{
		uint32 val = 0;
		val |= static_cast<uint32>(in->get());
		val |= static_cast<uint32>(in->get()<<8);
		val |= static_cast<uint32>(in->get()<<16);
		val |= static_cast<uint32>(in->get()<<24);
		return val;
	};

	//	Read a 4-byte long value, hsb first.
	virtual uint32 read4high()
	{
		uint32 val = 0;
		val |= static_cast<uint32>(in->get()<<24);
		val |= static_cast<uint32>(in->get()<<16);
		val |= static_cast<uint32>(in->get()<<8);
		val |= static_cast<uint32>(in->get());
		return val;
	};

	void read(void *b, sint32 len) { in->read(static_cast<char *>(b), len); };

	virtual void seek(uint32 pos)  { in->seekg(pos); };

	virtual void skip(sint32 pos)  { in->seekg(pos, std::ios::cur); };

	virtual uint32 getSize()
	{
		long pos = in->tellg();
		in->seekg(0, std::ios::end);
		long len = in->tellg();
		in->seekg(pos);
		return len;
	};

	virtual uint32 getPos() { return in->tellg(); };

};


class IBufferDataSource : public IDataSource
{
protected:
	const uint8* buf;
	const uint8* buf_ptr;

	uint32 size;

public:
	IBufferDataSource(const uint8* data, unsigned int len) {
		assert(data != 0 || len == 0);
		buf = buf_ptr = data;
		size = len;
	}

	void load(const uint8* data, unsigned int len) {
		assert(data != 0 || len == 0);
		buf = buf_ptr = data;
		size = len;
	}

	virtual ~IBufferDataSource() { }

	virtual uint32 read1() {
		uint8 b0;
		b0 = *buf_ptr++;
		return (b0);
	}

	virtual uint16 read2() {
		uint8 b0, b1;
		b0 = *buf_ptr++;
		b1 = *buf_ptr++;
		return (b0 | (b1 << 8));
	}

	virtual uint16 read2high() {
		uint8 b0, b1;
		b1 = *buf_ptr++;
		b0 = *buf_ptr++;
		return (b0 | (b1 << 8));
	}

	virtual uint32 read3() {
		uint8 b0, b1, b2;
		b0 = *buf_ptr++;
		b1 = *buf_ptr++;
		b2 = *buf_ptr++;
		return (b0 | (b1 << 8) | (b2 << 16));
	}

	virtual uint32 read4() {
		uint8 b0, b1, b2, b3;
		b0 = *buf_ptr++;
		b1 = *buf_ptr++;
		b2 = *buf_ptr++;
		b3 = *buf_ptr++;
		return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	}

	virtual uint32 read4high() {
		uint8 b0, b1, b2, b3;
		b3 = *buf_ptr++;
		b2 = *buf_ptr++;
		b1 = *buf_ptr++;
		b0 = *buf_ptr++;
		return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	}
	
	virtual void read(void *str, sint32 num_bytes) {
		std::memcpy(str, buf_ptr, num_bytes);
		buf_ptr += num_bytes;
	}

	virtual void seek(uint32 pos) {
		buf_ptr = buf + pos;
	}

	virtual void skip(sint32 delta) {
		buf_ptr += delta;
	}

	virtual uint32 getSize() {
		return size;
	}

	virtual uint32 getPos() {
		return (buf_ptr - buf);
	}
};


#endif

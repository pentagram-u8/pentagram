/*
 *	ODataSource.h - DataSource type for writing data
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

#ifndef ODATASOURCE_H
#define ODATASOURCE_H

#include "common_types.h"
#include <fstream>
#include <deque>
#include <vector>

class ODataSource
{
	public:
		ODataSource() {};
		virtual ~ODataSource() {};

		virtual void write1(uint32)=0;
		virtual void write2(uint16)=0;
		virtual void write2high(uint16)=0;
		virtual void write3(uint32)=0;
		virtual void write4(uint32)=0;
		virtual void write4high(uint32)=0;
		virtual void write(const void *str, uint32 num_bytes)=0;

		void writeX(uint32 val, uint32 num_bytes)
		{
			assert(num_bytes > 0 && num_bytes <= 4);
			if (num_bytes == 1) write1(val);
			else if (num_bytes == 2) write2(val);
			else if (num_bytes == 3) write3(val);
			else write4(val);
		}
		
		virtual void seek(uint32 pos)=0;
		virtual void skip(sint32 delta)=0;
		virtual uint32 getSize()=0;
		virtual uint32 getPos()=0;
};


class OBufferDataSource : public ODataSource
{
	private:
		std::deque<char> out;

	public:
	OBufferDataSource() {};

	virtual ~OBufferDataSource() {};

	const std::deque<char> &buf() const { return out; };

	virtual void write1(uint32 val)
	{
		out.push_back(static_cast<char> (val&0xff));
	};

	virtual void write2(uint16 val)
	{
		out.push_back(static_cast<char> (val&0xff));
		out.push_back(static_cast<char> ((val>>8)&0xff));
	};

	virtual void write2high(uint16 val)
	{
		out.push_back(static_cast<char> ((val>>8)&0xff));
		out.push_back(static_cast<char> (val&0xff));
	};

	virtual void write3(uint32 val)
	{
		out.push_back(static_cast<char> (val&0xff));
		out.push_back(static_cast<char> ((val>>8)&0xff));
		out.push_back(static_cast<char> ((val>>16)&0xff));
	};

	virtual void write4(uint32 val)
	{
		out.push_back(static_cast<char> (val&0xff));
		out.push_back(static_cast<char> ((val>>8)&0xff));
		out.push_back(static_cast<char> ((val>>16)&0xff));
		out.push_back(static_cast<char> ((val>>24)&0xff));
	};

	virtual void write4high(uint32 val)
	{
		out.push_back(static_cast<char> ((val>>24)&0xff));
		out.push_back(static_cast<char> ((val>>16)&0xff));
		out.push_back(static_cast<char> ((val>>8)&0xff));
		out.push_back(static_cast<char> (val&0xff));
	};

	virtual void write(const void *b, uint32 length) { write(b, length, length); };
	
	virtual void write(const void *b, uint32 length, uint32 pad_length)
	{
		for(uint32 i=0; i<length; i++)
			out.push_back(static_cast<const char *>(b)[i]);
		if(pad_length>length)
			for(pad_length-=length; pad_length>0; --pad_length)
				out.push_back(static_cast<char>(0x00));
	};

	virtual void clear()          { out.clear(); };

	virtual void seek(uint32 pos) { /*out->seekp(pos); FIXME: Do something here. */ };
	virtual void skip(sint32 pos) { /*out->seekp(pos, std::ios::cur); FIXME: Do something here. */ };

	virtual uint32 getSize()      { return out.size(); };
	
	virtual uint32 getPos() { return out.size(); /*return out->tellp(); FIXME: Do something here. */ };

};

class OFileDataSource : public ODataSource
{
	private:
		std::ofstream *out;

	public:
	OFileDataSource(std::ofstream *data_stream)
	{
		out = data_stream;
	};

	virtual ~OFileDataSource()
	{
		FORGET_OBJECT(out);
	};

	bool good() const { return out->good(); };

	virtual void write1(uint32 val)         
	{ 
		out->put(static_cast<char> (val&0xff));
	};

	virtual void write2(uint16 val)         
	{ 
		out->put(static_cast<char> (val&0xff));
		out->put(static_cast<char> ((val>>8)&0xff));
	};

	virtual void write2high(uint16 val)     
	{ 
		out->put(static_cast<char> ((val>>8)&0xff));
		out->put(static_cast<char> (val&0xff));
	};

	virtual void write3(uint32 val)         
	{ 
		out->put(static_cast<char> (val&0xff));
		out->put(static_cast<char> ((val>>8)&0xff));
		out->put(static_cast<char> ((val>>16)&0xff));
	};

	virtual void write4(uint32 val)         
	{ 
		out->put(static_cast<char> (val&0xff));
		out->put(static_cast<char> ((val>>8)&0xff));
		out->put(static_cast<char> ((val>>16)&0xff));
		out->put(static_cast<char> ((val>>24)&0xff));
	};

	virtual void write4high(uint32 val)     
	{ 
		out->put(static_cast<char> ((val>>24)&0xff));
		out->put(static_cast<char> ((val>>16)&0xff));
		out->put(static_cast<char> ((val>>8)&0xff));
		out->put(static_cast<char> (val&0xff));
	};

	virtual void write(const void *b, uint32 len) 
	{ 
		out->write(static_cast<const char *>(b), len); 
	};

	virtual void seek(uint32 pos) { out->seekp(pos); };

	virtual void skip(sint32 pos) { out->seekp(pos, std::ios::cur); };

	virtual uint32 getSize()
	{
		long pos = out->tellp();
		out->seekp(0, std::ios::end);
		long len = out->tellp();
		out->seekp(pos);
		return len;
	};

	virtual uint32 getPos() { return out->tellp(); };
};

#endif

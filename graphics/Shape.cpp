/*
 *  Copyright (C) 2003 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"

#include "Shape.h"
#include "ShapeFrame.h"
#include "ConvertShape.h"
#include "u8/ConvertShapeU8.h"
#include "crusader/ConvertShapeCrusader.h"
#include "IDataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Shape);

Shape::Shape(const uint8* data, uint32 size, const ConvertShapeFormat *format)
{
	// NB: U8 style!

	this->data = data;
	this->size = size;
	this->palette = 0;

	if (!format) format = DetectShapeFormat(data,size);
	
	if (!format) 
	{
		// Should be fatal?
		perr << "Error: Unable to detect shape format" << std::endl;
		return;
	}

	// Load it as u8
	if (format == &U8ShapeFormat || format == &U82DShapeFormat)
		LoadU8Format(data,size,format);
	else if (format == &PentagramShapeFormat)
		LoadPentagramFormat(data,size,format);
	else 
		LoadGenericFormat(data,size,format);
}

Shape::Shape(IDataSource *src, const ConvertShapeFormat *format)
{
	// NB: U8 style!

	this->size = src->getSize();
	uint8 *d = new uint8[this->size];
	this->data = d;
	src->read(d, this->size);
	this->palette = 0;

	if (!format) format = DetectShapeFormat(data,size);
	
	if (!format) 
	{
		// Should be fatal?
		perr << "Error: Unable to detect shape format" << std::endl;
		return;
	}

	// Load it as u8
	if (format == &U8ShapeFormat || format == &U82DShapeFormat)
		LoadU8Format(data,size,format);
	else if (format == &PentagramShapeFormat)
		LoadPentagramFormat(data,size,format);
	else 
		LoadGenericFormat(data,size,format);
}

Shape::~Shape()
{
	for (unsigned int i = 0; i < frames.size(); ++i)
		delete frames[i];

	delete[] const_cast<uint8*>(data);
}

// Some macros to make things easier
#define READ1(data,offset) (data[offset])
#define READ2(data,offset) (data[offset] + (data[offset+1] << 8))
#define READ3(data,offset) (data[offset] + (data[offset+1] << 8) + (data[offset+2] << 16))
#define READ4(data,offset) (data[offset] + (data[offset+1] << 8) + (data[offset+2] << 16) + (data[offset+3] << 24))

// This will load a u8 style shape 'optimzed'.
void Shape::LoadU8Format(const uint8* data, uint32 /*size*/, const ConvertShapeFormat* format)
{
	unsigned int framecount = READ2(data,4);

	frames.reserve(framecount);

	for (unsigned int i = 0; i < framecount; ++i) {
		uint32 frameoffset = READ3(data,6+6*i);
		uint32 framesize = READ2(data,10+6*i);
		
		frames.push_back(new ShapeFrame(data + frameoffset, framesize, format));
	}
}

// This will load a pentagram style shape 'optimzed'.
void Shape::LoadPentagramFormat(const uint8* data, uint32 /*size*/, const ConvertShapeFormat* format)
{
	unsigned int framecount = READ4(data,4);

	frames.reserve(framecount);

	for (unsigned int i = 0; i < framecount; ++i) {
		uint32 frameoffset = READ4(data,8+8*i);
		uint32 framesize = READ4(data,12+8*i);
		
		frames.push_back(new ShapeFrame(data + frameoffset, framesize, format));
	}
}

// This will load any sort of shape via a ConvertShapeFormat struct
void Shape::LoadGenericFormat(const uint8* data, uint32 size, const ConvertShapeFormat* format)
{
	uint32 framecount;
	uint32 frameoffset;
	uint32 framesize;
	IBufferDataSource ds(data,size);

	// Skip ident, unknown
	ds.skip(format->bytes_ident + format->bytes_header_unk);

	// Read framecount, default 1 if no
	if (format->bytes_num_frames) framecount = ds.readX(format->bytes_num_frames);
	else framecount = 1;

	frames.reserve(framecount);

	for (unsigned int i = 0; i < framecount; ++i) {
		// Read the offset
		if (format->bytes_frame_offset) frameoffset = ds.readX(format->bytes_frame_offset);
		else frameoffset = format->len_header + (format->len_frameheader*i);

		// Skip the unknown
		ds.skip(format->bytes_frameheader_unk);

		// Read frame_length
		if (format->bytes_frame_length) framesize = ds.readX(format->bytes_frame_length) + format->bytes_frame_length_kludge;
		else framesize = size-frameoffset;
		
		frames.push_back(new ShapeFrame(data + frameoffset, framesize, format));
	}
}

// This will detect the format of a shape
const ConvertShapeFormat *Shape::DetectShapeFormat(const uint8* data, uint32 size)
{
	IBufferDataSource ds(data,size);

	if (CheckShapeFormatUnsafe(&ds, &PentagramShapeFormat, size))
		return &PentagramShapeFormat;
	else if (CheckShapeFormatUnsafe(&ds, &U8SKFShapeFormat, size))
		return &U8SKFShapeFormat;
	else if (CheckShapeFormatUnsafe(&ds, &U8ShapeFormat, size))
		return &U8ShapeFormat;
	else if (CheckShapeFormatUnsafe(&ds, &U82DShapeFormat, size))
		return &U82DShapeFormat;
	else if (CheckShapeFormatUnsafe(&ds, &CrusaderShapeFormat, size))
		return &CrusaderShapeFormat;
	else if (CheckShapeFormatUnsafe(&ds, &Crusader2DShapeFormat, size))
		return &Crusader2DShapeFormat;

	return 0;
}

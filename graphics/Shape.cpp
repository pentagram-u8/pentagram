/*
 *  Copyright (C) 2003-2004 The Pentagram Team
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

DEFINE_CUSTOM_MEMORY_ALLOCATION(Shape);

Shape::Shape(const uint8* data, uint32 size, const ConvertShapeFormat *format,
			 const uint16 id, const uint32 shape) : flexId(id), shapenum(shape)
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
	: flexId(0), shapenum(0)
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

void Shape::getShapeId(uint16 & id, uint32 & shape)
{
	id = flexId;
	shape = shapenum;
}

// Some macros to make things easier
#define READ1(data,offset) (data[offset])
#define READ2(data,offset) (data[offset] + (data[offset+1] << 8))
#define READ3(data,offset) (data[offset] + (data[offset+1] << 8) + (data[offset+2] << 16))
#define READ4(data,offset) (data[offset] + (data[offset+1] << 8) + (data[offset+2] << 16) + (data[offset+3] << 24))

// This will load a u8 style shape 'optimzed'.
void Shape::LoadU8Format(const uint8* data, uint32 size, const ConvertShapeFormat* format)
{
	unsigned int framecount = READ2(data,4);

	if (framecount == 0) {
		LoadGenericFormat(data, size, format);
		return;
	}

	frames.reserve(framecount);

	for (unsigned int i = 0; i < framecount; ++i) {
		uint32 frameoffset = READ3(data,6+6*i);
		uint32 framesize = READ2(data,10+6*i);
		
		frames.push_back(new ShapeFrame(data + frameoffset, framesize, format));
	}
}

// This will load a pentagram style shape 'optimzed'.
void Shape::LoadPentagramFormat(const uint8* data, uint32 size, const ConvertShapeFormat* format)
{
	unsigned int framecount = READ4(data,4);

	if (framecount == 0) {
		LoadGenericFormat(data, size, format);
		return;
	}

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

	if (format->bytes_ident) {
		uint8* ident = new uint8[format->bytes_ident];
		ds.read(ident, format->bytes_ident);
		bool match = std::memcmp(ident,format->ident,format->bytes_ident) == 0;
		delete[] ident;

		if (!match) {
			frames.clear();
			return;
		}
	}

	// Read special buffer
	uint8 special[256];
	if (format->bytes_special) {
		memset(special, 0, 256);
		for (uint32 i = 0; i < format->bytes_special; i++) special[ds.read1()&0xFF] = i+2;
	}

	// Skip unknown
	ds.skip(format->bytes_header_unk);

	// Read framecount, default 1 if no
	if (format->bytes_num_frames) framecount = ds.readX(format->bytes_num_frames);
	else framecount = 1;
	if (framecount == 0) framecount = ConvertShape::CalcNumFrames(&ds,format,size,0);

	frames.reserve(framecount);

	for (unsigned int i = 0; i < framecount; ++i) {
		// Read the offset
		if (format->bytes_frame_offset) frameoffset = ds.readX(format->bytes_frame_offset) + format->bytes_special;
		else frameoffset = format->len_header + (format->len_frameheader*i);

		// Skip the unknown
		ds.skip(format->bytes_frameheader_unk);

		// Read frame_length
		if (format->bytes_frame_length) framesize = ds.readX(format->bytes_frame_length) + format->bytes_frame_length_kludge;
		else framesize = size-frameoffset;
		
		ConvertShapeFrame *prev=0, p;

		if (format->bytes_special && i > 0) {
			prev = &p;
			frames[i-1]->getConvertShapeFrame(p);
		}

		frames.push_back(new ShapeFrame(data + frameoffset, framesize, format, special, prev));
	}
}

// This will detect the format of a shape
const ConvertShapeFormat *Shape::DetectShapeFormat(const uint8* data, uint32 size)
{
	IBufferDataSource ds(data,size);
	return Shape::DetectShapeFormat(&ds, size);
}

const ConvertShapeFormat *Shape::DetectShapeFormat(IDataSource * ds, uint32 size)
{
	const ConvertShapeFormat *ret = nullptr; 
	uint32 original_pos = ds->getPos(); 

	const ConvertShapeFormat* formats_to_check[] = {
		&PentagramShapeFormat,
		&U8SKFShapeFormat,
		&U8ShapeFormat,
		&U82DShapeFormat,
		&CrusaderShapeFormat,
		&Crusader2DShapeFormat,
		&U8CMPShapeFormat
	};

	for (const ConvertShapeFormat* current_format_ptr : formats_to_check) {
		ds->seek(original_pos); // Correctly resets stream for each check
		if (ConvertShape::CheckUnsafe(ds, current_format_ptr, size)) {
			ret = current_format_ptr;
			break; 
		}
	}

	ds->seek(original_pos); // Correctly resets stream before returning
	return ret;
}

void Shape::getTotalDimensions(sint32& w,sint32& h,sint32& x,sint32& y) const
{
	if (frames.empty()) {
		w = 0; h = 0; x = 0; y = 0;
		return;
	}

	sint32 minx = 1000000, maxx = -1000000;
	sint32 miny = 1000000, maxy = -1000000;

	for (unsigned int i = 0; i < frames.size(); ++i) {
		ShapeFrame* frame = frames[i];
		if (-frame->xoff < minx)
			minx = -frame->xoff;
		if (-frame->yoff < miny)
			miny = -frame->yoff;
		if (frame->width-frame->xoff-1 > maxx)
			maxx = frame->width-frame->xoff-1;
		if (frame->height-frame->yoff-1 > maxy)
			maxy = frame->height-frame->yoff-1;
	}

	w = maxx - minx + 1;
	h = maxy - miny + 1;
	x = -minx;
	y = -miny;
}

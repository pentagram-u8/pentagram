/*
Copyright (C) 2003 The Pentagram Team

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

#ifndef SHAPE_H
#define SHAPE_H

#include <vector>

class ShapeFrame;
struct Palette;
struct ConvertShapeFormat;
class IDataSource;

class Shape 
{
public:
	// Parse data, create frames.
	// NB: Shape uses data without copying it. It is deleted on destruction
	// If format is not specified it will be autodetected
	Shape(const uint8* data, uint32 size, const ConvertShapeFormat *format);
	Shape(IDataSource *src, const ConvertShapeFormat *format);
	virtual ~Shape();
	void setPalette(const Palette* pal) { palette = pal; }
	const Palette* getPalette() const { return palette; }

	unsigned int frameCount() const { return frames.size(); }

	ShapeFrame* getFrame(unsigned int frame)
		{ if (frame < frames.size()) return frames[frame]; else return 0; }

	// This will detect the format of a shape
	static const ConvertShapeFormat *DetectShapeFormat(const uint8* data, uint32 size);

	ENABLE_RUNTIME_CLASSTYPE();

protected:

	// This will load a u8 style shape 'optimized'.
	void LoadU8Format(const uint8* data, uint32 size, const ConvertShapeFormat* format);

	// This will load a pentagram style shape 'optimized'.
	void LoadPentagramFormat(const uint8* data, uint32 size, const ConvertShapeFormat* format);

	// This will load any sort of shape via a ConvertShapeFormat struct
	// Crusader shapes must be loaded this way
	void LoadGenericFormat(const uint8* data, uint32 size, const ConvertShapeFormat* format);

	std::vector<ShapeFrame*> frames;

	const Palette* palette;

	const uint8* data;
	uint32 size;
};


#endif

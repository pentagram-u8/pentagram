/*
 *  Copyright (C) 2002, 2003 The Pentagram Team
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

#include "pent_include.h"
#include "ConvertShapeU8.h"

// This will attempt to detect a Shape as being Ultima8
bool AutoDetectShapeU8 (IDataSource *source)
{
	// What we will do is seek to byte 6 and get num frames. 
	source->skip(6);
	uint32 num_frames = source->read2();

	// Now read the size of the first frame
	uint32 frame_offset = source->read3();

	// Be a nice Function and skip back 11 bytes
	source->skip(-11);

	// Now for a trick :-)
	// If frame_offset == (6 + num_frames * 6) then we can be fairly sure this is Ultima8

	return frame_offset == (6 + num_frames * 6);
}

// Shape format configuration for Ultima8
const ConvertShapeFormat		U8ShapeFormat =
{
	"Ultima8",
	6,		// header
	"",		// ident
	0,		// bytes_ident 
	4,		// header_unk
	2,		// num_frames

	6,		// frameheader
	3,		// frame_offset
	1,		// frameheader_unk
	2,		// frame_length
	0,		// frame_length_kludge

	18,		// frameheader2
	8,		// frame_unknown
	2,		// frame_compression
	2,		// frame_width
	2,		// frame_height
	2,		// frame_xoff
	2,		// frame_yoff

	2		// line_offset
};

// Shape format configuration for Ultima8 2D interface components
const ConvertShapeFormat		U82DShapeFormat =
{
	"Ultima8 2D",
	6,		// header
	"",		// ident
	0,		// bytes_ident 
	4,		// header_unk
	2,		// num_frames

	6,		// frameheader
	3,		// frame_offset
	1,		// frameheader_unk
	2,		// frame_length
	8,		// frame_length_kludge

	18,		// frameheader2
	8,		// frame_unknown
	2,		// frame_compression
	2,		// frame_width
	2,		// frame_height
	2,		// frame_xoff
	2,		// frame_yoff

	2		// line_offset
};

// Shape format configuration for Ultima8 SKF
const ConvertShapeFormat		U8SKFShapeFormat =
{
	"Ultima8 SKF",
	2,		// header
	"",		// ident
	0,		// bytes_ident 
	0,		// header_unk
	2,		// num_frames

	6,		// frameheader
	3,		// frame_offset
	1,		// frameheader_unk
	2,		// frame_length
	0,		// frame_length_kludge

	18,		// frameheader2
	8,		// frame_unknown
	2,		// frame_compression
	2,		// frame_width
	2,		// frame_height
	2,		// frame_xoff
	2,		// frame_yoff

	2		// line_offset
};

/*
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

#include "pent_include.h"
#include "ConvertShapeCrusader.h"

// This will attempt to detect a Shape as being Crusader
bool AutoDetectShapeCrusader(IDataSource *source)
{
	// What we will do is seek to byte 6 and get num frames. 
	source->skip(6);
	uint32 num_frames = source->read2();

	// Now read the size of the first frame
	uint32 frame_offset = source->read3();

	// Be a nice Function and skip back 11 bytes
	source->skip(-11);

	// Now for a trick :-)
	// If frame_offset == (6 + num_frames * 8) then we can be fairly sure this is Crusader

	return frame_offset == (6 + num_frames * 8);
}

// Shape format configuration for Crusader
const ConvertShapeFormat		CrusaderShapeFormat =
{
	6,		// header
	4,		// header_unk
	2,		// num_frames

	8,		// frameheader
	3,		// frame_offset
	1,		// frameheader_unk
	4,		// frame_length

	28,		// frameheader2        20 for mouse/gumps
	8,		// frame_unknown       0  for mouse/gumps
	4,		// frame_compression
	4,		// frame_width
	4,		// frame_height
	4,		// frame_xoff
	4,		// frame_yoff

	4		// line_offset
};

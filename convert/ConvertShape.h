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

#ifndef CONVERTSHAPE_H
#define CONVERTSHAPE_H

#include "IDataSource.h"
#include "ODataSource.h"

// Convert shape C
struct ConvertShapeFormat
{													//	Ultima8	U8.SKF	Crusader
	uint32				len_header;					//	6		2		6
	uint32				bytes_header_unk;			//	4		0		4
	uint32				bytes_num_frames;			//	2		2		2

	uint32				len_frameheader;			//	6		6		8
	uint32				bytes_frame_offset;			//	3		3		3
	uint32				bytes_frameheader_unk;		//	1		1		2
	uint32				bytes_frame_length;			//	2		2		3

	uint32				len_frameheader2;			//	18		18		28
	uint32				bytes_frame_unknown;		//	8		8		8
	uint32				bytes_frame_compression;	//	2		2		4
	uint32				bytes_frame_width;			//	2		2		4
	uint32				bytes_frame_height;			//	2		2		4
	uint32				bytes_frame_xoff;			//	2		2		4
	uint32				bytes_frame_yoff;			//	2		2		4

	uint32				bytes_line_offset;			//	2		2		4
};

// ConvertShapeFrame structure

struct ConvertShapeFrame 
{
	uint8				header_unknown[2];

	uint8				unknown[8];
	uint32				compression;
	sint32				width;
	sint32				height;
	sint32				xoff;
	sint32				yoff;

	uint32				*line_offsets;		// Note these are offsets into rle_data

	uint32				bytes_rle;			// Number of bytes of RLE Data
	uint8				*rle_data;

	void Free()
	{
		delete [] line_offsets;
		line_offsets = 0;

		delete [] rle_data;
		rle_data = 0;
	}
};


// ConvertShape structure

class ConvertShape
{
	uint8				header_unknown[4];
	int					num_frames;
	ConvertShapeFrame	*frames;

public:
	ConvertShape() : num_frames(0), frames(0)
	{
	}

	~ConvertShape()
	{
		Free();
	}

	void Free()
	{
		if (frames) for (int i = 0; i < num_frames; i++)
		{
			frames[i].Free();
		}
		delete [] frames;
		frames = 0;
		num_frames = 0;
	}


	void Read(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len);
	void Write(ODataSource *source, const ConvertShapeFormat *csf, uint32 &write_len);
};


#endif //CONVERTSHAPE_H

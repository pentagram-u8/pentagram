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
#include "ConvertShape.h"

void ConvertShape::Read(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len)
{
	// Just to be safe
	int start_pos = source->getPos();

	// Read the header unknown
	if (csf->bytes_header_unk) source->read(header_unknown, csf->bytes_header_unk);

	//pout << std::hex;

	// Now read num_frames
	num_frames = source->readX(csf->bytes_num_frames);
	//pout << "num_frames " << num_frames << std::endl;

	// Create frames array
	frames = new ConvertShapeFrame[num_frames];
	std::memset (frames, 0, num_frames * sizeof(ConvertShapeFrame));

	// Now read the frames
	for (int f = 0; f < num_frames; f++) 
	{
		//pout << "Frame " << f << std::endl;
		ConvertShapeFrame *frame = frames+f;

		// Seek to initial pos
		source->seek(start_pos + csf->len_header + (csf->len_frameheader*f));

		// Read the offset
		uint32 frame_offset = source->readX(csf->bytes_frame_offset);
		//pout << "frame_offset " << frame_offset << std::endl;

		// Read the unknown
		if (csf->bytes_frameheader_unk) source->read(frame->header_unknown, csf->bytes_frameheader_unk);

		// Read frame_length
		uint32 frame_length = source->readX(csf->bytes_frame_length);
		//pout << "frame_length " << frame_length << std::endl;

		// Read next frame offset (assuming we are not end) 
		uint32 next_offset;
		if (f != (num_frames-1)) next_offset = source->readX(csf->bytes_frame_offset);
		else next_offset = real_len;

		// Fudge the framelen
		frame_length = next_offset - frame_offset;

		//pout << "frame_length " << frame_length << std::endl;


		// Seek to start of frame
		source->seek(start_pos + frame_offset);

		// Read unknown
		if (csf->bytes_frame_unknown) source->read(frame->unknown, csf->bytes_frame_unknown);

		// Frame details
		frame->compression = source->readX(csf->bytes_frame_compression);
		frame->width = source->readX(csf->bytes_frame_width);
		frame->height = source->readX(csf->bytes_frame_height);
		frame->xoff = source->readX(csf->bytes_frame_xoff);
		frame->yoff = source->readX(csf->bytes_frame_yoff);

		//pout << "compression " << frame->compression << std::endl;
		//pout << "width " << frame->width << std::endl;
		//pout << "height " << frame->height << std::endl;
		//pout << "xoff " << frame->xoff << std::endl;
		//pout << "yoff " << frame->yoff << std::endl;

		if (frame->height) {
			// Line offsets
			frame->line_offsets = new uint32 [frame->height];

			for (int i = 0; i < frame->height; i++) 
			{
				frame->line_offsets[i] = source->readX(csf->bytes_line_offset);

				// Now fudge with the value and turn it into an offset into the rle data
				frame->line_offsets[i] -= (frame->height-i)*csf->bytes_line_offset;
			}

			// Calculate the number of bytes of RLE data
			frame->bytes_rle = frame_length - (csf->len_frameheader2+(frame->height*csf->bytes_line_offset));

		}
		else 
			frame->line_offsets = 0;

		// Read the RLE Data
		if (frame->bytes_rle) {
			frame->rle_data = new uint8[frame->bytes_rle];
			source->read(frame->rle_data, frame->bytes_rle);
		}
		else 
			frame->rle_data = 0;
	}

	//ypout << std::dec;
}

void ConvertShape::Write(ODataSource *dest, const ConvertShapeFormat *csf, uint32 &write_len)
{
	// Just to be safe
	int start_pos = dest->getPos();

	// Write the header unknown
	if (csf->bytes_header_unk) dest->write(header_unknown, csf->bytes_header_unk);

	// Now write num_frames
	dest->writeX(num_frames, csf->bytes_num_frames);

	// Write filler space for the frame details
	for (int i = 0; i < num_frames*csf->len_frameheader; i++) dest->write1(0);

	// Now write the frames
	for (int f = 0; f < num_frames; f++) 
	{
		ConvertShapeFrame *frame = frames+f;

		// Get the frame offset
		uint32 frame_offset = dest->getPos() - start_pos;

		// Seek to the frame header pos
		dest->seek(start_pos + csf->len_header + (csf->len_frameheader*f));

		// Write the offset
		dest->writeX(frame_offset, csf->bytes_frame_offset);

		// Write the unknown
		if (csf->bytes_frameheader_unk) dest->write(frame->header_unknown, csf->bytes_frameheader_unk);

		// Calc and write frame_length
		uint32 frame_length = csf->len_frameheader2 + (frame->height*csf->bytes_line_offset) + frame->bytes_rle;
		dest->writeX(frame_length, csf->bytes_frame_length);

		// Seek to start of frame
		dest->seek(start_pos + frame_offset);

		// Write unknown
		if (csf->bytes_frame_unknown) dest->write(frame->unknown, csf->bytes_frame_unknown);

		// Frame details
		dest->writeX(frame->compression, csf->bytes_frame_compression);
		dest->writeX(frame->width, csf->bytes_frame_width);
		dest->writeX(frame->height, csf->bytes_frame_height);
		dest->writeX(frame->xoff, csf->bytes_frame_xoff);
		dest->writeX(frame->yoff, csf->bytes_frame_yoff);

		// Line offsets
		for (int i = 0; i < frame->height; i++) 
		{
			// Unfudge the value and write it
			int actual_offset = frame->line_offsets[i] + (frame->height-i)*csf->bytes_line_offset;

			dest->writeX(actual_offset, csf->bytes_line_offset);
		}

		// Write the RLE Data
		dest->write(frame->rle_data, frame->bytes_rle);
	}

	// Just cheat
	write_len = dest->getPos() - start_pos;
}

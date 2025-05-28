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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "ConvertShape.h"

#include "IDataSource.h"
#include "ODataSource.h"

#include <cstring>

// Helper to check if 'bytes_to_read' can be safely read/skipped from the current position.
// 'real_len' is the total size of the data source content.
static bool can_read_or_skip_bytes(IDataSource *source, uint32 bytes_to_process, uint32 real_len) {
	if (bytes_to_process == 0) return true; // Processing 0 bytes is always safe.
	if (source->eof()) return false;        // Already at or past EOF.
	// Check if current position + bytes_to_process would exceed the total length.
	return (source->getPos() + bytes_to_process <= real_len);
}


void ConvertShape::Read(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len)
{
	// Just to be safe
	uint32 start_pos = source->getPos();

	// Read the ident
	if (csf->bytes_ident)
	{
		char ident[4];
		source->read(ident, csf->bytes_ident);

		if (std::memcmp (ident, csf->ident, csf->bytes_ident))
		{
			perr << "Warning: Corrupt shape!" << std::endl;
			return;
		}
	}

	// Read special buffer
	uint8 special[256];
	if (csf->bytes_special) {
		memset(special, 0, 256);
		for (uint32 i = 0; i < csf->bytes_special; i++) special[source->read1()&0xFF] = i+2;
	}

	// Read the header unknown
	if (csf->bytes_header_unk) source->read(header_unknown, csf->bytes_header_unk);

#ifdef COMP_SHAPENUM
	if (shapenum == COMP_SHAPENUM) pout << std::hex;
#endif

	// Now read num_frames
	num_frames = 1;
	if (csf->bytes_num_frames) num_frames = source->readX(csf->bytes_num_frames);
	if (num_frames == 0) num_frames = CalcNumFrames(source,csf,real_len,start_pos);

#ifdef COMP_SHAPENUM
	if (shapenum == COMP_SHAPENUM) pout << "num_frames " << num_frames << std::endl;
#endif

//	if (num_frames == 0xFFFF || num_frames == 0xFFFFFF || num_frames == -1)
//	{
//		perr << "Corrupt shape? " << std::endl;
//		num_frames = 0;
//		frames = 0;
//		return;
//	}

	// Create frames array
	frames = new ConvertShapeFrame[num_frames];
	std::memset (frames, 0, num_frames * sizeof(ConvertShapeFrame));

	// Now read the frames
	for(uint32 f = 0; f < num_frames; ++f) 
	{
#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "Frame " << f << std::endl;
#endif
		ConvertShapeFrame *frame = frames+f;

#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "Seeking to " << (csf->len_header + (csf->len_frameheader*f)) << std::endl;
		if (shapenum == COMP_SHAPENUM) pout << "Real " << (start_pos + csf->len_header + (csf->len_frameheader*f)) << std::endl;
#endif
		// Seek to initial pos
		source->seek(start_pos + csf->len_header + (csf->len_frameheader*f));

#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "seeked to " << source->getPos() << std::endl;
#endif

		// Read the offset
		uint32 frame_offset = csf->len_header + (csf->len_frameheader*f);
		if (csf->bytes_frame_offset) frame_offset = source->readX(csf->bytes_frame_offset);
#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "frame_offset " << frame_offset << std::endl;
#endif

		// Read the unknown
		if (csf->bytes_frameheader_unk) source->read(frame->header_unknown, csf->bytes_frameheader_unk);

		// Read frame_length
		uint32 frame_length = real_len-frame_offset;
		if (csf->bytes_frame_length) frame_length = source->readX(csf->bytes_frame_length) + csf->bytes_frame_length_kludge;
#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "frame_length " << frame_length << std::endl;
#endif

		// Seek to start of frame
		source->seek(start_pos + frame_offset + csf->bytes_special);

		if (csf->bytes_special)
			frame->ReadCmpFrame(source, csf, special, f>0?frames+f-1:0);
		else 
			frame->Read(source, csf, frame_length);
	}

#ifdef COMP_SHAPENUM
	if (shapenum == COMP_SHAPENUM) pout << std::dec;
#endif
}

void ConvertShapeFrame::Read(IDataSource *source, const ConvertShapeFormat *csf, uint32 frame_length)
{
	// Read unknown
	if (csf->bytes_frame_unknown) source->read(unknown, csf->bytes_frame_unknown);

	// Frame details
	compression = source->readX(csf->bytes_frame_compression);
	width = source->readXS(csf->bytes_frame_width);
	height = source->readXS(csf->bytes_frame_height);
	xoff = source->readXS(csf->bytes_frame_xoff);
	yoff = source->readXS(csf->bytes_frame_yoff);

#ifdef COMP_SHAPENUM
	if (width <= 0 || height <= 0  ||shapenum == COMP_SHAPENUM )
	{
		pout << "compression " << compression << std::endl;
		pout << "width " << width << std::endl;
		pout << "height " << height << std::endl;
		pout << "xoff " << xoff << std::endl;
		pout << "yoff " << yoff << std::endl;
	}
#endif

	if (compression != 0 && compression != 1) {
		compression = 0;
		width = 0;
		height = 0;
		xoff = 0;
		yoff = 0;
		//perr << "Corrupt frame? (frame " << f << ")" << std::endl;
		perr << "Corrupt frame?" << std::endl;
	}

	if (height) {
		// Line offsets
		line_offsets = new uint32 [height];

		for(sint32 i = 0; i < height; ++i) 
		{
			line_offsets[i] = source->readX(csf->bytes_line_offset);

			// Now fudge with the value and turn it into an offset into the rle data
			// If required
			if (!csf->line_offset_absolute) 
				line_offsets[i] -= (height-i)*csf->bytes_line_offset;
		}

		// Calculate the number of bytes of RLE data
		bytes_rle = frame_length - (csf->len_frameheader2+(height*csf->bytes_line_offset));

#ifdef COMP_SHAPENUM
		if (bytes_rle < 0)
		{
			bytes_rle = 0;
			perr << "Corrupt frame?" << std::endl;
		}
		
#endif
	}
	else 
		line_offsets = 0;

	// Read the RLE Data
	if (bytes_rle) {
		rle_data = new uint8[bytes_rle];
		source->read(rle_data, bytes_rle);
	}
	else 
		rle_data = 0;
}

void ConvertShapeFrame::ReadCmpFrame(IDataSource *source, const ConvertShapeFormat *csf, const uint8 special[256], ConvertShapeFrame *prev)
{
	static OAutoBufferDataSource *rlebuf = 0;
	uint8 outbuf[512];

	// Read unknown
	if (csf->bytes_frame_unknown) source->read(unknown, csf->bytes_frame_unknown);

	// Frame details
	compression = source->readX(csf->bytes_frame_compression);
	width = source->readXS(csf->bytes_frame_width);
	height = source->readXS(csf->bytes_frame_height);
	xoff = source->readXS(csf->bytes_frame_xoff);
	yoff = source->readXS(csf->bytes_frame_yoff);

	line_offsets = new uint32 [height];

	if (!rlebuf) rlebuf = new OAutoBufferDataSource(1024);
	rlebuf->clear();

	for(sint32 y = 0; y < height; ++y) 
	{
		line_offsets[y] = rlebuf->getPos();

		sint32 xpos = 0;

		do
		{
			uint8 skip = source->read1();
			xpos += skip;

			if (xpos > width) {
				source->skip(-1); 
				skip = width-(xpos-skip);
			}

			rlebuf->write1(skip);

			if (xpos >= width) break;

			uint32 dlen = source->read1();
			uint8 *o = outbuf;

			// Is this required???? It seems hacky and pointless
			if (dlen == 0 || dlen == 1) {
				source->skip(-1); 
				rlebuf->skip(-1);
				rlebuf->write1(skip+(width-xpos));
				break;
			}

			int type = 0;
			
			if (compression)  {
				type = dlen & 1;
				dlen >>= 1;
			}

			if (!type) {

				uint32 extra = 0;

				for (uint32 j = 0; j < dlen; j++) {

					uint8 c = source->read1();

					if (special[c] && prev) {
						sint32 count = special[c];
						prev->GetPixels(o,count,xpos-xoff,y-yoff);
						o+=count;
						extra += count-1;
						xpos += count;
					}
					else if (c == 0xFF && prev) {
						sint32 count = source->read1();
						prev->GetPixels(o,count,xpos-xoff,y-yoff);
						o+=count;
						extra += count-2;
						xpos += count;
						j++;
					}
					else {
						*o++ = c;
						xpos++;
					}
				}

				if (((dlen+extra) << compression) > 255) {
					perr << "Error! Corrupt Frame. RLE dlen too large" << std::endl;
				}

				rlebuf->write1((dlen+extra) << compression);
				rlebuf->write(outbuf,dlen+extra);
			}
			else {
				rlebuf->write1((dlen<<1)|1);
				rlebuf->write1(source->read1());
				xpos+=dlen;
			}

		} while (xpos < width);
	}

	bytes_rle = rlebuf->getPos();
	rle_data = new uint8[bytes_rle];
	memcpy (rle_data, rlebuf->getBuf(), bytes_rle);
}

void ConvertShapeFrame::GetPixels(uint8 *buf, sint32 count, sint32 x, sint32 y)
{
	x += xoff;
	y += yoff;

	if (y > height) return;

	sint32 xpos = 0;
	const uint8 * linedata = rle_data + line_offsets[y];

	do {
		xpos += *linedata++;
	  
		if (xpos == width) break;

		sint32 dlen = *linedata++;
		int type = 0;
		
		if (compression) 
		{
			type = dlen & 1;
			dlen >>= 1;
		}

		if (x >= xpos && x < (xpos+dlen))
		{
			int diff = x-xpos;
			dlen-=diff;
			xpos = x;

			int num = count;
			if (dlen < count) num = dlen;

			if (!type) {
				const uint8 *l = (linedata+=diff);

				while (num--) {
					*buf++ = *l++;
					count--;
					x++;
				}
			}
			else {
				uint8 l = *linedata;

				while (num--) {
					*buf++ = l;
					count--;
					x++;
				}
			}

			if (count == 0) return;
		}
		
		if (!type) linedata+=dlen;
		else linedata++;

		xpos += dlen;

	} while (xpos < width);
}

int ConvertShape::CalcNumFrames(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len, uint32 start_pos_param) // Renamed start_pos to avoid conflict
{
	int f=0;
	uint32 first_offset = 0xFFFFFFFF;
	uint32 save_pos = source->getPos();

	const int MAX_CALC_FRAMES = 2048; // Safety limit

	for (f=0; f < MAX_CALC_FRAMES ;f++) { // Added safety limit to loop
		uint32 frame_header_abs_pos = start_pos_param + csf->len_header + (csf->len_frameheader*f);

		if (frame_header_abs_pos > real_len || (frame_header_abs_pos == real_len && (csf->bytes_frame_offset > 0 || csf->bytes_frameheader_unk > 0 || csf->bytes_frame_length > 0))) {
			break; 
		}
		source->seek(frame_header_abs_pos);
		// After seek, check if we are at EOF unexpectedly (e.g. if real_len was smaller than seek target)
		if (source->getPos() >= real_len && (csf->bytes_frame_offset > 0 || csf->bytes_frameheader_unk > 0 || csf->bytes_frame_length > 0)) {
			break;
		}


		if ((source->getPos()-start_pos_param) >= first_offset && first_offset != 0xFFFFFFFF) {
			// This condition implies we've looped back to an already seen offset or past it.
			// However, ensure we are not already at EOF due to a short file.
			if (source->eof() && source->getPos() >= real_len) break;
			// The original logic: if ((source->getPos()-start_pos) >= first_offset) break;
			// This check is tricky. If first_offset was from a very short frame at the end,
			// and now we are calculating a header position that is *before* it but still valid,
			// we might break prematurely. The primary safety is can_read_or_skip_bytes.
			// For now, let's keep the original logic but ensure reads are safe.
			if (source->getPos() >= (start_pos_param + first_offset) && first_offset != 0xFFFFFFFF) break;
		}

		uint32 frame_offset_val = csf->len_header + (csf->len_frameheader*f);
		if (csf->bytes_frame_offset) {
			if (!can_read_or_skip_bytes(source, csf->bytes_frame_offset, real_len)) { break; }
			frame_offset_val = source->readX(csf->bytes_frame_offset) + csf->bytes_special;
		}

		if (frame_offset_val < first_offset) {
			first_offset = frame_offset_val;
		}
		if (start_pos_param + frame_offset_val > real_len) { // Offset points beyond file
			break;
		}

		if (csf->bytes_frameheader_unk) {
			if (!can_read_or_skip_bytes(source, csf->bytes_frameheader_unk, real_len)) { break; }
			source->skip(csf->bytes_frameheader_unk);
		}

		// uint32 frame_length_val = real_len - frame_offset_val; // Not strictly needed for CalcNumFrames loop logic
		if (csf->bytes_frame_length) {
			if (!can_read_or_skip_bytes(source, csf->bytes_frame_length, real_len)) { break; }
			/* frame_length_val = */ source->readX(csf->bytes_frame_length); // Value not used here
		}
	}

	source->seek(save_pos);
	return f;
}

bool ConvertShape::Check(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len)
{
#if 0
	pout << "Testing " << csf->name << "..." << std::endl;
#endif
	bool result = true;

	// Just to be safe
	int start_pos = source->getPos();

	// Read the ident
	if (csf->bytes_ident)
	{
		char ident[5];
		ident[csf->bytes_ident] = 0;
		source->read(ident, csf->bytes_ident);

		if (std::memcmp (ident, csf->ident, csf->bytes_ident))
		{
			// Return to start position
			source->seek(start_pos);
			return false;
		}
	}

	// Read the header special colour
	if (csf->bytes_special) source->skip(csf->bytes_special);

	// Read the header unknown
	if (csf->bytes_header_unk) source->skip(csf->bytes_header_unk);

	// Now read num_frames
	int num_frames = 1;
	if (csf->bytes_num_frames) num_frames = source->readX(csf->bytes_num_frames);
	if (num_frames == 0) num_frames = CalcNumFrames(source,csf,real_len,start_pos);

	// Create frames array
	ConvertShapeFrame oneframe;
	std::memset (&oneframe, 0, sizeof(ConvertShapeFrame));

	// Now read the frames
	for (int f = 0; f < num_frames; f++) 
	{
		ConvertShapeFrame *frame = &oneframe;

		// Seek to initial pos
		source->seek(start_pos + csf->len_header + (csf->len_frameheader*f));

		// Read the offset
		uint32 frame_offset = csf->len_header + (csf->len_frameheader*f);
		if (csf->bytes_frame_offset) frame_offset = source->readX(csf->bytes_frame_offset) + csf->bytes_special;

		// Read the unknown
		if (csf->bytes_frameheader_unk) source->read(frame->header_unknown, csf->bytes_frameheader_unk);

		// Read frame_length
		uint32 frame_length = real_len-frame_offset;
		if (csf->bytes_frame_length) frame_length = source->readX(csf->bytes_frame_length) + csf->bytes_frame_length_kludge;

		// Invalid frame length
		if ((frame_length + frame_offset) > real_len)
		{
			result = false;
			break;
		}

		// Seek to start of frame
		source->seek(start_pos + frame_offset);

		// Read unknown
		if (csf->bytes_frame_unknown) source->read(frame->unknown, csf->bytes_frame_unknown);

		// Frame details
		frame->compression = source->readX(csf->bytes_frame_compression);
		frame->width = source->readXS(csf->bytes_frame_width);
		frame->height = source->readXS(csf->bytes_frame_height);
		frame->xoff = source->readXS(csf->bytes_frame_xoff);
		frame->yoff = source->readXS(csf->bytes_frame_yoff);

		if ((frame->compression != 0 && frame->compression != 1) || frame->width < 0 || frame->height < 0)
		{
			frame->compression = 0;
			frame->width = 0;
			frame->height = 0;
			frame->xoff = 0;
			frame->yoff = 0;
			result = false;
			break;
		}

		if (frame->height)
		{
			// Line offsets
			sint32 highest_offset_byte = 0;

			// Calculate the number of bytes of RLE data
			frame->bytes_rle = frame_length - (csf->len_frameheader2+(frame->height*csf->bytes_line_offset));

			// Totally invalid shape
			if (frame->bytes_rle < 0)
			{
				result = false;
				break;
			}

			// Only attempt to decompress the shape if we are not a compressed shapes
			if (!csf->bytes_special) {

				// Seek to first in offset table
				source->seek(start_pos + frame_offset + csf->len_frameheader2);

				// Loop through each of the frames and find the last rle run
				for (int i = 0; i < frame->height; i++) 
				{
					sint32 line_offset = source->readX(csf->bytes_line_offset);

					// Now fudge with the value and turn it into an offset into the rle data
					// if required
					if (!csf->line_offset_absolute) 
						line_offset -= (frame->height-i)*csf->bytes_line_offset;

					if (line_offset > frame->bytes_rle)
					{
						result = false;
						break;
					}

					if (line_offset > highest_offset_byte) highest_offset_byte = line_offset;
				};

				// Failed for whatever reason
				if (result == false) break;

				// Jump to the line offset and calculate the length of the run
				source->seek(highest_offset_byte + start_pos + frame_offset + csf->len_frameheader2 + frame->height*csf->bytes_line_offset);
				int xpos = 0;
				uint32 dlen = 0;

				// Compressed
				if (frame->compression) do
				{
					xpos += source->read1();
					if (xpos == frame->width) break;

					dlen = source->read1();
					int type = dlen & 1;
					dlen >>= 1;

					if (!type) source->skip(dlen);
					else source->skip(1);

					xpos += dlen;

				} while (xpos < frame->width);
				// Uncompressed
				else do
				{
					xpos += source->read1();
					if (xpos == frame->width) break;

					dlen = source->read1();
					source->skip(dlen);

					xpos += dlen;
				} while (xpos < frame->width);

				// Calc 'real' bytes rle
				sint32 highest_rle_byte = source->getPos();
				highest_rle_byte -= start_pos + frame_offset + csf->len_frameheader2 + frame->height*csf->bytes_line_offset;

				// Too many bytes
				if (highest_rle_byte > frame->bytes_rle)
				{
					result = false;
					break;
				}
			}
		}
	}

	// Free frames
	oneframe.Free();
	num_frames = 0;

	// Return to start position
	source->seek(start_pos);

	return result;
}

bool ConvertShape::CheckUnsafe(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len)
{
	bool result = true;
	uint32 start_pos = source->getPos(); // getPos() is uint32

	// Read the ident
	if (csf->bytes_ident)
	{
		if (!can_read_or_skip_bytes(source, csf->bytes_ident, real_len)) {
			source->seek(start_pos); return false;
		}
		char ident[5]; // Max 4 bytes + null
		if (csf->bytes_ident > 4) { /* Should not happen based on struct, but defensive */ source->seek(start_pos); return false; }
		ident[csf->bytes_ident] = 0;
		sint32 bytes_read = source->read(ident, csf->bytes_ident);
		if (bytes_read < static_cast<sint32>(csf->bytes_ident)) { // Short read
			source->seek(start_pos); return false;
		}

		if (std::memcmp (ident, csf->ident, csf->bytes_ident))
		{
			source->seek(start_pos);
			return false;
		}
	}

	if (csf->bytes_special) {
		if (!can_read_or_skip_bytes(source, csf->bytes_special, real_len)) {
			source->seek(start_pos); return false;
		}
		source->skip(csf->bytes_special);
	}

	if (csf->bytes_header_unk) {
		if (!can_read_or_skip_bytes(source, csf->bytes_header_unk, real_len)) {
			source->seek(start_pos); return false;
		}
		source->skip(csf->bytes_header_unk);
	}

	int num_frames = 1; // Default
	if (csf->bytes_num_frames) {
		if (!can_read_or_skip_bytes(source, csf->bytes_num_frames, real_len)) {
			source->seek(start_pos); return false;
		}
		num_frames = source->readX(csf->bytes_num_frames);
	}

	if (num_frames == 0 && csf->bytes_num_frames > 0) { // If num_frames was explicitly read as 0
		// Call CalcNumFrames only if num_frames was read as 0 from a field that exists.
		// Ensure CalcNumFrames is safe. The version above has some safety.
		num_frames = CalcNumFrames(source,csf,real_len,start_pos);
	} else if (num_frames == 0 && csf->bytes_num_frames == 0) {
		num_frames = 1; // Default if bytes_num_frames is 0
	}


	// Sanity check for num_frames to prevent excessive loops or large allocations
	const int MAX_FRAMES_TO_CHECK = 2048; // Limit for CheckUnsafe
	if (num_frames < 0 || num_frames > MAX_FRAMES_TO_CHECK) {
		// If num_frames is negative (possible if readXS was used and bytes_num_frames was large, though readX is used here)
		// or excessively large, consider it corrupt.
		source->seek(start_pos);
		return false;
	}
    
	// ConvertShapeFrame oneframe; // Not used for reading in CheckUnsafe, only for its members if needed
	// std::memset (&oneframe, 0, sizeof(ConvertShapeFrame)); // Not needed

	for (int f = 0; f < num_frames; f++) 
	{
		// ConvertShapeFrame *frame = &oneframe; // Not needed for CheckUnsafe's direct reads

		uint32 frame_header_abs_pos = start_pos + csf->len_header + (csf->len_frameheader*f);
		if (frame_header_abs_pos > real_len || (frame_header_abs_pos == real_len && (csf->bytes_frame_offset > 0 || csf->bytes_frameheader_unk > 0 || csf->bytes_frame_length > 0))) {
			result = false; break;
		}
		source->seek(frame_header_abs_pos);
		if (source->getPos() >= real_len && (csf->bytes_frame_offset > 0 || csf->bytes_frameheader_unk > 0 || csf->bytes_frame_length > 0)) {
			result = false; break; // Seek put us at EOF, cannot read frame header fields
		}


		uint32 frame_offset_val = csf->len_header + (csf->len_frameheader*f);
		if (csf->bytes_frame_offset) {
			if (!can_read_or_skip_bytes(source, csf->bytes_frame_offset, real_len)) { result = false; break; }
			frame_offset_val = source->readX(csf->bytes_frame_offset) + csf->bytes_special;
		}

		if (csf->bytes_frameheader_unk) {
			if (!can_read_or_skip_bytes(source, csf->bytes_frameheader_unk, real_len)) { result = false; break; }
			source->skip(csf->bytes_frameheader_unk); // CheckUnsafe doesn't need to store this
		}

		uint32 frame_length_val = real_len - frame_offset_val; // Default assumption
		if (frame_offset_val > real_len) { result = false; break; } // frame_offset itself is out of bounds

		if (csf->bytes_frame_length) {
			if (!can_read_or_skip_bytes(source, csf->bytes_frame_length, real_len)) { result = false; break; }
			frame_length_val = source->readX(csf->bytes_frame_length) + csf->bytes_frame_length_kludge;
		}

		if ((start_pos + frame_offset_val + frame_length_val) > real_len) { // Check combined offset + length
			result = false; break;
		}
		if (start_pos + frame_offset_val > real_len) { // Check offset itself again after potential read
			result = false; break;
		}


		uint32 frame_data_abs_pos = start_pos + frame_offset_val;
		if (frame_data_abs_pos > real_len || (frame_data_abs_pos == real_len && (csf->bytes_frame_unknown > 0 || csf->bytes_frame_compression > 0 /*etc*/))) {
			result = false; break;
		}
		source->seek(frame_data_abs_pos);
		if (source->getPos() >= real_len && (csf->bytes_frame_unknown > 0 || csf->bytes_frame_compression > 0 /*etc*/)) {
			result = false; break;
		}


		if (csf->bytes_frame_unknown) {
			if (!can_read_or_skip_bytes(source, csf->bytes_frame_unknown, real_len)) { result = false; break; }
			source->skip(csf->bytes_frame_unknown);
		}

		sint32 compression_val, width_val, height_val; // xoff, yoff not strictly needed for CheckUnsafe logic after this
		if (!can_read_or_skip_bytes(source, csf->bytes_frame_compression, real_len)) { result = false; break; }
		compression_val = source->readX(csf->bytes_frame_compression);
		if (!can_read_or_skip_bytes(source, csf->bytes_frame_width, real_len)) { result = false; break; }
		width_val = source->readXS(csf->bytes_frame_width);
		if (!can_read_or_skip_bytes(source, csf->bytes_frame_height, real_len)) { result = false; break; }
		height_val = source->readXS(csf->bytes_frame_height);
        
		// xoff, yoff are read in original CheckUnsafe, so we should read them to advance pointer correctly, even if not used for logic
		if (csf->bytes_frame_xoff > 0) {
			if (!can_read_or_skip_bytes(source, csf->bytes_frame_xoff, real_len)) { result = false; break; }
			/* sxoff_val = */ source->readXS(csf->bytes_frame_xoff);
		}
		if (csf->bytes_frame_yoff > 0) {
			if (!can_read_or_skip_bytes(source, csf->bytes_frame_yoff, real_len)) { result = false; break; }
			/* syoff_val = */ source->readXS(csf->bytes_frame_yoff);
		}


		if ((compression_val != 0 && compression_val != 1) || width_val < 0 || height_val < 0) {
			result = false; break;
		}

		if (height_val > 0) {
			sint32 header2_size = static_cast<sint32>(csf->len_frameheader2);
			sint32 line_offsets_total_bytes = height_val * static_cast<sint32>(csf->bytes_line_offset);

			if (line_offsets_total_bytes < 0) { result = false; break; } // Overflow

			sint32 min_frame_data_needed = header2_size + line_offsets_total_bytes;
			if (min_frame_data_needed < 0) { result = false; break; } // Overflow

			// frame_length_val is the length of the data starting from frame_data_abs_pos
			// Check if frame_length_val is sufficient for len_frameheader2 and all line_offsets
			if (static_cast<sint32>(frame_length_val) < min_frame_data_needed && csf->bytes_line_offset > 0) { // only if line offsets exist
				result = false; break;
			}
			// The original CheckUnsafe calculates bytes_rle and checks if it's < 0.
			// frame->bytes_rle = frame_length - (csf->len_frameheader2+(frame->height*csf->bytes_line_offset));
			// if (frame->bytes_rle < 0) { result = false; break; }
			// This is equivalent to the check above.
		}
		if (!result) break; // If any check failed in the loop
	}

	// oneframe.Free(); // Not allocated
	source->seek(start_pos);
	return result;
}

void ConvertShape::Write(ODataSource *dest, const ConvertShapeFormat *csf, uint32 &write_len)
{
	// Just to be safe
	uint32 start_pos = dest->getPos();

	// Write the ident
	if (csf->bytes_ident) dest->write(csf->ident, csf->bytes_ident);

	// Write the header unknown
	if (csf->bytes_header_unk) dest->write(header_unknown, csf->bytes_header_unk);

	// Now write num_frames
	if (csf->bytes_num_frames) dest->writeX(num_frames, csf->bytes_num_frames);
	else if (!csf->bytes_num_frames && num_frames > 1)
	{
		perr << "Error: Unable to convert multiple frame shapes to " << csf->name << std::endl; 
		return;
	}

	// Write filler space for the frame details
	for (uint32 i = 0; i < num_frames*csf->len_frameheader; i++) dest->write1(0);

	// Now write the frames
	for(uint32 f = 0; f < num_frames; f++) 
	{
		ConvertShapeFrame *frame = frames+f;

		// Get the frame offset
		uint32 frame_offset = dest->getPos() - start_pos;

		// Seek to the frame header pos
		dest->seek(start_pos + csf->len_header + (csf->len_frameheader*f));

		// Write the offset
		if (csf->bytes_frame_offset) dest->writeX(frame_offset, csf->bytes_frame_offset);

		// Write the unknown
		if (csf->bytes_frameheader_unk) dest->write(frame->header_unknown, csf->bytes_frameheader_unk);

		// Calc and write frame_length
		if (csf->bytes_frame_length)
		{
			uint32 frame_length = csf->len_frameheader2 + (frame->height*csf->bytes_line_offset) + frame->bytes_rle;
			dest->writeX(frame_length - csf->bytes_frame_length_kludge, csf->bytes_frame_length);
		}

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
		for (sint32 i = 0; i < frame->height; i++) 
		{
			sint32 actual_offset = frame->line_offsets[i];
			
			// Unfudge the value and write it, if requiretd
			if (!csf->line_offset_absolute)  
				actual_offset += (frame->height-i)*csf->bytes_line_offset;

			dest->writeX(actual_offset, csf->bytes_line_offset);
		}

		// Write the RLE Data
		dest->write(frame->rle_data, frame->bytes_rle);
	}

	// Just cheat
	write_len = dest->getPos() - start_pos;
}


// Shape format configuration for Pentagram
const ConvertShapeFormat		PentagramShapeFormat =
{
	"Pentagram",
	8,		// header
	"PSHP",	// ident
	4,		// bytes_ident 
	0,		// bytes_special
	0,		// header_unk
	4,		// num_frames

	8,		// frameheader
	4,		// frame_offset
	0,		// frameheader_unk
	4,		// frame_length
	0,		// frame_length_kludge

	20,		// frameheader2
	0,		// frame_unknown
	4,		// frame_compression
	4,		// frame_width
	4,		// frame_height
	4,		// frame_xoff
	4,		// frame_yoff

	4,		// line_offset
	1		// line_offset_absolute
};

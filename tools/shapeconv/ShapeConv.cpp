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

#include "util.h"

#include "FileSystem.h"

#include "Args.h"

#include "ConvertShape.h"

#include "crusader/ConvertShapeCrusader.h"
#include "u8/ConvertShapeU8.h"

const ConvertShapeFormat		AutoShapeFormat =
{
	"Auto Detected",
	
	0,		// len_header
	0,		// ident

	0,		// bytes_ident
	0,		// bytes_header_unk
	0,		// bytes_num_frames

	0,		// len_frameheader
	0,		// bytes_frame_offset
	0,		// bytes_frameheader_unk
	0,		// bytes_frame_length
	0,		// bytes_frame_length_kludge

	0,		// len_frameheader2
	0,		// bytes_frame_unknown
	0,		// bytes_frame_compression
	0,		// bytes_frame_width
	0,		// bytes_frame_height
	0,		// bytes_frame_xoff
	0,		// bytes_frame_yoff

	0,		// bytes_line_offset
	0		// line_offset_absolute
};

// Convert formats we are using
const ConvertShapeFormat *read_format = &AutoShapeFormat;
const ConvertShapeFormat *write_format = &U8ShapeFormat;

//#define EXPORT_SHAPENUM
#ifdef EXPORT_SHAPENUM
int shapenum;
#endif

// Nasty yucky stuff
void ConvertFlexes(IDataSource *readfile, ODataSource *writefile)
{
	ConvertShape	shape;
	uint32			i;

	// Number of flex entries
	readfile->seek(0x54);
	uint32 num_entries = readfile->read4();

	// Write blank stuff for output 
	for (i = 0; i < 0x52; i++) writefile->write1(0x1A);
	writefile->write2(0);

	// Write num entries
	writefile->write4(num_entries);

	// Offset to begin writing a shape at
	uint32	write_offset = 0x80 + 8 * num_entries;

	// Write blank index table
	for (i = 0x58; i < write_offset; i++)  writefile->write1(0);

	// Convert shapes
	con.Printf ("Convering %i shapes...\n", num_entries);
	for (uint32 s = 0; s < num_entries; s++)
	{
#ifdef EXPORT_SHAPENUM
		shapenum = s;
#endif

		// Get the read offset and size
		readfile->seek(0x80 + 8*s);
		uint32 read_offset = readfile->read4();
		uint32 read_size = readfile->read4();

		if (!read_size) continue;

		// Seek to shape
		readfile->seek(read_offset);

		// Detect ShapeFormat
		if (read_format == &AutoShapeFormat)
		{
			pout << "Auto detecting format..." << std::endl;

			if (ConvertShape::Check(readfile, &U8ShapeFormat, read_size))
				read_format = &U8ShapeFormat;
			else if (ConvertShape::Check(readfile, &U82DShapeFormat, read_size))
				read_format = &U82DShapeFormat;
			else if (ConvertShape::Check(readfile, &U8SKFShapeFormat, read_size))
				read_format = &U8SKFShapeFormat;
			else if (ConvertShape::Check(readfile, &CrusaderShapeFormat, read_size))
				read_format = &CrusaderShapeFormat;
			else if (ConvertShape::Check(readfile, &Crusader2DShapeFormat, read_size))
				read_format = &Crusader2DShapeFormat;
			else if (ConvertShape::Check(readfile, &PentagramShapeFormat, read_size))
				read_format = &PentagramShapeFormat;
			else if (ConvertShape::Check(readfile, &U8CMPShapeFormat, read_size))
				read_format = &U8CMPShapeFormat;
			else
			{
				perr << "Error: Unable to detect shape format!" << std::endl;
				return;
			}
			pout << "Detected input format as: " << read_format->name << std::endl;
		}
		
		// Read shape
		con.Printf ("Reading shape %i...\n", s);
		shape.Read(readfile, read_format, read_size);

		// Write shape
		con.Printf ("Writing shape %i...\n", s);
		writefile->seek(write_offset);
		uint32 write_size;
		shape.Write(writefile, write_format, write_size);

		// Update the table
		writefile->seek(0x80 + 8*s);
		writefile->write4(write_offset);
		writefile->write4(write_size);

		// Update the write_offset
		write_offset += write_size;

		// Free it
		shape.Free();
	}
	pout << "Done!" << std::endl;
}

void ConvertShp(IDataSource *readfile, ODataSource *writefile)
{
#ifdef EXPORT_SHAPENUM
	shapenum = 1;
#endif
	ConvertShape shape;
	uint32 read_size = readfile->getSize();

	// Detect ShapeFormat
	if (read_format == &AutoShapeFormat)
	{
		pout << "Auto detecting format..." << std::endl;
		if (ConvertShape::Check(readfile, &U8ShapeFormat, read_size))
			read_format = &U8ShapeFormat;
		else if (ConvertShape::Check(readfile, &U82DShapeFormat, read_size))
			read_format = &U82DShapeFormat;
		else if (ConvertShape::Check(readfile, &U8SKFShapeFormat, read_size))
			read_format = &U8SKFShapeFormat;
		else if (ConvertShape::Check(readfile, &CrusaderShapeFormat, read_size))
			read_format = &CrusaderShapeFormat;
		else if (ConvertShape::Check(readfile, &Crusader2DShapeFormat, read_size))
			read_format = &Crusader2DShapeFormat;
		else if (ConvertShape::Check(readfile, &PentagramShapeFormat, read_size))
			read_format = &PentagramShapeFormat;
		else if (ConvertShape::Check(readfile, &U8CMPShapeFormat, read_size))
			read_format = &U8CMPShapeFormat;
		else
		{
			perr << "Error: Unable to detect shape format!" << std::endl;
			return;
		}
		pout << "Detected input format as: " << read_format->name << std::endl;
	}

	con.Printf ("Reading shape...\n");
	shape.Read(readfile, read_format, read_size);
	uint32 write_size;
	con.Printf ("Writing shape...\n");
	shape.Write(writefile, write_format, write_size);
	shape.Free();
	pout << "Done!" << std::endl;
}

const ConvertShapeFormat *GetShapeFormat(const char *game)
{
	if (!Pentagram::strcasecmp(game, "u8")) return &U8ShapeFormat;
	else if (!Pentagram::strcasecmp(game, "u82D")) return &U82DShapeFormat;
	else if (!Pentagram::strcasecmp(game, "u8skf")) return &U8SKFShapeFormat;
	else if (!Pentagram::strcasecmp(game, "cru")) return &CrusaderShapeFormat;
	else if (!Pentagram::strcasecmp(game, "cru2D")) return &Crusader2DShapeFormat;
	else if (!Pentagram::strcasecmp(game, "pent")) return &PentagramShapeFormat;

	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		perr << "Usage: ShapeConv <inflx> <outflx> [--ifmt u8|u82D|u8skf|u8cmp|cru|cru2D|pent|auto] [--ofmt u8|u82D|u8skf|cru|cru2D|pent] [--singlefile]" << std::endl;
		perr << std::endl;
		perr << "Default input format: Auto Detect" << std::endl;
		perr << "Default output format: Ultima 8" << std::endl;
		return 1;
	}

	Args		parameters;
	std::string	ifmt, ofmt;
	bool		singlefile=false;
	//bool		auto_detect=false; // Darke: UNUSED?

	parameters.declare("--ifmt",		&ifmt,      "auto");
	parameters.declare("--ofmt",		&ofmt,      "u8");
	parameters.declare("--singlefile",	&singlefile, true);

	parameters.process(argc, argv);

	if (!Pentagram::strcasecmp(ifmt.c_str(), "auto")) read_format = &AutoShapeFormat;
	else if (!Pentagram::strcasecmp(ifmt.c_str(), "u8cmp")) read_format = &U8CMPShapeFormat;
	else read_format = GetShapeFormat(ifmt.c_str());

	write_format = GetShapeFormat(ofmt.c_str());

	if (!read_format) 
	{
		perr << "Unknown input format: " << ifmt << std::endl;
		return -1;
	}

	if (!write_format) 
	{
		perr << "Unknown output format: " << ifmt << std::endl;
		return -1;
	}

	// Check to see if the file ends in .shp. If it does, assume single shape file
	if (std::strlen(argv[1]) > 4 && !Pentagram::strcasecmp(argv[1]+std::strlen(argv[1])-4, ".shp"))
		singlefile = true;

	if (singlefile) pout << "Single shape mode" << std::endl;

	if (singlefile)
		pout << "Converting " << read_format->name << " format shape in '"<< argv[1] << "' to " << write_format->name << " format in '"<< argv[2] << "'" << std::endl;
	else
		pout << "Converting " << read_format->name << " format shapes in flex '"<< argv[1] << "' to " << write_format->name << " format in '"<< argv[2] << "'" << std::endl;

	// Create filesystem object
	FileSystem filesys(true);

	// Load read flex/shape file
	IDataSource *readfile = filesys.ReadFile(argv[1]);

	// Uh oh, couldn't load it
	if(readfile==0)
	{
		perr << "Error reading file '" << argv[1] << "'" << std::endl;
		return 1;
	}

	// Load write flex/shape file
	ODataSource *writefile = filesys.WriteFile(argv[2]);

	// Uh oh, couldn't load it
	if(writefile==0)
	{
		perr << "Error writing file '" << argv[2] << "'" << std::endl;
		return 1;
	}

	if (!singlefile)
		ConvertFlexes(readfile, writefile);
	else
		ConvertShp(readfile, writefile);

	// Clean up
	delete readfile;
	delete writefile;

	return 0;
}

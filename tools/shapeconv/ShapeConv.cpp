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
#include "Q_strcasecmp.h"

#include "FileSystem.h"

#include "ConvertShape.h"

#include "crusader/ConvertShapeCrusader.h"
#include "u8/ConvertShapeU8.h"

// Convert formats we are using
const ConvertShapeFormat *read_format = &CrusaderShapeFormat;
const ConvertShapeFormat *write_format = &U8ShapeFormat;

//int shapenum;

// Nasty yucky stuff
void ConvertFlexes(IFileDataSource *readfile, OFileDataSource *writefile)
{
	ConvertShape	shape;
	uint32			i;

	// Number of flex entries
	readfile->seek(0x54);
	uint32 num_entries = readfile->read4();

	// Write blank stuff for output 
	for (i = 0; i < 0x54; i++) writefile->write1(0);

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
		//shapenum = s;

		// Get the read offset and size
		readfile->seek(0x80 + 8*s);
		uint32 read_offset = readfile->read4();
		uint32 read_size = readfile->read4();

		if (!read_size) continue;

		// Read shape
		con.Printf ("Reading shape %i...\n", s);
		readfile->seek(read_offset);
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

int main(int argc, char **argv)
{
	if (argc < 3) {
		perr << "Usage: ShapeConv <inflx> <outflx> [-u8tocru|-crutou8]" << std::endl;
		return 1;
	}

	if (argc >= 4 && ~Q_strcasecmp(argv[3], "-u8tocru"))
	{
		pout << "Converting Ultima8 Format shapes in '"<< argv[1] << "' to Crusader Format in '"<< argv[2] << "'" << std::endl;
		read_format = &U8ShapeFormat;
		write_format = &CrusaderShapeFormat;
	}
	else
	{
		pout << "Converting Crusader Format shapes in '"<< argv[1] << "' to Ultima8 Format in '"<< argv[2] << "'" << std::endl;
	}

	// Create filesystem object
	FileSystem filesys;

	// Load read flex file
	IFileDataSource *readfile = filesys.ReadFile(argv[1]);

	// Uh oh, couldn't load it
	if(readfile==0)
	{
		perr << "Error reading file '" << argv[1] << "'" << std::endl;
		return 1;
	}

	// Load write flex file
	OFileDataSource *writefile = filesys.WriteFile(argv[2]);

	// Uh oh, couldn't load it
	if(writefile==0)
	{
		perr << "Error writing file '" << argv[2] << "'" << std::endl;
		return 1;
	}

	ConvertFlexes(readfile, writefile);

	// Clean up
	delete readfile;
	delete writefile;

	return 0;
}

/*
Copyright (C) 2003  The Pentagram Team

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

#include "pent_include.h"

#include "MusicFlex.h"
#include "XMidiFile.h"
#include "IDataSource.h"

using std::memset;
using std::strncpy;

MusicFlex::MusicFlex(IDataSource* ds) : Flex(ds)
{
	songs = new XMidiFile * [count];

	memset(info,0, sizeof(SongInfo*)*128);
	memset(songs,0, sizeof(XMidiFile*)*count);
	loadSongInfo();
}

MusicFlex::~MusicFlex()
{
	uint32 i;
	for (i = 0; i < 128; i++)
	{
		delete info[i];
		info[i] = 0;
	}

	for (i = 0; i < count; i++)
	{
		delete songs[i];
		songs[i] = 0;
	}

	delete [] songs;
}

MusicFlex::SongInfo::SongInfo() : num_measures(0), loop_jump(0)
{
	memset(filename,0,16);
	memset(transitions,0,128*sizeof(int*));
}

MusicFlex::SongInfo::~SongInfo()
{
	for (int i = 0; i < 128; i++)
	{
		delete [] transitions[i];
		transitions[i] = 0;
	}
}

void MusicFlex::cache(uint32 index)
{
	if (index >= count) return;

	if (songs[index]) return;

	// This will cache the data
	uint8 *buf = get_object(index);
	uint32 size = get_size(index);

	if (!buf || !size) return;
	IBufferDataSource ds(buf,size);

	XMidiFile *x = new XMidiFile(&ds, XMIDIFILE_CONVERT_NOCONVERSION);

	if (x->number_of_tracks())
		songs[index] = x;
	else
		delete x;

	delete [] buf;
}

void MusicFlex::cache()
{
	for (uint32 i = 0; i < count; i++)
	{
		cache(i);
	}
}

void MusicFlex::uncache()
{
	for (uint32 i = 0; i < count; i++)
	{
		delete songs[i];
		songs[i] = 0;
	}
}

void MusicFlex::loadSongInfo()
{
	// This will cache the data
	uint8 *buf = get_object(0);
	uint32 size = get_size(0);

	if (!buf || !size)
	{
		perr << "Unable to load song info from sound/music.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	IBufferDataSource ds(buf,size);
	std::string line;

	// Read first section till we hit a #
	for (;;)
	{
		ds.readline(line);	

		// We have hit the end of the section
		if (line.at(0) == '#') break;

		std::string::size_type	begIdx, endIdx;

		// Find the first not space, which will get us the name
 		begIdx = line.find_first_not_of(' ');
 		endIdx = line.find_first_of(' ', begIdx);
		std::string name = line.substr(begIdx,endIdx-begIdx);

		// Now find the first not space after the name, which will get us the num
		begIdx = line.find_first_not_of(' ', endIdx);
 		endIdx = line.find_first_of(' ', begIdx);
		int num = line.at(begIdx);

		// Now number of measures
 		begIdx = line.find_first_not_of(' ', endIdx);
 		endIdx = line.find_first_of(' ', begIdx);
		int measures = std::atoi(line.substr(begIdx,endIdx-begIdx).c_str());

		// Now finally loop_jump
 		begIdx = line.find_first_not_of(' ', endIdx);
 		endIdx = line.find_first_of(' ', begIdx);
		int loop_jump = std::atoi(line.substr(begIdx,endIdx-begIdx).c_str());

		// Uh oh
		if (num < 0 || num > 127)
		{
			perr << "Invalid Section 1 song info data. num out of range. Exiting." << std::endl;
			std::exit(-1);
		}

		if (info[num])
		{
			perr << "Invalid Section 1 song info data. num already defined. Exiting." << std::endl;
			std::exit(-1);
		}

		info[num] = new SongInfo();

		strncpy(info[num]->filename, name.c_str(), 16);
		info[num]->num_measures = measures;
		info[num]->loop_jump = loop_jump;
	};

	// Read 'Section2', or more like skip it, since it's only trans.xmi
	// Read first section till we hit a #
	for (;;)
	{
		ds.readline(line);	

		// We have hit the end of the section
		if (line.at(0) == '#') break;
	}

	// Skip 'Section3'
	for (;;)
	{
		ds.readline(line);	

		// We have hit the end of the section
		if (line.at(0) == '#') break;
	}

	// Read 'Section4' (trans info)
	for (;;)
	{
		ds.readline(line);	

		// We have hit the end of the section
		if (line.at(0) == '#') break;

		std::string::size_type	begIdx, endIdx;

		// Get 'from' name
 		begIdx = line.find_first_not_of(' ');
 		endIdx = line.find_first_of(' ', begIdx);
		std::string from = line.substr(begIdx,endIdx-begIdx);

		// Get 'to' name
		begIdx = line.find_first_not_of(' ', endIdx);
 		endIdx = line.find_first_of(' ', begIdx);
		std::string to = line.substr(begIdx,endIdx-begIdx);

		// Find index of from name
		int fi;
		for (fi = 0; fi < 128; fi++)
		{
			if (info[fi] && from == info[fi]->filename) break;
		}

		if (fi == 128)
		{
			perr << "Invalid Section 4 song info data. Unable to find 'from' index (" << from << "). Exiting." << std::endl;
			std::exit(-1);
		}

		// Find index of to name
		int ti;
		for (ti= 0; ti < 128; ti++)
		{
			if (info[ti] && to == info[ti]->filename) break;
		}

		if (ti == 128)
		{
			perr << "Invalid Section 4 song info data. Unable to find 'to' index (" << to << "). Exiting." << std::endl;
			std::exit(-1);
		}

		// Allocate Transition info
		info[fi]->transitions[ti] = new int[info[fi]->num_measures];

		// Now attempt to read the trans info for the
		for (int m = 0; m < info[fi]->num_measures; m++)
		{
			// Get trans info name
	 		begIdx = line.find_first_not_of(' ', endIdx);
 			endIdx = line.find_first_of(' ', begIdx);

			if (begIdx == std::string::npos)
			{
				perr << "Invalid Section 4 song info data. Unable to read transitions for all measures. Exiting." << std::endl;
				std::exit(-1);
			}

			std::string trans = line.substr(begIdx,endIdx-begIdx);
			const char *str = trans.c_str();

			int num = 0;

			// Overlayed
			if (*str == '!')
				num = -1-atoi(str+1);
			else
				num = atoi(str+1);

			info[fi]->transitions[ti][m] = num;
		}
	}

	// Skip all remaining sections

	delete buf;
}


/*
Copyright (C) 2003-2005  The Pentagram Team

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

#ifndef MUSICFLEX_H_INCLUDED
#define MUSICFLEX_H_INCLUDED

#include "filesys/Archive.h"

class XMidiFile;

class MusicFlex : public Pentagram::Archive
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	struct SongInfo
	{
		SongInfo();
		~SongInfo();

		char		filename[16];
		int			num_measures;
		int			loop_jump;
		int			*transitions[128];
	};

	MusicFlex (IDataSource* ds);
	~MusicFlex();

	//! Get an xmidi
	XMidiFile * getXMidi(uint32 index)
	{
		if (index >= count) return 0;
		cache(index);
		return songs[index];
	}

	//! Get song info
	const SongInfo * getSongInfo(uint32 index) const
	{
		if (index > 127) return 0;
		return info[index];
	}

	//! Get the Adlib Timbres (index 259)
	IDataSource * getAdlibTimbres();

	virtual void cache(uint32 index);
	virtual void uncache(uint32 index);
	virtual bool isCached(uint32 index);

private:
	SongInfo		*info[128];
	XMidiFile		**songs;

	//! Load the song info
	void			loadSongInfo();
};


#endif //MUSICFLEX_H_INCLUDED

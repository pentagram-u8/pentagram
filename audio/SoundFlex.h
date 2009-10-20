/*
Copyright (C) 2005  The Pentagram Team

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

#ifndef SOUNDFLEX_H_INCLUDED
#define SOUNDFLEX_H_INCLUDED

#include "filesys/Archive.h"

namespace Pentagram {
	class AudioSample;
};

class SoundFlex : protected Pentagram::Archive
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	SoundFlex (IDataSource* ds);
	~SoundFlex();

	//! Get an audiosample
	Pentagram::AudioSample * getSample(uint32 index)
	{
		if (index >= count) return 0;
		cache(index);
		return samples[index];
	}

	virtual void cache(uint32 index);
	virtual void uncache(uint32 index);
	virtual bool isCached(uint32 index);

private:
	Pentagram::AudioSample		**samples;
};


#endif //SOUNDFLEX_H_INCLUDED

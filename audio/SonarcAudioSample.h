/*
Copyright (C) 2005 The Pentagram team

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
#ifndef SONARCAUDIOSAMPLE_H_INCLUDED
#define SONARCAUDIOSAMPLE_H_INCLUDED

#include "AudioSample.h"

namespace Pentagram {

class SonarcAudioSample : public AudioSample
{
	struct SonarcDecompData {
		uint32		pos;
		uint32		sample_pos;
	};

	static bool	GeneratedOneTable;
	static int	OneTable[256];

	static void GenerateOneTable();

	static void decode_EC(int mode, int samplecount,
						const uint8* source, int sourcesize,
						uint8* dest);
	static void decode_LPC(int order, int nsamples,
						uint8* dest, const uint8* factors);
	static int audio_decode(const uint8* source, uint8* dest);

	uint32		src_offset;

public:
	SonarcAudioSample(uint8 *buffer_, uint32 size_);
	virtual ~SonarcAudioSample(void);

	virtual void initDecompressor(void *DecompData) const;
	virtual uint32 decompressFrame(void *DecompData, void *samples) const;
	virtual void rewind(void *DecompData) const;
};

};

#endif

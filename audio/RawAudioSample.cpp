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

#include "pent_include.h"
#include "RawAudioSample.h"
#include "IDataSource.h"

namespace Pentagram {

RawAudioSample::RawAudioSample(uint8* buffer_, uint32 size_, uint32 rate_,
							   bool signeddata_, bool stereo_)
	: AudioSample(buffer_, size_), signeddata(signeddata_)
{
	sample_rate = rate_;
	bits = 8;
	stereo = stereo_;
	frame_size = 512;
	decompressor_size = sizeof(RawDecompData);
	length = size_;
}

RawAudioSample::~RawAudioSample()
{

}

void RawAudioSample::initDecompressor(void *DecompData) const
{
	RawDecompData *decomp = reinterpret_cast<RawDecompData *>(DecompData);
	decomp->pos = 0;
}

void RawAudioSample::rewind(void *DecompData) const
{
	initDecompressor(DecompData);
}

uint32 RawAudioSample::decompressFrame(void *DecompData, void *samples) const
{
	RawDecompData *decomp = reinterpret_cast<RawDecompData *>(DecompData);

	if (decomp->pos == buffer_size) return 0;

	uint32 count = frame_size;
	if (decomp->pos + count > buffer_size)
		count = buffer_size - decomp->pos;

	if (!signeddata) {
		std::memcpy(samples, buffer+decomp->pos, count);
	} else {
		uint8* dest = static_cast<uint8*>(samples);
		for (unsigned int i = 0; i < count; ++i)
			dest[i] = buffer[decomp->pos+i] + 128;
	}

	decomp->pos += count;

	return count;
}



}

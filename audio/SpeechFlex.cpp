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
#include "pent_include.h"
#include "SpeechFlex.h"
#include <cstdlib>
#include <string>

// p_dynamic_class stuff 
DEFINE_RUNTIME_CLASSTYPE_CODE(SpeechFlex, SoundFlex);

SpeechFlex::SpeechFlex(IDataSource* ds) : SoundFlex(ds)
{
	uint32 size = getRawSize(0);
	uint8* buf = getRawObject(0);
	char* strings = reinterpret_cast<char*>(buf);
	char* strend = strings+size;

	// Now we s
	while (strings < strend) {

		char *end = reinterpret_cast<char*>(std::memchr(strings,0,size));
		if (!end) end = strend;

		phrases.push_back(Pentagram::istring(strings,end-strings));

		strings = end+1;
	}

	delete [] buf;

}

SpeechFlex::~SpeechFlex(void)
{
}

int	SpeechFlex::getIndexForPhrase(std::string &phrase) const
{
	std::vector<Pentagram::istring>::const_iterator it;
	int i = 1;

	for(it = phrases.begin(); it != phrases.end(); ++it)
	{
		if (phrase == *it) return i;
		i++;
	}

	return 0;
}


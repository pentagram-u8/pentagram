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

		Pentagram::istring text = Pentagram::istring(strings,end-strings);
		std::string::size_type pos1 = text.find_first_not_of(' ');
		std::string::size_type pos2 = text.find_last_not_of(' ');
		text = text.substr(pos1, pos2-pos1+1);

		// pout << "Found string: \"" << text << "\"" << std::endl;

		phrases.push_back(text);

		strings = end+1;
	}

	delete [] buf;

}

SpeechFlex::~SpeechFlex(void)
{
}

int	SpeechFlex::getIndexForPhrase(std::string &phrase,
								  uint32 start, uint32& end) const
{
	std::vector<Pentagram::istring>::const_iterator it;
	int i = 1;

	std::string text = phrase.substr(start);

	std::string::size_type pos1 = text.find_first_not_of(' ');
	if (pos1 == std::string::npos) return 0;

	std::string::size_type pos2 = text.find_last_not_of(' ');
	text = text.substr(pos1, pos2-pos1+1);

	// pout << "Looking for string: \"" << text << "\"" << std::endl;

	for(it = phrases.begin(); it != phrases.end(); ++it)
	{
		if (text.find(*it) == 0) {
			// pout << "Found: " << i << std::endl;
			end = (*it).size() + start + pos1;
			if (end >= start + pos2)
				end = phrase.size();
			return i;
		}
		i++;
	}

	// pout << "Not found" << std::endl;

	return 0;
}


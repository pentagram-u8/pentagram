/*
Copyright (C) 2003-2006  The Pentagram Team

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
#include "AudioSample.h"
#include "util.h"

// p_dynamic_class stuff 
DEFINE_RUNTIME_CLASSTYPE_CODE(SpeechFlex, SoundFlex);

SpeechFlex::SpeechFlex(IDataSource* ds) : SoundFlex(ds)
{
	uint32 size = getRawSize(0);
	uint8* buf = getRawObject(0);

	Pentagram::istring strings(reinterpret_cast<char*>(buf), size);
	std::vector<Pentagram::istring> s;
	Pentagram::SplitString(strings, 0, s);

	for (unsigned int i = 0; i < s.size(); ++i) {
		Pentagram::TabsToSpaces(s[i], 1);
		Pentagram::TrimSpaces(s[i]);

//		pout << "Found string: \"" << s[i] << "\"" << std::endl;

		phrases.push_back(s[i]);
	}

	delete [] buf;

}

SpeechFlex::~SpeechFlex(void)
{
}

int	SpeechFlex::getIndexForPhrase(const std::string &phrase,
								  uint32 start, uint32& end) const
{
	std::vector<Pentagram::istring>::const_iterator it;
	int i = 1;

	std::string text = phrase.substr(start);
	Pentagram::TabsToSpaces(text, 1);

	std::string::size_type pos1 = text.find_first_not_of(' ');
	if (pos1 == std::string::npos) return 0;

	std::string::size_type pos2 = text.find_last_not_of(' ');
	text = text.substr(pos1, pos2-pos1+1);

//	pout << "Looking for string: \"" << text << "\"" << std::endl;

	for(it = phrases.begin(); it != phrases.end(); ++it)
	{
		if (text.find(*it) == 0) {
//			pout << "Found: " << i << std::endl;
			end = (*it).size() + start + pos1;
			if (end >= start + pos2)
				end = phrase.size();
			return i;
		}
		i++;
	}

//	pout << "Not found" << std::endl;

	return 0;
}

uint32 SpeechFlex::getSpeechLength(const std::string &phrase)
{
	uint32 start = 0, end = 0;
	uint32 length = 0;

	while (end < phrase.size()) {
		start = end;
		int index = getIndexForPhrase(phrase, start, end);
		if (!index) break;

		Pentagram::AudioSample* sample = getSample(index);
		if (!sample) break;

		uint32 samples = sample->getLength();
		uint32 rate = sample->getRate();
		bool stereo = sample->isStereo();
		if (stereo) rate *= 2;

		length += (samples * 1000) / rate;
		length += 33; // one engine frame of overhead between speech samples
	}

	return length;
}

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
#ifndef SPEECHFLEX_H_INCLUDED
#define SPEECHFLEX_H_INCLUDED

#include "SoundFlex.h"

#include <vector>

class SpeechFlex : public SoundFlex
{
	std::vector<Pentagram::istring>	phrases;

public:
	// p_dynamic_class stuff 
	ENABLE_RUNTIME_CLASSTYPE();

	SpeechFlex(IDataSource* ds);
	virtual ~SpeechFlex(void);

	int	getIndexForPhrase(const std::string &phrase,
						  uint32 start, uint32& end) const;

	//! get total length (in milliseconds) of speech for a phrase
	uint32 getSpeechLength(const std::string &phrase);
};

#endif

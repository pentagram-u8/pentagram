/*
Copyright (C) 2003-2005 The Pentagram team

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

#include "GameInfo.h"

char GameInfo::getLanguageFileLetter() const
{
	switch (language) {
	case GAMELANG_ENGLISH:
		return 'e';
	case GAMELANG_FRENCH:
		return 'f';
	case GAMELANG_GERMAN:
		return 'g';
	case GAMELANG_SPANISH:
		return 'e';
	default:
		return 0;
	}
}

std::string GameInfo::getGameTitle() const
{
	switch (type) {
	case GAME_U8:
		return "Ultima 8: Pagan";
		break;
	case GAME_REMORSE:
		return "Crusader: No Remorse";
		break;
	case GAME_REGRET:
		return "Crusader: No Regret";
	default:
		return "";
	} 
}

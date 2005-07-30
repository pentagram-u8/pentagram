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

GameInfo::GameInfo()
	: type(GAME_UNKNOWN), version(0), language(GAMELANG_UNKNOWN)
{
	for (int i = 0; i < 16; ++i)
		md5[i] = 0;
}

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
		break;
	case GAME_PENTAGRAM_MENU:
		return "Pentagram Menu";
		break;
	default:
		return "";
	} 
}

std::string GameInfo::getPrintDetails() const
{
	std::string ret;
	switch (type) {
	case GAME_U8:
		ret = "Ultima 8: Pagan, ";
		break;
	case GAME_REMORSE:
		ret = "Crusader: No Remorse, ";
		break;
	case GAME_REGRET:
		ret = "Crusader: No Regret, ";
		break;
	case GAME_PENTAGRAM_MENU:
		ret = "Pentagram Menu, ";
		break;
	default:
		ret = "Unknown, ";
		break;
	}

	switch(language) {
	case GAMELANG_ENGLISH:
		ret += "English";
		break;
	case GAMELANG_FRENCH:
		ret += "French";
		break;
	case GAMELANG_GERMAN:
		ret += "German";
		break;
	case GAMELANG_SPANISH:
		ret += "Spanish";
		break;
	default:
		ret += "Unknown";
		break;
	}

	return ret;
}


std::string GameInfo::getPrintableMD5() const
{
	std::string ret;

	char buf[33];
	for (int i = 0; i < 16; ++i) {
		sprintf(buf+2*i, "%02x", md5[i]);
	}

	ret = buf;

	return ret;
}

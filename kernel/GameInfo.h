/*
Copyright (C) 2003 The Pentagram team

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

#ifndef GAMEINFO_H
#define GAMEINFO_H

//! GameInfo contains detailed information about the game
struct GameInfo {
	enum GameType {
		GAME_UNKNOWN = 0,
		GAME_U8
	} type;

	//! version number, encoded as 100*major + minor
	//! so, 2.12 becomes 212
	//! 0 = unknown
	int version;

	enum GameLanguage {
		LANG_UNKNOWN = 0,
		LANG_ENGLISH,
		LANG_FRENCH,
		LANG_GERMAN,
		LANG_SPANISH
	} language;

	GameInfo() : type(GAME_UNKNOWN), version(0), language(LANG_UNKNOWN) { }
};

#endif

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

#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <vector>

class Flex;
class MainShapeFlex;
class ShapeFlex;
class Usecode;
class Glob;

class GameData
{
public:
	GameData();
	~GameData();

	static GameData* get_instance() { return gamedata; }

	void loadU8Data(); // probably only temporary

	Usecode* getMainUsecode() const { return mainusecode; }
	MainShapeFlex* getMainShapes() const { return mainshapes; }
	Flex* getFixed() const { return fixed; }
	Glob* getGlob(uint32 glob) const;
	ShapeFlex* getFonts() const { return fonts; }

private:
	Flex* fixed;
	MainShapeFlex* mainshapes;
	Usecode* mainusecode;
	std::vector<Glob*> globs;
	ShapeFlex* fonts;

	static GameData* gamedata;
};


#endif

/*
Copyright (C) 2004 The Pentagram team

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

#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <vector>
#include <map>

namespace Pentagram { class Font; }
class IDataSource;

#ifdef USE_SDLTTF
class TTFont;
#include <SDL_ttf.h>
#endif

class FontManager
{
public:
	FontManager();
	~FontManager();

	static FontManager* get_instance() { return fontmanager; }

	//! get a Font by fontnum
	//! \param fontnum the number of the font
	//! \param allowOverride if true, allow an override font to be used
	Pentagram::Font* getFont(unsigned int fontnum, bool allowOverride=false);

	//! open a TTF and assign it an alias.
	//! \param name the name to assign to the TTF
	//! \param ds the datasource to load the TTF from. Will be deleted.
	//! \param pointsize pointsize of the font to use
	bool openTTF(Pentagram::istring name, IDataSource* ds, int pointsize);

	//! override a font with a TTF
	//! \param fontnum the font to override
	//! \param ttf the alias of the TTF to override it with
	//! \param rgb the color to use for the font
	bool addTTFOverride(unsigned int fontnum, Pentagram::istring ttf,
						uint32 rgb);

private:

	std::vector<Pentagram::Font*> overrides;

#ifdef USE_SDLTTF
	std::map<Pentagram::istring, TTF_Font*> ttfs;
#endif

	static FontManager* fontmanager;
};


#endif

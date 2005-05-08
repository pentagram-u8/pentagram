/*
Copyright (C) 2004-2005 The Pentagram team

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

#if defined(HAVE_SDL_SDL_TTF_H)
#include <SDL/SDL_ttf.h>
#else
#include "SDL_ttf.h"
#endif

#include "FontManager.h"

#include "Font.h"
#include "GameData.h"
#include "ShapeFont.h"
#include "FontShapeArchive.h"
#include "IDataSource.h"
#include "FileSystem.h"
#include "TTFont.h"

FontManager* FontManager::fontmanager = 0;

FontManager::FontManager()
{
	assert(fontmanager == 0);
	fontmanager = this;

	TTF_Init();
}

FontManager::~FontManager()
{
	for (unsigned int i = 0; i < overrides.size(); ++i)
		delete overrides[i];
	overrides.clear();

	for (unsigned int i = 0; i < ttfonts.size(); ++i)
		delete ttfonts[i];
	ttfonts.clear();

	std::map<TTFId, TTF_Font*>::iterator iter;
	for (iter = ttf_fonts.begin(); iter != ttf_fonts.end(); ++iter)
		TTF_CloseFont(iter->second);
	ttf_fonts.clear();

	TTF_Quit();

	assert(fontmanager == this);
	fontmanager = 0;
}

Pentagram::Font* FontManager::getGameFont(unsigned int fontnum,
										  bool allowOverride)
{
	if (allowOverride && fontnum < overrides.size() && overrides[fontnum])
		return overrides[fontnum];

	return GameData::get_instance()->getFonts()->getFont(fontnum);
}

Pentagram::Font* FontManager::getTTFont(unsigned int fontnum)
{
	if (fontnum >= ttfonts.size())
		return 0;
	return ttfonts[fontnum];
}


TTF_Font* FontManager::getTTF_Font(std::string filename, int pointsize)
{
	TTFId id;
	id.filename = filename;
	id.pointsize = pointsize;

	std::map<TTFId, TTF_Font*>::iterator iter;
	iter = ttf_fonts.find(id);

	if (iter != ttf_fonts.end())
		return iter->second;

	IDataSource* fontids;
	fontids = FileSystem::get_instance()->ReadFile("@data/" + filename);
	if (!fontids) {
		perr << "Failed to open TTF: @data/" << filename << std::endl;
		return 0;
	}

	// open font using SDL_RWops.
	// Note: The RWops and IDataSource will be deleted by the TTF_Font
	TTF_Font* font = TTF_OpenFontRW(fontids->getRWops(), 1, pointsize);

	if (!font) {
		perr << "Failed to open TTF: @data/" << filename
			 << ": " << TTF_GetError() << std::endl;
		return 0;
	}

	ttf_fonts[id] = font;

#ifdef DEBUG
	pout << "Opened TTF: @data/" << filename << "." << std::endl;
#endif

	return font;
}

bool FontManager::addTTFOverride(unsigned int fontnum, std::string filename,
								 int pointsize, uint32 rgb, int bordersize)
{
	TTF_Font* f = getTTF_Font(filename, pointsize);
	if (!f)
		return false;

	TTFont* font = new TTFont(f, rgb, bordersize);

	if (fontnum >= overrides.size())
		overrides.resize(fontnum+1);

	if (overrides[fontnum])
		delete overrides[fontnum];

	overrides[fontnum] = font;

#ifdef DEBUG
	pout << "Added TTF override for font " << fontnum << std::endl;
#endif

	return true;
}


bool FontManager::loadTTFont(unsigned int fontnum, std::string filename,
							 int pointsize, uint32 rgb, int bordersize)
{
	TTF_Font* f = getTTF_Font(filename, pointsize);
	if (!f)
		return false;

	TTFont* font = new TTFont(f, rgb, bordersize);

	if (fontnum >= ttfonts.size())
		ttfonts.resize(fontnum+1);

	if (ttfonts[fontnum])
		delete ttfonts[fontnum];

	ttfonts[fontnum] = font;

	return true;
}


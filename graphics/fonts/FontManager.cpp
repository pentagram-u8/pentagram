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

#include "pent_include.h"

#include "FontManager.h"

#include "Font.h"
#include "GameData.h"
#include "ShapeFont.h"
#include "FontShapeArchive.h"
#include "IDataSource.h"

#ifdef USE_SDLTTF
#include "TTFont.h"
#endif

FontManager* FontManager::fontmanager = 0;

FontManager::FontManager()
{
	assert(fontmanager == 0);
	fontmanager = this;

#ifdef USE_SDLTTF
	TTF_Init();
#endif
}

FontManager::~FontManager()
{
	for (unsigned int i = 0; i < overrides.size(); ++i)
		delete overrides[i];
	overrides.clear();

#ifdef USE_SDLTTF
	std::map<Pentagram::istring, TTF_Font*>::iterator iter;
	for (iter = ttfs.begin(); iter != ttfs.end(); ++iter)
		TTF_CloseFont(iter->second);
	ttfs.clear();

	TTF_Quit();
#endif

	assert(fontmanager == this);
	fontmanager = 0;
}

Pentagram::Font* FontManager::getFont(unsigned int fontnum,
									  bool allowOverride)
{
	if (allowOverride && fontnum < overrides.size() && overrides[fontnum])
		return overrides[fontnum];

	return GameData::get_instance()->getFonts()->getFont(fontnum);
}

bool FontManager::openTTF(Pentagram::istring name, IDataSource* ds, int size)
{
#ifdef USE_SDLTTF
	assert(ds);

	// open font using SDL_RWops.
	// Note: The RWops and IDataSource will be deleted by the TTF_Font
	TTF_Font* font = TTF_OpenFontRW(ds->getRWops(), 1, size);

	if (!font) {
		perr << "Failed to open TTF " << name << ": " << TTF_GetError()
			 << std::endl;
		return false;
	}

	if (ttfs[name])
		TTF_CloseFont(ttfs[name]);

	ttfs[name] = font;

	pout << "Opened TTF " << name << "." << std::endl;

	return true;

#else
	delete ds;
	return false;
#endif
}

bool FontManager::addTTFOverride(unsigned int fontnum, Pentagram::istring ttf,
								 uint32 rgb, int bordersize)
{
#ifdef USE_SDLTTF

	std::map<Pentagram::istring, TTF_Font*>::iterator iter;
	iter = ttfs.find(ttf);
	if (iter == ttfs.end())
		return false;

	TTFont* font = new TTFont(iter->second, rgb, bordersize);

	if (fontnum >= overrides.size())
		overrides.resize(fontnum+1);

	if (overrides[fontnum])
		delete overrides[fontnum];

	overrides[fontnum] = font;

//	pout << "Added TTF override for font " << fontnum << std::endl;

	return true;

#else
	return false;
#endif
}

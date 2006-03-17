/*
Copyright (C) 2004-2006 The Pentagram team

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


// This is TTF_Font struct from SDL_ttf
typedef struct _TTF_Font TTF_Font;
class IDataSource;

class TTFont;


class FontManager
{
public:
	FontManager(bool ttf_antialiasing);
	~FontManager();

	static FontManager* get_instance() { return fontmanager; }

	//! get a Font by fontnum (for game fonts)
	//! \param fontnum the number of the font
	//! \param allowOverride if true, allow an override font to be used
	Pentagram::Font* getGameFont(unsigned int fontnum,
								 bool allowOverride=false);

	//! get a TTF font (for non-game fonts)
	Pentagram::Font* getTTFont(unsigned int ttfnum);

	//! override a game font with a TTF.
	//! \param fontnum the font to override
	//! \param filename the filename of the TTF
	//! \param pointsize the pointsize to use
	//! \param rgb the color to use for the font
	//! \param bordersize the size of the black border to add
	//! \param SJIS true for a Japanese game font
	bool addTTFOverride(unsigned int fontnum, std::string filename,
						int pointsize, uint32 rgb, int bordersize,
						bool SJIS=false);

	//! override a game font with a Japanese font.
	//! \param fontnum the font to override
	//! \param jpfont the fontnum of the Japanese font to use
	//! \param rgb the color to use
	bool addJPOverride(unsigned int fontnum, unsigned int jpfont, uint32 rgb);

	//! load a TTF (for non-game fonts)
	bool loadTTFont(unsigned int ttfnum, std::string filename,
					int pointsize, uint32 rgb, int bordersize);

	// Reset the game fonts
	void resetGameFonts();
private:

	struct TTFId {
		std::string filename;
		int pointsize;
		bool operator<(const TTFId& other) const {
			return (pointsize < other.pointsize ||
					(pointsize == other.pointsize &&
					 filename < other.filename));
		}
	};
	std::map<TTFId, TTF_Font*> ttf_fonts;
	bool ttf_antialiasing;

	//! Get a (possibly cached) TTF_Font structure for filename/pointsize,
	//! loading it if necessary.
	TTF_Font* getTTF_Font(std::string filename, int pointsize);

	//! Override fontnum with override
	void setOverride(unsigned int fontnum, Pentagram::Font* override);

	std::vector<Pentagram::Font*> overrides;

	std::vector<Pentagram::Font*> ttfonts;

	static FontManager* fontmanager;
};


#endif

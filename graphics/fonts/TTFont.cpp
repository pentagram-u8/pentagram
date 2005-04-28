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

#include "SDL_ttf.h"

#include "RenderSurface.h"
#include "TTFont.h"
#include "TTFRenderedText.h"
#include "Texture.h"
#include "IDataSource.h"
#include "encoding.h"

#include <iomanip>

DEFINE_RUNTIME_CLASSTYPE_CODE(TTFont,Pentagram::Font);


TTFont::TTFont(TTF_Font* font, uint32 rgb_, int bordersize_)
{
	ttf_font = font;

//	rgb = PACK_RGB8( (rgb_>>16)&0xFF , (rgb_>>8)&0xFF , rgb_&0xFF );
	// This should be performed by PACK_RGB8, but it is not initialized at this point.
	rgb = (rgb_>>16)&0xFF | ((rgb_>>8)&0xFF)<<8 | (rgb_&0xFF)<<16;
	bordersize = bordersize_;
}

TTFont::~TTFont()
{

}

int TTFont::getHeight()
{
	return TTF_FontHeight(ttf_font) + 2*bordersize; // constant (border)
}

int TTFont::getBaseline()
{
	return TTF_FontAscent(ttf_font);
}

int TTFont::getBaselineSkip()
{
	return TTF_FontLineSkip(ttf_font);
}

void TTFont::getStringSize(const std::string& text, int& width, int& height)
{
	// convert to unicode
#if 0
	uint16* unicodetext = new uint16[text.size()+1];
	for (unsigned int i = 0; i < text.size(); ++i) {
		unicodetext[i] = encoding[(unsigned char)(text[i])];
	}
	unicodetext[text.size()] = 0;
#else
	const char* t = text.c_str();
	uint16* unicodetext = new uint16[strlen(t)+1];
	for (unsigned int i = 0; i < strlen(t); ++i) {
		unicodetext[i] = encoding[static_cast<unsigned char>(t[i])];
	}
	unicodetext[strlen(t)] = 0;
#endif

	TTF_SizeUNICODE(ttf_font, unicodetext, &width, &height);
	delete[] unicodetext;
#if 0
	pout << "StringSize: " << width << "," << height << ": " << text << std::endl;
	pout << "height: " << TTF_FontHeight(ttf_font) << std::endl;
	pout << "lineskip: " << TTF_FontLineSkip(ttf_font) << std::endl;
	pout << "ascent: " << TTF_FontAscent(ttf_font) << std::endl;
	pout << "descent: " << TTF_FontDescent(ttf_font) << std::endl;
#endif

	width += 2*bordersize;
	height += 2*bordersize;
}

RenderedText* TTFont::renderText(const std::string& text,
								 unsigned int& remaining,
								 int width, int height,
								 TextAlign align, bool u8specials,
								 std::string::size_type cursor)
{
	int resultwidth, resultheight;
	std::list<PositionedText> lines = typesetText(text, remaining,
												  width, height,
												  align, u8specials,
												  resultwidth, resultheight,
												  cursor);

	SDL_Color white = { 0xFF , 0xFF , 0xFF, 0 };

	// create 32bit RGBA texture buffer
	uint32* buf = new uint32[resultwidth*resultheight];
	memset(buf, 0, 4*resultwidth*resultheight);

	Texture* texture = new Texture;
	texture->buffer = buf;
	texture->width = resultwidth;
	texture->height = resultheight;

#if 0
	pout << "Total size: " << resultwidth << "," << resultheight << std::endl;
#endif

	std::list<PositionedText>::iterator iter;
	for (iter = lines.begin(); iter != lines.end(); ++iter)
	{
		// convert to unicode
		const char* t = iter->text.c_str();
		uint16* unicodetext = new uint16[strlen(t)+1];
//		perr << "Text: " << t << std::endl;
//		perr << "Hex: " << std::hex;
		for (unsigned int i = 0; i < strlen(t); ++i) {
//			perr << std::setw(2) << std::setfill('0') << (unsigned int)(t[i]) << " ";
			unicodetext[i] = encoding[static_cast<unsigned char>(t[i])];
		}
//		perr << std::dec << std::endl;
		unicodetext[strlen(t)] = 0;

		// let SDL_ttf render the text
		SDL_Surface* textsurf = TTF_RenderUNICODE_Solid(ttf_font,
														unicodetext,
														white);
		if (textsurf) {
			SDL_LockSurface(textsurf);

#if 0
			pout << iter->dims.w << "," << iter->dims.h << " vs. "
				 << textsurf->w << "," << textsurf->h << ": " << iter->text
				 << std::endl;
#endif

			// render the text surface into our texture buffer
			for (int y = 0; y < textsurf->h; y++) {
				uint8* surfrow = static_cast<uint8*>(textsurf->pixels) + y * textsurf->pitch;
				// CHECKME: bordersize!
				uint32* bufrow = buf + (iter->dims.y+y+bordersize)*resultwidth;
				for (int x = 0; x < textsurf->w; x++) {
					if (surfrow[x] == 1) {
//					bufrow[iter->dims.x+x+1] = 0xFFFFFFFF;
						bufrow[iter->dims.x+x+bordersize] = rgb | 0xFF000000;
						if (bordersize <= 0) continue;
						if (bordersize == 1) {
							// optimize common case
							for (int dx = -1; dx <= 1; dx++) {
								for (int dy = -1; dy <= 1; dy++) {
									if (x + 1 + iter->dims.x + dx >= 0 &&
										x + 1 + iter->dims.x + dx < resultwidth &&
										y + 1 + dy >= 0 && y + 1 + dy < resultheight)
									{
										if (buf[(y+iter->dims.y+dy+1)*resultwidth + x+1+iter->dims.x+dx] == 0) {
											buf[(y+iter->dims.y+dy+1)*resultwidth + x+1+iter->dims.x+dx] = 0xFF000000;
										}
									}
								}
							}
							continue;
						}
						for (int dx = -bordersize; dx <= bordersize; dx++) {
							for (int dy = -bordersize; dy <= bordersize; dy++) {
								if (x + bordersize + iter->dims.x + dx >= 0 &&
									x + bordersize + iter->dims.x + dx < resultwidth &&
									y + bordersize + dy >= 0 && y + bordersize + dy < resultheight)
								{
									if (buf[(y+iter->dims.y+dy+bordersize)*resultwidth + x+bordersize+iter->dims.x+dx] == 0) {
										buf[(y+iter->dims.y+dy+bordersize)*resultwidth + x+bordersize+iter->dims.x+dx] = 0xFF000000;
									}
								}
							}
						}
					}
				}
			}

			SDL_UnlockSurface(textsurf);
			SDL_FreeSurface(textsurf);
		}

		if (iter->cursor != std::string::npos) {
			int w, h;
			assert(iter->cursor <= strlen(t));
			unicodetext[iter->cursor] = 0;
			TTF_SizeUNICODE(ttf_font, unicodetext,&w,&h);
			for (int y = 0; y < iter->dims.h; y++) {
				uint32* bufrow = buf + (iter->dims.y+y)*resultwidth;
				bufrow[iter->dims.x+w+bordersize] = 0xFF000000;
//				if (bordersize > 0)
//					bufrow[iter->dims.x+w+bordersize-1] = 0xFF000000;
			}
		}

		delete[] unicodetext;

	}

	return new TTFRenderedText(texture, resultwidth, resultheight,
							   getBaselineSkip() - getHeight(), this);
}


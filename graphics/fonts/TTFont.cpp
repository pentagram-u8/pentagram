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

#ifdef USE_SDLTTF

#include "SDL_ttf.h"

#include "RenderSurface.h"
#include "TTFont.h"
#include "TTFRenderedText.h"
#include "Texture.h"
#include "IDataSource.h"

#include <iomanip>

DEFINE_RUNTIME_CLASSTYPE_CODE(TTFont,Pentagram::Font);


TTFont::TTFont(IDataSource* font, uint32 rgb_, int pointsize)
{
	// open font using SDL_RWops.
	// Note: The RWops and IDataSource will be deleted by the TTF_Font
	ttf_font = TTF_OpenFontRW(font->getRWops(), 1, pointsize);
//	rgb = PACK_RGB8( (rgb_>>16)&0xFF , (rgb_>>8)&0xFF , rgb_&0xFF );
	// This should be performed by PACK_RGB8, but it is not initialized at this point.
	rgb = (rgb_>>16)&0xFF | ((rgb_>>8)&0xFF)<<8 | (rgb_&0xFF)<<16;
}

TTFont::~TTFont()
{
	if (ttf_font) {
		TTF_CloseFont(ttf_font);
		ttf_font = 0;
	}
}

int TTFont::getHeight()
{
	return TTF_FontHeight(ttf_font) + 2; // constant (border)
}

int TTFont::getBaseline()
{
	return TTF_FontAscent(ttf_font);
}

int TTFont::getBaselineSkip()
{
	return TTF_FontLineSkip(ttf_font);
}

static uint16 encoding[] = {
0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
0x2022,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
0x00C7,0x00FC,0x00E9,0x00E2,0x00E4,0x00E0,0x00E5,0x00E7,
0x00EA,0x00EB,0x00E8,0x00EF,0x00EE,0x00EC,0x00C4,0x00C5,
0x00C9,0x00E6,0x00C6,0x00F4,0x00F6,0x00F2,0x00FB,0x00F9,
0x00FF,0x00D6,0x00DC,0x00F8,0x00A3,0x00D8,0x00D7,0x0192,
0x00E1,0x00ED,0x00F3,0x00FA,0x00F1,0x00D1,0x00AA,0x00BA,
0x00BF,0x00AE,0x00AC,0x00BD,0x00BC,0x00A1,0x00AB,0x00BB,
0x2591,0x2592,0x2593,0x2502,0x2524,0x00C1,0x00C2,0x00C0,
0x00A9,0x2563,0x2551,0x2557,0x255D,0x00A2,0x00A5,0x2510,
0x2514,0x2534,0x252C,0x251C,0x2500,0x253C,0x00E3,0x00C3,
0x255A,0x2554,0x2569,0x2566,0x2560,0x2550,0x256C,0x00A4,
0x00F0,0x00D0,0x00CA,0x00CB,0x00C8,0x0131,0x00CD,0x00CE,
0x00CF,0x2518,0x250C,0x2588,0x2584,0x00A6,0x00CC,0x2580,
0x00D3,0x00DF,0x00D4,0x00D2,0x00F5,0x00D5,0x00B5,0x00FE,
0x00DE,0x00DA,0x00DB,0x00D9,0x00FD,0x00DD,0x00AF,0x00B4,
0x00AD,0x00B1,0x2017,0x00BE,0x00B6,0x00A7,0x00F7,0x00B8,
0x00B0,0x00A8,0x00B7,0x00B9,0x00B3,0x00B2,0x25A0,0x00A0
};


void TTFont::getStringSize(std::string& text, int& width, int& height)
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
		unicodetext[i] = encoding[(unsigned char)(t[i])];
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

	width += 2; // constant (border)
	height += 2;
}

RenderedText* TTFont::renderText(std::string text,
									  unsigned int& remaining,
									  int width, int height,
									  TextAlign align)
{
	int resultwidth, resultheight;
	std::list<PositionedText> lines = typesetText(text, remaining,
												  width, height, align,
												  resultwidth, resultheight);

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
			unicodetext[i] = encoding[(unsigned char)(t[i])];
		}
//		perr << std::dec << std::endl;
		unicodetext[strlen(t)] = 0;

		// let SDL_ttf render the text
		SDL_Surface* textsurf = TTF_RenderUNICODE_Solid(ttf_font,
														unicodetext,
														white);
		delete[] unicodetext;
		if (!textsurf) continue;
		SDL_LockSurface(textsurf);

#if 0
		pout << iter->dims.w << "," << iter->dims.h << " vs. " << textsurf->w
			 << "," << textsurf->h << ": " << iter->text << std::endl;
#endif

		// render the text surface into our texture buffer
		for (int y = 0; y < textsurf->h; y++) {
			uint8* surfrow = (uint8*)(textsurf->pixels) + y * textsurf->pitch;
			uint32* bufrow = buf + (iter->dims.y+y+1)*resultwidth;
			for (int x = 0; x < textsurf->w; x++) {
				if (surfrow[x] == 1) {
//					bufrow[iter->dims.x+x+1] = 0xFFFFFFFF;
					bufrow[iter->dims.x+x+1] = rgb | 0xFF000000;
					for (int dx = -1; dx < 2; dx++) {
						for (int dy = -1; dy < 2; dy++) {
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
				}
			}
		}

		SDL_UnlockSurface(textsurf);
		SDL_FreeSurface(textsurf);
	}

	return new TTFRenderedText(texture, resultwidth, resultheight, this);
}


#endif

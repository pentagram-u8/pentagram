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

#include "TTFont.h"
#include "TTFRenderedText.h"
#include "Texture.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(TTFont,Pentagram::Font);


TTFont::TTFont(std::string fontfile, int pointsize)
{
	// FIXME: FileSystem-ize this
	ttf_font = TTF_OpenFont(fontfile.c_str(), pointsize);
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

void TTFont::getStringSize(std::string& text, int& width, int& height)
{
	TTF_SizeText(ttf_font, text.c_str(), &width, &height);

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

	SDL_Color white = { 0xFF, 0xFF, 0xFF, 0 };

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
		// let SDL_ttf render the text
		SDL_Surface* textsurf = TTF_RenderText_Solid(ttf_font,
													 iter->text.c_str(),
													 white);
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
					bufrow[iter->dims.x+x+1] = 0xFFFFFFFF;
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

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

#include "pent_include.h"

#include "Font.h"

namespace Pentagram {

DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Font)

Font::Font()
{

}


Font::~Font()
{
}


void Font::getTextSize(const std::string& text,
					   int& resultwidth, int& resultheight,
					   unsigned int& remaining,
					   int width, int height, TextAlign align,
					   bool u8specials)
{
	std::list<PositionedText> tmp;
	tmp = typesetText<Traits>(this, text, remaining,
							  width, height, align, u8specials,
							  resultwidth, resultheight);
}

}




template<class T>
static void findWordEnd(const std::string& text,
						std::string::const_iterator& iter, bool u8specials)
{
	while (iter != text.end()) {
		if (T::isSpace(iter, u8specials)) return; 
		T::advance(iter);
	}
}

template<class T>
static void passSpace(const std::string& text,
					  std::string::const_iterator& iter, bool u8specials)
{
	while (iter != text.end()) {
		if (!T::isSpace(iter, u8specials)) return;
		T::advance(iter);
	}
	return;
}




/*
  Special characters in U8:

@ = bullet for conversation options
~ = line break
% = tab
* = line break on graves and plaques, possibly page break in books
CHECKME: any others? (page breaks for books?)

*/

template<class T>
std::list<PositionedText> typesetText(Pentagram::Font* font,
									  const std::string& text,
									  unsigned int& remaining,
									  int width, int height,
									  Pentagram::Font::TextAlign align,
									  bool u8specials,
									  int& resultwidth, int& resultheight,
									  std::string::size_type cursor)
{
#if 0
	pout << "typeset (" << width << "," << height << ") : "
		 << text << std::endl;
#endif

	// be optimistic and assume everything will fit
	remaining = text.size();

	std::string curline;

	int totalwidth = 0;
	int totalheight = 0;

	std::list<PositionedText> lines;
	PositionedText line;

	std::string::const_iterator iter = text.begin();
	std::string::const_iterator cursoriter = text.begin();
	if (cursor != std::string::npos) cursoriter += cursor;
	std::string::const_iterator curlinestart = text.begin();

	bool breakhere = false;
	while (true)
	{
		if (iter == text.end() || breakhere || T::isBreak(iter, u8specials))
		{
			// break here
			int stringwidth = 0, stringheight = 0;
			font->getStringSize(curline, stringwidth, stringheight);
			line.dims.x = 0; line.dims.y = totalheight;
			line.dims.w = stringwidth;
			line.dims.h = stringheight;
			line.text = curline;
			line.cursor = std::string::npos;
			if (cursor != std::string::npos && cursoriter >= curlinestart &&
				(cursoriter < iter || (!breakhere && cursoriter == iter)))
			{
				line.cursor = cursoriter - curlinestart;
				if (line.dims.w == 0) {
					stringwidth = line.dims.w = 2;
				}
			}
			lines.push_back(line);

			if (stringwidth > totalwidth) totalwidth = stringwidth;
			totalheight += font->getBaselineSkip();

			curline = "";

			if (iter == text.end())
				break; // done

			if (breakhere) {
				breakhere = false;
				curlinestart = iter;
			} else {
				T::advance(iter);
				curlinestart = iter;
			}

			if (height != 0 && totalheight + font->getHeight() > height) {
				// next line won't fit
				remaining = curlinestart - text.begin();
				break;
			}

		} else {

			// see if next word still fits on the current line
			std::string::const_iterator nextword = iter;
			passSpace<T>(text, nextword, u8specials);

			// process spaces
			bool foundLF = false;
			std::string spaces;
			for (; iter < nextword; T::advance(iter)) {
				if (T::isBreak(iter, u8specials)) {
					foundLF = true;
					break;
				} else if (T::isTab(iter, u8specials)) {
					spaces.append("    ");
				} else if (!curline.empty()) {
					spaces.append(" ");
				}
			}
			if (foundLF) continue;

			// process word
			std::string::const_iterator endofnextword = iter;
			findWordEnd<T>(text, endofnextword, u8specials);
			int stringwidth = 0, stringheight = 0;
			std::string newline = curline + spaces +
				text.substr(nextword-text.begin(),endofnextword-nextword);
			font->getStringSize(newline, stringwidth, stringheight);

			// if not, break line before this word
			if (width != 0 && stringwidth > width) {
				if (!curline.empty()) {
					iter = nextword;
				} else {
					// word is longer than the line; have to break in mid-word
					// FIXME: this is rather inefficient; binary search?
					// (OTOH, it should rarely be used, so no priority)
					// FIXME: can't break Japanese lines after each character.
					// http://www.wesnoth.org/wiki/JapaneseTranslation has a
					// nice list under 'Word-Wrapping'.
					iter = nextword;
					std::string::const_iterator saveiter;
					newline.clear();
					do {
						curline = newline;
						saveiter = iter;

						// try next character
						T::advance(iter);
						newline = text.substr(nextword-text.begin(),
											  iter-nextword);
						font->getStringSize(newline, stringwidth,stringheight);
					} while (stringwidth <= width);
					iter = saveiter;
				}
				breakhere = true;
				continue;
			} else {
				// copy next word into curline
				curline = newline;
				iter = endofnextword;
			}
		}
	}

	if (lines.size() == 1 && align == Pentagram::Font::TEXT_LEFT) {
		// only one line, so use the actual text width
	    width = totalwidth;
	}

	if (width != 0) totalwidth = width;

	// adjust total height
	totalheight -= font->getBaselineSkip();
	totalheight += font->getHeight();

	// fixup x coordinates of lines
	std::list<PositionedText>::iterator lineiter;
	for (lineiter = lines.begin(); lineiter != lines.end(); ++lineiter) {
		switch (align) {
		case Pentagram::Font::TEXT_LEFT:
			break;
		case Pentagram::Font::TEXT_RIGHT:
			lineiter->dims.x = totalwidth - lineiter->dims.w;
			break;
		case Pentagram::Font::TEXT_CENTER:
			lineiter->dims.x = (totalwidth - lineiter->dims.w) / 2;
			break;
		}
#if 0
		pout << lineiter->dims.x << "," << lineiter->dims.y << " "
			 << lineiter->dims.w << "," << lineiter->dims.h << ": "
			 << lineiter->text << std::endl;
#endif
	}

	resultwidth = totalwidth;
	resultheight = totalheight;

	return lines;
}


// explicit instantiations
template
std::list<PositionedText> typesetText<Pentagram::Font::Traits>
(Pentagram::Font* font, const std::string& text,
 unsigned int& remaining, int width, int height,
 Pentagram::Font::TextAlign align, bool u8specials,
 int& resultwidth, int& resultheight, std::string::size_type cursor);

template
std::list<PositionedText> typesetText<Pentagram::Font::SJISTraits>
(Pentagram::Font* font, const std::string& text,
 unsigned int& remaining, int width, int height,
 Pentagram::Font::TextAlign align, bool u8specials,
 int& resultwidth, int& resultheight, std::string::size_type cursor);

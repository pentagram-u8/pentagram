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

#include "Font.h"

namespace Pentagram {

DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Font)

Font::Font()
{

}


Font::~Font()
{

}

static inline bool isSpace(char c)
{
	return (c == ' ' || c == '\t' || c == '%' || c == '\n' || c == '\r' ||
			c == '~' || c == '*');			
}

static inline bool isTab(char c)
{
	return (c == '\t' || c == '%');
}

static inline bool isBreak(char c)
{
	return (c == '\n' || c == '~' || c == '*');
}


static unsigned int findWordEnd(std::string& text, unsigned int start)
{
	unsigned int index = start;
	while (index < text.size()) {
		if (isSpace(text[index]))
			return index; 

		index++;
	}
	return index;
}

static unsigned int passSpace(std::string& text, unsigned int start)
{
	unsigned int index = start;
	while (index < text.size()) {
		if (!isSpace(text[index]))
			return index; 

		index++;
	}
	return index;
}

void Font::getTextSize(std::string text,
					   int& resultwidth, int& resultheight,
					   unsigned int& remaining,
					   int width, int height,
					   TextAlign align)
{
	std::list<PositionedText> tmp = typesetText(text, remaining,
												width, height, align,
												resultwidth, resultheight);
}

/*
  Special characters in U8:

@ = bullet for conversation options
~ = line break
% = tab
* = line break on graves and plaques, possibly page break in books
CHECKME: any others? (page breaks for books?)

*/

std::list<PositionedText> Font::typesetText(std::string& text,
											unsigned int& remaining,
											int width, int height,
											TextAlign align,
											int& resultwidth,
											int& resultheight)
{
#if 0
	pout << "typeset (" << width << "," << height << ") : "
		 << text << std::endl;
#endif

	// be optimistic and assume everything will fit
	remaining = text.size();

	unsigned int curlinestart = 0;
	std::string curline;

	int totalwidth = 0;
	int totalheight = 0;

	std::list<PositionedText> lines;
	PositionedText line;

	unsigned int i = 0;

	bool breakhere = false;
	while (true)
	{
		if (i >= text.size() || breakhere || isBreak(text[i]))
		{
			// break here
			int stringwidth = 0, stringheight = 0;
			getStringSize(curline, stringwidth, stringheight);
			line.dims.x = 0; line.dims.y = totalheight;
			line.dims.w = stringwidth;
			line.dims.h = stringheight;
			line.text = curline;
			lines.push_back(line);

			if (stringwidth > totalwidth) totalwidth = stringwidth;
			totalheight += getBaselineSkip();

			curline = "";

			if (i >= text.size())
				break; // done

			if (breakhere) {
				breakhere = false;
				curlinestart = i;
			} else {
				i = curlinestart = i+1; // FIXME: CR/LF?
			}

			if (height != 0 && totalheight + getHeight() > height) {
				// next line won't fit
				remaining = curlinestart;
				break;
			}

		} else {

			// see if next word still fits on the current line
			unsigned int nextword = passSpace(text, i);

			// process spaces
			bool foundLF = false;
			std::string spaces;
			for (; i < nextword; ++i) {
				if (isBreak(text[i])) {
					foundLF = true;
					break;
				} else if (isTab(text[i])) {
					spaces.append("    ");
				} else if (!curline.empty()){
					spaces.append(" ");
				}
			}
			if (foundLF) continue;

			// process word
			unsigned int endofnextword = findWordEnd(text, nextword);
			int stringwidth = 0, stringheight = 0;
			std::string newline = curline + spaces +
				text.substr(nextword,endofnextword-nextword);
			getStringSize(newline, stringwidth, stringheight);

			// if not, break line before this word
			if (width != 0 && stringwidth > width && !curline.empty()) {
				breakhere = true;
				i = nextword;
				continue;
			} else {
				// copy next word into curline
				curline = newline;
				i = endofnextword;
			}
		}
	}

	if (lines.size() == 1 && align == TEXT_LEFT) {
		// only one line, so use the actual text width
	    width = totalwidth;
	}

	if (width != 0) totalwidth = width;

	// adjust total height
	totalheight -= getBaselineSkip();
	totalheight += getHeight();

	// fixup x coordinates of lines
	std::list<PositionedText>::iterator iter;
	for (iter = lines.begin(); iter != lines.end(); ++iter) {
		switch (align) {
		case TEXT_LEFT:
			break;
		case TEXT_RIGHT:
			iter->dims.x = totalwidth - iter->dims.w;
			break;
		case TEXT_CENTER:
			iter->dims.x = (totalwidth - iter->dims.w) / 2;
			break;
		}
#if 0
		pout << iter->dims.x << "," << iter->dims.y << " "
			 << iter->dims.w << "," << iter->dims.h << ": "
			 << iter->text << std::endl;
#endif
	}

	resultwidth = totalwidth;
	resultheight = totalheight;

	return lines;
}



}



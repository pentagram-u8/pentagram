/*
 *  Copyright (C) 2006 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef PNGWRITER_H
#define PNGWRITER_H

class ODataSource;
struct Texture;

class PNGWriter {
public:
	PNGWriter(ODataSource* ods);
	~PNGWriter();

	bool init(uint32 width, uint32 height, const std::string& comment);
	bool writeRows(unsigned int nrows, Texture* img);
	bool finish();

private:
	ODataSource* ds;
	uint32 width;
	void* png;
	void* info;
};



#endif

/*
Copyright (C) 2003 The Pentagram team

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

#ifndef BITSET_H
#define BITSET_H

class IDataSource;
class ODataSource;

class BitSet {
public:
	BitSet();
	BitSet(unsigned int size);
	~BitSet();

	//! set the size. The old value is cleared
	//! \param size the new size (in bits)
	void setSize(unsigned int size);

	//! get a value
	//! \param pos zero-based position (in bits)
	//! \param n number of bits (no greater than 32)
	//! \return the value these bits represent
	uint32 getBits(unsigned int pos, unsigned int n);

	//! set a value
	//! \param pos zero-based position (in bits)
	//! \param n number of bits (no greater than 32)
	//! \param bits the value to set
	void setBits(unsigned int pos, unsigned int n, uint32 bits);

	void save(ODataSource* ods);
	bool load(IDataSource* ids, uint32 version);

private:
	unsigned int size;
	unsigned int bytes;
	uint8* data;
};


#endif

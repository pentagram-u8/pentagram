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

#include "pent_include.h"

#include "ShapeInfo.h"

bool ShapeInfo::getTypeFlag(int typeflag)
{
	// This is not nice. The Typeflags in U8 were stored in an 8 byte array
	// and they could access them with a number from 0 to 63
	// Problem: We don't don't store in an 8 byte array so we can't access 
	// with a number from 0 to 63

	// So what we do is split the flag up into the bits

	if (typeflag <= 11)			// flags		Byte 0, 1:0-3	Bits  0-11
	{
		return (flags >> typeflag) & 1;
	}
	else if (typeflag <= 15)	// family		Byte 1:4-7		Bits 11-15
	{
		return (family >> (typeflag-12)) & 1;
	}
	else if (typeflag <= 19)	// equiptype	Byte 2:0-3		Bits 16-19
	{
		return (equiptype >> (typeflag-16)) & 1;
	}
	else if (typeflag <= 23)	// x			Byte 2:4-7		Bits 20-23
	{
		return (x >> (typeflag-20)) & 1;
	}
	else if (typeflag <= 27)	// y			Byte 3:0-3		Bits 24-27
	{
		return (y >> (typeflag-24)) & 1;
	}
	else if (typeflag <= 31)	// z			Byte 3:4-7		Bits 28-31
	{
		return (z >> (typeflag-28)) & 1;
	}
	else if (typeflag <= 35)	// animtype		Byte 4:0-3		Bits 32-35
	{
		return (animtype >> (typeflag-32)) & 1;
	}
	else if (typeflag <= 39)	// animdata		Byte 4:4-7		Bits 36-49
	{
		return (animdata >> (typeflag-36)) & 1;
	}
	else if (typeflag <= 43)	// unknown		Byte 5:0-3		Bits 40-43
	{
		return (unknown >> (typeflag-40)) & 1;
	}
	else if (typeflag == 47)	// flags		Byte 5:4-7		Bits 44-47
	{
		return (flags >> (12+typeflag-44)) & 1;
	}
	else if (typeflag <= 55)	// weight		Byte 6			Bits 48-55
	{
		return (weight >> (typeflag-48)) & 1;
	}
	else if (typeflag <= 63)	// volume		Byte 7			Bits 56-63
	{
		return (volume >> (typeflag-56)) & 1;
	}

	return false;
}

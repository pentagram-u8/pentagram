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

#ifndef WORLDPOINT_H_INCLUDED
#define WORLDPOINT_H_INCLUDED

struct WorldPoint
{
	uint8 buf[5];

	uint16 getX() const 
		{ return buf[0] + (buf[1]<<8); }
	uint16 getY() const 
		{ return buf[2] + (buf[3]<<8); }
	uint16 getZ() const 
		{ return buf[4]; }

	void setX(sint32 x) 
		{ buf[0] = static_cast<uint8>(x); buf[1] = static_cast<uint8>(x>>8); }
	void setY(sint32 y) 
		{ buf[2] = static_cast<uint8>(y); buf[3] = static_cast<uint8>(y>>8); }
	void setZ(sint32 z) 
		{ buf[4] = static_cast<uint8>(z); }
};

#endif // WORLDPOINT_H_INCLUDED

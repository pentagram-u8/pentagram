/*
 *	Direction.h - Directions.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2001  The Exult Team
 *  Copyright (C) 2002-2010  The Pentagram Team
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

#ifndef DIRECTION_H
#define DIRECTION_H

/*
 *	Directions:
 */
enum Direction
{
	north = 0,
	northeast = 1,
	east = 2,
	southeast = 3,
	south = 4,
	southwest = 5,
	west = 6,
	northwest = 7
};

/*
 * Tables to map a Direction to x/y deltas
 */
const int x_fact[] = {  0, +1, +1, +1,  0, -1, -1, -1 };
const int y_fact[] = { -1, -1,  0, +1, +1, +1,  0, -1 };

/*
 *	Return the direction for a given slope (0-7).
 *	NOTE:  Assumes cartesian coords, NOT screen coords. (which have y
 *		growing downwards).
 *
 *  NOTE: The returned direction is rotated 45 degrees clockwise! This is
 *  how U8 things should be.
 */

inline Direction Get_direction (int deltay, int deltax)
{
	if (deltax == 0)
		return deltay > 0 ? northwest : southeast;
	int dydx = (1024*deltay)/deltax;// Figure 1024*tan.
	if (dydx >= 0)
		if (deltax > 0)	// Top-right
			return dydx <= 424 ? northeast : dydx <= 2472 ? north
								: northwest;
		else			// Bottom-left.
			return dydx <= 424 ? southwest : dydx <= 2472 ? south
								: southeast;
	else
		if (deltax > 0)	// Bottom-right.
			return dydx >= -424 ? northeast : dydx >= -2472 ? east
								: southeast;
		else			// Top-left
			return dydx >= -424 ? southwest : dydx >= -2472 ? west
								: northwest;
}


inline Direction Get_WorldDirection (int deltay, int deltax)
{
	if (deltax == 0) {
		if (deltay == 0) return northeast; // for better compatibility with U8
		return deltay > 0 ? south : north;
	}
	int dydx = (1024*deltay)/deltax;

	if (dydx >= 0)
		if (deltax > 0) // south-east
			return dydx <= 424 ? east : dydx <= 2472 ? southeast : south;
		else            // north-west
			return dydx <= 424 ? west : dydx <= 2472 ? northwest : north;
	else
		if (deltax > 0) // north-east
			return dydx >= -424 ? east : dydx >= -2472 ? northeast : north;
		else            // south-west
			return dydx >= -424 ? west : dydx >= -2472 ? southwest : south;
}


#endif

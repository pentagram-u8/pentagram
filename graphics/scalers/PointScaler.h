/*
Copyright (C) 2005 The Pentagram Team

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

#ifndef POINTSCALER_H_INCLUDED
#define POINTSCALER_H_INCLUDED

#include "Scaler.h"

namespace Pentagram {

class PointScaler : public Scaler
{
public:
	PointScaler();

	virtual const uint32	ScaleBits();		//< bits for supported integer scaling
	virtual const bool		ScaleArbitrary();	//< supports arbitrary scaling of any degree 

	virtual const char *	ScalerName();		//< Name Of the Scaler (1 word)
	virtual const char *	ScalerDesc();		//< Desciption of the Scaler
	virtual const char *	ScalerCopyright();	//< Scaler Copyright info
};

extern PointScaler point_scaler;

};

#endif

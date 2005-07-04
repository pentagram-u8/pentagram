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

#ifndef _2XSAISCALER_H_INCLUDED
#define _2XSAISCALER_H_INCLUDED

#include "Scaler.h"

namespace Pentagram {

class _2xSaIScaler : public Scaler
{
public:
	_2xSaIScaler();

	virtual const uint32	ScaleBits() const;			//< bits for supported integer scaling
	virtual const bool		ScaleArbitrary() const;		//< supports arbitrary scaling of any degree 

	virtual const char *	ScalerName() const;			//< Name Of the Scaler (1 word)
	virtual const char *	ScalerDesc() const;			//< Desciption of the Scaler
	virtual const char *	ScalerCopyright() const;	//< Scaler Copyright info
};

class Super2xSaIScaler : public Scaler
{
public:
	Super2xSaIScaler();

	virtual const uint32	ScaleBits() const;			//< bits for supported integer scaling
	virtual const bool		ScaleArbitrary() const;		//< supports arbitrary scaling of any degree 

	virtual const char *	ScalerName() const;			//< Name Of the Scaler (1 word)
	virtual const char *	ScalerDesc() const;			//< Desciption of the Scaler
	virtual const char *	ScalerCopyright() const;	//< Scaler Copyright info
};

class SuperEagleScaler : public Scaler
{
public:
	SuperEagleScaler();

	virtual const uint32	ScaleBits() const;			//< bits for supported integer scaling
	virtual const bool		ScaleArbitrary() const;		//< supports arbitrary scaling of any degree 

	virtual const char *	ScalerName() const;			//< Name Of the Scaler (1 word)
	virtual const char *	ScalerDesc() const;			//< Desciption of the Scaler
	virtual const char *	ScalerCopyright() const;	//< Scaler Copyright info
};

extern const _2xSaIScaler		_2xSaI_scaler;
extern const Super2xSaIScaler	Super2xSaI_scaler;
extern const SuperEagleScaler	SuperEagle_scaler;

#ifdef COMPILE_GAMMA_CORRECT_SCALERS

class GC_2xSaIScaler : public Scaler
{
public:
	GC_2xSaIScaler();

	virtual const uint32	ScaleBits() const;			//< bits for supported integer scaling
	virtual const bool		ScaleArbitrary() const;		//< supports arbitrary scaling of any degree 

	virtual const char *	ScalerName() const;			//< Name Of the Scaler (1 word)
	virtual const char *	ScalerDesc() const;			//< Desciption of the Scaler
	virtual const char *	ScalerCopyright() const;	//< Scaler Copyright info
};

class GC_Super2xSaIScaler : public Scaler
{
public:
	GC_Super2xSaIScaler();

	virtual const uint32	ScaleBits() const;			//< bits for supported integer scaling
	virtual const bool		ScaleArbitrary() const;		//< supports arbitrary scaling of any degree 

	virtual const char *	ScalerName() const;			//< Name Of the Scaler (1 word)
	virtual const char *	ScalerDesc() const;			//< Desciption of the Scaler
	virtual const char *	ScalerCopyright() const;	//< Scaler Copyright info
};

class GC_SuperEagleScaler : public Scaler
{
public:
	GC_SuperEagleScaler();

	virtual const uint32	ScaleBits() const;			//< bits for supported integer scaling
	virtual const bool		ScaleArbitrary() const;		//< supports arbitrary scaling of any degree 

	virtual const char *	ScalerName() const;			//< Name Of the Scaler (1 word)
	virtual const char *	ScalerDesc() const;			//< Desciption of the Scaler
	virtual const char *	ScalerCopyright() const;	//< Scaler Copyright info
};

extern const GC_2xSaIScaler			GC_2xSaI_scaler;
extern const GC_Super2xSaIScaler	GC_Super2xSaI_scaler;
extern const GC_SuperEagleScaler	GC_SuperEagle_scaler;
#endif
};

#endif

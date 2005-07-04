/*
 *  Copyright (C) 2002  Ryan Nunn and The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SCALERMANAGER_H_INCLUDED
#define SCALERMANAGER_H_INCLUDED

#include <vector>
#include "istring.h"

namespace Pentagram {
	class	Scaler;
};

//
// This entire class is just static
//
class ScalerManager {
	std::vector<const Pentagram::Scaler*>		scalers;

	static ScalerManager *scaler_man;

	// Constructor
	ScalerManager();

public:
	// Destructor
	~ScalerManager();

	//! Get instance, or create
	static ScalerManager *get_instance() { 
			if (!scaler_man) scaler_man = new ScalerManager;
			return scaler_man; 
	}

	//! Adds a scaler, returns the index
	uint32 AddScaler(const Pentagram::Scaler *scaler);


	//! Get the total Number of scalers
	uint32 GetNumScalers();

	//! Get the Scaler Index from its name
	uint32 GetIndexForName(const Pentagram::istring name);

	//! Get Name of a Scaler from its Index
	const char *GetNameForIndex(uint32 index);

	//! Get a Scaler from its Index
	const Pentagram::Scaler	*GetScaler(uint32 index);

	//! Get a Scaler from its name
	const Pentagram::Scaler	*GetScaler(const Pentagram::istring name);

	//! Get the Point Sampling Scaler
	const Pentagram::Scaler	*GetPointScaler();
};


#endif // SCALERMANAGER_H_INCLUDED

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

#include "pent_include.h"
#include "ScalerManager.h"
#include "Scaler.h"
#include "scalers/PointScaler.h"

ScalerManager *ScalerManager::scaler_man = 0;

//
// Constructor
//
ScalerManager::ScalerManager()
{
}

//
// Destructor
//
ScalerManager::~ScalerManager()
{
}

//
// Get the total Number of scalers
//
uint32 ScalerManager::GetNumScalers()
{
	return scalers.size();
}

//
// Get the Index of a scaler from its Name
//
uint32 ScalerManager::GetIndexForName(const Pentagram::istring name)
{
	std::vector<const Pentagram::Scaler*>::iterator it;
	uint32 index = 0;

	for (it = scalers.begin(); it != scalers.end(); ++it, ++index) {

		if (name == (*it)->ScalerName()) return index;
	}

	return 0xFFFFFFFF;
}

//
// Get Name of a Scaler from its Index
//
const char *ScalerManager::GetNameForIndex(uint32 index)
{
	if (index >= scalers.size()) return 0;

	return scalers[index]->ScalerName();
}


//
// Get a Scaler from its Index
//
const Pentagram::Scaler *ScalerManager::GetScaler(uint32 index)
{
	if (index >= scalers.size()) return 0;
	
	return scalers[index];
}

//
// Get the Index of a scaler from its Name
//
const Pentagram::Scaler	*ScalerManager::GetScaler(const Pentagram::istring name)
{
	std::vector<const Pentagram::Scaler*>::iterator it;

	for (it = scalers.begin(); it != scalers.end(); ++it) {

		if (name == (*it)->ScalerName()) return *it;
	}

	return 0;
}

//
// Adds a scaler
// 
uint32 ScalerManager::AddScaler(const Pentagram::Scaler *scaler)
{
	if (!scaler) return 0xFFFFFFFF;

	std::vector<const Pentagram::Scaler*>::iterator it;
	uint32 index = 0;

	for (it = scalers.begin(); it != scalers.end(); ++it, ++index) {

		if (scaler == (*it)) return index;
	}

	scalers.push_back(scaler);
	return scalers.size()-1;
}

//
// Get the Point Sampling Scaler
//
const Pentagram::Scaler *ScalerManager::GetPointScaler()
{
	// Point scaler is always first
	return &Pentagram::point_scaler;
}



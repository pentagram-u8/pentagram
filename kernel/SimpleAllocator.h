/*
Copyright (C) 2003-2005 The Pentagram team

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

#ifndef SIMPLE_ALLOCATOR_H
#define SIMPLE_ALLOCATOR_H

#include "Allocator.h"
#include "SimplePool.h"
#include <vector>

class SimpleAllocator: public Allocator
{
public:
	SimpleAllocator(size_t nodeCapacity, uint32 nodes);
	virtual ~SimpleAllocator();

	ENABLE_RUNTIME_CLASSTYPE();

	virtual void * allocate(size_t size);

	virtual Pool * findPool(void * ptr);

	size_t getNodeCapacity() {return nodeCapacity;}

private:
	std::vector<SimplePool *> pools;

	size_t nodeCapacity;
	uint32 nodes;
};

#endif

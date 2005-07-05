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

#ifndef SEGMENTED_ALLOCATOR_H
#define SEGMENTED_ALLOCATOR_H

#include "Allocator.h"
#include "SegmentedPool.h"
#include <vector>

class SegmentedAllocator: public Allocator
{
public:
	SegmentedAllocator(size_t nodeCapacity, uint32 nodes);
	virtual ~SegmentedAllocator();

	ENABLE_RUNTIME_CLASSTYPE();

	virtual void * allocate(size_t size);

	virtual Pool * findPool(void * ptr);

	virtual void freeResources();

	virtual size_t getCapacity() {return nodeCapacity;}

	void printInfo();

private:
	std::vector<SegmentedPool *> pools;

	size_t nodeCapacity;
	uint32 nodes;
};

#endif

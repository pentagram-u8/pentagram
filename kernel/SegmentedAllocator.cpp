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
#include "pent_include.h"

#include "SegmentedAllocator.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(SegmentedAllocator,Allocator);

SegmentedAllocator::SegmentedAllocator(size_t nodeCapacity_, uint32 nodes_): Allocator(), nodes(nodes_)
{
	pools.push_back(new SegmentedPool(nodeCapacity_, nodes_));
	nodeCapacity = pools[0]->getNodeCapacity();
//	pout << "Initial Pool Created: Nodes - " << nodes << ", Node Capacity - "
//		<< nodeCapacity << std::endl;
}

SegmentedAllocator::~SegmentedAllocator()
{
	std::vector<SegmentedPool *>::iterator i;
	for (i = pools.begin(); i != pools.end(); ++i)
	{
		delete *i;
	}

	pools.clear();
}

void * SegmentedAllocator::allocate(size_t size)
{
	std::vector<SegmentedPool *>::iterator i;
	SegmentedPool * p;

	if (size > nodeCapacity)
		return 0;

	for (i = pools.begin(); i != pools.end(); ++i)
	{
		if (! (*i)->isFull())
			return (*i)->allocate(size);
	}

	// else we need a new pool
	p = new SegmentedPool(nodeCapacity, nodes);
	if (p)
	{
//		pout << "New Pool Created: Nodes - " << nodes << ", Node Capacity - "
//			<< nodeCapacity << std::endl;

		pools.push_back(p);
		return p->allocate(size);
	}

	// fail
	return 0;
}

Pool * SegmentedAllocator::findPool(void * ptr)
{
	std::vector<SegmentedPool *>::iterator i;
	for (i = pools.begin(); i != pools.end(); ++i)
	{
		if ((*i)->inPool(ptr))
			return *i;
	}
	return 0;
}

void SegmentedAllocator::freeResources()
{
	if (pools.empty())
		return;

	// Pop back only -- it should suffice.
	while(pools.back()->isEmpty())
	{
		delete pools.back();
		pools.pop_back();

		if (pools.empty())
			return;
	}
}

void SegmentedAllocator::printInfo()
{
	std::vector<SegmentedPool *>::iterator it;
	int i = 0;

	pout << "Pools: " <<  pools.size() << std::endl;
	for (it = pools.begin(); it != pools.end(); ++it)
	{
		pout << "  Pool " << i++ << ":" << std::endl;
		(*it)->printInfo();
	}
}

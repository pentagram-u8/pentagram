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

#ifndef SIMPLE_POOL_H
#define SIMPLE_POOL_H

#include "Pool.h"
// Think about adding magic to the front of the
// PoolNode to identify them.

class SimplePool;

struct SimplePoolNode
{
	SimplePool * pool;
	size_t size;
};

class SimplePool: public Pool
{
public:
	SimplePool(size_t nodeCapacity, uint32 nodes);
	virtual ~SimplePool();

	ENABLE_RUNTIME_CLASSTYPE();

	virtual void * allocate(size_t size);
	virtual void deallocate(void * ptr);

	virtual bool isFull() {return freeNodePos == 0;}
	virtual bool isEmpty() {return freeNodePos == nodes;}

	virtual bool inPool(void * ptr) {return (ptr > startOfPool && ptr < endOfPool);}

	size_t getNodeCapacity() {return nodeCapacity;}

	SimplePoolNode* getPoolNode(void * ptr);
private:
	uint8 * startOfPool;
	uint8 * endOfPool;

	size_t nodeOffset;
	size_t nodeCapacity;
	uint32 nodes;

	// Array stack to hold pointers to free nodes
	SimplePoolNode** freeNodes;
	uint32 freeNodePos;
};

#endif

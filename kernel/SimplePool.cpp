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

#include "SimplePool.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(SimplePool,Pool);

//	Memory is aligned to the next largest multiple of sizeof(x) from the base adddress plus the size.
//	Although, this may not be very helpful if the base address is not a multiple of sizeof(X).
//	example: sizeof(x) = 0x8, object size = 0xFFE2:
//			0xFFE2 + 0x8 - 1 = 0xFFE9;
//			0xFFE9 & ~(0x8 - 0x1) -> 0xFFE9 & 0xFFF8 = 0xFFE8

#define OFFSET_ALIGN(X) ( (X + sizeof(uintptr) - 1) & ~(sizeof(uintptr) - 1) )

// We padd both the PoolNode and the memory to align it.

SimplePool::SimplePool(size_t nodeCapacity_, uint32 nodes_): Pool(), freeNodePos(nodes_)
{
	SimplePoolNode* node;
	uint32 i;

	//Give it it's real capacity.
	nodeCapacity = OFFSET_ALIGN(nodeCapacity_);
	nodes = nodes_;
	
	// Node offesets are alligned to the next uintptr offset after the real size
	nodeOffset = OFFSET_ALIGN(sizeof(SimplePoolNode)) + nodeCapacity;

	startOfPool = new uint8[nodeOffset * nodes_];
	endOfPool = startOfPool + (nodeOffset * nodes_);

	freeNodes = new SimplePoolNode*[nodes_];

/*
	con.Printf("Pool Info:\n start %X\tend %X\n nodeOffset %X\t nodeCapacity %X\n nodes %X\n",
			startOfPool, endOfPool, nodeOffset, nodeCapacity, nodes);
*/

	for (i = 0; i < nodes_; ++i)
	{
		node = reinterpret_cast<SimplePoolNode*>(startOfPool + i * nodeOffset);
		node->pool = this;
		node->size = 0;
		freeNodes[i] = node;
/*
		con.Printf("Node %d: %X\n", i, node);
		if (node == getPoolNode((uint8*) node + 6))
			pout << "Node located" << std::endl;
		else
			pout << "Wrong Node" << std::endl;
*/
	}
}

SimplePool::~SimplePool()
{
	assert(isEmpty());

	delete [] startOfPool;
	delete [] freeNodes;
}

void * SimplePool::allocate(size_t size)
{
	SimplePoolNode* node;

	if (isFull() || size > nodeCapacity)
		return 0;

	--freeNodePos;
	node = freeNodes[freeNodePos];
	node->size = size;

//	con.Printf("Allocating Node 0x%08X\n", node);
	return (reinterpret_cast<uint8 *>(node) + OFFSET_ALIGN(sizeof(SimplePoolNode)) );
}

void SimplePool::deallocate(void * ptr)
{
	SimplePoolNode* node;

	if (inPool(ptr))
	{
		node = getPoolNode(ptr);
		node->size = 0;
		assert(node->pool == this);

//	con.Printf("Free Node 0x%08X\n", node);
		freeNodes[freeNodePos] = node;
		++freeNodePos;
	}
}

SimplePoolNode* SimplePool::getPoolNode(void * ptr)
{
	uint32 pos = (reinterpret_cast<uint8 *>(ptr) - startOfPool) / nodeOffset;
	return reinterpret_cast<SimplePoolNode*>(startOfPool + pos * nodeOffset);
}


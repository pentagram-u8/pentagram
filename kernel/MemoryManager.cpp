/*
Copyright (C) 2002-2004 The Pentagram team

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

#include "MemoryManager.h"

#include "Object.h"
#include "SimpleAllocator.h"

MemoryManager* MemoryManager::memorymanager = 0;

MemoryManager::MemoryManager()
{
	assert(memorymanager == 0);
	memorymanager = this;

	allocators[objectAllocator] = new SimpleAllocator(256, 10000);
	allocators[processAllocator] = new SimpleAllocator(256, 10000);
	allocators[UCProcessAllocator] = new SimpleAllocator(4224, 10);
}

MemoryManager::~MemoryManager()
{
	memorymanager = 0;

	delete allocators[objectAllocator];
	delete allocators[processAllocator];
	delete allocators[UCProcessAllocator];
}

void * MemoryManager::allocate(size_t size)
{
	void * ptr = malloc(size);
	con.Printf("MemoryManager::allocate - Allocated %d bytes to 0x%X\n", size, ptr);

	return ptr;
}

void MemoryManager::deallocate(void * ptr)
{
	Pool * p;
	p = allocators[objectAllocator]->findPool(ptr);

	con.Printf("MemoryManager::deallocate - deallocating memory at 0x%X\n", ptr);
	// try other allocators
	if (!p)
		p = allocators[processAllocator]->findPool(ptr);

	if (!p)
		p = allocators[UCProcessAllocator]->findPool(ptr);

	if (p)
	{
		p->deallocate(ptr);
	}
	else
	{
		// Pray!
		free(ptr);
	}
}

void MemoryManager::ConCmd_MemInfo(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	//! Todo: Write this!
}

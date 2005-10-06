/*
Copyright (C) 2003-2004 The Pentagram team

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

#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

class Allocator;

class MemoryManager
{
public:
	MemoryManager();
	~MemoryManager();

	static MemoryManager* get_instance() { return memorymanager; }

	//! Allocates memory with the default allocator or malloc
	static void * allocate(size_t size)
		{ return memorymanager ? memorymanager->_allocate(size) : 0; }

	//! Checks all known Allocators to free memory
	static void deallocate(void * ptr)
		{ memorymanager->_deallocate(ptr); }

	Allocator * getAllocator(uint16 index)
		{ return index < allocatorCount ? allocators[index] : 0; }

	uint16 getAllocatorCount()
		{ return allocatorCount; }

	void freeResources();

	//! "MemoryManager::MemInfo" console command
	static void ConCmd_MemInfo(const Console::ArgvType &argv);

	//! "MemoryManager::test" console command
	static void ConCmd_test(const Console::ArgvType &argv);

private:
	Allocator* allocators[10];
	uint16 allocatorCount;

	void * _allocate(size_t size);
	void _deallocate(void * ptr);

	static MemoryManager* memorymanager;
};

#endif

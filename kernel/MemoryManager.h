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

#include "Allocator.h"

#define DEFINE_CUSTOM_MEMORY_ALLOCATION(Classname, AllocatorType)	\
void * Classname::operator new(size_t size) {						\
	void * ptr = MemoryManager::get_instance()						\
		->getAllocator(AllocatorType)->allocate(size);				\
	if (!ptr) ptr = MemoryManager::get_instance()->allocate(size);	\
	return ptr;														\
}																	\
																	\
void Classname::operator delete(void * ptr) {						\
	Pool * p = MemoryManager::get_instance()						\
		->getAllocator(AllocatorType)->findPool(ptr);				\
	if (p) p->deallocate(ptr);										\
	else MemoryManager::get_instance()->deallocate(ptr);			\
}																	\
																	\
void * Classname::operator new(size_t size, Allocator * a) {		\
	void * ptr = a->allocate(size);									\
	if (!ptr) ptr = MemoryManager::get_instance()->allocate(size);	\
	return ptr;														\
}																	\
																	\
void Classname::operator delete(void * ptr, Allocator * a) {		\
	Pool * p = a->findPool(ptr);									\
	if (p) p->deallocate(ptr);										\
	else MemoryManager::get_instance()->deallocate(ptr);			\
}

class MemoryManager
{
public:
	MemoryManager();
	~MemoryManager();

	static MemoryManager* get_instance() { return memorymanager; }

	//! Allocates memory with the default allocator or malloc
	void * allocate(size_t size);

	//! Checks all known Allocators to free memory
	void deallocate(void * ptr);

	enum AllocatorType
	{
		objectAllocator = 0,
		processAllocator,
		UCProcessAllocator
	};

	Allocator * getAllocator(enum AllocatorType type)
		{ return allocators[type]; }

	//! "MemoryManager::MemInfo" console command
	static void ConCmd_MemInfo(const Console::ArgsType &args, const Console::ArgvType &argv);

	//! "MemoryManager::test" console command
//	static void ConCmd_test(const Console::ArgsType &args, const Console::ArgvType &argv);

private:
	Allocator* allocators[3];

	static MemoryManager* memorymanager;
};

#endif

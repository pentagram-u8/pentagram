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
#include "SegmentedAllocator.h"

MemoryManager* MemoryManager::memorymanager = 0;

MemoryManager::MemoryManager()
{
	assert(memorymanager == 0);
	memorymanager = this;

	allocators[objectAllocator] = new SegmentedAllocator(256, 10000);
	allocators[processAllocator] = new SegmentedAllocator(256, 10000);
	allocators[UCProcessAllocator] = new SegmentedAllocator(4224, 10);
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
#ifdef DEBUG
	con.Printf("MemoryManager::allocate - Allocated %d bytes to 0x%X\n", size, ptr);
#endif

	return ptr;
}

void MemoryManager::deallocate(void * ptr)
{
	Pool * p;
	p = allocators[objectAllocator]->findPool(ptr);

#ifdef DEBUG
	con.Printf("MemoryManager::deallocate - deallocating memory at 0x%X\n", ptr);
#endif

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

#if 0
// Test classes purely here to check the speed of Allocators vs. normal allocation
class TestClassOne
{
public:
	TestClassOne()
	{
	}

	virtual ~TestClassOne()
	{
	}

	ENABLE_RUNTIME_CLASSTYPE();
private:
	int arr[32];
};

DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(TestClassOne);

class TestClassTwo: public TestClassOne
{
public:
	TestClassTwo()
	{
	}

	virtual ~TestClassTwo()
	{
	}

	ENABLE_RUNTIME_CLASSTYPE();
	ENABLE_CUSTOM_MEMORY_ALLOCATION();
private:
	int arr[32];
};

DEFINE_RUNTIME_CLASSTYPE_CODE(TestClassTwo, TestClassOne);
DEFINE_CUSTOM_MEMORY_ALLOCATION(TestClassTwo, MemoryManager::objectAllocator);

void MemoryManager::ConCmd_test(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	int i;
	TestClassOne * test[10000];

	// Un pooled
	for (i = 0; i < 10000; ++i)
	{
		test[i] = new TestClassOne();
	}

	for (i = 0; i < 10000; ++i)
	{
		delete test[i];
	}

	// pooled with the objectAllocator
	for (i = 0; i < 10000; ++i)
	{
		test[i] = new TestClassTwo();
	}

	for (i = 0; i < 10000; ++i)
	{
		delete test[i];
	}

	delete twoAllocator;
}
#endif


/*
 *	Folder.h - The core of the folding utility
 *
 *  Copyright (C) 2002 The Pentagram Team
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

#ifndef FOLDER_H
#define FOLDER_H

#include "GenericNodes.h"
#include "IfNode.h"
#include "CallNodes.h"

#include <deque>
#include <set>
#include <utility>

class Unit
{
	public:
		Unit() : debugOffset(0) {};
		
		void fold(Node *n);
		
		void print_extern_unk(Console &o, const uint32 isize) const;
		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(OBufferDataSource &o) const;
		
		void setDebugOffset(const uint32 newDebugOffset) { debugOffset=newDebugOffset; };
		void setClassName(const std::string &newName) { name=newName; };
		void registerExternIntrinsic(CallNode *i) { externIntrinsics.insert(i); };
		void registerExternFunc(CallNode *f) { externFuncs.insert(f); };
		
		void setJump(IfNode *in) { ifstack.push_back(in); };
		const std::deque<IfNode *> &IfStack() const { return ifstack; };
		
	private:
		std::deque<Node *> nodes;
		/* a bizzare way of walking through the if stack:
			Now the general way we're going to walk through and correctly associate
			a if{...} with the appropriate nodes list when called is that for
		*/
		std::deque<IfNode *> ifstack;
		std::deque<IfNode *> elsestack;
		//DELstd::deque<uint32>   jneoffsets; // the 'jump to' offsets for jne
		
		uint32 debugOffset;
		std::string name;

		std::set<CallNode *> externFuncs;
		std::set<CallNode *> externIntrinsics;
		
		friend bool print_assert(const Node *n, const Unit *u);
};

class Folder
{
	public:
		Folder() : curr(0), num(10) {};
		~Folder() {};
		void fold(Node *n);

		void print_unk(Console &o) const;
		void print_asm(Console &o) const;
		void print_bin(OBufferDataSource &o) const;

		inline void NewUnit()
		{
			if(curr!=0) units.push_back(curr);
			curr = new Unit();
		};

	private:
		Unit *curr;
		std::deque<Unit *> units;
		uint32 num;
};


#endif

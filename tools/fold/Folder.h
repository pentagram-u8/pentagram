/*
 *	Folder.h - The core of the folding utility
 *
 *  Copyright (C) 2002-2003 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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
#include "FuncNodes.h"
#include "CommonBases.h"

#include <deque>
#include <set>

class Unit
{
	public:
		Unit(const uint32 _id) : id(_id) {};
		
		void print_extern_unk(Console &o, const uint32 isize) const;
		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource &o) const;
		
		std::string name; // the cannonical name of the unit
		uint32 id; // the cannonical id number of the class
		
		std::set<DCCallNode *>   externFuncs;
		std::set<DCCallNode *>   externIntrinsics;
		std::deque<DCFuncNode *> functions;
};

class DCUnit : public Unit
{
	public:
		DCUnit(const uint32 _id) : Unit(_id), debugOffset(0) {};
		
		const bool fold(Node *n);
		
		void setDebugOffset(const uint32 newDebugOffset) { debugOffset=newDebugOffset; };
		void setClassName(const std::string &newName) { name=newName; };
		void registerExternIntrinsic(DCCallNode *i) { externIntrinsics.insert(i); };
		void registerExternFunc(DCCallNode *f) { externFuncs.insert(f); };
		
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
		
		uint32 debugOffset;

		friend bool print_assert(const Node *n, const DCUnit *u);
};

class Folder
{
	public:
		Folder() : curr(0), num(10) {};
		~Folder() {};
		void fold(Node *n);

		void print_unk(Console &o) const;
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource &o) const;

		inline void SelectUnit(const uint32 id)
		{
			///con.Printf(">>> SelectingUnit: %d OldUnit: %d <<<\n", id, (curr ? curr->id : 0));
			if(curr!=0 && curr->id!=id)
				NewUnit(id);
			else if(curr==0)
				NewUnit(id);
			// else, the id==the old one, so we don't need to change units, return.
		};
		
		inline void NewUnit(const uint32 id)
		{
			if(curr!=0) units.push_back(curr);
			curr = new DCUnit(id);
		};
		
		inline void FinalUnit()
		{
			if(curr!=0)
				units.push_back(curr);
			curr=0;
		}

	private:
		DCUnit *curr;
		std::deque<DCUnit *> units;
		uint32 num;
};


#endif

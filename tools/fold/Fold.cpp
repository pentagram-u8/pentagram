/*
 *	fold.cpp - parts of a disassembler for u8usecode
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

#include "pent_include.h"

#define FOLD

#include <vector>
using	std::vector;
#include <string>
using	std::string;
#include "IDataSource.h"

class Node;

Node *newNode(uint32 pos);

vector<bool> ismember; // tells if the 'intrinsic' call is a call to a member function or not
bool print_disasm=false;

void initfolding();

#include "Disasm.cpp"

#include "Folder.h"

#include "u8/ConvertUsecodeU8.h"
#include "crusader/ConvertUsecodeCrusader.h"


void initfolding()
{
	// if we're crusader, we don't have to do any initialisation since
	// there's no known intrinsics
	//if(crusader) return;
	
	const char * const *i=convert->intrinsics();
	
	while(*i!=0)
	{
		//std::cout << *i << std::endl;
		if(string(*i).find(":")==string::npos) // it's a global
			ismember.push_back(false);
		else // it's a member
			ismember.push_back(true);
		++i;
	}
}

/*
Copyright (C) 2002 The Pentagram team

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

#ifndef UCMACHINE_H
#define UCMACHINE_H

#include "UCStack.h"

class UCProcess;

class UCMachine
{
public:
	UCMachine();
	~UCMachine();

	static UCMachine* get_instance() { return ucmachine; }

	bool execProcess(UCProcess* proc);

private:

	// this technically isn't a stack, but UCStack supports the access 
	// functions we need
	UCStack globals;

	static UCMachine* ucmachine;
};


#endif

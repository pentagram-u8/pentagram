/*
Copyright (C) 2002-2003 The Pentagram team

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

#ifndef UCPROCESS_H
#define UCPROCESS_H

#include "Process.h"
#include "UCStack.h"
#include "IDataSource.h"

class Usecode;


// probably won't inherit from Process directly in the future
class UCProcess : public Process
{
	friend class UCMachine;

public:
	UCProcess(Usecode* usecode_, uint16 classid_,
			  uint16 offset_, uint32 this_ptr = 0);
	UCProcess(Usecode* usecode_, uint16 classid_,
			  uint16 offset_, const uint8* args, uint32 argsize);
    ~UCProcess();

	// p_dynamic_cast stuff
	ENABLE_DYNAMIC_CAST(UCProcess);

	virtual bool run(const uint32 framenum);

protected:

	void call(uint16 classid_, uint16 offset_);
	bool ret();

	// item we are assigned to
	uint16 item_num;

	uint16 type;

	// stack base pointer
	uint16 bp;

	Usecode* usecode;

	uint16 classid;
	uint16 ip;

	// data stack
	UCStack stack;
};

#endif

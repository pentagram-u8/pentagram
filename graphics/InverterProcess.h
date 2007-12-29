/*
 *  Copyright (C) 2004  Pentagram Team
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

#ifndef INVERTERPROCESS_H
#define INVERTERPROCESS_H

#include "Process.h"
#include "intrinsics.h"

class InverterProcess : public Process
{
public:
	InverterProcess();
	InverterProcess(unsigned int targetstate);
	virtual ~InverterProcess();

	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE();

	void setTarget(unsigned int target) { targetstate = target; }

	virtual void run();

	static ProcId invertScreen();
	static void ConCmd_invertScreen(const Console::ArgvType &argv);

	INTRINSIC(I_invertScreen);

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);

	static InverterProcess* inverter;
	unsigned int targetstate;
};


#endif //PALETTEFADERPROCESS_H_INCLUDED

/*
Copyright (C) 2004 The Pentagram team

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

#ifndef COMBATPROCESS_H
#define COMBATPROCESS_H

#include "Process.h"

class Actor;

class CombatProcess : public Process
{
public:
	CombatProcess();
	CombatProcess(Actor* actor);

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	virtual bool run(const uint32 framenum);

	ObjId getTarget();
	void setTarget(ObjId target_);

	virtual void dumpInfo();

	bool loadData(IDataSource* ids);
protected:
	virtual void saveData(ODataSource* ods);

	bool isValidTarget(Actor* target);
	bool isEnemy(Actor* target);
	ObjId seekTarget();
	bool inAttackRange();

	ObjId target;
	ObjId fixedTarget;

	enum CombatMode {
		CM_WAIT = 0,
		CM_ATTACK
	};
};


#endif

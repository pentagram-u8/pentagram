/*
Copyright (C) 2003 The Pentagram team

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

#ifndef ACTORANIM_H
#define ACTORANIM_H

#include <vector>
#include "AnimAction.h"

class ActorAnim
{
	friend class AnimDat;
public:
	ActorAnim() {}
	~ActorAnim()
	{
		for (unsigned int i = 0; i < actions.size(); ++i)
			delete actions[i];
	}

	AnimAction* getAction(unsigned int n) {
		if (n >= actions.size()) return 0;
		return actions[n];
	}

private:
	std::vector<AnimAction*> actions; // list of this actor's actions
	                                  // (0 if actor doesn't have action)
};

#endif

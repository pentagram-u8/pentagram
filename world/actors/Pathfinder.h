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

#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <vector>
#include <queue>
#include "Animation.h"

class Actor;
class Item;

struct PathfindingState
{
	sint32 x, y, z;
	Animation::Sequence lastanim;
	uint32 direction;
	bool flipped;
	bool firststep;

	void load(Actor* actor);
	bool checkPoint(sint32 x_, sint32 y_, sint32 z_,int xyRange,int zRange);
	bool checkItem(Item* item, int xyRange, int zRange);
};

struct PathfindingAction
{
	Animation::Sequence action;
	uint32 direction;
};

struct PathNode;

class PathNodeCmp {
public:
	bool operator()(PathNode* n1, PathNode* n2);
};

class Pathfinder
{
public:
	Pathfinder();
	~Pathfinder();

	void init(Actor* actor, PathfindingState* state=0);
	void setTarget(sint32 x, sint32 y, sint32 z);
	void setTarget(Item* item);

	//! try to reach the target by pathfinding
	bool canReach();

	//! pathfind. If true, the found path is returned in path
	bool pathfind(std::vector<PathfindingAction>& path);

protected:
	PathfindingState start;
	Actor* actor;
	sint32 targetx, targety, targetz;
	Item* targetitem;
	std::vector<PathfindingState> visited;
	std::priority_queue<PathNode*,std::vector<PathNode*>,PathNodeCmp> nodes;

	bool alreadyVisited(sint32 x, sint32 y, sint32 z);
	void expandNode(PathNode* node);
	unsigned int costHeuristic(PathNode* node);
	bool checkTarget(PathNode* node);
};

#endif

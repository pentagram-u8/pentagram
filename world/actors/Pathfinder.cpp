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

#include "pent_include.h"
#include "Pathfinder.h"
#include "Actor.h"
#include <cmath>

struct PathNode
{
	PathfindingState state;
	unsigned int cost;
	unsigned int heuristicTotalCost;
	PathNode* parent;
};


void PathfindingState::load(Actor* actor)
{
	actor->getLocation(x, y, z);
	lastanim = actor->getLastAnim();
	direction = actor->getDir();
	firststep = actor->getActorFlags() & Actor::ACT_FIRSTSTEP;
	flipped = actor->getFlags() & Item::FLG_FLIPPED;
}

bool PathfindingState::checkPoint(sint32 x_, sint32 y_, sint32 z_,
								  int xyRange, int zRange)
{
	return (abs(z - z_) <= zRange && abs(x - x_) <= xyRange &&
			abs(y - y_) < xyRange);
}

bool PathfindingState::checkItem(Item* item, int xyRange, int zRange)
{
	sint32 itemX, itemY, itemZ;
	sint32 itemXd, itemYd, itemZd;
	sint32 itemXmin, itemYmin;

	item->getLocationAbsolute(itemX, itemY, itemZ);
	item->getFootpadWorld(itemXd, itemYd, itemZd);

	itemXmin = itemX - itemXd;
	itemYmin = itemY - itemYd;

	int range = 0;
	if (x - itemX > range)
		range = x - itemX;
	if (itemXmin - x > range)
		range = itemXmin - x;
	if (y - itemY > range)
		range = y - itemY;
	if (itemYmin - y > range)
		range = itemYmin - y;

	// FIXME: check z properly

	return (range <= xyRange);
}

bool PathNodeCmp::operator()(PathNode* n1, PathNode* n2)
{
	return (n1->heuristicTotalCost > n2->heuristicTotalCost);
}

Pathfinder::Pathfinder()
{

}

Pathfinder::~Pathfinder()
{
	//TODO: clean up nodes
}

void Pathfinder::init(Actor* actor_, PathfindingState* state)
{
	actor = actor_;

	if (state)
		start = *state;
	else
		start.load(actor);
}

void Pathfinder::setTarget(sint32 x, sint32 y, sint32 z)
{
	targetx = x;
	targety = y;
	targetz = z;
	targetitem = 0;
}

void Pathfinder::setTarget(Item* item)
{
	targetitem = item;
	while (targetitem->getParentAsContainer())
		targetitem = targetitem->getParentAsContainer();

	// set target to centre of item for the cost heuristic
	item->getCentre(targetx, targety, targetz);
	targetz = item->getZ();
}

bool Pathfinder::canReach()
{
	std::vector<PathfindingAction> path;
	return pathfind(path);
}

bool Pathfinder::alreadyVisited(sint32 x, sint32 y, sint32 z)
{
	//! this needs optimization

	for (unsigned int i = 0; i < visited.size(); ++i)
	{
		if (visited[i].checkPoint(x,y,z,8,0))
		{
			return true;
		}
	}

	return false;
}

bool Pathfinder::checkTarget(PathNode* node)
{
	// TODO: these ranges are probably a bit too high,
	// but otherwise it won't work properly yet -wjp
	if (targetitem)
		return node->state.checkItem(targetitem, 32, 8);
	else
		return node->state.checkPoint(targetx, targety, targetz, 32, 8);
}

unsigned int Pathfinder::costHeuristic(PathNode* node)
{
	unsigned int cost = node->cost;

	double sqrddist;

	sqrddist = (targetx - node->state.x)*(targetx - node->state.x);
	sqrddist += (targety - node->state.y)*(targety - node->state.y);

	unsigned int dist = static_cast<unsigned int>(std::sqrt(sqrddist));

#if 0
	//!! TODO: divide dist by walking distance
	// (using 32 for now)
	dist /= 32;

	node->heuristicTotalCost = cost + (dist*4); //!! constant
#else
	node->heuristicTotalCost = cost + dist;
#endif

	return node->heuristicTotalCost;
}

void Pathfinder::expandNode(PathNode* node)
{
	Animation::Sequence c_walk_anim = Animation::walk;
	PathfindingState state;

	// try walking in all 8 directions

	for (uint32 dir = 0; dir < 8; ++dir) {
		state = node->state;

		bool ok = actor->tryAnim(c_walk_anim, dir, &state);

		if (ok)
			ok = !alreadyVisited(state.x, state.y, state.z);

		if (ok) {
			PathNode* newnode = new PathNode();
			newnode->state = state;
			newnode->parent = node;

			double sqrddist;
			
			sqrddist = ((newnode->state.x - node->state.x)*
						(newnode->state.x - node->state.x));
			sqrddist += ((newnode->state.y - node->state.y)*
						 (newnode->state.y - node->state.y));
			sqrddist += ((newnode->state.z - node->state.z)*
						 (newnode->state.z - node->state.z));
			
			unsigned int dist = static_cast<unsigned int>(std::sqrt(sqrddist));

			int turn = state.direction - node->state.direction;
			if (turn > 4) turn = 8 - turn;

			newnode->cost = node->cost + dist + 16*turn; //!! constant

			costHeuristic(newnode);
#if 0
			perr << "trying dir " << dir << " from ("
				 << node->state.x << "," << node->state.y << "), cost = "
				 << newnode->cost << ", heurtotcost = "
				 << newnode->heuristicTotalCost << std::endl;
#endif

			nodes.push(newnode);
			visited.push_back(state);
		}
	}
}

bool Pathfinder::pathfind(std::vector<PathfindingAction>& path)
{
	//!! FIXME: memory leaks

	pout << "Actor " << actor->getObjId();

	if (targetitem) {
		pout << " pathfinding to item: ";
		targetitem->dumpInfo();
	} else {
		pout << " pathfinding to (" << targetx << "," << targety << "," << targetz << ")" << std::endl;
	}

	path.clear();

	PathNode* startnode = new PathNode();
	startnode->state = start;
	startnode->cost = 0;
	startnode->parent = 0;

	nodes.push(startnode);

	unsigned int expandednodes = 0;
	const unsigned int NODELIMIT = 200; //! constant
	bool found = false;

	while (expandednodes < NODELIMIT && !nodes.empty() && !found) {
		PathNode* node = nodes.top(); nodes.pop();

#if 0
		pout << "Trying node: (" << node->state.x << "," << node->state.y
			 << "," << node->state.z << ") target=(" << targetx << ","
			 << targety << "," << targetz << ")" << std::endl;
#endif

		if (checkTarget(node)) {
			// done!

			// find path length
			PathNode* n = node;
			unsigned int length = 0;
			while (n->parent) {
				n = n->parent;
				length++;
			}
#if 0
			pout << "Pathfinder: path found (length = " << length << ")"
				 << std::endl;
#endif

			unsigned int i = length;
			if (length > 0) length++; // add space for final 'stand' action
			path.resize(length);

			// now backtrack through the nodes to assemble the final animation
			while (node->parent) {
				PathfindingAction action;
				action.action = node->state.lastanim;
				action.direction = node->state.direction;
				path[--i] = action;
#if 0
				pout << "anim = " << node->state.lastanim << ", dir = "
					 << node->state.direction << std::endl;
#endif

				//TODO: check how turns work
				//TODO: append final 'stand' animation

				node = node->parent;
			}

			if (length) {
				path[length-1].action = Animation::stand;
				path[length-1].direction = path[length-2].direction;
			}

			return true;
		}

		expandNode(node);
		expandednodes++;
	}

	return false;
}

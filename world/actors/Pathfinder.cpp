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
}

bool PathfindingState::checkPoint(sint32 x_, sint32 y_, sint32 z_,
								  int horRange, int verRange)
{
	//!! arbitrary constant
	return (abs(z - z_) <= verRange && abs(x - x_) <= horRange &&
			abs(y - y_) < horRange);
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
		if (visited[i].checkPoint(x,y,z,32,0))
		{
			return true;
		}
	}

	return false;
}

bool Pathfinder::checkTarget(PathNode* node)
{
	return node->state.checkPoint(targetx, targety, targetz, 128, 8);
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
	const int c_walk_anim = 0;
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

	perr << "Pathfining to (" << targetx << "," << targety << "," << targetz << ")" << std::endl;


	path.clear();

	PathNode* startnode = new PathNode();
	startnode->state = start;
	startnode->cost = 0;
	startnode->parent = 0;

	nodes.push(startnode);

	unsigned int expandednodes = 0;
	const unsigned int NODELIMIT = 1000; //! constant
	bool found = false;

	while (expandednodes < NODELIMIT && !nodes.empty() && !found) {
		PathNode* node = nodes.top(); nodes.pop();

#if 0
		perr << "Trying node: (" << node->state.x << "," << node->state.y
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
			path.resize(length);

			perr << "Pathfinder: path found (length = " << length << ")"
				 << std::endl;

			// now backtrack through the nodes to assemble the final animation
			while (node->parent) {
				PathfindingAction action;
				action.action = node->state.lastanim;
				action.direction = node->state.direction;
				path[--length] = action;
#if 0
				perr << "anim = " << node->state.lastanim << ", dir = "
					 << node->state.direction << std::endl;
#endif

				//TODO: check how turns work

				node = node->parent;
			}

			return true;
		}

		expandNode(node);
		expandednodes++;
	}

	return false;
}

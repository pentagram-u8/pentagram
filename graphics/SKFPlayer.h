/*
Copyright (C) 2002-2004 The Pentagram team

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

#ifndef SKFPlayer_H
#define SKFPlayer_H

#include <vector>

struct SKFEvent;
class Flex;
class RenderSurface;
class IDataSource;
namespace Pentagram { struct Palette; }

class SKFPlayer {
public:
	SKFPlayer(Flex* movie, int width, int height);
	~SKFPlayer();

	void run();
	void paint(RenderSurface* surf, int lerp);

	void start();
	void stop();
	bool isPlaying() const { return playing; }

private:

	void parseEventList(IDataSource* eventlist);

	int width, height;
	Flex* skf;
	std::vector<SKFEvent*> events;
	unsigned int curframe, curobject;
	unsigned int curaction;
	unsigned int curevent;
	bool playing;
	unsigned int timer;
	uint8 fadecolour, fadelevel;
	RenderSurface* buffer;
};

#endif

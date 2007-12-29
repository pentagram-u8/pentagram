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

#ifndef QUICKAVATARMOVERPROCESS_H
#define QUICKAVATARMOVERPROCESS_H

#include "Process.h"

class QuickAvatarMoverProcess : public Process
{
public:
	QuickAvatarMoverProcess();

	QuickAvatarMoverProcess(int x, int y, int z, int _dir);
	virtual ~QuickAvatarMoverProcess();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	virtual void run();
	virtual void terminate();

	static bool isQuarterSpeed() {return quarter;}
	static bool isClipping() {return clipping;}
	static void setQuarterSpeed(bool q) {quarter = q;}
	static void toggleClipping() {clipping = !clipping;}

	static void terminateMover(int _dir);
	static void startMover(int x, int y, int z, int _dir);

	static void ConCmd_startMoveUp(const Console::ArgvType &argv);
	static void ConCmd_startMoveDown(const Console::ArgvType &argv);
	static void ConCmd_startMoveLeft(const Console::ArgvType &argv);
	static void ConCmd_startMoveRight(const Console::ArgvType &argv);
	static void ConCmd_startAscend(const Console::ArgvType &argv);
	static void ConCmd_startDescend(const Console::ArgvType &argv);

	static void ConCmd_stopMoveUp(const Console::ArgvType &argv);
	static void ConCmd_stopMoveDown(const Console::ArgvType &argv);
	static void ConCmd_stopMoveLeft(const Console::ArgvType &argv);
	static void ConCmd_stopMoveRight(const Console::ArgvType &argv);
	static void ConCmd_stopAscend(const Console::ArgvType &argv);
	static void ConCmd_stopDescend(const Console::ArgvType &argv);

	static void ConCmd_toggleQuarterSpeed(const Console::ArgvType &argv);
	static void ConCmd_toggleClipping(const Console::ArgvType &argv);

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);

	int dx, dy, dz, dir;
	static ProcId amp[6];
	static bool clipping;
	static bool quarter;
};

#endif


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

#ifndef APPLICATION_H
#define APPLICATION_H

class Kernel;
class UCMachine;
class FileSystem;
class ResizableGump;
class ConsoleGump;
class RenderSurface;
struct Texture;

// extremely simplified stub-ish Application class
class Application {
public:
	Application(int argc, char *argv[]);
	virtual ~Application();

	virtual void run();
	virtual void paint();

protected:
	Kernel* kernel;
	UCMachine* ucmachine;
	FileSystem* filesystem;
	ResizableGump* desktop;
	ConsoleGump* console;
	RenderSurface *screen;
	Texture *confont;

};

#endif

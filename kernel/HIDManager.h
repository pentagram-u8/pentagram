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

#ifndef HIDMANAGER_H
#define HIDMANAGER_H

#include <vector>
#include "SDL_events.h"
#include "HIDBinding.h"
#include "Mouse.h"

//! Responsible to loading the keybindings and storing them
class HIDManager
{
public:
	HIDManager();
	~HIDManager();

	//! obtain the singleton instance of the HIDManager
	static HIDManager * get_instance() { return hidmanager; }
	
	//! obtain the HIDBinding associated with the event
	//! \param event an SDL_Event used to find an appropriate HIDBinding
	//! \return a HIDBinding or null if no binding was present
	HIDBinding getBinding(const SDL_Event& event);
	
	//! loads the keybindings from the configuration
	void loadBindings();

	//! loads a single keybinding
	//! \param control a key or button to bind
	//! \param bindingName name of the HIDBinding
    void bind(const Pentagram::istring& control, const Pentagram::istring& bindingName);

	//! "bind" console command
	static void HIDManager::ConCmd_bind(const Console::ArgsType &args, const Console::ArgvType &argv);
	
	//! stores the names of all controls bond to bindingName in a vector
	//! \param bindingName name of a HIDBinding
	//! \param controls vector to store the controls attached to bindingName
	void getBindings(const Pentagram::istring& bindingName, std::vector<const char *>& controls);

private:
	HIDBindingMap bindingMap; 
	HIDBinding keybindings[SDLK_LAST];
	HIDBinding mousebindings[NUM_MOUSEBUTTONS+1];
	static HIDManager* hidmanager;
};

#endif

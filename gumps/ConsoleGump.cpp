/*
 *  Copyright (C) 2003  The Pentagram Team
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

#include "pent_include.h"
#include "ConsoleGump.h"
#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ConsoleGump,Gump);

using Pentagram::istring;

Pentagram::istring	ConsoleGump::commandBuffer;
std::map<Pentagram::istring,ConsoleGump::ConsoleFunction> ConsoleGump::ConsoleCommands;

ConsoleGump::ConsoleGump()
	: Gump()
{
}

ConsoleGump::ConsoleGump(int X, int Y, int Width, int Height) :
	Gump(X,Y,Width,Height, 0, 0, LAYER_CONSOLE), scroll_state(NORMAL_DISPLAY)
{
	commandBuffer.clear();

	// Resize it
	con.CheckResize(Width);

	// Lets try adding a Console command!
	ConsoleGump::AddConsoleCommand("CmdList", ConCmd_CmdList);
}

ConsoleGump::~ConsoleGump()
{
}

void ConsoleGump::PaintThis(RenderSurface *surf, sint32 lerp_factor)
{
	Gump::PaintThis(surf,lerp_factor);

	if (scroll_state == NOTIFY_OVERLAY)
	{
		con.DrawConsoleNotify(surf);
	}
	else if (scroll_state != WAITING_TO_SHOW)
	{
		int h = dims.h;
		if (scroll_state == SCROLLING_TO_SHOW_1) 
			h = (h*(000+lerp_factor))/1024;
		else if (scroll_state == SCROLLING_TO_SHOW_2) 
			h = (h*(256+lerp_factor))/1024;
		else if (scroll_state == SCROLLING_TO_SHOW_3) 
			h = (h*(512+lerp_factor))/1024;
		else if (scroll_state == SCROLLING_TO_SHOW_4) 
			h = (h*(768+lerp_factor))/1024;

		else if (scroll_state == SCROLLING_TO_HIDE_1) 
			h = (h*(1024-lerp_factor))/1024;
		else if (scroll_state == SCROLLING_TO_HIDE_2)
			h = (h*(768-lerp_factor))/1024;
		else if (scroll_state == SCROLLING_TO_HIDE_3)
			h = (h*(512-lerp_factor))/1024;
		else if (scroll_state == SCROLLING_TO_HIDE_4)
			h = (h*(256-lerp_factor))/1024;

		con.DrawConsole(surf,h, commandBuffer.c_str(), commandBuffer.size());
	}
}

void ConsoleGump::ToggleConsole()
{
	switch (scroll_state)
	{
	case WAITING_TO_HIDE:
		scroll_state = SCROLLING_TO_SHOW_4;
		break;

	case SCROLLING_TO_HIDE_1:
		scroll_state = SCROLLING_TO_SHOW_3;
		break;

	case SCROLLING_TO_HIDE_2:
		scroll_state = SCROLLING_TO_SHOW_2;
		break;

	case SCROLLING_TO_HIDE_3:
		scroll_state = SCROLLING_TO_SHOW_1;
		break;

	case SCROLLING_TO_HIDE_4:
		scroll_state = WAITING_TO_SHOW;
		break;

	case NOTIFY_OVERLAY:
		scroll_state = WAITING_TO_SHOW;
		break;

	case WAITING_TO_SHOW:
		scroll_state = SCROLLING_TO_HIDE_4;
		break;

	case SCROLLING_TO_SHOW_1:
		scroll_state = SCROLLING_TO_HIDE_3;
		break;

	case SCROLLING_TO_SHOW_2:
		scroll_state = SCROLLING_TO_HIDE_2;
		break;

	case SCROLLING_TO_SHOW_3:
		scroll_state = SCROLLING_TO_HIDE_1;
		break;

	case SCROLLING_TO_SHOW_4:
		scroll_state = WAITING_TO_HIDE;
		break;

	case NORMAL_DISPLAY:
		scroll_state = WAITING_TO_HIDE;
		GUIApp::get_instance()->leaveTextMode(this);
		commandBuffer.clear();
		break;

	default:
		break;
	}
}


void ConsoleGump::HideConsole()
{
	switch (scroll_state)
	{
	case WAITING_TO_SHOW:
		scroll_state = SCROLLING_TO_HIDE_4;
		break;

	case SCROLLING_TO_SHOW_1:
		scroll_state = SCROLLING_TO_HIDE_3;
		break;

	case SCROLLING_TO_SHOW_2:
		scroll_state = SCROLLING_TO_HIDE_2;
		break;

	case SCROLLING_TO_SHOW_3:
		scroll_state = SCROLLING_TO_HIDE_1;
		break;

	case SCROLLING_TO_SHOW_4:
		scroll_state = WAITING_TO_HIDE;
		break;

	case NORMAL_DISPLAY:
		scroll_state = WAITING_TO_HIDE;
		GUIApp::get_instance()->leaveTextMode(this);
		commandBuffer.clear();
		break;

	default:
		break;
	}
}


void ConsoleGump::ShowConsole()
{
	switch (scroll_state)
	{
	case WAITING_TO_HIDE:
		scroll_state = SCROLLING_TO_SHOW_4;
		break;

	case SCROLLING_TO_HIDE_1:
		scroll_state = SCROLLING_TO_SHOW_3;
		break;

	case SCROLLING_TO_HIDE_2:
		scroll_state = SCROLLING_TO_SHOW_2;
		break;

	case SCROLLING_TO_HIDE_3:
		scroll_state = SCROLLING_TO_SHOW_1;
		break;

	case SCROLLING_TO_HIDE_4:
		scroll_state = WAITING_TO_SHOW;
		break;

	case NOTIFY_OVERLAY:
		scroll_state = WAITING_TO_SHOW;
		break;

	default:
		break;
	}
}

bool ConsoleGump::ConsoleIsVisible()
{
	return scroll_state == NORMAL_DISPLAY;
}

bool ConsoleGump::Run(const uint32 framenum)
{
	Gump::Run(framenum);

	con.setFrameNum(framenum);

	switch (scroll_state)
	{
	case WAITING_TO_HIDE:
		scroll_state = SCROLLING_TO_HIDE_1;
		break;

	case SCROLLING_TO_HIDE_1:
		scroll_state = SCROLLING_TO_HIDE_2;
		break;

	case SCROLLING_TO_HIDE_2:
		scroll_state = SCROLLING_TO_HIDE_3;
		break;

	case SCROLLING_TO_HIDE_3:
		scroll_state = SCROLLING_TO_HIDE_4;
		break;

	case SCROLLING_TO_HIDE_4:
		scroll_state = NOTIFY_OVERLAY;
		break;

	case WAITING_TO_SHOW:
		scroll_state = SCROLLING_TO_SHOW_1;
		break;

	case SCROLLING_TO_SHOW_1:
		scroll_state = SCROLLING_TO_SHOW_2;
		break;

	case SCROLLING_TO_SHOW_2:
		scroll_state = SCROLLING_TO_SHOW_3;
		break;

	case SCROLLING_TO_SHOW_3:
		scroll_state = SCROLLING_TO_SHOW_4;
		break;

	case SCROLLING_TO_SHOW_4:
		scroll_state = NORMAL_DISPLAY;
		GUIApp::get_instance()->enterTextMode(this);
		commandBuffer.clear();
		break;

	default:
		break;
	}

	return true;	// Always repaint, even though we really could just try to detect it
}

void ConsoleGump::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Gump::saveData(ods);

	// Don't save scroll state, since we'll alway raise the console upon loading
}

bool ConsoleGump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Gump::loadData(ids)) return false;

	scroll_state = NOTIFY_OVERLAY;

	return true;
}

bool ConsoleGump::OnTextInput(int unicode)
{
	bool handled = false;
	if (scroll_state == NORMAL_DISPLAY) {

		commandBuffer += (char) unicode;

		handled = true;
	}
	return handled;
}


bool ConsoleGump::OnKeyDown(int key)
{
	bool handled = false;
	if (scroll_state == NORMAL_DISPLAY)
	{
		switch(key)
		{
			// Command completion
		case SDLK_TAB:
		if (!commandBuffer.empty()) {

			int count = 0;
			istring common;
			std::map<Pentagram::istring,ConsoleGump::ConsoleFunction>::iterator it;
			std::map<Pentagram::istring,ConsoleGump::ConsoleFunction>::iterator found = 0;

			for (it = ConsoleCommands.begin(); it != ConsoleCommands.end(); ++it)
				if (it->second) {
					if (it->first.compare(0, commandBuffer.size(), commandBuffer))
						continue;

					if (!count)
					{
						common = it->first;
						found = it;
					}
					else
					{
						istring::iterator it1 = common.begin();
						istring::const_iterator it2 = it->first.begin();
						int comsize = 0;

						while (it1 != common.end())
						{
							if (!istring::traits_type::eq(*it1,*it2)) break;
							
							comsize++;
							++it1;
							++it2;
						}

						common.resize(comsize);
					}
					count++;
				}

				if (count) 
				{

					if (count > 1) {
						pout.printf("]%s\n", commandBuffer.c_str());
						ConCmd_CmdList(commandBuffer);
						commandBuffer = common;
					}
					else 
						commandBuffer = common + " ";
				}
			}
			break;

		case SDLK_BACKSPACE:
			if (!commandBuffer.empty()) commandBuffer.erase(commandBuffer.end()-1);
			break;

		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			if (!commandBuffer.empty()) {
				pout.printf("]%s\n", commandBuffer.c_str());
				ConsoleGump::ExecuteQueuedCommand();	// <- WARNING: This CAN delete us!
				return true;
			}
		break;

		case SDLK_PAGEUP: {
			con.ScrollConsole(-3);
			break;
		}
		case SDLK_PAGEDOWN: {
			con.ScrollConsole(3); 
			break;
		}

		case SDLK_KP0:
		case SDLK_KP1:
		case SDLK_KP2:
		case SDLK_KP3:
		case SDLK_KP4:
		case SDLK_KP5:
		case SDLK_KP6:
		case SDLK_KP7:
		case SDLK_KP8:
		case SDLK_KP9:
			OnTextInput(key - SDLK_KP0 + '0');
			break;

		default:
			break;
		}
		handled = true;
	}
	return handled;
}

void ConsoleGump::OnFocus(bool gain)
{
	/*
	if (scroll_state == NORMAL_DISPLAY) {
		if (gain)
			GUIApp::get_instance()->enterTextMode(this);
		else
			GUIApp::get_instance()->leaveTextMode(this);
	}
	*/

}

void ConsoleGump::AddConsoleCommand(const Pentagram::istring &command, ConsoleFunction function)
{
	ConsoleCommands[command] = function;
}

void ConsoleGump::RemoveConsoleCommand(const Pentagram::istring &command)
{
	ConsoleCommands[command] = 0;
}

void ConsoleGump::ExecuteConsoleCommand(const istring &command, const istring &args)
{
	std::map<Pentagram::istring,ConsoleGump::ConsoleFunction>::iterator it;
	
	it = ConsoleCommands.find(command);

	if (it != ConsoleCommands.end())
		it->second(args);
	else
		pout << "Unknown command: " << command.c_str() << std::endl;
}

void ConsoleGump::ExecuteQueuedCommand()
{
	if (commandBuffer.empty()) return;

	istring::size_type	begIdx, endIdx;

	begIdx = commandBuffer.find_first_not_of(' ');
	if (begIdx != istring::npos) {

		istring command, args;
		endIdx = commandBuffer.find_first_of(' ', begIdx);
		if( endIdx == istring::npos )
			command = commandBuffer.substr(begIdx);
		else {
			command = commandBuffer.substr(begIdx, endIdx-begIdx);

			begIdx = commandBuffer.find_first_not_of(' ', endIdx);
			if (begIdx != istring::npos)
				args = commandBuffer.substr(begIdx);
		}
		ExecuteConsoleCommand(command, args);
	}
	commandBuffer.clear();
}

void ConsoleGump::ConCmd_CmdList(const Pentagram::istring &args)
{
	std::map<Pentagram::istring,ConsoleGump::ConsoleFunction>::iterator it;
	int i = 0;

	//pout << std::endl;

	for (it = ConsoleCommands.begin(); it != ConsoleCommands.end(); ++it)
		if (it->second) {
			if (!args.empty() && it->first.compare(0, args.size(), args))
				continue;

			pout << " " << it->first.c_str() << std::endl;
			i ++;
		}

	pout << i << " commands" << std::endl;
}

// Colourless Protection

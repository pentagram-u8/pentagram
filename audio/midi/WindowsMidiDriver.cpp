/*
Copyright (C) 2003  The Pentagram Team

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

// Windows Stuff
#ifdef WIN32

#include "WindowsMidiDriver.h"

const MidiDriver::MidiDriverDesc WindowsMidiDriver::desc = 
		MidiDriver::MidiDriverDesc ("Windows", createInstance);

using std::endl;
#include "Q_strcasecmp.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <mmsystem.h>
#include <winbase.h>

WindowsMidiDriver::WindowsMidiDriver() : 
	LowLevelMidiDriver(), dev_num(-1), midi_port(0)
{
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	midi_port2 = 0;
#endif
}

int WindowsMidiDriver::open()
{
	int i;
	// Get Win32 Midi Device num
	//config->value("config/audio/midi/win32_device", dev_num, -1);
	dev_num = -1;
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	int dev_num2 = -2;
#endif

	// List all the midi devices.
	MIDIOUTCAPS caps;
	signed long dev_count = (signed long) midiOutGetNumDevs(); 
	pout << dev_count << " Midi Devices Detected" << endl;
	pout << "Listing midi devices:" << endl;

	for (i = -1; i < dev_count; i++)
	{
		midiOutGetDevCaps ((UINT) i, &caps, sizeof(caps));
		pout << i << ": " << caps.szPname << endl;
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
		if (!Pentagram::Q_strncasecmp(caps.szPname, "SB Live! Synth A", 16)) dev_num = i;
		else if (!Pentagram::Q_strncasecmp(caps.szPname, "SB Live! Synth B", 16)) dev_num2 = i;
#endif
	}

	if (dev_num < -1 || dev_num >= dev_count)
	{
		perr << "Warning Midi device in config is out of range." << endl;
		dev_num = -1;
	}

	midiOutGetDevCaps ((UINT) dev_num, &caps, sizeof(caps));
	pout << "Using device " << dev_num << ": "<< caps.szPname << endl;

	UINT mmsys_err = midiOutOpen (&midi_port, dev_num, 0, 0, 0);

#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	if (dev_num2 != -2 && mmsys_err != MMSYSERR_NOERROR)
	{
		midiOutGetDevCaps ((UINT) dev_num2, &caps, sizeof(caps));
		if (dev_num2 != -2) pout << "Using device " << dev_num2 << ": "<< caps.szPname << endl;
		mmsys_err = midiOutOpen (&midi_port2, dev_num2, 0, 0, 0);
	}
#endif

	if (mmsys_err != MMSYSERR_NOERROR)
	{
		char buf[512];

		mciGetErrorString(mmsys_err, buf, 512);
		perr << "Unable to open device: " << buf << endl;

		return 1;
	}

	// Set Win32 Midi Device num
	//config->set("config/audio/midi/win32_device", dev_num, true);
	
	return 0;
}

void WindowsMidiDriver::close()
{
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	if (midi_port2 != 0) midiOutClose (midi_port2);
	midi_port2 = 0;
#endif
	midiOutClose (midi_port);
	midi_port = 0;
}

void WindowsMidiDriver::send(uint32 message)
{
#ifdef WIN32_USE_DUAL_MIDIDRIVERS
	if (message & 0x1 && midi_port2 != 0) 
		midiOutShortMsg (midi_port2,  message);
	else
		midiOutShortMsg (midi_port,  message);
#else
	midiOutShortMsg (midi_port,  message);
#endif
}

void WindowsMidiDriver::increaseThreadPriority()
{
	SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
}

void WindowsMidiDriver::yield()
{
	Sleep(1);
}

#endif //WIN32

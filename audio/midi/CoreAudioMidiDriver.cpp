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

#ifdef MACOSX

#include "CoreAudioMidiDriver.h"

const MidiDriver::MidiDriverDesc CoreAudioMidiDriver::desc = 
		MidiDriver::MidiDriverDesc ("CoreAudioMidiDriver", createInstance);

CoreAudioMidiDriver::CoreAudioMidiDriver() : 
	au_MusicDevice(0), au_output(0)
{
}

int CoreAudioMidiDriver::open()
{
	if (au_output != NULL)
		return 1;

	int err;
	AudioUnitConnection auconnect;
	ComponentDescription compdesc;
	Component compid;

	// Open the Music Device
	compdesc.componentType = kAudioUnitComponentType;
	compdesc.componentSubType = kAudioUnitSubType_MusicDevice;
	compdesc.componentManufacturer = kAudioUnitID_DLSSynth;
	compdesc.componentFlags = 0;
	compdesc.componentFlagsMask = 0;
	compid = FindNextComponent(NULL, &compdesc);
	au_MusicDevice = (AudioUnit) OpenComponent(compid);

	// open the output unit
	au_output = (AudioUnit) OpenDefaultComponent(kAudioUnitComponentType, kAudioUnitSubType_Output);

	// connect the units
	auconnect.sourceAudioUnit = au_MusicDevice;
	auconnect.sourceOutputNumber = 0;
	auconnect.destInputNumber = 0;
	err =
		AudioUnitSetProperty(au_output, kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, 0,
												 (void *)&auconnect, sizeof(AudioUnitConnection));

	// initialize the units
	AudioUnitInitialize(au_MusicDevice);
	AudioUnitInitialize(au_output);

	// start the output
	AudioOutputUnitStart(au_output);

	return 0;
}

void CoreAudioMidiDriver::close()
{
	// Stop the output
	AudioOutputUnitStop(au_output);

	// Cleanup
	CloseComponent(au_output);
	au_output = 0;
	CloseComponent(au_MusicDevice);
	au_MusicDevice = 0;
}

void CoreAudioMidiDriver::send(uint32 message)
{
	unsigned char first_byte, second_byte, status_byte;
	status_byte = (message & 0x000000FF);
	first_byte = (message & 0x0000FF00) >> 8;
	second_byte = (message & 0x00FF0000) >> 16;

#ifdef COREAUDIO_REVERB_HACK
	if ((status_byte&0xF0) == 0xB0 && first_byte == 0x5b)
		return;
#endif

	MusicDeviceMIDIEvent(au_MusicDevice, status_byte, first_byte, second_byte, 0);
}

void CoreAudioMidiDriver::increaseThreadPriority()
{
//	SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
}

void CoreAudioMidiDriver::yield()
{
//	Sleep(1);
}

#endif //MACOSX

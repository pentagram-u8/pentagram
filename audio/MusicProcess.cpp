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
#include "MusicProcess.h"
#include "GameData.h"
#include "MusicFlex.h"
#include "MidiDriver.h"
#include "XMidiFile.h"
#include "XMidiEventList.h"

using std::memset;

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MusicProcess,Process);

MusicProcess * MusicProcess::the_music_process = 0;

MusicProcess::MusicProcess(MidiDriver *drv) :
	driver(drv), state(MUSIC_NORMAL), current_track(0),
	wanted_track(0)
{
	memset(song_branches, -1, 128*sizeof(int));

	the_music_process = this;
}

MusicProcess::~MusicProcess()
{
	the_music_process = 0;
}

void MusicProcess::playMusic(int track)
{
	if (!driver) return;
	else if (track < 0 || track > 128)
	{
		playMusic(0);
		return;
	}

	// It's already playing and we are not transitioning
	if (current_track == track && state == MUSIC_NORMAL)
	{
		return;
	}
	else if (current_track == 0 || state != MUSIC_NORMAL)
	{
		wanted_track = track;
		state = MUSIC_PLAY_WANTED;
	}
	// We want to do a transition
	else
	{
		const MusicFlex::SongInfo *info = GameData::get_instance()->getMusic()->getSongInfo(current_track);

		uint32 measure = driver->getSequenceCallbackData(0);

		// No transition info, so fast change
		if (!info || !info->transitions[track] || !info->transitions[track][measure])
		{
			current_track = 0;
			playMusic(track);
			return;
		}

		// Get transition info
		int trans = info->transitions[track][measure];
		bool speed_hack = false;

		if (trans < 0)
		{
			trans = (-trans)-1;
			speed_hack = true;
		}
		else
		{
			driver->finishSequence(0);
			trans = trans-1;
		}

		// Now get the transition midi
		int xmidi_index = driver->isFMSynth()?260:258;
		XMidiFile *xmidi = GameData::get_instance()->getMusic()->getXMidi(xmidi_index);
		XMidiEventList *list;

		if (xmidi) list = xmidi->GetEventList(trans);
		else list = 0;

		if (list)
		{
			driver->startSequence(1, list, false, 255, song_branches[track]);
			if (speed_hack) driver->setSequenceSpeed(1,200);
		}
		else driver->finishSequence(1);
		wanted_track = track;

		state = MUSIC_TRANSITION;
	}
}

bool MusicProcess::run(const uint32)
{
	if (!driver) return false;

	switch (state)
	{
	case MUSIC_NORMAL:
		break;

	case MUSIC_TRANSITION:
		if (!driver->isSequencePlaying(1))
		{
			state = MUSIC_PLAY_WANTED;
			driver->pauseSequence(0);
			driver->finishSequence(0);
		}
		break;

	case MUSIC_PLAY_WANTED:
		{
			driver->finishSequence(0);
			driver->finishSequence(1);
		
			XMidiFile *xmidi = 0;
			int xmidi_index = driver->isFMSynth()?wanted_track+128:wanted_track;
			if (wanted_track) xmidi = GameData::get_instance()->getMusic()->getXMidi(xmidi_index);
			if (xmidi)
			{
				XMidiEventList *list = xmidi->GetEventList(0);
				if (song_branches[wanted_track] != -1)
				{
					XMidiEvent *event = list->findBranchEvent(song_branches[wanted_track]);
					if (!event) song_branches[wanted_track] = 0;
				}

				driver->startSequence(0, list, true, 255, song_branches[wanted_track]);
				current_track = wanted_track;
				song_branches[wanted_track]++;
			}
			else
			{
				current_track = wanted_track = 0;
			}
			state = MUSIC_NORMAL;
		}
		break;
	}

	return false;
}

uint32 MusicProcess::I_playMusic(const uint8* args,
										unsigned int /*argsize*/)
{
	ARG_UINT8(song);
	if (the_music_process) the_music_process->playMusic(song&0x7F);
	return 0;
}

uint32 MusicProcess::I_musicStop(const uint8* args,
										unsigned int /*argsize*/)
{
	if (the_music_process) the_music_process->playMusic(0);
	return 0;
}


// protection


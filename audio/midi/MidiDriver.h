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

#ifndef MIDIDRIVER_H_INCLUDED
#define MIDIDRIVER_H_INCLUDED

class XMidiEventList;
class IDataSource;

//! The Basic High Level Pentagram Midi Driver interface.
class	MidiDriver
{
public:
	//! Initialize the driver
	//! \arg sample_rate The sample rate for software synths
	//! \arg stereo Specifies if a software synth must produce stero sound
	//! \return Non zero on failure
	virtual int			initMidiDriver(uint32 sample_rate, bool stereo) = 0;

	//! Destroy the driver
	virtual void		destroyMidiDriver() = 0;

	//! Get the maximum number of playing sequences supported by this this driver
	//! \return The maximum number of playing sequences
	virtual int			maxSequences() = 0;

	//! Set the global volume level
	//! \arg vol The new global volume level (0-255)
	virtual void		setGlobalVolume(int vol) = 0;

	//! Start playing a sequence
	//! \arg seq_num The Sequence number to use.
	//! \arg list The XMidiEventList to play
	//! \arg repeat If true, endlessly repeat the track
	//! \arg activate If true, set the sequence as active
	//! \arg vol The volume level to start playing the sequence at (0-255)
	virtual void		startSequence(int seq_num, XMidiEventList *list, bool repeat, int vol, int branch = -1) = 0;

	//! Finish playing a sequence, and free the data
	//! \arg seq_num The Sequence number to stop
	virtual void		finishSequence(int seq_num) = 0;

	//! Pause the playback of a sequence
	//! \arg seq_num The Sequence number to pause
	virtual void		pauseSequence(int seq_num) = 0;

	//! Unpause the playback of a sequence
	//! \arg seq_num The Sequence number to unpause
	virtual void		unpauseSequence(int seq_num) = 0;

	//! Set the volume of a sequence
	//! \arg seq_num The Sequence number to set the volume for
	//! \arg vol The new volume level for the sequence (0-255)
	virtual void		setSequenceVolume(int seq_num, int vol) = 0;

	//! Set the speed of a sequence
	//! \arg seq_num The Sequence number to change it's speed
	//! \arg speed The new speed for the sequence (percentage)
	virtual void		setSequenceSpeed(int seq_num, int speed) = 0;

	//! Check to see if a sequence is playing (doesn't check for pause state)
	//! \arg seq_num The Sequence number to check
	//! \return true is sequence is playing, false if not playing
	virtual bool		isSequencePlaying(int seq_num) = 0;

	//! Get the callback data for a specified sequence
	//! \arg seq_num The Sequence to get callback data from
	virtual uint32		getSequenceCallbackData(int seq_num) { return 0; }

	//! Is this a Software Synth/Sample producer
	virtual bool		isSampleProducer() { return false; }

	//! Produce Samples when doing Software Synthesizing
	//! \arg samples The buffer to fill with samples
	//! \arg bytes The number of bytes of music to produce
	virtual void		produceSamples(sint16 *samples, uint32 bytes) { }

	//! Is this a FM Synth and should use the Adlib Tracks?
	virtual bool		isFMSynth() { return false; }

	//! Destructor
	virtual ~MidiDriver() { };
};

#endif //MIDIDRIVER_H_INCLUDED

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

#ifndef LOWLEVELMIDIDRIVER_H_INCLUDED
#define LOWLEVELMIDIDRIVER_H_INCLUDED

#include "MidiDriver.h"
#include "XMidiSequenceHandler.h"

class XMidiEventList;
class XMidiSequence;

#include <SDL.h>
#include <SDL_thread.h>

//! Specifies the max number of simultaneous playing sequences supported 
//! \note Only 2 simultaneous playing sequences required for Ultima 8
#define LLMD_NUM_SEQ	4

//! An Abstract implementation of MidiDriver for Simple Low Level support of Midi playback
//! 
//! \note An implementation of LowLevelMidiDriver needs to implement the open(), close() 
//!  and send() functions. Implementing increaseThreadPriority() is optional, however it
//!  is strongly recommended that it is implemented. If it's not implemented, the main
//!  Pentagram thread MAY use too much CPU time and cause timing problems.
//!  Similar, implemeting yield() is a good idea too.
class	LowLevelMidiDriver : public MidiDriver, private XMidiSequenceHandler
{
public:
	LowLevelMidiDriver();
	virtual ~LowLevelMidiDriver();

	// MidiDriver Implementation
	virtual int			initMidiDriver(uint32 sample_rate, bool stereo);
	virtual void		destroyMidiDriver();
	virtual int			maxSequences();
	virtual void		setGlobalVolume(int vol);

	virtual void		startSequence(int seq_num, XMidiEventList *list, bool repeat, int vol, int branch = -1);
	virtual void		finishSequence(int seq_num);
	virtual void		pauseSequence(int seq_num);
	virtual void		unpauseSequence(int seq_num);
	virtual void		setSequenceVolume(int seq_num, int vol);
	virtual void		setSequenceSpeed(int seq_num, int speed);
	virtual bool		isSequencePlaying(int seq_num);
	virtual uint32		getSequenceCallbackData(int seq_num);

	virtual void		produceSamples(sint16 *samples, uint32 bytes);

protected:

	// Will be wanted by software drivers
	uint32			sample_rate;
	bool			stereo;

	//! Open the Midi Device
	//! \return 0 on sucess. Non zero on failure.
	virtual int			open()=0;

	//! Close the Midi Device
	virtual void		close()=0;

	//! Send a message to the Midi Device
	virtual void		send(uint32 message)=0;

	//! Increate the Thread Priority of the Play (current) thread
	virtual void		increaseThreadPriority() { };

	//! Allows LowLevelMidiDrivers to produce samples
	virtual void		lowLevelProduceSamples(sint16 *samples, uint32 num_samples) { };

	//! Yield execution of the current thread
	virtual void		yield() { SDL_Delay(1); }

private:

	struct ComMessage {
		int				type;
		int				sequence;
		union
		{
			struct {
				XMidiEventList	*list;
				bool			repeat;
				int				volume;
				int				branch;
			} play;

			struct {
				bool			paused;
			} pause;

			struct {
				int				level;
			} volume;

			struct {
				int				percentage;
			} speed;

			struct {
				int				code;
			} init_failed;

		} data;
	};

	sint32			initialized;

	// Communications
	bool			playing[LLMD_NUM_SEQ];			// Only set by thread
	sint32			callback_data[LLMD_NUM_SEQ];	// Only set by thread
	ComMessage		message;						// Set by both

	SDL_mutex		*mutex;
	SDL_mutex		*cbmutex;
	sint32			getComMessage(sint32 *val);
	void			lockComMessage();
	void			unlockComMessage();

	// Shared Data
	int				global_volume;
	uint32			xmidi_clock;					// Xmidi clock, returned by getTickCount
	int				chan_locks[16];					// Which seq a chan has been locked by
	int				chan_map[LLMD_NUM_SEQ][16];		// Maps from locked logical chan to phyiscal
	XMidiSequence	*sequences[LLMD_NUM_SEQ];

	// Software Synth only Data
	uint32			total_seconds;					// xmidi_clock = total_seconds*6000 
	uint32			samples_this_second;			//		+ samples_this_second*6000/sample_rate;
	uint32			samples_per_iteration;

	// Thread Based Only Data
	SDL_Thread		*thread;

	// Shared Methods

	//! Play all sequences, handle communications requests
	//! /return true if terminating
	bool			playSequences();

	// Thread Methods
	int				initThreadedSynth();
	void			destroyThreadedSynth();
	static int		threadMain_Static(void *data);
	int				threadMain();

	// Software methods
	int				initSoftwareSynth();
	void			destroySoftwareSynth();

	// XMidiSequenceHandler implementation
	virtual void	sequenceSendEvent(uint16 sequence_id, uint32 message);
	virtual uint32	getTickCount(uint16 sequence_id);
	virtual void	handleCallbackTrigger(uint16 sequence_id, uint8 data);

	int				protectChannel(uint16 sequence_id, int chan, bool protect);
	int				lockChannel(uint16 sequence_id, int chan, bool lock);

	int				unlockAndUnprotectChannel(uint16 sequence_id);

	//! Mute all phyisical channels
	void			muteAllChannels();
};

#endif //LOWLEVELMIDIDRIVER_H_INCLUDED

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
#include "memset_n.h"

#include "LowLevelMidiDriver.h"
#include "XMidiSequence.h"

#define LLMD_MSG_PLAY					1
#define LLMD_MSG_FINISH					2
#define LLMD_MSG_PAUSE					3
#define LLMD_MSG_SET_VOLUME				4
#define LLMD_MSG_SET_SPEED				5

// These are only used by thread
#define LLMD_MSG_THREAD_INIT			-1	
#define LLMD_MSG_THREAD_INIT_FAILED		-2
#define LLMD_MSG_THREAD_EXIT			-3

//#define DO_SMP_TEST

#ifdef DO_SMP_TEST
#define giveinfo() perr << __FILE__ << ":" << __LINE__ << std::endl; perr.flush();
#else
#define giveinfo()
#endif

using std::string;
using std::endl;

LowLevelMidiDriver::LowLevelMidiDriver() :
	initialized(false), mutex(0), cbmutex(0), 
	global_volume(255), thread(0)
{
}

LowLevelMidiDriver::~LowLevelMidiDriver()
{
	// Just kill it
	if (initialized) 
	{
		perr <<	"Warning: Destructing LowLevelMidiDriver and destroyMidiDriver() wasn't called!" << std::endl;
		if (thread) SDL_KillThread(thread);
	}
	thread = 0;
}

//
// MidiDriver API
//

int LowLevelMidiDriver::initMidiDriver(uint32 samp_rate, bool is_stereo)
{
	// Destroy first before re-initing
	if (initialized) destroyMidiDriver();

	string s;

	// Reset the current stream states
	std::memset(sequences, 0, sizeof (XMidiSequence*) * LLMD_NUM_SEQ);
	std::memset(chan_locks, -1, sizeof (sint32) * 16);
	std::memset(chan_map, -1, sizeof (sint32) * LLMD_NUM_SEQ * 16);
	for (int i = 0; i < LLMD_NUM_SEQ; i++) {
		playing[i] = false;
		callback_data[i] = -1;
	}

	mutex = SDL_CreateMutex();
	cbmutex = SDL_CreateMutex();
	thread = 0;
	sample_rate = samp_rate;
	stereo = is_stereo;

	int code = 0;

	if (isSampleProducer()) code = initSoftwareSynth();
	else code = initThreadedSynth();

	if (code)
	{
		perr << "Failed to initialize midi player (code: " << code << ")" << endl;
		SDL_DestroyMutex(mutex);
		SDL_DestroyMutex(cbmutex);
		thread = 0;
		mutex = 0;
		cbmutex = 0;
	}
	else
		initialized = true;

	return code;
}

void LowLevelMidiDriver::destroyMidiDriver()
{
	if (!initialized) return;

	waitTillNoComMessages();

	if (isSampleProducer()) destroySoftwareSynth();
	else destroyThreadedSynth();

	SDL_DestroyMutex(mutex);
	SDL_DestroyMutex(cbmutex);
	cbmutex = 0;
	mutex = 0;
	thread = 0;
	initialized = false;

	giveinfo();
}

int LowLevelMidiDriver::maxSequences()
{
	return LLMD_NUM_SEQ;
}

void LowLevelMidiDriver::startSequence(int seq_num, XMidiEventList *eventlist, bool repeat, int vol, int branch)
{
	if (seq_num >= LLMD_NUM_SEQ || seq_num < 0) return;

	if (!initialized)
		return;

	eventlist->incerementCounter();

	ComMessage message(LLMD_MSG_PLAY);
	message.sequence = seq_num;
	message.data.play.list = eventlist;
	message.data.play.repeat = repeat;
	message.data.play.volume = vol;
	message.data.play.branch = branch;

	sendComMessage(message);
}

void LowLevelMidiDriver::finishSequence(int seq_num)
{
	if (seq_num >= LLMD_NUM_SEQ || seq_num < 0) return;
	if (!initialized) return;

	ComMessage message(LLMD_MSG_FINISH);
	message.sequence = seq_num;
	messages.push(message);

	sendComMessage(message);
}

void LowLevelMidiDriver::setSequenceVolume(int seq_num, int vol)
{
	if (seq_num >= LLMD_NUM_SEQ || seq_num < 0) return;
	if (vol < 0 || vol > 255) return;
	if (!initialized) return;

	ComMessage message(LLMD_MSG_SET_VOLUME);
	message.sequence = seq_num;
	message.data.volume.level = vol;
	messages.push(message);

	sendComMessage(message);
}

void LowLevelMidiDriver::setGlobalVolume(int vol)
{
	if (vol < 0 || vol > 255) return;
	if (!initialized) return;

	ComMessage message(LLMD_MSG_SET_VOLUME);
	message.sequence = -1;
	message.data.volume.level = vol;

	sendComMessage(message);
}

void LowLevelMidiDriver::setSequenceSpeed(int seq_num, int speed)
{
	if (seq_num >= LLMD_NUM_SEQ || seq_num < 0) return;
	if (speed < 0) return;
	if (!initialized) return;

	ComMessage message(LLMD_MSG_SET_SPEED);
	message.sequence = seq_num;
	message.data.speed.percentage = speed;

	sendComMessage(message);
}

bool LowLevelMidiDriver::isSequencePlaying(int seq_num)
{
	if (seq_num >= LLMD_NUM_SEQ || seq_num < 0) return false;

	waitTillNoComMessages();
	
	lockComMessage();
	bool ret = playing[seq_num];
	unlockComMessage();

	return ret;
}

void LowLevelMidiDriver::pauseSequence(int seq_num)
{
	if (seq_num >= LLMD_NUM_SEQ || seq_num < 0) return;
	if (!initialized) return;

	ComMessage message(LLMD_MSG_PAUSE);
	message.sequence = seq_num;
	message.data.pause.paused = true;

	sendComMessage(message);
}

void LowLevelMidiDriver::unpauseSequence(int seq_num)
{
	if (seq_num >= LLMD_NUM_SEQ || seq_num < 0) return;
	if (!initialized) return;

	ComMessage message(LLMD_MSG_PAUSE);
	message.sequence = seq_num;
	message.data.pause.paused = false;

	sendComMessage(message);
}

uint32 LowLevelMidiDriver::getSequenceCallbackData(int seq_num)
{
	if (seq_num >= LLMD_NUM_SEQ || seq_num < 0) return 0;

	SDL_mutexP(cbmutex);
	uint32 ret = callback_data[seq_num];
	SDL_mutexV(cbmutex);

	return ret;
}

//
// Communications
// 

sint32 LowLevelMidiDriver::peekComMessageType()
{
	lockComMessage();
	sint32 ret = 0;
	if (messages.size()) ret = messages.front().type;
	unlockComMessage();
	return ret;
}

void LowLevelMidiDriver::sendComMessage(ComMessage& message)
{
	lockComMessage();
	messages.push(message);
	unlockComMessage();
}

void LowLevelMidiDriver::waitTillNoComMessages()
{
	while (peekComMessageType()) yield ();
}

void LowLevelMidiDriver::lockComMessage()
{
	SDL_mutexP(mutex);
}

void LowLevelMidiDriver::unlockComMessage()
{
	SDL_mutexV(mutex);
}

//
// Thread Stuff
//

int LowLevelMidiDriver::initThreadedSynth()
{
	// Create the thread
	giveinfo();
	
	ComMessage message(LLMD_MSG_THREAD_INIT);
	sendComMessage(message);

	thread = SDL_CreateThread (threadMain_Static, static_cast<void*>(this));

	while (peekComMessageType() == LLMD_MSG_THREAD_INIT) 
		yield ();

	int code = 0;

	lockComMessage();
	{
		while (!messages.empty()) 
		{
			if (messages.front().type == LLMD_MSG_THREAD_INIT_FAILED)
				code = messages.front().data.init_failed.code;
			messages.pop();
		}
	}
	unlockComMessage();

	return code;
}

void LowLevelMidiDriver::destroyThreadedSynth()
{
	ComMessage message(LLMD_MSG_THREAD_EXIT);
	sendComMessage(message);

	int count = 0;
	
	while (count < 100)
	{
		giveinfo();
		// Wait 1 MS before trying again
		if (peekComMessageType() == LLMD_MSG_THREAD_EXIT)
			yield ();
		else break;
		
		count++;
	}

	// We waited a while and it still didn't terminate
	if (count == 100 && peekComMessageType() == LLMD_MSG_THREAD_EXIT)
		SDL_KillThread (thread);

	lockComMessage();
	{
		// Get rid of all the messages
		while (!messages.empty()) messages.pop();
	}
	unlockComMessage();

}

int LowLevelMidiDriver::threadMain_Static(void *data)
{
	giveinfo();
	LowLevelMidiDriver *ptr=static_cast<LowLevelMidiDriver *>(data);
	giveinfo();
	return ptr->threadMain();
}

int LowLevelMidiDriver::threadMain()
{
	giveinfo();

	// Open the device
	int code = open();

	lockComMessage();
	{
		// Pop all the messages
		while(!messages.empty()) messages.pop();

		// If we have an error code, push it to the message queue then return
		// else we are ok to go
		if (code)
		{
			ComMessage message(LLMD_MSG_THREAD_INIT_FAILED);
			message.data.init_failed.code = code;
			messages.push(message);
		}
	}
	unlockComMessage();

	if (code) return code;

	// Increase the thread priority, IF possible
	increaseThreadPriority();

	// Execute the play loop
	for (;;)
	{
		xmidi_clock = SDL_GetTicks()*6;
		if (playSequences()) break;
		yield();
	}

	// Close the device
	close();

	lockComMessage();
	{
		// Pop all messages
		while(!messages.empty()) messages.pop();
	}
	unlockComMessage();

	return 0;
}

//
// Software Synth Stuff
//

int LowLevelMidiDriver::initSoftwareSynth()
{
	// Open the device
	int ret = open();

	// Uh oh, failed
	if (ret) return 1;

	// Now setup all our crap
	total_seconds = 0;
	samples_this_second = 0;

	// This value doesn't 'really' matter all that much
	// Smaller values are more accurate (more iterations)

	if (sample_rate == 11025)
		samples_per_iteration = 49;					// exactly 225 times a second
	if (sample_rate == 22050)
		samples_per_iteration = 98;					// exactly 225 times a second
	else if (sample_rate == 44100)
		samples_per_iteration = 147;				// exactly 300 times a second
	else 
	{
		samples_per_iteration = sample_rate/480;	// Approx 480 times a second

		// Try to see if it can be 240 times/second
		if (!(samples_per_iteration&1)) samples_per_iteration>>=1;
		// Try to see if it can be 120 times/second
		if (!(samples_per_iteration&1)) samples_per_iteration>>=1;
	}

	return 0;
}

void LowLevelMidiDriver::destroySoftwareSynth()
{
	close();
}

void LowLevelMidiDriver::produceSamples(sint16 *samples, uint32 bytes)
{
	// Hey, we're not supposed to be here
	if (!initialized) return;

	int stereo_mult = 1;
	if (stereo) stereo_mult = 2;

	uint32 num_samples = bytes/(2*stereo_mult);

	// Now, do the note playing iterations
	while (num_samples > 0)
	{
		uint32 samples_to_produce = samples_per_iteration;
		if (samples_to_produce > num_samples) samples_to_produce = num_samples;

		// Increment the timing counter(s)
		samples_this_second += samples_to_produce;
		while (samples_this_second > sample_rate)
		{
			total_seconds++;
			samples_this_second -= sample_rate;
		}

		// Calc Xmidi Clock
		xmidi_clock = (total_seconds*6000) + (samples_this_second*6000)/sample_rate;

		// We don't care about the return code
		playSequences();

		// Produce the samples
		lowLevelProduceSamples(samples, samples_to_produce);

		// Increment the counters
		samples += samples_to_produce*stereo_mult;
        num_samples -= samples_to_produce;
	}
}

//
// Shared Stuff
//

bool LowLevelMidiDriver::playSequences ()
{
	int i;

	// Play all notes, from all sequences
	for (i = 0; i < LLMD_NUM_SEQ; i++)
	{
		int seq = i;

		while (sequences[seq] && !peekComMessageType())
		{
			sint32 time_till_next = sequences[seq]->playEvent();

			if (time_till_next > 0) break;
			else if (time_till_next == -1)
			{
				delete sequences[seq]; 
				sequences[seq] = 0;
				lockComMessage();
					playing[seq] = false;
				unlockComMessage();
			}
		}
	}

	// Did we get issued a music command?
	lockComMessage();
	{
		while (!messages.empty()) 
		{
			ComMessage &message = messages.front();

			switch (message.type)
			{
			case LLMD_MSG_FINISH:
				{
					delete sequences[message.sequence]; 
					sequences[message.sequence] = 0;
					playing[message.sequence] = false;
					callback_data[message.sequence] = -1;
					unlockAndUnprotectChannel(message.sequence);
				}
				break;

			case LLMD_MSG_THREAD_EXIT:
				{
					for (i = 0; i < LLMD_NUM_SEQ; i++)
					{
						delete sequences[i]; 
						sequences[i] = 0;
						playing[i] = false;
						callback_data[i] = -1;
						unlockAndUnprotectChannel(i);
					}
				}
				unlockComMessage();
				return true;

			case LLMD_MSG_SET_VOLUME:
				{
					if (message.sequence == -1)
					{
						global_volume = message.data.volume.level;
						for (i = 0; i < LLMD_NUM_SEQ; i++)
							if (sequences[i])
								sequences[i]->setVolume(sequences[i]->getVolume());
					}
					else if (sequences[message.sequence]) 
						sequences[message.sequence]->setVolume(message.data.volume.level);
				}
				break;

			case LLMD_MSG_SET_SPEED:
				{
					if (sequences[message.sequence]) 
						sequences[message.sequence]->setSpeed(message.data.speed.percentage);
				}
				break;

			case LLMD_MSG_PAUSE:
				{
					if (sequences[message.sequence]) 
					{
						if (!message.data.pause.paused) sequences[message.sequence]->unpause();
						else sequences[message.sequence]->pause();
					}
				}
				break;

			case LLMD_MSG_PLAY:
				{
					// Kill the previous stream
					delete sequences[message.sequence]; 
					sequences[message.sequence] = 0;
					playing[message.sequence] = false;
					callback_data[message.sequence] = -1;
					unlockAndUnprotectChannel(message.sequence);

					giveinfo();

					if (message.data.play.list) 
					{
						sequences[message.sequence] = new XMidiSequence(
											this, 
											message.sequence,
											message.data.play.list, 
											message.data.play.repeat, 
											message.data.play.volume,
											message.data.play.branch);

						playing[message.sequence] = true;

						uint16 mask = sequences[message.sequence]->getChanMask();

						// Allocate some channels
						/*
						for (i = 0; i < 16; i++)
							if (mask & (1<<i)) allocateChannel(message.sequence, i);
						*/
					}

				}
				break;

				// Uh we have no idea what it is
			default:
				break;
			}

			// Pop it
			messages.pop();
		}
	}
	unlockComMessage();

	return false;
}

void LowLevelMidiDriver::sequenceSendEvent(uint16 sequence_id, uint32 message)
{
	int log_chan = message & 0xF;
	message &= 0xFFFFFFF0;	// Strip the channel number

	// Controller handling
	if ((message & 0x00F0) == (MIDI_STATUS_CONTROLLER << 4))
	{
		// Screw around with volume
		if ((message & 0xFF00) == (7 << 8))
		{
			int vol = (message >> 16) & 0xFF;
			message &= 0x00FFFF;
			// Global volume
			vol = (vol * global_volume) / 0xFF;
			message |= vol << 16;
		}
		else if ((message & 0xFF00) == (XMIDI_CONTROLLER_CHAN_LOCK << 8))
		{
			lockChannel(sequence_id, log_chan, ((message>>16)&0xFF)>=64);
			return;
		}
		else if ((message & 0xFF00) == (XMIDI_CONTROLLER_CHAN_LOCK_PROT << 8))
		{
			protectChannel(sequence_id, log_chan, ((message>>16)&0xFF)>=64);
			return;
		}
	}

	// Ok, get the physical channel number from the logical.
    int phys_chan = chan_map[sequence_id][log_chan];

	if (phys_chan == -2) return;
	else if (phys_chan < 0) phys_chan = log_chan;

	send(message|phys_chan);
}

uint32 LowLevelMidiDriver::getTickCount(uint16 sequence_id)
{
	return xmidi_clock;
}

void LowLevelMidiDriver::handleCallbackTrigger(uint16 sequence_id, uint8 data)
{
	SDL_mutexP(cbmutex);
	callback_data[sequence_id] = data;
	SDL_mutexV(cbmutex);
}

int LowLevelMidiDriver::protectChannel(uint16 sequence_id, int chan, bool protect)
{
	// Unprotect the channel
	if (!protect) 
	{
		chan_locks[chan] = -1;
		chan_map[sequence_id][chan] = -1;
	}
	// Protect the channel (if required)
	else if (chan_locks[chan] != -2)
	{
		// First check to see if the channel has been locked by something
		int relock_sid = -1;
		int relock_log = -1;
		if (chan_locks[chan] != -1)
		{
			relock_sid = chan_locks[chan];

			// It has, so what we want to do is unlock the channel, then 
			for (int c = 0; c < 16; c++)
			{
				if (chan_map[relock_sid][c] == chan) 
				{
					relock_log = c;
					break;
				}
			}

			// Release the previous lock
			lockChannel(relock_sid, relock_log, false);
		}

		// Now protect the channel
		chan_locks[chan] = -2;
		chan_map[sequence_id][chan] = -3;

		// And attempt to get the other a new channel to lock
		if (relock_sid != -1) 
			lockChannel(relock_sid, relock_log, true);
	}

	return 0;
}

int LowLevelMidiDriver::lockChannel(uint16 sequence_id, int chan, bool lock)
{
	// When locking, we want to get the highest chan number with the lowest 
	// number of notes playing, that aren't already locked and don't have
	// protection
	if (lock)
	{
		int notes_on[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		int s, c, phys;

		for (s = 0; s < LLMD_NUM_SEQ; s++)
		{
			if (sequences[s]) for (c = 0; c < 16; c++)
			{
				phys = chan_map[s][c];
				if (phys == -1) phys = c;
				if (phys != -2)
					notes_on[phys] += sequences[s]->countNotesOn(c);
			}
		}

		phys = -1;
		int prev_max = 128;
		for (c = 0; c < 16; c++)
		{
			// Protected or locked
			if (chan_locks[c] != -1) continue;
			
			if (notes_on[c] <= prev_max) 
			{
				prev_max = notes_on[c];
				phys = c;
			}
		}

		// Oh no, no channel to lock!
		if (phys == -1) return -1;

		// Now tell everyone that they lost their channel
		for (s = 0; s < LLMD_NUM_SEQ; s++)
		{
			// Make sure they are mapping defualt
			if (sequences[s] && chan_map[s][phys] == -1) 
			{
				sequences[s]->loseChannel(phys);
				chan_map[s][phys] = -2;	// Can't use it
			}
		}

		// We are losing our old logical channel too
		if (phys != chan) sequences[sequence_id]->loseChannel(chan);

		// Ok, got our channel
		chan_map[sequence_id][chan] = phys;
		chan_locks[phys] = sequence_id;
		sequences[sequence_id]->gainChannel(chan);
	}
	// Unlock the channel
	else
	{
		int phys = chan_map[sequence_id][chan];

		// Not locked
		if (phys < 0) return -1;

		// First, we'll lose our logical channel
		if (sequences[sequence_id]) 
			sequences[sequence_id]->loseChannel(chan);

		// Now unlock it
		chan_map[sequence_id][chan] = -1;
		chan_locks[phys] = -1;

		// Gain our logical channel back
		if (phys != chan && sequences[sequence_id]) 
			sequences[sequence_id]->gainChannel(chan);

		// Now let everyone gain their channel that we stole
		for (int s = 0; s < LLMD_NUM_SEQ; s++)
		{
			// Make sure they are mapping defualt
			if (sequences[s] && chan_map[s][phys] == -2) 
			{
				chan_map[s][phys] = -1;
				sequences[s]->gainChannel(phys);
			}
		}
	}

	return 0;
}

int LowLevelMidiDriver::unlockAndUnprotectChannel(uint16 sequence_id)
{
	// For each channel
	for (int c = 0; c < 16; c++)
	{
		int phys = chan_map[sequence_id][c];

		// Doesn't need anything done to it
		if (phys != -3) continue;

		// We are protecting
		if (phys == -3)
		{
			protectChannel(sequence_id, c, false);
		}
		// We are locking
		else
		{
			lockChannel(sequence_id, c, false);
		}
	}
	return 0;
}


// Protection

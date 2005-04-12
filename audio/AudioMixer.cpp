/*
Copyright (C) 2005 The Pentagram team

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
#include "AudioMixer.h"
#include "SettingManager.h"
#include "Kernel.h"

#include "AudioProcess.h"
#include "MusicProcess.h"
#include "AudioChannel.h"

#include "MidiDriver.h"
#if defined(WIN32) && !defined(UNDER_CE)
#include "WindowsMidiDriver.h"
#endif
#ifdef MACOSX
#include "CoreAudioMidiDriver.h"
#endif
#ifdef USE_FMOPL_MIDI
#include "FMOplMidiDriver.h"
#endif
#ifdef USE_TIMIDITY_MIDI
#include "TimidityMidiDriver.h"
#endif
#ifdef USE_ALSA_MIDI
#include "ALSAMidiDriver.h"
#endif
#ifdef UNIX
#include "UnixSeqMidiDriver.h"
#endif


namespace Pentagram {

AudioMixer *AudioMixer::the_audio_mixer = 0;

AudioMixer::AudioMixer(int sample_rate_, bool stereo_, int num_channels_) : 
		audio_ok(false), 
		sample_rate(sample_rate_), stereo(stereo_),
		midi_driver(0), midi_volume(255),
		num_channels(num_channels_), channels(0)
{
	the_audio_mixer = this;

	pout << "Initializing AudioMixer" << std::endl;

	SDL_AudioSpec desired, obtained;

	desired.format = AUDIO_S16SYS;
	desired.freq = sample_rate_;
	desired.channels = stereo_?2:1;
	desired.samples = 1024;
	desired.callback = sdlAudioCallback;
	desired.userdata = reinterpret_cast<void*>(this);

#ifdef UNDER_CE
	desired.freq = 11025;
	desired.channels = 1;
#endif

	// Open SDL Audio (even though we may not need it)
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	int ret = SDL_OpenAudio(&desired, &obtained);
	audio_ok = (ret == 0);

	if (audio_ok) {
		// Lock the audio
		Lock();

		sample_rate = obtained.freq;
		stereo = obtained.channels == 2;

		init_midi();

		channels = new AudioChannel*[num_channels];
		for (int i=0;i<num_channels;i++)
			channels[i] = new AudioChannel(sample_rate,stereo);

		// Unlock it
		Unlock();

		// GO GO GO!
		SDL_PauseAudio(0);
	}

	Kernel *kernel = Kernel::get_instance();

	// Create the Audio Process
	Process *ap = new AudioProcess();
	kernel->addProcess(ap);

	// Create the Music Process
	Process *mp = new MusicProcess(midi_driver);
	kernel->addProcess(mp);
}

AudioMixer::~AudioMixer(void)
{
	the_audio_mixer = 0;

	SDL_CloseAudio();
	if (midi_driver)  midi_driver->destroyMidiDriver();
	delete midi_driver;
	midi_driver = 0;

	if (channels) for (int i=0;i<num_channels;i++) delete channels[i];
	delete [] channels;
}

void AudioMixer::Lock()
{
	SDL_LockAudio();
}

void AudioMixer::Unlock()
{
	SDL_UnlockAudio();
}

void AudioMixer::reset()
{
	if (!audio_ok) return;

	Lock();

	if (midi_driver) {
		for (int i = 0; i < midi_driver->maxSequences(); i++) {
			midi_driver->finishSequence(i);
		}
	}

	if (channels) for (int i=0;i<num_channels;i++) channels[i]->stop();

	Unlock();
}

int AudioMixer::playSample(AudioSample *sample, int loop, int priority, bool paused, uint32 pitch_shift_, int lvol, int rvol)
{
	if (!audio_ok || !channels) return -1;

	int lowest = -1;
	int lowprior = 65536;

	// Lock the audio
	Lock();

	int i;
	for (i=0;i<num_channels;i++)
	{
		if (!channels[i]->isPlaying()) {
			lowest = i;
			break;
		}
		else if (channels[i]->getPriority() < priority) {
			lowprior = channels[i]->getPriority();
			lowest = i;
		}
	}

	if (i != num_channels || lowprior < priority)
		channels[lowest]->playSample(sample,loop,priority,paused,pitch_shift_,lvol,rvol);
	else 
		lowest = -1;

	// Unlock
	Unlock();

	return lowest;
}

bool AudioMixer::isPlaying(int chan)
{
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return 0;

	Lock();

		bool playing = channels[chan]->isPlaying();

	Unlock();

	return playing;
}

void AudioMixer::stopSample(int chan)
{
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return;

	Lock();

		channels[chan]->stop();

	Unlock();
}

void AudioMixer::setPaused(int chan, bool paused)
{
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return;

	Lock();

		channels[chan]->setPaused(paused);

	Unlock();
}

bool AudioMixer::isPaused(int chan)
{
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return false;

	Lock();

		bool ret = channels[chan]->isPaused();

	Unlock();

	return ret;
}

void AudioMixer::setVolume(int chan, int lvol, int rvol)
{
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return;

	Lock();

		channels[chan]->setVolume(lvol,rvol);

	Unlock();
}

void AudioMixer::getVolume(int chan, int &lvol, int &rvol)
{
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return;

	Lock();

		channels[chan]->getVolume(lvol,rvol);

	Unlock();
}
	

void AudioMixer::sdlAudioCallback(void *userdata, Uint8 *stream, int len)
{
	AudioMixer *mixer = reinterpret_cast<AudioMixer *>(userdata);

	mixer->MixAudio(reinterpret_cast<sint16*>(stream), len);
}

void AudioMixer::MixAudio(sint16 *stream, uint32 bytes)
{
	if (!audio_ok) return;

	if (midi_driver && midi_driver->isSampleProducer())
		midi_driver->produceSamples(stream, bytes);

	if (channels) for (int i=0;i<num_channels;i++)
		if (channels[i]->isPlaying()) channels[i]->resampleAndMix(stream,bytes);
}

void AudioMixer::init_midi()
{
	MidiDriver * new_driver = 0;
	pout << "Initializing MidiDriver" << std::endl;

	SettingManager *settingman = SettingManager::get_instance();

	std::vector<const MidiDriver::MidiDriverDesc*>	midi_drivers;

	// Now, add the drivers in order of priority.
	// Do OS Native drivers first, then Timidity, then FMOpl

#ifdef MACOSX
	midi_drivers.push_back(CoreAudioMidiDriver::getDesc());
#endif
#if defined(WIN32) && !defined(UNDER_CE)
	midi_drivers.push_back(WindowsMidiDriver::getDesc());
#endif
#ifdef USE_TIMIDITY_MIDI
	midi_drivers.push_back(TimidityMidiDriver::getDesc());
#endif
#ifdef USE_FMOPL_MIDI
	midi_drivers.push_back(FMOplMidiDriver::getDesc());
#endif
#ifdef USE_ALSA_MIDI
	midi_drivers.push_back(ALSAMidiDriver::getDesc());
#endif
#ifdef UNIX
	midi_drivers.push_back(UnixSeqMidiDriver::getDesc());
#endif

	// First thing attempt to find the Midi driver as specified in the config
	std::string desired_driver;
	settingman->setDefault("midi_driver", "default");
	settingman->get("midi_driver", desired_driver);
	const char * drv = desired_driver.c_str();

	// Has the config file specified disabled midi?
	if (audio_ok && Pentagram::strcasecmp(drv, "disabled"))
	{
		std::vector<const MidiDriver::MidiDriverDesc*>::iterator it;

		// Ok, it hasn't so search for the driver
		for (it = midi_drivers.begin(); it < midi_drivers.end(); it++) {

			// Found it (case insensitive)
			if (!Pentagram::strcasecmp(drv, (*it)->name)) {

				pout << "Trying config specified Midi driver: `" << (*it)->name << "'" << std::endl;

				new_driver = (*it)->createInstance();
				if (new_driver) {

					if (new_driver->initMidiDriver(sample_rate,stereo)) {
						delete new_driver;
						new_driver = 0; 
					} 
				}
			}
		}

		// Uh oh, we didn't manage to load a driver! 
		// Search for the first working one
		if (!new_driver) for (it = midi_drivers.begin(); it < midi_drivers.end(); it++) {

			pout << "Trying: `" << (*it)->name << "'" << std::endl;

			new_driver = (*it)->createInstance();
			if (new_driver) {

				// Got it
				if (!new_driver->initMidiDriver(sample_rate,stereo)) 
					break;

				// Oh well, try the next one
				delete new_driver;
				new_driver = 0; 
			}
		}
	}
	else
	{
		new_driver = 0; // silence :-)
	}

	// If the driver is a 'sample' producer we need to hook it to SDL
	if (new_driver)
	{
		new_driver->setGlobalVolume(midi_volume);

		midi_driver = new_driver;
	}
}

};

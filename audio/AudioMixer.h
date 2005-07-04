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
#ifndef AUDIOMIXER_H_INCLUDED
#define AUDIOMIXER_H_INCLUDED

class MidiDriver;

namespace Pentagram {
class AudioChannel;
class AudioSample;

class AudioMixer
{
public:
	AudioMixer(int sample_rate, bool stereo, int num_channels);
	~AudioMixer(void);

	MidiDriver* getMidiDriver() const { return midi_driver; }

	static AudioMixer*	get_instance() { return the_audio_mixer; }

	void			reset();
	void			createProcesses();

	int				playSample(AudioSample *sample, int loop, int priority, bool paused, uint32 pitch_shift, int lvol, int rvol);
	bool			isPlaying(int chan);
	void			stopSample(int chan);
	
	void			setPaused(int chan, bool paused);
	bool			isPaused(int chan);

	void			setVolume(int chan, int lvol, int rvol);
	void			getVolume(int chan, int &lvol, int &rvol);

	void			openMidiOutput();
	void			closeMidiOutput();

private:
	bool			audio_ok;
	uint32			sample_rate;
	bool			stereo;
	MidiDriver *	midi_driver;
	int				midi_volume;

	int				num_channels;
	AudioChannel	**channels;

	void			init_midi();
	static void		sdlAudioCallback(void *userdata, uint8 *stream, int len);

	void			MixAudio(sint16 *stream, uint32 bytes);

	static AudioMixer* the_audio_mixer;

	void			Lock();
	void			Unlock();
};

};

#endif //AUDIOMIXER_H_INCLUDED

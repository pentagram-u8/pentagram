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
#include "AudioProcess.h"
#include "intrinsics.h"
#include "Object.h"

#include "GameData.h"
#include "SoundFlex.h"
#include "SpeechFlex.h"
#include "AudioSample.h"
#include "AudioMixer.h"

#include "IDataSource.h"
#include "ODataSource.h"

using Pentagram::AudioSample;
using Pentagram::AudioMixer;

// p_dynamic_class stuff 
DEFINE_RUNTIME_CLASSTYPE_CODE(AudioProcess,Process);

AudioProcess * AudioProcess::the_audio_process = 0;

AudioProcess::AudioProcess(void)
{
	the_audio_process = this;
	type = 1; // persistent
}

AudioProcess::~AudioProcess(void)
{
	the_audio_process = 0;
}

bool AudioProcess::run(const uint32)
{
	AudioMixer *mixer = AudioMixer::get_instance();

	// Update the channels
	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ) {
		bool finished = false;
		if (!mixer->isPlaying(it->channel)) {
			if (it->sfxnum == -1)
				finished = !continueSpeech(*it);
			else
				finished = true;
		}

		if (finished)
			it = sample_info.erase(it);
		else
			++it;
	}

	return false;
}

bool AudioProcess::continueSpeech(SampleInfo& si)
{
	assert(si.sfxnum == -1);

	SpeechFlex *speechflex;
	speechflex = GameData::get_instance()->getSpeechFlex(si.priority);
	if (!speechflex) return false;

	if (si.curspeech_end >= si.barked.size()) return false;

	si.curspeech_start = si.curspeech_end;
	int index = speechflex->getIndexForPhrase(si.barked,
											  si.curspeech_start,
											  si.curspeech_end);
	if (!index) return false;

	AudioSample *sample = speechflex->getSample(index);
	if (!sample) return false;

	// hack to prevent playSample from deleting 'si'
	si.channel = -1;
	int channel = playSample(sample,200,0);
	if (channel == -1)
		return false;

	si.channel = channel;
	return true;
}


void AudioProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);

	ods->write1(sample_info.size());

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ++it) {
		ods->write2(it->sfxnum);
		ods->write2(it->priority);
		ods->write2(it->objid);
		ods->write2(it->loops);

		if (it->sfxnum == -1)	// Speech
		{
			ods->write4(it->barked.size());
			ods->write(it->barked.c_str(),it->barked.size());
		}
	}
}

bool AudioProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	uint32 count = ids->read1();

	while (count--) {
		sint16 sfxnum = ids->read2();
		sint16 priority = ids->read2();
		sint16 objid = ids->read2();
		sint16 loops = ids->read2();


		if (sfxnum != -1)	// SFX
			playSFX(sfxnum,priority,objid,loops);

		else {					// Speech
			uint32 slen = ids->read4();

			char* buf = new char[slen+1];
			ids->read(buf, slen);
			buf[slen] = 0;
			std::string text = buf;
			delete[] buf;

			playSpeech(text,priority,objid);
		}
	}

	return true;
}

int AudioProcess::playSample(AudioSample* sample, int priority, int loops)
{
	AudioMixer *mixer = AudioMixer::get_instance();
	int channel = mixer->playSample(sample,loops,priority);

	if (channel == -1) return channel;

	// Erase old sample using channel (if any)
	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ) {
		if (it->channel == channel) {
			it = sample_info.erase(it);
		}
		else {
			++it;
		}
	}

	return channel;
}

void AudioProcess::playSFX(int sfxnum, int priority, ObjId objid, int loops)
{
	//con.Printf("playSFX(%i, %i, 0x%X, %i)\n", sfxnum, priority, objid, loops);

	SoundFlex *soundflx= GameData::get_instance()->getSoundFlex();
	
	AudioMixer *mixer = AudioMixer::get_instance();

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ) {
		if (it->sfxnum == sfxnum && it->objid == objid && it->loops == loops) {

			// Exactly the same (and playing) so just return
			//if (it->priority == priority) 
			if (mixer->isPlaying(it->channel))
			{
				pout << "Sound already playing" << std::endl;
				return;
			}
			else {
				it = sample_info.erase(it);
				continue;
			}
		}

		++it;
	}

	AudioSample *sample = soundflx->getSample(sfxnum);
	if (!sample) return;

	int channel = playSample(sample,priority,loops);
	if (channel == -1) return;

	// Update list
	sample_info.push_back(SampleInfo(sfxnum,priority,objid,loops,channel));
}

void AudioProcess::stopSFX(int sfxnum, ObjId objid)
{
	//con.Printf("stopSFX(%i, 0x%X)\n", sfxnum, objid);

	AudioMixer *mixer = AudioMixer::get_instance();

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ) {
		if (it->sfxnum == sfxnum && it->objid == objid) {
			if (mixer->isPlaying(it->channel)) mixer->stopSample(it->channel);
			it = sample_info.erase(it);
		}
		else {
			++it;
		}
	}
}

bool AudioProcess::isSFXPlaying(int sfxnum)
{
	con.Printf("isSFXPlaying(%i)\n", sfxnum);

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ++it) {
		if (it->sfxnum == sfxnum)
			return true;
	}

	return false;
}

//
// Speech
//

bool AudioProcess::playSpeech(std::string &barked, int shapenum, ObjId objid)
{
	SpeechFlex *speechflex = GameData::get_instance()->getSpeechFlex(shapenum);

	if (!speechflex) return false;

	AudioMixer *mixer = AudioMixer::get_instance();

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end();) {

		if (it->sfxnum == -1 && it->barked == barked && 
			it->priority == shapenum && it->objid == objid) {

			if (mixer->isPlaying(it->channel)) {
				pout << "Speech already playing" << std::endl;
				return true;
			} 
			else {
				it = sample_info.erase(it);
				continue;
			}
		}

		++it;
	}

	uint32 speech_start = 0;
	uint32 speech_end;
	int index = speechflex->getIndexForPhrase(barked,speech_start,speech_end);
	if (!index) return false;

	AudioSample *sample = speechflex->getSample(index);
	if (!sample) return false;

	int channel = playSample(sample,200,0);

	if (channel == -1) return false;

	// Update list
	sample_info.push_back(SampleInfo(barked,shapenum,objid,channel,
									 speech_start,speech_end));

	return true;
}

void AudioProcess::stopSpeech(std::string &barked, int shapenum, ObjId objid)
{
	AudioMixer *mixer = AudioMixer::get_instance();

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ) {
		if (it->sfxnum == -1 && it->priority == shapenum &&
			it->objid == objid && it->barked == barked)
		{
			if (mixer->isPlaying(it->channel)) mixer->stopSample(it->channel);
			it = sample_info.erase(it);
		}
		else {
			++it;
		}
	}
}

bool AudioProcess::isSpeechPlaying(std::string &barked, int shapenum)
{
	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ++it) {
		if (it->sfxnum == -1 && it->priority == shapenum &&
			it->barked == barked)
		{
			return true;
		}
	}

	return false;
}


//
// Intrinsics
//

uint32 AudioProcess::I_playSFX(const uint8* args, unsigned int argsize)
{
	ARG_SINT16(sfxnum);

	sint16 priority = 0x60;
	if (argsize >= 4) {
		ARG_SINT16(priority_);
		priority = priority_;
	}

	ObjId objid = 0;
	if (argsize == 6) {
		ARG_OBJID(objid_);
		objid = objid_;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if(ap) ap->playSFX(sfxnum,priority,objid,0);
	else perr << "Error: No AudioProcess" << std::endl;

	return 0;
}

uint32 AudioProcess::I_playAmbientSFX(const uint8* args, unsigned int argsize)
{
	ARG_SINT16(sfxnum);

	sint16 priority = 0x60;
	if (argsize >= 4) {
		ARG_SINT16(priority_);
		priority = priority_;
	}

	ObjId objid = 0;
	if (argsize == 6) {
		ARG_OBJID(objid_);
		objid = objid_;
	}

//	con.Printf("playAmbientSFX(%i, %i, 0x%X)\n", sfxnum, priority, objID);
	AudioProcess *ap = AudioProcess::get_instance();
	if(ap) ap->playSFX(sfxnum,priority,objid,-1);
	else perr << "Error: No AudioProcess" << std::endl;

	return 4;
}

uint32 AudioProcess::I_isSFXPlaying(const uint8* args, unsigned int argsize)
{
	ARG_SINT16(sfxnum);

	AudioProcess *ap = AudioProcess::get_instance();
	if(ap) return ap->isSFXPlaying(sfxnum);
	else perr << "Error: No AudioProcess" << std::endl;
	return 0;
}

uint32 AudioProcess::I_setVolumeSFX(const uint8* args, unsigned int /*argsize*/)
{
	// Sets volume for last played instances of sfxnum (???)
	ARG_SINT16(sfxnum);
	ARG_SINT16(volume);

	con.Printf("setVolumeSFX(%i, %i)\n", sfxnum, volume);
	return 5;
}

uint32 AudioProcess::I_stopSFX(const uint8* args, unsigned int argsize)
{
	ARG_SINT16(sfxnum);

	ObjId objid = 0;
	if (argsize == 4) {
		ARG_OBJID(objid_);
		objid = objid_;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if(ap) ap->stopSFX(sfxnum,objid);
	else perr << "Error: No AudioProcess" << std::endl;

	return 7;
}

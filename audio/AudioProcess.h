/*
Copyright (C) 2005-2007 The Pentagram team

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

#ifndef AUDIOPROCESS_H_INCLUDED
#define AUDIOPROCESS_H_INCLUDED

#include "Process.h"
#include "intrinsics.h"
#include <list>
#include <string>

namespace Pentagram {
	class AudioSample;
}

class AudioProcess :
	public Process
{
	struct SampleInfo {
		sint32		sfxnum;
		sint32		priority;
		ObjId		objid;
		sint32		loops;
		sint32		channel;
		std::string barked;
		uint32		curspeech_start, curspeech_end;
		uint32		pitch_shift;	// 0x10000 is normal
		uint16		volume;			// 0-256
		sint16		lvol;
		sint16		rvol;
		
		SampleInfo() : sfxnum(-1) { }
		SampleInfo(sint32 s,sint32 p,ObjId o,sint32 l,sint32 c,uint32 ps,uint16 v, sint16 lv, sint16 rv) : 
			sfxnum(s),priority(p),objid(o),loops(l),channel(c),
			pitch_shift(ps), volume(v), lvol(lv), rvol(rv) { }
		SampleInfo(std::string &b,sint32 shpnum,ObjId o,sint32 c,
				   uint32 s,uint32 e,uint32 ps,uint16 v, sint16 lv, sint16 rv) : 
			sfxnum(-1),priority(shpnum),objid(o),loops(0),channel(c),barked(b),
			curspeech_start(s), curspeech_end(e), pitch_shift(ps), volume(v), 
			lvol(lv), rvol(rv) { }
	};

	std::list<SampleInfo>	sample_info;

public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE();

	AudioProcess(void);
	virtual ~AudioProcess(void);

	//! Get the current instance of the Audio Processes
	static AudioProcess	* get_instance() { return the_audio_process; }

	INTRINSIC(I_playSFX);
	INTRINSIC(I_playAmbientSFX);
	INTRINSIC(I_isSFXPlaying);
	INTRINSIC(I_setVolumeSFX);
	INTRINSIC(I_stopSFX);

	static void ConCmd_listSFX(const Console::ArgvType &argv);
	static void ConCmd_stopSFX(const Console::ArgvType &argv);
	static void ConCmd_playSFX(const Console::ArgvType &argv);


	virtual void run();

	void playSFX(int sfxnum, int priority, ObjId objid, int loops,
				 bool no_duplicates, uint32 pitch_shift,
				 uint16 volume, sint16 lvol, sint16 rvol);

	void playSFX(int sfxnum, int priority, ObjId objid, int loops,
				 bool no_duplicates=false, uint32 pitch_shift=0x10000,
				 uint16 volume=0x80) {
		 playSFX(sfxnum, priority, objid, loops, no_duplicates, pitch_shift, volume, -1, -1);
	}

	void stopSFX(int sfxnum, ObjId objid);
	bool isSFXPlaying(int sfxnum);
	void setVolumeSFX(int sfxnum, uint8 volume);

	bool playSpeech(std::string &barked, int shapenum, ObjId objid, 
					uint32 pitch_shift=0x10000,uint16 volume=256);
	void stopSpeech(std::string &barked, int shapenum, ObjId objid);
	bool isSpeechPlaying(std::string &barked, int shapenum);

	//! get length (in milliseconds) of speech
	uint32 getSpeechLength(std::string &barked, int shapenum) const;

	//! play a sample (without storing a SampleInfo)
	//! returns channel sample is played on, or -1
	int playSample(Pentagram::AudioSample* sample, int priority, int loops, 
				   uint32 pitch_shift=0x10000, sint16 lvol=256, sint16 rvol=256);

	//! pause all currently playing samples
	void pauseAllSamples();
	//! unpause all currently playing samples
	void unpauseAllSamples();

	//! stop all samples except speech
	void stopAllExceptSpeech();

	// AudioProcess::playSound console command
	//static void ConCmd_playSound(const Console::ArgvType &argv);

	bool loadData(IDataSource* ids, uint32 version);

private:
	virtual void saveData(ODataSource* ods);
	uint32 paused;

	//! play the next speech sample for the text in this SampleInfo
	//! note: si is reused if successful
	//! returns true if there was speech left to play, or false if finished
	bool continueSpeech(SampleInfo& si);

	bool calculateSoundVolume(ObjId objid, sint16 &lvol, sint16 &rvol) const;

	static AudioProcess	*	the_audio_process;
};

#endif

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
		
		SampleInfo() : sfxnum(-1) { }
		SampleInfo(sint32 s,sint32 p,ObjId o,sint32 l,sint32 c) : 
			sfxnum(s),priority(p),objid(o),loops(l),channel(c) { }
		SampleInfo(std::string &b,sint32 shpnum,ObjId o,sint32 c,
				   uint32 s,uint32 e) : 
			sfxnum(-1),priority(shpnum),objid(o),loops(0),channel(c),barked(b),
			curspeech_start(s), curspeech_end(e) { }
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

	virtual bool run(const uint32 framenum);

	void playSFX(int sfxnum, int priority, ObjId objid, int loops,
				 bool no_duplicates=false);
	void stopSFX(int sfxnum, ObjId objid);
	bool isSFXPlaying(int sfxnum);

	bool playSpeech(std::string &barked, int shapenum, ObjId objid);
	void stopSpeech(std::string &barked, int shapenum, ObjId objid);
	bool isSpeechPlaying(std::string &barked, int shapenum);

	//! get length (in milliseconds) of speech
	uint32 getSpeechLength(std::string &barked, int shapenum) const;

	//! play a sample (without storing a SampleInfo)
	//! returns channel sample is played on, or -1
	int playSample(Pentagram::AudioSample* sample, int priority, int loops);

	// AudioProcess::playSound console command
	//static void ConCmd_playSound(const Console::ArgsType &args, const Console::ArgvType &argv);

	bool loadData(IDataSource* ids, uint32 version);

private:
	virtual void saveData(ODataSource* ods);

	//! play the next speech sample for the text in this SampleInfo
	//! note: si is reused if successful
	//! returns true if there was speech left to play, or false if finished
	bool continueSpeech(SampleInfo& si);

	static AudioProcess	*	the_audio_process;
};

#endif

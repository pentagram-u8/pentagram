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

#ifndef FMOPLDRV_H
#define FMOPLDRV_H

#ifdef USE_FMOPL_MIDI

#include "LowLevelMidiDriver.h"
#include "fmopl.h"

class IDataSource;

//
// The Internal OplDriver. Only slightly modified for exult
//
class FMOplMidiDriver : public LowLevelMidiDriver
{

public:

	/* error codes returned by open.
	 * can be converted to a string with get_error_name()
	 */
	enum {
		MERR_CANNOT_CONNECT = 1,
		MERR_STREAMING_NOT_AVAILABLE = 2,
		MERR_DEVICE_NOT_AVAILABLE = 3,
		MERR_ALREADY_OPEN = 4,
	};

	// Modulation Registers
	#define INDEX_AVEKM_M	0
	#define INDEX_KSLTL_M	2
	#define INDEX_AD_M		4
	#define INDEX_SR_M		6
	#define INDEX_WAVE_M	8

	// Carrier Registers
	#define INDEX_AVEKM_C	1
	#define INDEX_KSLTL_C	3
	#define INDEX_AD_C		5
	#define INDEX_SR_C		7
	#define INDEX_WAVE_C	9

	#define INDEX_FB_C		10
	#define INDEX_PERC		11

	#define CHP_CHAN		0
	#define CHP_NOTE		1
	#define CHP_COUNTER		2
	#define CHP_VEL			3

	// LowLevelMidiDriver implementation
	virtual int			open();
	virtual void		close();
	virtual void		send(uint32 b);
	virtual void		lowLevelProduceSamples(sint16 *samples, uint32 num_samples);

	// MidiDriver overloads
	virtual bool		isSampleProducer() { return true; }
	virtual bool		isFMSynth() { return true; }

	/* retrieve a string representation of an error code */
	static const char *get_error_name(int error_code);

	FMOplMidiDriver();
	virtual ~FMOplMidiDriver();

private:
	struct midi_channel {
		int inum;
		unsigned char ins[12];
		bool xmidi;
		int	xmidi_bank;
		int vol;
		int expression;
		int nshift;
		int on;
		int pitchbend;
	};
	struct xmidibank {
		unsigned char	insbank[128][12];
	};

	enum {
		ADLIB_MELODIC = 0,
		ADLIB_RYTHM = 1
	};

	void midi_write_adlib(unsigned int reg, unsigned char val);
	void midi_fm_instrument(int voice, unsigned char *inst);
	int  midi_calc_volume(int chan, int vel);
	void midi_update_volume(int chan);
	void midi_fm_volume(int voice, int volume);
	void midi_fm_playnote(int voice, int note, int volume, int pitchbend);
	void midi_fm_endnote(int voice);
	unsigned char adlib_data[256];


	int chp[9][4];
	unsigned char	myinsbank[128][12];
	xmidibank		*xmidibanks[128];
	void			loadAdlibTimbres(IDataSource *ds);

	FM_OPL *_opl;
	midi_channel ch[16];
};

#endif //USE_FMOPL_MIDI

#endif //FMOPLDRV_H

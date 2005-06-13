/* 
 * Copyright (C) 2001-2005 The ScummVM project
 * Copyright (C) 2005 The Pentagram Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef USE_FLUIDSYNTH_MIDI

#include "LowLevelMidiDriver.h"
#include <fluidsynth.h>

class FluidSynthMidiDriver : public LowLevelMidiDriver {
private:
	fluid_settings_t *_settings;
	fluid_synth_t *_synth;
	int _soundFont;

	const static MidiDriverDesc	desc;
	static MidiDriver *createInstance() {
		return new FluidSynthMidiDriver();
	}

public:
	static const MidiDriverDesc* getDesc() { return &desc; }
	FluidSynthMidiDriver();

protected:
	// Because GCC complains about casting from const to non-const...
	void setInt(const char *name, int val);
	void setNum(const char *name, double num);
	void setStr(const char *name, const char *str);

	// LowLevelMidiDriver implementation
	virtual int open();
	virtual void close();
	virtual void send(uint32 b);
	virtual void lowLevelProduceSamples(sint16 *samples, uint32 num_samples);

	// MidiDriver overloads
	virtual bool		isSampleProducer() { return true; }
	virtual bool		noTimbreSupport() { return true; }
};


#endif

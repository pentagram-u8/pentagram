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

#ifndef SEQUENCEHANDLER_H_INCLUDED
#define SEQUENCEHANDLER_H_INCLUDED

//! Abstract class for handling the playing of XMidiSequence objects
class XMidiSequenceHandler
{
public:
	//! An event sent from a sequence to play
	//! \arg sequence_id The id of the sequence that is attempting to send the event
	//! \arg message The Event being sent
	virtual void	sequenceSendEvent(uint16 sequence_id, uint32 message) = 0;

	//! An event sent from a sequence to play
	//! \arg sequence_id The id of the sequence requesting the tick count
	//! \return Number of ticks, in 6000ths of a second
	virtual uint32	getTickCount(uint16 sequence_id) = 0;

	//! Handle an XMIDI Callback Trigger Event
	//! \arg sequence_id The id of the sequence doing the callback
	//! \arg data data[1] of the XMIDI_CONTROLLER_CALLBACK_TRIG event
	virtual void	handleCallbackTrigger(uint16 sequence_id, uint8 data) = 0;
};

#endif //SEQUENCEHANDLER_H_INCLUDED

/*
Copyright (C) 2002-2005 The Pentagram team

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

#ifndef SKFPlayer_H
#define SKFPlayer_H

#include <vector>
#include <map>

struct SKFEvent;
class RawArchive;
class RenderSurface;
class RenderedText;
class IDataSource;
namespace Pentagram { struct Palette; }

class SKFPlayer {
public:
	SKFPlayer(RawArchive* movie, int width, int height, bool introMusicHack = false);
	~SKFPlayer();

	void run();
	void paint(RenderSurface* surf, int lerp);

	void start();
	void stop();
	bool isPlaying() const { return playing; }

	// PNG Export: Automatically saves each frame as a PNG file during playback

private:

	void parseEventList(IDataSource* eventlist);
	void savePNGFrame();  // Saves current frame buffer as PNG file to disk
	void saveAudioAsWAV(uint8* audioData, uint32 dataSize, uint32 sampleRate, bool stereo, unsigned int audioCounter);
	void saveExternalSFXAsWAV(int sfxnum);  // Saves external sound effect as WAV file
	void saveRawAudioAsWAV(uint8* audioData, uint32 dataSize, uint32 sampleRate, bool stereo, const std::string& filename);  // Helper for raw audio export
	std::string getRelativeTimeString();  // Returns time relative to current reference frame (first frame or frame 241)
	std::string getAudioRelativeTimeString(unsigned int frame);  // Returns time relative to specified audio frame
	bool shouldLogAudioForFrame(unsigned int frame);  // Check if audio should be logged for this frame

	int width, height;
	RawArchive* skf;
	std::vector<SKFEvent*> events;
	unsigned int curframe, curobject;
	unsigned int curaction;
	unsigned int curevent;
	bool playing;
	unsigned int lastupdate;
	unsigned int timer;
	unsigned int framerate;
	uint8 fadecolour, fadelevel;
	RenderSurface* buffer;
	RenderedText* subs;
	int subtitley;
	bool introMusicHack;
	unsigned int pngFrameCounter;
	unsigned int audioCounter;  // Counter for exported audio files
	
	// Frame timing tracking
	unsigned int lastFrameTime;
	unsigned int currentFrameStartTime;
	unsigned int skfStartTime;  // Timestamp when SKF playback started
	
	// Command line option flags
	bool extractFrames;
	bool extractAudio;
	bool logFrames;
	bool logAudio;  // Flag to track when --log_skf_audio option was used
	std::vector<unsigned int> logAudioFrames;
	
	// Audio frame timing tracking for --log_skf_audio
	std::map<unsigned int, unsigned int> audioFrameTimes;  // Maps frame number to timestamp
};

#endif

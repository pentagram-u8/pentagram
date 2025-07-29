/*
 *  Copyright (C) 2004-2005  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "SKFPlayer.h"
#include "CoreApp.h"

#include "u8/ConvertShapeU8.h"
#include "RawArchive.h"
#include "Shape.h"
#include "Texture.h"
#include "SoftRenderSurface.h"
#include "PaletteManager.h"
#include "MusicProcess.h"
#include "AudioProcess.h"
#include "IDataSource.h"
#include "AudioMixer.h"
#include "RawAudioSample.h"
#include "Font.h"
#include "FontManager.h"
#include "RenderedText.h"
#include "GameData.h"
#include "SoundFlex.h"
#include "AudioSample.h"

#include "SDL.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <png.h>

// WAV file header structure
struct WAVHeader {
	char riff[4];           // "RIFF"
	uint32_t fileSize;      // File size - 8
	char wave[4];           // "WAVE"
	char fmt[4];            // "fmt "
	uint32_t fmtSize;       // Format chunk size (16 for PCM)
	uint16_t audioFormat;   // Audio format (1 for PCM)
	uint16_t numChannels;   // Number of channels
	uint32_t sampleRate;    // Sample rate
	uint32_t byteRate;      // Byte rate
	uint16_t blockAlign;    // Block align
	uint16_t bitsPerSample; // Bits per sample
	char data[4];           // "data"
	uint32_t dataSize;      // Data size
};


enum SKFAction {
	SKF_PlayMusic = 3,
	SKF_SlowStopMusic = 4,
	SKF_PlaySFX = 5,
	SKF_StopSFX = 6,
	SKF_SetSpeed = 7,
	SKF_FadeOut = 8,
	SKF_FadeIn = 9,
	SKF_Wait = 12,
	SKF_PlaySound = 14,
	SKF_FadeWhite = 15,
	SKF_ClearSubs = 18
};

struct SKFEvent {
	unsigned int frame;
	SKFAction action;
	unsigned int data;
};

// number of steps in a fade
static const int FADESTEPS = 16; // HACK: half speed


SKFPlayer::SKFPlayer(RawArchive* movie, int width_, int height_, bool introMusicHack_)
	: width(width_), height(height_), skf(movie),
	  curframe(0), curobject(0), curaction(0), curevent(0), playing(false),
	  timer(0), framerate(15), fadecolour(0), fadelevel(0), buffer(0), subs(0),
	  introMusicHack(introMusicHack_), pngFrameCounter(0), audioCounter(0), lastFrameTime(0), currentFrameStartTime(0), skfStartTime(0)
{
	// Get command line options from CoreApp
	CoreApp* app = CoreApp::get_instance();
	if (app) {
		extractFrames = app->oExtractSKFFrames;
		extractAudio = app->oExtractSKFAudio;
		logFrames = app->oLogSKFFrames;
		logAudio = app->oLogSKFAudio;
		logAudioFrames = app->oLogSKFAudioFrames;
		
		// Debug output to verify options are being read
		pout << "SKFPlayer: Command line options detected - ";
		pout << "extractFrames=" << (extractFrames ? "true" : "false") << ", ";
		pout << "extractAudio=" << (extractAudio ? "true" : "false") << ", ";
		pout << "logFrames=" << (logFrames ? "true" : "false") << ", ";
		pout << "logAudio=" << (logAudio ? "true" : "false") << ", ";
		pout << "logAudioFrames.size()=" << logAudioFrames.size() << std::endl;
	} else {
		// Default to all disabled if no CoreApp (fallback)
		pout << "SKFPlayer: Warning - CoreApp not available, defaulting to all options disabled" << std::endl;
		extractFrames = false;
		extractAudio = false;
		logFrames = false;
		logAudio = false;
		logAudioFrames.clear();
	}

	IDataSource* eventlist = skf->get_datasource(0);
	if (!eventlist)
	{
		perr << "No eventlist found in SKF" << std::endl;
		return;
	}

	parseEventList(eventlist);
	delete eventlist;

	buffer = RenderSurface::CreateSecondaryRenderSurface(width, height);
}

SKFPlayer::~SKFPlayer()
{
	for (unsigned int i = 0; i < events.size(); ++i)
		delete events[i];

	delete skf;
	delete buffer;
	delete subs;
}

void SKFPlayer::parseEventList(IDataSource* eventlist)
{
	uint16 frame = eventlist->read2();
	while (frame != 0xFFFF) {
		SKFEvent* ev = new SKFEvent;
		ev->frame = frame;
		ev->action = static_cast<SKFAction>(eventlist->read2());
		ev->data = eventlist->read2();
		events.push_back(ev);

		frame = eventlist->read2();
	}
}

void SKFPlayer::start()
{
	buffer->BeginPainting();
	buffer->Fill32(0, 0, 0, width, height);
	buffer->EndPainting();
	MusicProcess* musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(0);
	playing = true;
	lastupdate = SDL_GetTicks();
	skfStartTime = lastupdate;  // Record when SKF playback started
	
	// Initialize frame timing
	currentFrameStartTime = SDL_GetTicks();
	lastFrameTime = 0;
	
	// Only log startup messages if corresponding features are enabled
	if (extractFrames) {
		pout << "SKFPlayer: PNG export enabled - frames will be saved as skf_frame_XXXXXX.png" << std::endl;
	}
	if (extractAudio) {
		pout << "SKFPlayer: Audio export enabled - embedded audio will be saved as skf_audio_XXXX.wav" << std::endl;
		pout << "SKFPlayer: External SFX export enabled - external sound effects will be saved as skf_external_sfx_XXXX.wav" << std::endl;
	}
	if (logFrames) {
		pout << "SKFPlayer: Frame timing logging enabled" << std::endl;
	}
	if (logAudio) {
		if (!logAudioFrames.empty()) {
			pout << "SKFPlayer: Audio logging enabled for frames: ";
			for (size_t i = 0; i < logAudioFrames.size(); ++i) {
				pout << logAudioFrames[i];
				if (i < logAudioFrames.size() - 1) pout << ", ";
			}
			pout << std::endl;
		} else {
			pout << "SKFPlayer: Audio logging enabled for all frames from start" << std::endl;
		}
	}
}

void SKFPlayer::stop()
{
	MusicProcess* musicproc = MusicProcess::get_instance();
	if (musicproc && !introMusicHack) musicproc->playMusic(0);
	playing = false;
}

void SKFPlayer::paint(RenderSurface* surf, int /*lerp*/)
{
	if (!buffer) return;

	Texture* tex = buffer->GetSurfaceAsTexture();

	if (!fadelevel) {
		surf->Blit(tex, 0, 0, width, height, 0, 0);
		if (subs)
			subs->draw(surf, 60, subtitley);
	} else {
		uint32 fade = TEX32_PACK_RGBA(fadecolour,fadecolour,fadecolour,
									  (fadelevel*255)/FADESTEPS);
		surf->FadedBlit(tex, 0, 0, width, height, 0, 0, fade);
		if (subs)
			subs->drawBlended(surf, 60, subtitley, fade);
	}
}

void SKFPlayer::run()
{
	if (!playing || !buffer) return;

	// if doing something, continue
	if (curaction) {
		if (curaction == SKF_FadeOut || curaction == SKF_FadeWhite) {
			fadelevel++;
			if (fadelevel == FADESTEPS) curaction = 0; // done
		} else if (curaction == SKF_FadeIn) {
			fadelevel--;
			if (fadelevel == 0) curaction = 0; // done
		} else {
			pout << "Unknown fade action: " << curaction << std::endl;
		}
	}

	// CHECKME: this timing may not be accurate enough...
	uint32 now = SDL_GetTicks();
	if (lastupdate + (1000/framerate) > now) return;

	lastupdate += (1000/framerate);

	// if waiting, continue to wait
	if (timer) {
		timer--;
		return;
	}

	Pentagram::Font* redfont;
	redfont = FontManager::get_instance()->getGameFont(6, true);

	MusicProcess* musicproc = MusicProcess::get_instance();
	AudioProcess* audioproc = AudioProcess::get_instance();

	// handle events for the current frame
	while (curevent < events.size() && events[curevent]->frame <= curframe) {
//		pout << "event " << curevent << std::endl;
		switch (events[curevent]->action)
		{
		case SKF_FadeOut:
			curaction = SKF_FadeOut;
			fadecolour = 0;
			fadelevel = 0;
//			pout << "FadeOut" << std::endl;
			break;
		case SKF_FadeIn:
			curaction = SKF_FadeIn;
			fadelevel = FADESTEPS;
//			pout << "FadeIn" << std::endl;
			break;
		case SKF_FadeWhite:
			curaction = SKF_FadeWhite;
			fadecolour = 0xFF;
			fadelevel = 0;
//			pout << "FadeWhite" << std::endl;
			break;
		case SKF_Wait:
//			pout << "Wait " << events[curevent]->data << std::endl;
			timer = events[curevent]->data;
			curevent++;
			return;
		case SKF_PlayMusic:
//			pout << "PlayMusic " << events[curevent]->data << std::endl;
			if (shouldLogAudioForFrame(curframe)) {
				pout << "Frame " << curframe << " - Playing music track " << events[curevent]->data << " (from external music file)" << getAudioRelativeTimeString(curframe) << std::endl;
			}
			if (musicproc) musicproc->playMusic(events[curevent]->data);
			break;
		case SKF_SlowStopMusic:
			if (shouldLogAudioForFrame(curframe)) {
				pout << "Frame " << curframe << " - SlowStopMusic" << getAudioRelativeTimeString(curframe) << std::endl;
			}
			if (musicproc && !introMusicHack) musicproc->playMusic(0);
			break;
		case SKF_PlaySFX:
//			pout << "PlaySFX " << events[curevent]->data << std::endl;
			if (shouldLogAudioForFrame(curframe)) {
				pout << "Frame " << curframe << " - Playing sound effect " << events[curevent]->data << " (from external SFX file)" << getAudioRelativeTimeString(curframe) << std::endl;
			}
			
			// Export external SFX as WAV only if audio extraction is enabled
			if (extractAudio) {
				saveExternalSFXAsWAV(events[curevent]->data);
			}
			
			if (audioproc) audioproc->playSFX(events[curevent]->data,0x60,0,0);
			break;
		case SKF_StopSFX:
//			pout << "StopSFX" << events[curevent]->data << std::endl;
			if (audioproc) audioproc->stopSFX(events[curevent]->data,0);
			break;
		case SKF_SetSpeed:
			POUT("SetSpeed " << events[curevent]->data);
//			framerate = events[curevent]->data;
			break;
		case SKF_PlaySound:
		{
//			pout << "PlaySound " << events[curevent]->data << std::endl;
			if (shouldLogAudioForFrame(curframe)) {
				pout << "Frame " << curframe << " - Playing embedded audio from SKF object " << events[curevent]->data << getAudioRelativeTimeString(curframe) << std::endl;
			}

			if (audioproc) {
				uint8* buffer = skf->get_object(events[curevent]->data);
				uint32 bufsize = skf->get_size(events[curevent]->data);
				Pentagram::AudioSample* s;
				uint32 rate = buffer[6] + (buffer[7]<<8);
				bool stereo = (buffer[8] == 2);
				
				// Save audio as WAV file only if audio extraction is enabled
				if (extractAudio) {
					saveAudioAsWAV(buffer + 34, bufsize - 34, rate, stereo, audioCounter);
					audioCounter++;
				}
				
				s = new Pentagram::RawAudioSample(buffer+34, bufsize-34,
												  rate, true, stereo);
				audioproc->playSample(s, 0x60, 0);
				if (shouldLogAudioForFrame(curframe)) {
					pout << "  -> Audio format: " << rate << "Hz, " << (stereo ? "stereo" : "mono") << ", size: " << (bufsize-34) << " bytes" << std::endl;
				}
				// FIXME: memory leak! (sample is never deleted)
			}

			// subtitles
			char* textbuf = reinterpret_cast<char*>(
				skf->get_object(events[curevent]->data-1));
			uint32 textsize = skf->get_size(events[curevent]->data-1);
			if (textsize > 7) {
				std::string subtitle = (textbuf+6);
				delete subs;
				subtitley = textbuf[4] + (textbuf[5]<<8);
				unsigned int remaining;
				subs = redfont->renderText(subtitle, remaining, 200, 0,
										   Pentagram::Font::TEXT_CENTER);
				if (shouldLogAudioForFrame(curframe)) {
					pout << "Frame " << curframe << " - Subtitle displayed: \"" << subtitle << "\" at Y=" << subtitley << std::endl;
				}
			}
			delete textbuf;


			break;
		}
		case SKF_ClearSubs:
//			pout << "ClearSubs" << std::endl;
			if (shouldLogAudioForFrame(curframe)) {
				pout << "Frame " << curframe << " - Subtitles cleared" << std::endl;
			}
			delete subs;
			subs = 0;
			break;
		default:
			pout << "Unknown action" << std::endl;
			break;
		}

		curevent++;
	}

	// Record timestamp for audio logging frames BEFORE incrementing curframe
	if (!logAudioFrames.empty()) {
		for (unsigned int audioFrame : logAudioFrames) {
			if (curframe == audioFrame && audioFrameTimes.find(audioFrame) == audioFrameTimes.end()) {
				// First time reaching this audio frame, record the timestamp
				audioFrameTimes[audioFrame] = SDL_GetTicks();
				pout << "*** AUDIO REFERENCE TIME SET: Frame " << audioFrame << " reached at " << audioFrameTimes[audioFrame] << "ms ***" << std::endl;
			}
		}
	}

	curframe++;

	PaletteManager* palman = PaletteManager::get_instance();
	IDataSource* object;

	uint16 objecttype = 0;
	do {
		curobject++;
		if (curobject >= skf->getCount()) {
			stop(); // done
			return;
		}

		// read object
		object = skf->get_datasource(curobject);
		if (!object || object->getSize() < 2)
			continue;

		objecttype = object->read2();

//		pout << "Object " << curobject << "/" << skf->getCount()
//			 << ", type = " << objecttype << std::endl;


		if (objecttype == 1) {
			palman->load(PaletteManager::Pal_Movie, *object);
		}

		if (objecttype != 2)
			delete object;

	} while (objecttype != 2);

	if (objecttype == 2) {
		uint32 now = SDL_GetTicks();
		
		// Log previous frame duration only if frame logging is enabled
		if (logFrames && currentFrameStartTime > 0) {
			uint32 frameDuration = now - currentFrameStartTime;
			pout << "Frame " << (curframe - 1) << " total display time: " << frameDuration << "ms" << std::endl;
		}
		
		// Update frame timing for the new frame
		currentFrameStartTime = now;
		
		object->seek(0);
		Shape* shape = new Shape(object, &U8SKFShapeFormat);
		Pentagram::Palette* pal= palman->getPalette(PaletteManager::Pal_Movie);
		shape->setPalette(pal);
		buffer->BeginPainting();
		buffer->Paint(shape, 0, 0, 0);
		buffer->EndPainting();
		delete shape;
		
		//pout << "Rendering new frame " << curframe << " at time " << now << "ms" << std::endl;
		
		// Save frame as PNG only if frame extraction is enabled
		if (extractFrames) {
			try {
				savePNGFrame();
			} catch (...) {
				perr << "Exception occurred while saving PNG frame " << pngFrameCounter << std::endl;
			}
		}
	
		delete object;
	}

	timer = 1; // HACK! timing is rather broken currently...
}

// PNG Export Functionality
// This method saves the current frame buffer as a PNG file to disk
// Files are saved as skf_frame_XXXXXX.png where XXXXXX is a 6-digit frame counter
void SKFPlayer::savePNGFrame()
{
	if (!buffer) return;

	// Generate filename with frame number
	std::ostringstream filename;
	filename << "skf_frame_" << std::setfill('0') << std::setw(6) << pngFrameCounter << ".png";
	
	//pout << "Saving frame " << pngFrameCounter << " to " << filename.str() << std::endl;
	
	// Get the surface texture - this is managed by the RenderSurface, don't delete it
	Texture* tex = buffer->GetSurfaceAsTexture();
	if (!tex) {
		perr << "Failed to get surface texture for PNG output" << std::endl;
		return;
	}

	// Validate texture properties
	if (!tex->buffer || tex->width <= 0 || tex->height <= 0) {
		perr << "Invalid texture properties for PNG output" << std::endl;
		return;
	}

	// Create a simple PNG export without using the problematic ODataSource/PNGWriter chain
	FILE* fp = fopen(filename.str().c_str(), "wb");
	if (!fp) {
		perr << "Failed to create PNG file: " << filename.str() << std::endl;
		return;
	}
	
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		perr << "Failed to create PNG write struct" << std::endl;
		fclose(fp);
		return;
	}
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		perr << "Failed to create PNG info struct" << std::endl;
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		return;
	}
	
	// Set up error handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		perr << "PNG write error occurred" << std::endl;
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return;
	}
	
	// Initialize PNG output
	png_init_io(png_ptr, fp);
	
	// Set image properties - use RGB format to avoid alpha channel issues for now
	int width = tex->width;
	int height = tex->height;
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
				 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	// Write PNG header
	png_write_info(png_ptr, info_ptr);
	
	// Configure PNG to handle our 32-bit ARGB data properly
	png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);  // Ignore alpha channel 
	png_set_bgr(png_ptr);  // Handle BGR/RGB conversion
	
	// Write image data row by row, using the original texture data
	for (int y = 0; y < height; y++) {
		png_write_row(png_ptr, reinterpret_cast<png_bytep>(&tex->buffer[y * width]));
	}
	
	// Finish PNG
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	
	pngFrameCounter++;
}

// Audio Export Functionality
// This method saves embedded audio data from SKF as WAV files to disk
void SKFPlayer::saveAudioAsWAV(uint8* audioData, uint32 dataSize, uint32 sampleRate, bool stereo, unsigned int audioCounter)
{
	if (!audioData || dataSize == 0) return;

	// Generate filename with audio counter
	std::ostringstream filename;
	filename << "skf_audio_" << std::setfill('0') << std::setw(4) << audioCounter << ".wav";
	
	pout << "  -> Saving audio to " << filename.str() << std::endl;
	
	// Open file for writing
	std::ofstream wavFile(filename.str().c_str(), std::ios::binary);
	if (!wavFile.good()) {
		perr << "Failed to create WAV file: " << filename.str() << std::endl;
		return;
	}

	// Prepare WAV header
	WAVHeader header;
	uint16_t numChannels = stereo ? 2 : 1;
	uint16_t bitsPerSample = 8; // SKF audio is 8-bit
	uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
	uint16_t blockAlign = numChannels * (bitsPerSample / 8);
	
	// Fill WAV header
	std::memcpy(header.riff, "RIFF", 4);
	header.fileSize = 36 + dataSize; // Header size (44) - 8 + data size
	std::memcpy(header.wave, "WAVE", 4);
	std::memcpy(header.fmt, "fmt ", 4);
	header.fmtSize = 16;
	header.audioFormat = 1; // PCM
	header.numChannels = numChannels;
	header.sampleRate = sampleRate;
	header.byteRate = byteRate;
	header.blockAlign = blockAlign;
	header.bitsPerSample = bitsPerSample;
	std::memcpy(header.data, "data", 4);
	header.dataSize = dataSize;
	
	// Write header
	wavFile.write(reinterpret_cast<const char*>(&header), sizeof(WAVHeader));
	
	// SKF audio data is signed 8-bit, but WAV 8-bit format expects unsigned
	// Convert signed to unsigned by adding 128 to each sample (same as RawAudioSample does)
	uint8* convertedData = new uint8[dataSize];
	for (uint32 i = 0; i < dataSize; i++) {
		// Convert signed 8-bit to unsigned 8-bit
		convertedData[i] = static_cast<signed char>(audioData[i]) + 128;
	}
	
	// Write converted audio data
	wavFile.write(reinterpret_cast<const char*>(convertedData), dataSize);
	
	delete[] convertedData;
	wavFile.close();
	
	pout << "  -> WAV file saved: " << sampleRate << "Hz, " << numChannels << " channel(s), " << bitsPerSample << "-bit signed->unsigned, " << dataSize << " bytes" << std::endl;
}

// Raw Audio Export Helper
// This method saves raw audio data as WAV without any format conversion
void SKFPlayer::saveRawAudioAsWAV(uint8* audioData, uint32 dataSize, uint32 sampleRate, bool stereo, const std::string& filename)
{
	if (!audioData || dataSize == 0) return;

	pout << "  -> Saving audio to " << filename << std::endl;
	
	// Open file for writing
	std::ofstream wavFile(filename.c_str(), std::ios::binary);
	if (!wavFile.good()) {
		perr << "Failed to create WAV file: " << filename << std::endl;
		return;
	}

	// Prepare WAV header
	WAVHeader header;
	uint16_t numChannels = stereo ? 2 : 1;
	uint16_t bitsPerSample = 8;
	uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
	uint16_t blockAlign = numChannels * (bitsPerSample / 8);
	
	// Fill WAV header
	std::memcpy(header.riff, "RIFF", 4);
	header.fileSize = 36 + dataSize;
	std::memcpy(header.wave, "WAVE", 4);
	std::memcpy(header.fmt, "fmt ", 4);
	header.fmtSize = 16;
	header.audioFormat = 1; // PCM
	header.numChannels = numChannels;
	header.sampleRate = sampleRate;
	header.byteRate = byteRate;
	header.blockAlign = blockAlign;
	header.bitsPerSample = bitsPerSample;
	std::memcpy(header.data, "data", 4);
	header.dataSize = dataSize;
	
	// Write header
	wavFile.write(reinterpret_cast<const char*>(&header), sizeof(WAVHeader));
	
	// Write raw audio data without conversion (external SFX may already be unsigned)
	wavFile.write(reinterpret_cast<const char*>(audioData), dataSize);
	
	wavFile.close();
	
	pout << "  -> WAV file saved: " << sampleRate << "Hz, " << numChannels << " channel(s), " << bitsPerSample << "-bit raw, " << dataSize << " bytes" << std::endl;
}

// External SFX Export Functionality
// This method saves external sound effects from the game's SoundFlex as WAV files
void SKFPlayer::saveExternalSFXAsWAV(int sfxnum)
{
	SoundFlex* soundflex = GameData::get_instance()->getSoundFlex();
	if (!soundflex) {
		perr << "No SoundFlex available for external SFX export" << std::endl;
		return;
	}
	
	Pentagram::AudioSample* sample = soundflex->getSample(sfxnum);
	if (!sample) {
		perr << "Failed to get external SFX sample " << sfxnum << std::endl;
		return;
	}
	
	// Generate filename with SFX number
	std::ostringstream filename;
	filename << "skf_external_sfx_" << std::setfill('0') << std::setw(4) << sfxnum << ".wav";
	
	pout << "  -> Saving external SFX " << sfxnum << " to " << filename.str() << std::endl;
	
	// Get sample properties
	uint32 sampleRate = sample->getRate();
	bool stereo = sample->isStereo();
	uint32 bits = sample->getBits();
	uint32 totalLength = sample->getLength();
	
	// For now, only handle 8-bit samples (most common in U8)
	if (bits != 8) {
		perr << "Unsupported bit depth for external SFX: " << bits << " bits" << std::endl;
		return;
	}
	
	// Create decompressor
	uint32 decompSize = sample->getDecompressorDataSize();
	uint8* decompData = new uint8[decompSize];
	sample->initDecompressor(decompData);
	
	// Decompress all audio data
	std::vector<uint8> audioData;
	audioData.reserve(totalLength);
	
	uint32 frameSize = sample->getFrameSize();
	uint8* frameBuffer = new uint8[frameSize];
	
	while (audioData.size() < totalLength) {
		uint32 decompressed = sample->decompressFrame(decompData, frameBuffer);
		if (decompressed == 0) break; // End of audio
		
		for (uint32 i = 0; i < decompressed && audioData.size() < totalLength; i++) {
			audioData.push_back(frameBuffer[i]);
		}
	}
	
	delete[] frameBuffer;
	delete[] decompData;
	
	if (audioData.empty()) {
		perr << "No audio data decompressed for SFX " << sfxnum << std::endl;
		return;
	}
	
	// Save as WAV using the raw audio method (no signed conversion for external SFX)
	saveRawAudioAsWAV(audioData.data(), audioData.size(), sampleRate, stereo, filename.str());
	
	pout << "  -> External SFX exported: " << sampleRate << "Hz, " << (stereo ? "stereo" : "mono") << ", " << bits << "-bit, " << audioData.size() << " bytes" << std::endl;
}

// Helper method to get time relative to the current reference frame
std::string SKFPlayer::getRelativeTimeString()
{
	// This method is now deprecated - use getAudioRelativeTimeString for audio timing
	return " (timing reference removed)";
}

// Helper method to get time relative to the appropriate audio logging frame
std::string SKFPlayer::getAudioRelativeTimeString(unsigned int frame)
{
	if (logAudioFrames.empty()) {
		// No specific frames specified, show time since SKF start
		uint32 now = SDL_GetTicks();
		long relativeTime = static_cast<long>(now) - static_cast<long>(skfStartTime);
		
		std::ostringstream timeStr;
		timeStr << " (" << relativeTime << "ms since start)";
		return timeStr.str();
	}
	
	// Determine which audio frame to use as reference based on current frame position
	unsigned int referenceFrame = logAudioFrames[0]; // Start with first frame
	
	// Switch to later reference frames as we pass them
	for (size_t i = 1; i < logAudioFrames.size(); ++i) {
		if (frame >= logAudioFrames[i]) {
			referenceFrame = logAudioFrames[i];
		} else {
			break; // Don't use frames we haven't reached yet
		}
	}
	
	// Check if we have recorded the timestamp for this reference frame
	auto it = audioFrameTimes.find(referenceFrame);
	if (it != audioFrameTimes.end()) {
		uint32 now = SDL_GetTicks();
		long relativeTime = static_cast<long>(now) - static_cast<long>(it->second);
		
		std::ostringstream timeStr;
		timeStr << " (";
		if (relativeTime >= 0) {
			timeStr << "+" << relativeTime << "ms after frame " << referenceFrame << ")";
		} else {
			timeStr << relativeTime << "ms before frame " << referenceFrame << ")";
		}
		
		return timeStr.str();
	} else {
		// Reference frame timestamp not recorded yet, fall back to indicating it's before the reference
		std::ostringstream timeStr;
		timeStr << " (before frame " << referenceFrame << ")";
		return timeStr.str();
	}
}

// Helper method to check if audio should be logged for this frame
bool SKFPlayer::shouldLogAudioForFrame(unsigned int frame)
{
	// If --log_skf_audio was not used at all, don't log
	if (!logAudio) return false;
	
	if (logAudioFrames.empty()) {
		// --log_skf_audio was used without parameters, log all audio from start
		return true;
	}
	
	// --log_skf_audio was used with specific frames, log from the first specified frame
	return frame >= logAudioFrames[0];
}


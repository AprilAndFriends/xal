/// @file
/// @author  Boris Mikic
/// @version 3.11
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Provides an interface to play and control audio data.

#ifndef XAL_PLAYER_H
#define XAL_PLAYER_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "xalExport.h"

namespace xal
{
	class AudioManager;
	class Buffer;
	class Category;
	class Sound;

	class xalExport Player
	{
	public:
		friend class AudioManager;

		Player(Sound* sound);
		virtual ~Player();

		float getGain();
		void setGain(float value);
		float getPitch();
		void setPitch(float value);
		HL_DEFINE_GET(Sound*, sound, Sound);
		hstr getName();
		hstr getFilename();
		hstr getRealFilename();
		float getDuration();
		int getSize();
		float getTimePosition();
		unsigned int getSamplePosition();

		Category* getCategory();

		bool isPlaying();
		bool isPaused();
		bool isFading();
		bool isFadingIn();
		bool isFadingOut();
		HL_DEFINE_IS(looping, Looping);

		void play(float fadeTime = 0.0f, bool looping = false);
		void stop(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);

	protected:
		float gain;
		float pitch;
		bool paused;
		bool looping;
		float fadeSpeed;
		float fadeTime;
		float offset; // TODO - should be removed?
		Sound* sound;
		Buffer* buffer;
		int bufferIndex;
		int processedByteCount;
		float idleTime;

		float _getGain();
		void _setGain(float value);
		float _getPitch();
		void _setPitch(float value);

		virtual void _update(float timeDelta);

		void _play(float fadeTime = 0.0f, bool looping = false);
		void _stop(float fadeTime = 0.0f);
		void _pause(float fadeTime = 0.0f);

		float _calcGain();

		inline virtual bool _systemIsPlaying() { return false; }
		inline virtual unsigned int _systemGetBufferPosition() { return 0; }
		inline virtual float _systemGetOffset() { return 0.0f; }
		inline virtual void _systemSetOffset(float value) { }
		inline virtual bool _systemPreparePlay() { return true; }
		inline virtual void _systemPrepareBuffer() { }
		inline virtual void _systemUpdateGain() { }
		inline virtual void _systemUpdatePitch() { }
		inline virtual void _systemPlay() { }
		inline virtual int _systemStop() { return 0; }
		inline virtual int _systemUpdateStream() { return 0; }

	private:
		void _stopSound(float fadeTime = 0.0f);

	};

}
#endif

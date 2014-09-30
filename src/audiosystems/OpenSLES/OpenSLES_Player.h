/// @file
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the Player for OpenSLES.

#ifdef _OPENSLES
#ifndef XAL_OPENSLES_PLAYER_H
#define XAL_OPENSLES_PLAYER_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "Player.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;
	class OpenSLES_AudioManager;

	class xalExport OpenSLES_Player : public Player
	{
	public:
		friend class OpenSLES_AudioManager;
		
		OpenSLES_Player(Sound* sound);
		~OpenSLES_Player();

	protected:
		bool playing;
		bool active;
		bool stillPlaying;
		SLObjectItf playerObject;
		SLPlayItf player;
		SLVolumeItf playerVolume;
		SLAndroidSimpleBufferQueueItf playerBufferQueue;
		unsigned char* streamBuffers[STREAM_BUFFER_COUNT]; // OpenSLES does not keep audio data alive so streamed audio has to be cached
		int buffersSubmitted;
		int _buffersProcessed;
		hmutex mutex;

		void _update(float timeDelta);

		bool _systemIsPlaying();
		unsigned int _systemGetBufferPosition();
		bool _systemPreparePlay();
		void _systemPrepareBuffer();
		void _systemUpdateGain();
		void _systemUpdatePitch();
		void _systemPlay();
		int _systemStop();
		int _systemUpdateStream();

		void _submitBuffer(unsigned char* stream, int size);
		int _fillStreamBuffers(int count);
		void _submitStreamBuffers(int count);

		int _getLastProcessedBuffersCount();

		static void _playCallback(SLPlayItf player, void* context, SLuint32 event);

	};

}
#endif
#endif

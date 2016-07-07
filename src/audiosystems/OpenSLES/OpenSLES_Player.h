/// @file
/// @version 3.5
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
#include "Utility.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;
	class OpenSLES_AudioManager;

	/// @note In this audio-system looping non-streamed sounds are implemented behave very similarly to streamed sounds due to certain constraints in the backend.
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
		SLAndroidSimpleBufferQueueState playerBufferQueueState;
		unsigned char* streamBuffers[STREAM_BUFFER_COUNT]; // OpenSLES does not keep audio data alive so streamed audio has to be cached
		int buffersSubmitted;

		void _update(float timeDelta);

		bool _systemIsPlaying() const;
		unsigned int _systemGetBufferPosition() const;
		bool _systemNeedsStreamedBufferPositionCorrection() const { return false; }
		bool _systemPreparePlay();
		void _systemPrepareBuffer();
		void _systemUpdateGain();
		void _systemUpdatePitch();
		void _systemPlay();
		int _systemStop();
		void _systemUpdateNormal();
		int _systemUpdateStream();

		void _submitBuffer(hstream& stream);
		int _fillStreamBuffers(int count);
		void _submitStreamBuffers(int count);

		int _getProcessedBuffersCount();

		static void _playCallback(SLPlayItf player, void* context, SLuint32 event);

	};

}
#endif
#endif

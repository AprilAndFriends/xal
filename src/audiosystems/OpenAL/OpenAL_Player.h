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
/// Represents an implementation of the Player for OpenAL.

#ifdef _OPENAL
#ifndef XAL_OPENAL_PLAYER_H
#define XAL_OPENAL_PLAYER_H

#include "Player.h"
#include "Utility.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;
	class OpenAL_AudioManager;

	class xalExport OpenAL_Player : public Player
	{
	public:
		friend class OpenAL_AudioManager;
		
		OpenAL_Player(Sound* sound);
		~OpenAL_Player();

		inline unsigned int getSourceId() const { return this->sourceId; }
		inline void setSourceId(unsigned int value) { this->sourceId = value; }
		
		void createOpenALBuffers();
		void destroyOpenALBuffers();

	protected:
		unsigned int sourceId;
		unsigned int bufferIds[STREAM_BUFFER_COUNT];

		void _update(float timeDelta);

		bool _systemIsPlaying() const;
		unsigned int _systemGetBufferPosition() const;
		float _systemGetOffset() const;
		void _systemSetOffset(float value);
		bool _systemPreparePlay();
		void _systemPrepareBuffer();
		void _systemUpdateGain();
		void _systemUpdatePitch();
		void _systemPlay();
		int _systemStop();
		int _systemUpdateStream();

		int _getQueuedBuffersCount() const;
		int _getProcessedBuffersCount() const;
		int _fillBuffers(int index, int count);
		void _queueBuffers(int index, int count);
		void _queueBuffers();
		void _unqueueBuffers(int index, int count);
		void _unqueueBuffers();
#ifdef _MAC
		float macSampleOffset;
#endif
		bool pendingPitchUpdate;

	};

}
#endif
#endif

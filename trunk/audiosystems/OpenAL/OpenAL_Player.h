/// @file
/// @author  Boris Mikic
/// @version 2.41
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the Player for OpenAL.

#ifdef HAVE_OPENAL
#ifndef XAL_OPENAL_PLAYER_H
#define XAL_OPENAL_PLAYER_H

#include "Player.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;

	class xalExport OpenAL_Player : public Player
	{
	public:
		OpenAL_Player(Sound* sound, Buffer* buffer);
		~OpenAL_Player();

		unsigned int getSourceId() { return this->sourceId; }
		void setSourceId(unsigned int value) { this->sourceId = value; }

	protected:
		unsigned int sourceId;
		unsigned int bufferIds[STREAM_BUFFER_COUNT];

		void _update(float k);

		bool _systemIsPlaying();
		unsigned int _systemGetBufferPosition();
		float _systemGetOffset();
		void _systemSetOffset(float value);
		bool _systemPreparePlay();
		void _systemPrepareBuffer();
		void _systemUpdateGain(float gain);
		void _systemPlay();
		int _systemStop();
		int _systemUpdateStream();

		int _getQueuedBuffersCount();
		int _getProcessedBuffersCount();
		int _fillBuffers(int index, int count);
		void _queueBuffers(int index, int count);
		void _queueBuffers();
		void _unqueueBuffers(int index, int count);
		void _unqueueBuffers();
		
	};

}
#endif
#endif

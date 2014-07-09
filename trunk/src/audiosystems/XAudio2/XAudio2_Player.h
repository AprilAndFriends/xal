/// @file
/// @version 3.14
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the Player for XAudio2.

#ifdef _XAUDIO2
#ifndef XAL_XAUDIO2_PLAYER_H
#define XAL_XAUDIO2_PLAYER_H

#include <xaudio2.h>

#include <hltypes/hplatform.h>

#include "Player.h"
#include "xalExport.h"

using namespace Microsoft::WRL;

namespace xal
{
	class Buffer;
	class Sound;

	class xalExport XAudio2_Player : public Player
	{
	public:
		class CallbackHandler : public IXAudio2VoiceCallback
		{
		public:
			CallbackHandler(bool* active);
			~CallbackHandler();

			STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32 bytesRequired);
			STDMETHOD_(void, OnVoiceProcessingPassEnd)();
			STDMETHOD_(void, OnStreamEnd)();
			STDMETHOD_(void, OnBufferStart)(void* bufferContext);
			STDMETHOD_(void, OnBufferEnd)(void* bufferContext);
			STDMETHOD_(void, OnLoopEnd)(void* bufferContext);
			STDMETHOD_(void, OnVoiceError)(void* bufferContext, HRESULT error);

		protected:
			bool* active;

		};

		IXAudio2SourceVoice* sourceVoice;

		XAudio2_Player(Sound* sound);
		~XAudio2_Player();

	protected:
		bool playing;
		bool active;
		bool stillPlaying;
		CallbackHandler* callbackHandler;
		unsigned char* streamBuffers[STREAM_BUFFER_COUNT]; // XAudio2 does not keep audio data alive so streamed audio has to be cached
		int buffersSubmitted;

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

	private:
		XAUDIO2_BUFFER xa2Buffer;
		XAUDIO2_VOICE_STATE xa2State[5]; // using different buffers for different purposes to avoid problems in threading

	};

}
#endif
#endif

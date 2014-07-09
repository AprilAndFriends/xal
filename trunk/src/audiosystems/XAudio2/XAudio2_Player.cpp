/// @file
/// @version 3.14
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _XAUDIO2
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "Buffer.h"
#include "XAudio2_Player.h"
#include "XAudio2_AudioManager.h"
#include "Sound.h"
#include "xal.h"

#define AUDIO_DEVICE ((XAudio2_AudioManager*)xal::mgr)->xa2Device

using namespace Microsoft::WRL;

namespace xal
{
	XAudio2_Player::CallbackHandler::CallbackHandler(bool* active)
	{
		this->active = active;
	}

	XAudio2_Player::CallbackHandler::~CallbackHandler()
	{
	}

	void XAudio2_Player::CallbackHandler::OnVoiceProcessingPassStart(UINT32 bytesRequired)
	{
	}

	void XAudio2_Player::CallbackHandler::OnVoiceProcessingPassEnd()
	{
	}

	void XAudio2_Player::CallbackHandler::OnStreamEnd()
	{
	}

	void XAudio2_Player::CallbackHandler::OnBufferStart(void* bufferContext)
	{
		*this->active = true;
	}

	void XAudio2_Player::CallbackHandler::OnBufferEnd(void* bufferContext)
	{
		*this->active = false;
	}

	void XAudio2_Player::CallbackHandler::OnLoopEnd(void* bufferContext)
	{
	}

	void XAudio2_Player::CallbackHandler::OnVoiceError(void* bufferContext, HRESULT error)
	{
	}

	XAudio2_Player::XAudio2_Player(Sound* sound) : Player(sound), playing(false), active(false),
		stillPlaying(false), sourceVoice(NULL), buffersSubmitted(0)
	{
		this->callbackHandler = new XAudio2_Player::CallbackHandler(&this->active);
		memset(&this->xa2Buffer, 0, sizeof(XAUDIO2_BUFFER));
		for_iter (i, 0, STREAM_BUFFER_COUNT)
		{
			this->streamBuffers[i] = NULL;
		}
		if (this->sound->isStreamed())
		{
			for_iter (i, 0, STREAM_BUFFER_COUNT)
			{
				this->streamBuffers[i] = new unsigned char[STREAM_BUFFER_SIZE];
			}
		}
	}

	XAudio2_Player::~XAudio2_Player()
	{
		if (this->sourceVoice != NULL)
		{
			this->sourceVoice->DestroyVoice();
			this->sourceVoice = NULL;
		}
		_HL_TRY_DELETE(this->callbackHandler);
		for_iter (i, 0, STREAM_BUFFER_COUNT)
		{
			_HL_TRY_DELETE_ARRAY(this->streamBuffers[i]);
		}
	}

	void XAudio2_Player::_update(float timeDelta)
	{
		this->stillPlaying = this->active;
		Player::_update(timeDelta);
		if (!this->stillPlaying && this->playing)
		{
			this->_stop();
		}
	}

	bool XAudio2_Player::_systemIsPlaying()
	{
		return this->playing;
	}

	unsigned int XAudio2_Player::_systemGetBufferPosition()
	{
		this->sourceVoice->GetState(&this->xa2State[0], 0);
		return (unsigned int)(this->xa2State[0].SamplesPlayed * this->buffer->getChannels() * this->buffer->getBitsPerSample() * 0.125f);
	}

	bool XAudio2_Player::_systemPreparePlay()
	{
		if (this->sourceVoice != NULL)
		{
			return true;
		}
		WAVEFORMATEX wavefmt;
		wavefmt.cbSize = 0;
		wavefmt.nChannels = this->buffer->getChannels();
		wavefmt.nSamplesPerSec = this->buffer->getSamplingRate();
		wavefmt.wBitsPerSample = this->buffer->getBitsPerSample();
		wavefmt.wFormatTag = WAVE_FORMAT_PCM;
		wavefmt.nBlockAlign = wavefmt.nChannels * wavefmt.wBitsPerSample / 8; // standard calculation of WAV PCM data
		wavefmt.nAvgBytesPerSec = wavefmt.nSamplesPerSec * wavefmt.nBlockAlign; // standard calculation of WAV PCM data
		HRESULT result = AUDIO_DEVICE->CreateSourceVoice(&this->sourceVoice, &wavefmt, 0, XAUDIO2_DEFAULT_FREQ_RATIO,
			this->callbackHandler, NULL, NULL);
		if (FAILED(result))
		{
			this->sourceVoice = NULL;
			return false;
		}
		return true;
	}

	void XAudio2_Player::_systemPrepareBuffer()
	{
		if (!this->sound->isStreamed())
		{
			if (!this->paused)
			{
				this->_submitBuffer(this->buffer->getStream(), this->buffer->getSize());
			}
			return;
		}
		int count = STREAM_BUFFER_COUNT;
		if (this->paused)
		{
			this->sourceVoice->GetState(&this->xa2State[1], XAUDIO2_VOICE_NOSAMPLESPLAYED);
			this->buffersSubmitted = this->xa2State[1].BuffersQueued;
			count -= this->buffersSubmitted;
		}
		else
		{
			this->buffersSubmitted = 0;
		}
		if (count > 0)
		{
			count = this->_fillStreamBuffers(count);
			if (count > 0)
			{
				this->_submitStreamBuffers(count);
			}
		}
	}

	void XAudio2_Player::_systemUpdateGain()
	{
		if (this->sourceVoice != NULL)
		{
			this->sourceVoice->SetVolume(this->_calcGain());
		}
	}

	void XAudio2_Player::_systemUpdatePitch()
	{
		if (this->sourceVoice != NULL)
		{
			this->sourceVoice->SetFrequencyRatio(this->pitch);
		}
	}

	void XAudio2_Player::_systemPlay()
	{
		HRESULT result = this->sourceVoice->Start();
		if (!FAILED(result))
		{
			this->playing = true;
			this->stillPlaying = true;
			this->active = true; // required, because otherwise the buffer will think it's done
		}
		else
		{
			hlog::warn(xal::logTag, "Could not start: " + this->sound->getFilename());
		}
	}

	int XAudio2_Player::_systemStop()
	{
		if (this->playing)
		{
			HRESULT result = this->sourceVoice->Stop();
			if (!FAILED(result))
			{
				if (this->paused)
				{
					if (this->sound->isStreamed())
					{
						this->sourceVoice->GetState(&this->xa2State[2], XAUDIO2_VOICE_NOSAMPLESPLAYED);
						int processed = this->buffersSubmitted - this->xa2State[2].BuffersQueued;
						this->buffersSubmitted -= processed;
						result = processed * STREAM_BUFFER_SIZE;
					}
				}
				else
				{
					this->sourceVoice->FlushSourceBuffers();
					this->buffer->rewind();
					if (this->sound->isStreamed())
					{
						this->buffersSubmitted = 0;
					}
				}
				this->playing = false;
				this->stillPlaying = false;
			}
			else
			{
				hlog::warn(xal::logTag, "Could not stop: " + this->sound->getFilename());
			}
		}
		return 0;
	}

	int XAudio2_Player::_systemUpdateStream()
	{
		this->sourceVoice->GetState(&this->xa2State[3], XAUDIO2_VOICE_NOSAMPLESPLAYED);
		int processed = this->buffersSubmitted - this->xa2State[3].BuffersQueued;
		if (processed == 0)
		{
			this->stillPlaying = true; // don't remove, it prevents streamed sounds from being stopped for whatever illogical reason within XAudio2
			return 0;
		}
		this->buffersSubmitted -= processed;
		int count = this->_fillStreamBuffers(processed);
		if (count > 0)
		{
			this->_submitStreamBuffers(count);
			this->stillPlaying = true; // in case underrun happened, sound is regarded as stopped by XAudio2 so let's just bitch-slap it and get this over with
		}
		this->sourceVoice->GetState(&this->xa2State[4], XAUDIO2_VOICE_NOSAMPLESPLAYED);
		if (this->xa2State[4].BuffersQueued == 0)
		{
			this->_stop();
		}
		return (processed * STREAM_BUFFER_SIZE);
	}

	void XAudio2_Player::_submitBuffer(unsigned char* stream, int size)
	{
		this->xa2Buffer.AudioBytes = size;
		this->xa2Buffer.pAudioData = stream;
		this->xa2Buffer.LoopCount = (this->looping ? XAUDIO2_LOOP_INFINITE : 0);
		HRESULT result = this->sourceVoice->SubmitSourceBuffer(&this->xa2Buffer);
		if (FAILED(result))
		{
			hlog::warn(xal::logTag, "Could not submit source buffer!");
		}
	}

	int XAudio2_Player::_fillStreamBuffers(int count)
	{
		int size = this->buffer->load(this->looping, count * STREAM_BUFFER_SIZE);
		int filled = (size + STREAM_BUFFER_SIZE - 1) / STREAM_BUFFER_SIZE;
		unsigned char* stream = this->buffer->getStream();
		int currentSize;
		for_iter (i, 0, filled)
		{
			currentSize = hmin(size, STREAM_BUFFER_SIZE);
			memcpy(this->streamBuffers[this->bufferIndex], &stream[i * STREAM_BUFFER_SIZE], currentSize);
			if (currentSize < STREAM_BUFFER_SIZE)
			{
				memset(&this->streamBuffers[this->bufferIndex][currentSize], 0, STREAM_BUFFER_SIZE - currentSize);
			}
			this->bufferIndex = (this->bufferIndex + 1) % STREAM_BUFFER_COUNT;
			size -= STREAM_BUFFER_SIZE;
		}
		return filled;
	}

	void XAudio2_Player::_submitStreamBuffers(int count)
	{
		HRESULT result;
		this->xa2Buffer.AudioBytes = STREAM_BUFFER_SIZE;
		this->xa2Buffer.LoopCount = 0;
		int index = (this->bufferIndex + STREAM_BUFFER_COUNT - count) % STREAM_BUFFER_COUNT;
		for_iter (i, 0, count)
		{
			this->xa2Buffer.pAudioData = this->streamBuffers[index];
			result = this->sourceVoice->SubmitSourceBuffer(&this->xa2Buffer);
			if (FAILED(result))
			{
				hlog::warn(xal::logTag, "Could not submit streamed source buffer!");
			}
			index = (index + 1) % STREAM_BUFFER_COUNT;
		}
		this->buffersSubmitted += count;
	}

}
#endif

/// @file
/// @author  Boris Mikic
/// @version 2.41
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_DIRECTSOUND
#include <dsound.h>
#include <windows.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Buffer.h"
#include "DirectSound_Player.h"
#include "DirectSound_AudioManager.h"
/*
#ifdef HAVE_WAV
#include "DirectSound_WAV_Source.h"
#endif
*/
#include "Sound.h"
#include "xal.h"

namespace xal
{
	DirectSound_Player::DirectSound_Player(Sound* sound, Buffer* buffer) :
		Player(sound, buffer), dsBuffer(NULL), bufferCount(0), bufferQueued(0)
	{
	}

	DirectSound_Player::~DirectSound_Player()
	{
		// AudioManager calls _stop before destruction
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Release();
			this->dsBuffer = NULL;
		}
	}

	bool DirectSound_Player::_systemIsPlaying()
	{
		if (this->dsBuffer == NULL)
		{
			return false;
		}
		unsigned long status;
		this->dsBuffer->GetStatus(&status);
		return ((status & DSBSTATUS_PLAYING) != 0);
	}

	unsigned int DirectSound_Player::_systemGetBufferPosition()
	{
		unsigned long position;
		this->dsBuffer->GetCurrentPosition(&position, NULL);
		return (unsigned int)position;
	}

	float DirectSound_Player::_systemGetOffset()
	{
		if (this->dsBuffer == NULL)
		{
			return 0.0f;
		}
		return (float)this->_systemGetBufferPosition();
	}

	void DirectSound_Player::_systemSetOffset(float value)
	{
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->SetCurrentPosition((DWORD)value);
		}
	}

	bool DirectSound_Player::_systemPreparePlay()
	{
		if (this->dsBuffer != NULL)
		{
			return true;
		}
		this->buffer->prepare();
		WAVEFORMATEX wavefmt;
		/*
#ifdef HAVE_WAV
		DirectSound_WAV_Source* wavSource = dynamic_cast<DirectSound_WAV_Source*>(this->buffer->getSource());
		if (wavSource != NULL)
		{
			wavefmt = wavSource->getWavefmt();
		}
		else
#endif
		*/
		{
			wavefmt.cbSize = 0;
			wavefmt.nChannels = this->buffer->getChannels();
			wavefmt.nSamplesPerSec = this->buffer->getSamplingRate();
			wavefmt.wBitsPerSample = this->buffer->getBitsPerSample();
			wavefmt.wFormatTag = WAVE_FORMAT_PCM;
			wavefmt.nBlockAlign = wavefmt.nChannels * wavefmt.wBitsPerSample / 8; // standard calculation of WAV PCM data
			wavefmt.nAvgBytesPerSec = wavefmt.nSamplesPerSec * wavefmt.nBlockAlign; // standard calculation of WAV PCM data
		}
		DSBUFFERDESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(DSBUFFERDESC));
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		bufferDesc.dwFlags = (DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS);
		bufferDesc.dwBufferBytes = (!this->sound->isStreamed() ? this->buffer->getSize() : STREAM_BUFFER);
		bufferDesc.lpwfxFormat = &wavefmt;
		HRESULT result = ((DirectSound_AudioManager*)xal::mgr)->dsDevice->CreateSoundBuffer(&bufferDesc, &this->dsBuffer, NULL);
		if (FAILED(result))
		{
			this->dsBuffer = NULL;
			this->buffer->free();
			return false;
		}
		return true;
	}

	void DirectSound_Player::_systemPrepareBuffer()
	{
		if (!this->sound->isStreamed())
		{
			this->_copyBuffer(this->buffer->getStream(), this->buffer->getSize());
			return;
		}
		int count = STREAM_BUFFER_COUNT;
		if (!this->paused)
		{
			this->bufferIndex = 0;
		}
		else
		{
			count -= this->bufferQueued;
		}
		if (count > 0)
		{
			count = this->_fillBuffers(this->bufferIndex, count);
			this->bufferCount += count;
			if (count > 0)
			{
				this->_copyBuffer(this->buffer->getStream(), STREAM_BUFFER_SIZE, count);
			}
			if (this->bufferQueued < STREAM_BUFFER_COUNT)
			{
				count = STREAM_BUFFER_COUNT - this->bufferQueued;
				this->_copySilence(STREAM_BUFFER_SIZE, count);
			}
		}
	}

	void DirectSound_Player::_copyBuffer(unsigned char* stream, int size, int count)
	{
		void* write1 = NULL;
		void* write2 = NULL;
		unsigned long length1;
		unsigned long length2;
		HRESULT result = this->dsBuffer->Lock(this->bufferIndex * STREAM_BUFFER_SIZE, size * count, &write1, &length1, &write2, &length2, 0);
		if (FAILED(result))
		{
			xal::log("cannot lock buffer for " + this->sound->getRealFilename());
			return;
		}
		if (write1 != NULL)
		{
			memcpy(write1, stream, length1);
		}
		if (write2 != NULL)
		{
			memcpy(write2, &stream[length1], length2);
		}
		this->dsBuffer->Unlock(write1, length1, write2, length2);
		if (this->sound->isStreamed())
		{
			this->bufferIndex = (this->bufferIndex + count) % STREAM_BUFFER_COUNT;
			this->bufferQueued += count;
		}
	}

	void DirectSound_Player::_copySilence(int size, int count)
	{
		void* write1 = NULL;
		void* write2 = NULL;
		unsigned long length1;
		unsigned long length2;
		HRESULT result = this->dsBuffer->Lock(this->bufferIndex * STREAM_BUFFER_SIZE, size * count, &write1, &length1, &write2, &length2, 0);
		if (FAILED(result))
		{
			xal::log("cannot lock buffer for " + this->sound->getRealFilename());
			return;
		}
		if (write1 != NULL)
		{
			memset(write1, 0, length1);
		}
		if (write2 != NULL)
		{
			memset(write2, 0, length2);
		}
		this->dsBuffer->Unlock(write1, length1, write2, length2);
		if (this->sound->isStreamed())
		{
			this->bufferIndex = (this->bufferIndex + count) % STREAM_BUFFER_COUNT;
			this->bufferQueued += count;
		}
	}

	void DirectSound_Player::_systemUpdateGain(float fain)
	{
		if (this->dsBuffer != NULL)
		{
			LONG value = DSBVOLUME_MIN;
			if (gain > 0.0f)
			{
				value = (LONG)(log10(gain) / 4 * (DSBVOLUME_MAX - DSBVOLUME_MIN));
			}
			this->dsBuffer->SetVolume(value);
		}
	}

	void DirectSound_Player::_systemUpdatePitch()
	{
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->SetFrequency((DWORD)(this->pitch * this->buffer->getSamplingRate()));
		}
	}

	void DirectSound_Player::_systemPlay()
	{
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Play(0, 0, ((this->looping || this->sound->isStreamed()) ? DSBPLAY_LOOPING : 0));
		}
	}

	int DirectSound_Player::_systemStop()
	{
		int result = 0;
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Stop();
			if (this->sound->isStreamed())
			{
				if (this->paused)
				{
					int processed = this->_getProcessedBuffersCount();
					this->bufferIndex = (this->bufferIndex + processed) % STREAM_BUFFER_COUNT;
					this->bufferCount -= processed;
					this->bufferQueued -= processed;
					result = processed * STREAM_BUFFER_SIZE;
				}
				else
				{
					this->bufferIndex = 0;
					this->bufferCount = 0;
					this->bufferQueued = 0;
					this->buffer->rewind();
				}
			}
		}
		return result;
	}

	int DirectSound_Player::_systemUpdateStream()
	{
		if (this->bufferCount == 0)
		{
			this->_stopSound();
			return 0;
		}
		int processed = this->_getProcessedBuffersCount();
		if (processed == 0)
		{
			return 0;
		}
		this->bufferCount = hmax(this->bufferCount - processed, 0);
		this->bufferQueued = hmax(this->bufferQueued - processed, 0);
		int count = this->_fillBuffers(this->bufferIndex, processed);
		if (count > 0)
		{
			this->_copyBuffer(this->buffer->getStream(), STREAM_BUFFER_SIZE, count);
			this->bufferCount += count;
		}
		if (!this->looping && this->bufferQueued < STREAM_BUFFER_COUNT)
		{
			count = STREAM_BUFFER_COUNT - this->bufferQueued;
			this->_copySilence(STREAM_BUFFER_SIZE, count);
		}
		if (this->bufferCount == 0)
		{
			this->_stopSound();
		}
		return (processed * STREAM_BUFFER_SIZE);
	}

	int DirectSound_Player::_getProcessedBuffersCount()
	{
		return ((this->_systemGetBufferPosition() / STREAM_BUFFER_SIZE +
			STREAM_BUFFER_COUNT - this->bufferIndex) % STREAM_BUFFER_COUNT);
	}

	int DirectSound_Player::_fillBuffers(int index, int count)
	{
		int size = this->buffer->load(this->looping, count * STREAM_BUFFER_SIZE);
		return (size + STREAM_BUFFER_SIZE - 1) / STREAM_BUFFER_SIZE;
	}

}
#endif
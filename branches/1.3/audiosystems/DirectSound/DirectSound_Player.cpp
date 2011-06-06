/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_DIRECTSOUND
#include <dsound.h>
#include <windows.h>

#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "Buffer.h"
#include "DirectSound_Player.h"
#include "DirectSound_AudioManager.h"
#if HAVE_WAV
#include "DirectSound_WAV_Source.h"
#endif
#include "Sound.h"
#include "xal.h"

namespace xal
{
	DirectSound_Player::DirectSound_Player(Sound* sound, Buffer* buffer) :
		Player(sound, buffer), playing(false), dsBuffer(NULL), bufferCount(0)
	{
	}

	DirectSound_Player::~DirectSound_Player()
	{
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Release();
			this->dsBuffer = NULL;
		}
	}

	void DirectSound_Player::_update(float k)
	{
		Player::_update(k);
		if (this->playing)
		{
			unsigned long status;
			this->dsBuffer->GetStatus(&status);
			if ((status & DSBSTATUS_PLAYING) == 0)
			{
				this->playing = false;
			}
		}
	}

	float DirectSound_Player::_sysGetOffset()
	{
		if (this->dsBuffer == NULL)
		{
			return 0.0f;
		}
		unsigned long position;
		this->dsBuffer->GetCurrentPosition(&position, NULL);
		return (float)position;
	}

	void DirectSound_Player::_sysSetOffset(float value)
	{
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->SetCurrentPosition((DWORD)value);
		}
	}

	bool DirectSound_Player::_sysPreparePlay()
	{
		if (this->dsBuffer != NULL)
		{
			return true;
		}
		this->buffer->prepare();
		WAVEFORMATEX wavefmt;
#if HAVE_WAV
		DirectSound_WAV_Source* wavSource = dynamic_cast<DirectSound_WAV_Source*>(this->buffer->getSource());
		if (wavSource != NULL)
		{
			wavefmt = wavSource->getWavefmt();
		}
		else
#endif
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
		bufferDesc.dwFlags = (DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS);
		bufferDesc.dwBufferBytes = (!this->sound->isStreamed() ? this->buffer->getSize() : STREAM_BUFFER_COUNT * STREAM_BUFFER_SIZE);
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

	void DirectSound_Player::_sysPrepareBuffer()
	{
		if (!this->sound->isStreamed())
		{
			this->_copyBuffer(0, this->buffer->getStream(), this->buffer->getSize());
			return;
		}
		int count = STREAM_BUFFER_COUNT;
		if (!this->paused)
		{
			this->bufferIndex = 0;
		}
		else
		{
			count = STREAM_BUFFER_COUNT - this->bufferCount;
		}
		if (count > 0)
		{
			count = this->_fillBuffers(this->bufferIndex, count);
			this->bufferCount += count;
			if (count > 0)
			{
				this->_copyBuffer(this->bufferIndex, this->buffer->getStream(), count * STREAM_BUFFER_SIZE);
				if (this->bufferCount < STREAM_BUFFER_COUNT)
				{
					count = STREAM_BUFFER_COUNT - this->bufferCount;
					int size = count * STREAM_BUFFER_SIZE;
					unsigned char* stream = new unsigned char[size];
					memset(stream, 0, size * sizeof(unsigned char));
					this->_copyBuffer(this->bufferCount, stream, size);
					delete [] stream;
				}
			}
		}
	}

	void DirectSound_Player::_copyBuffer(int index, unsigned char* stream, int size)
	{
		void* write1 = NULL;
		void* write2 = NULL;
		unsigned long length1;
		unsigned long length2;
		HRESULT result = this->dsBuffer->Lock(index * STREAM_BUFFER_SIZE, size, &write1, &length1, &write2, &length2, 0);
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
	}

	void DirectSound_Player::_sysUpdateGain()
	{
		if (this->dsBuffer != NULL)
		{
			float gain = this->_calcGain();
			LONG value = DSBVOLUME_MIN;
			if (gain > 0.0f)
			{
				value = (LONG)(log10(gain) / 4 * (DSBVOLUME_MAX - DSBVOLUME_MIN));
			}
			this->dsBuffer->SetVolume(value);
		}
	}

	void DirectSound_Player::_sysUpdateFadeGain()
	{
		if (this->dsBuffer != NULL)
		{
			float gain = this->_calcFadeGain();
			LONG value = DSBVOLUME_MIN;
			if (gain > 0.0f)
			{
				value = (LONG)(log10(gain) / 4 * (DSBVOLUME_MAX - DSBVOLUME_MIN));
			}
			this->dsBuffer->SetVolume(value);
		}
	}

	void DirectSound_Player::_sysPlay()
	{
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Play(0, 0, ((this->looping || this->sound->isStreamed()) ? DSBPLAY_LOOPING : 0));
			this->playing = true;
		}
	}

	void DirectSound_Player::_sysStop()
	{
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Stop();
			this->playing = false;
			if (this->sound->isStreamed())
			{
				if (this->paused)
				{
					int processed = this->_getProcessedBuffersCount();
					this->bufferIndex = (this->bufferIndex + processed) % STREAM_BUFFER_COUNT;
					this->bufferCount -= processed;
				}
				else
				{
					this->bufferIndex = 0;
					this->bufferCount = 0,
					this->buffer->rewind();
				}
			}
		}
	}

	void DirectSound_Player::_sysUpdateStream()
	{
		if (this->bufferCount == 0)
		{
			this->_stopSound();
			return;
		}
		int processed = this->_getProcessedBuffersCount();
		if (processed == 0)
		{
			return;
		}
		this->bufferCount = hmax(this->bufferCount - processed, 0);
		int count = this->_fillBuffers(this->bufferIndex, processed);
		if (count > 0)
		{
			this->_copyBuffer(this->bufferIndex, this->buffer->getStream(), count * STREAM_BUFFER_SIZE);
			this->bufferIndex = (this->bufferIndex + count) % STREAM_BUFFER_COUNT;
			this->bufferCount += count;
		}
		if (this->bufferCount < STREAM_BUFFER_COUNT && !this->looping)
		{
			count = STREAM_BUFFER_COUNT - this->bufferCount;
			int size = count * STREAM_BUFFER_SIZE;
			unsigned char* stream = new unsigned char[size];
			memset(stream, 0, size * sizeof(unsigned char));
			this->_copyBuffer(this->bufferCount, stream, size);
			delete [] stream;
		}
		if (this->bufferCount == 0)
		{
			this->_stopSound();
		}
	}

	int DirectSound_Player::_getProcessedBuffersCount()
	{
		unsigned long position;
		this->dsBuffer->GetCurrentPosition(&position, NULL);
		return ((position / STREAM_BUFFER_SIZE + STREAM_BUFFER_COUNT - this->bufferIndex) % STREAM_BUFFER_COUNT);
	}

	int DirectSound_Player::_fillBuffers(int index, int count)
	{
		int size = this->buffer->load(this->looping, count);
		return (size + STREAM_BUFFER_SIZE - 1) / STREAM_BUFFER_SIZE;
	}

}
#endif
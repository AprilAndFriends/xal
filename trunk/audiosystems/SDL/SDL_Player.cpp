/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_SDL
#include "Buffer.h"
#include "SDL_Player.h"
#include "Sound.h"

namespace xal
{
	SDL_Player::SDL_Player(Sound* sound, Buffer* buffer) : Player(sound, buffer), playing(false)
	{
	}

	SDL_Player::~SDL_Player()
	{
	}

	void SDL_Player::_update(float k)
	{
		Player::_update(k);
		/*
		if (this->playing)
		{
			unsigned long status;
			this->dsBuffer->GetStatus(&status);
			if ((status & DSBSTATUS_PLAYING) == 0)
			{
				this->playing = false;
			}
		}
		*/
	}

	float SDL_Player::_sysGetOffset()
	{
		return 0.0f;
		/*
		if (this->dsBuffer == NULL)
		{
			return 0.0f;
		}
		unsigned long position;
		this->dsBuffer->GetCurrentPosition(&position, NULL);
		return (float)position;
		*/
	}

	void SDL_Player::_sysSetOffset(float value)
	{
		/*
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->SetCurrentPosition((DWORD)value);
		}
		*/
	}

	bool SDL_Player::_sysPreparePlay()
	{
		return false;
		/*
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
		*/
	}

	void SDL_Player::_sysPrepareBuffer()
	{
		/*
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
		*/
	}

	void SDL_Player::_sysUpdateGain()
	{
		/*
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
		*/
	}

	void SDL_Player::_sysUpdateFadeGain()
	{
		/*
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
		*/
	}

	void SDL_Player::_sysPlay()
	{
		/*
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Play(0, 0, ((this->looping || this->sound->isStreamed()) ? DSBPLAY_LOOPING : 0));
			this->playing = true;
		}
		*/
	}

	void SDL_Player::_sysStop()
	{
		/*
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
		*/
	}

	void SDL_Player::_sysUpdateStream()
	{
		/*
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
		*/
	}

}
#endif
/// @file
/// @version 3.11
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if _COREAUDIO
#include <AudioUnit/AudioUnit.h>

#include "Buffer.h"
#include "CoreAudio_AudioManager.h"
#include "CoreAudio_Player.h"
#include "Sound.h"
#include "Endianess.h"
#include "xal.h"

namespace xal
{
	CoreAudio_Player::CoreAudio_Player(Sound* sound) : Player(sound), playing(false),
		position(0), currentGain(1.0f), readPosition(0), writePosition(0)
	{
		memset(this->circleBuffer, 0, STREAM_BUFFER * sizeof(unsigned char));
	}

	CoreAudio_Player::~CoreAudio_Player()
	{
	}

	void CoreAudio_Player::_getData(int size, unsigned char** data1, int* size1, unsigned char** data2, int* size2)
	{
		if (!this->sound->isStreamed())
		{
			int streamSize = this->buffer->load(this->looping, size);
			if (streamSize == 0)
			{
				*data1 = NULL;
				*size1 = 0;
				*data2 = NULL;
				*size2 = 0;
				return;
			}
			unsigned char* stream = this->buffer->getStream();
			*data1 = &stream[this->readPosition];
			*size1 = hmin(hmin(streamSize, streamSize - this->readPosition), size);
			*data2 = NULL;
			*size2 = 0;
			if (this->looping && this->readPosition + size > streamSize)
			{
				*data2 = stream;
				*size2 = size - *size1;
				this->readPosition = (this->readPosition + size) % streamSize;
			}
			else
			{
				this->readPosition = hmin(this->readPosition + size, streamSize);
			}
			return;
		}
		*data1 = &this->circleBuffer[this->readPosition];
		*size1 = size;
		*data2 = NULL;
		*size2 = 0;
		if (this->readPosition + size > STREAM_BUFFER)
		{
			*size1 = STREAM_BUFFER - this->readPosition;
			*data2 = this->circleBuffer;
			*size2 = size - *size1;
		}
		this->readPosition = (this->readPosition + size) % STREAM_BUFFER;
	}

	void CoreAudio_Player::_update(float timeDelta)
	{
		Player::_update(timeDelta);
		int size = this->buffer->getSize();
		if (size > 0 && this->position >= size)
		{
			if (this->looping)
			{
				this->position -= this->position / size * size;
			}
			else if (this->playing)
			{
				this->_systemStop();
			}
		}
	}

	void CoreAudio_Player::mixAudio(unsigned char* stream, int length, bool first)
	{
		if (!this->playing)
		{
			return;
		}
		unsigned char* data1;
		unsigned char* data2;
		int size1;
		int size2;
		this->_getData(length, &data1, &size1, &data2, &size2);
		if (size1 > 0)
		{
			if (first && this->currentGain == 1.0f)
			{
				memcpy(stream, data1, size1);
				if (size2 > 0)
				{
					memcpy(&stream[size1], data2, size2);
				}
#ifdef __BIG_ENDIAN__
				short* sStream = (short*)stream;
				for (int i = 0; i < size1; i++)
				{
					XAL_NORMALIZE_ENDIAN(sStream[i]);
				}
				for (int i = 0; i < size2; i++)
				{
					XAL_NORMALIZE_ENDIAN(sStream[size1 + i]);
				}
#endif				
			}
			else
			{
				short* sStream = (short*)stream;
				short* sData1 = (short*)data1;
				short* sData2 = (short*)data2;
				size1 = size1 * sizeof(unsigned char) / sizeof(short);
				size2 = size2 * sizeof(unsigned char) / sizeof(short);
				if (!first)
				{
					for (int i = 0; i < size1; i++)
					{
						XAL_NORMALIZE_ENDIAN(sStream[i]);
						sStream[i] = (short)hclamp((int)(sStream[i] + this->currentGain * sData1[i]), -32768, 32767);
						XAL_NORMALIZE_ENDIAN(sStream[i]);
					}
					for (int i = 0; i < size2; i++)
					{
						XAL_NORMALIZE_ENDIAN(sStream[size1 + i]);
						sStream[size1 + i] = (short)hclamp((int)(sStream[size1 + i] + this->currentGain * sData2[i]), -32768, 32767);
						XAL_NORMALIZE_ENDIAN(sStream[size1 + i]);
					}
				}
				else
				{
					for (int i = 0; i < size1; i++)
					{
						XAL_NORMALIZE_ENDIAN(sStream[i]);
						sStream[i] = (short)(sData1[i] * this->currentGain);
						XAL_NORMALIZE_ENDIAN(sStream[i]);
					}
					for (int i = 0; i < size2; i++)
					{
						XAL_NORMALIZE_ENDIAN(sStream[size1 + i]);
						sStream[size1 + i] = (short)(sData2[i] * this->currentGain);
						XAL_NORMALIZE_ENDIAN(sStream[size1 + i]);
					}
				}
			}
			this->position += size1 + size2;
		}
	}

	float CoreAudio_Player::_systemGetOffset()
	{
		return this->offset;
	}

	void CoreAudio_Player::_systemSetOffset(float value)
	{
		this->offset = value;
	}

	bool CoreAudio_Player::_systemPreparePlay()
	{
		return true;
	}

	void CoreAudio_Player::_systemPrepareBuffer()
	{
		if (!this->sound->isStreamed())
		{
			this->buffer->load(this->looping, this->buffer->getSize());
			return;
		}
		if (!this->paused)
		{
			this->readPosition = 0;
			this->writePosition = 0;
			int size = this->_fillBuffer(STREAM_BUFFER_SIZE);
			if (size < STREAM_BUFFER)
			{
				memset(&this->circleBuffer[size], 0, (STREAM_BUFFER - size) * sizeof(unsigned char));
			}
		}
	}

	void CoreAudio_Player::_systemUpdateGain(float gain)
	{
		this->currentGain = gain;
	}

	void CoreAudio_Player::_systemPlay()
	{
		this->playing = true;
	}

	int CoreAudio_Player::_systemStop()
	{
		this->playing = false;
		if (!this->paused)
		{
			this->position = 0;
			this->readPosition = 0;
			this->writePosition = 0;
			this->buffer->rewind();
		}
		return 0;
	}

	int CoreAudio_Player::_systemUpdateStream()
	{
		int count = 0, result = 0;
		if (this->readPosition > this->writePosition)
		{
			count = (this->readPosition - this->writePosition) / STREAM_BUFFER_SIZE;
		}
		else if (this->readPosition < this->writePosition)
		{
			count = (STREAM_BUFFER - this->writePosition + this->readPosition) / STREAM_BUFFER_SIZE;
		}
		if (count >= STREAM_BUFFER_COUNT / 2)
		{
			result = this->_fillBuffer(STREAM_BUFFER_SIZE);
		}
		return result;
	}

	int CoreAudio_Player::_fillBuffer(int size)
	{
		int streamSize = this->buffer->load(this->looping, size);
		unsigned char* stream = this->buffer->getStream();
		if (this->writePosition + streamSize <= STREAM_BUFFER)
		{
			memcpy(&this->circleBuffer[this->writePosition], stream, streamSize * sizeof(unsigned char));
		}
		else
		{
			int remaining = STREAM_BUFFER - this->writePosition;
			memcpy(&this->circleBuffer[this->writePosition], stream, remaining * sizeof(unsigned char));
			memcpy(this->circleBuffer, stream, (streamSize - remaining) * sizeof(unsigned char));
		}
		this->writePosition = (this->writePosition + streamSize) % STREAM_BUFFER;
		if (!this->looping && streamSize < size) // fill with silence if source is at the end
		{
			streamSize = size - streamSize;
			if (this->writePosition + streamSize <= STREAM_BUFFER)
			{
				memset(&this->circleBuffer[this->writePosition], 0, streamSize * sizeof(unsigned char));
			}
			else
			{
				int remaining = STREAM_BUFFER - this->writePosition;
				memset(&this->circleBuffer[this->writePosition], 0, remaining * sizeof(unsigned char));
				memset(this->circleBuffer, 0, (streamSize - remaining) * sizeof(unsigned char));
			}
			this->writePosition = (this->writePosition + streamSize) % STREAM_BUFFER;
			streamSize = size;
		}
		return streamSize;
	}

}
#endif
/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hexception.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Category.h"
#include "Player.h"

namespace xal
{
	Category::Category(chstr name, BufferMode bufferMode, SourceMode sourceMode) : gain(1.0f),
		gainFadeTarget(-1.0f), gainFadeSpeed(-1.0f), gainFadeTime(0.0f)
	{
		this->name = name;
		this->bufferMode = bufferMode;
		this->sourceMode = sourceMode;
	}

	Category::~Category()
	{
	}

	float Category::getGain()
	{
		hmutex::ScopeLock lock(&xal::manager->mutex);
		return this->_getGain();
	}

	float Category::_getGain() const
	{
		float result = this->gain;
		if (this->_isGainFading())
		{
			result += (this->gainFadeTarget - this->gain) * this->gainFadeTime;
		}
		return result;
	}

	void Category::setGain(float value)
	{
		hmutex::ScopeLock lock(&xal::manager->mutex);
		this->_setGain(value);
	}

	void Category::_setGain(float value)
	{
		this->gain = value;
		this->gainFadeTarget = -1.0f;
		this->gainFadeSpeed = -1.0f;
		this->gainFadeTime = 0.0f;
		foreach (Player*, it, xal::manager->players)
		{
			(*it)->_systemUpdateGain();
		}
	}

	bool Category::isStreamed() const
	{
		return (this->bufferMode == BufferMode::Streamed);
	}

	bool Category::isMemoryManaged() const
	{
		return (this->bufferMode == BufferMode::Managed);
	}

	bool Category::isGainFading()
	{
		hmutex::ScopeLock lock(&xal::manager->mutex);
		return this->_isGainFading();
	}

	bool Category::_isGainFading() const
	{
		return (this->gainFadeTarget >= 0.0f && this->gainFadeSpeed > 0.0f);
	}

	void Category::fadeGain(float gainTarget, float fadeTime)
	{
		hmutex::ScopeLock lock(&xal::manager->mutex);
		if (fadeTime > 0.0f)
		{
			this->gainFadeTarget = hclamp(gainTarget, 0.0f, 1.0f);
			this->gainFadeTime = 0.0f;
			this->gainFadeSpeed = 1.0f / fadeTime;
		}
	}

	void Category::_update(float timeDelta)
	{
		if (timeDelta > 0.0f && this->_isGainFading())
		{
			this->gainFadeTime += this->gainFadeSpeed * timeDelta;
			if (this->gainFadeTime >= 1.0f)
			{
				this->gain = this->gainFadeTarget;
				this->gainFadeTarget = -1.0f;
				this->gainFadeSpeed = -1.0f;
				this->gainFadeTime = 0.0f;
			}
		}
	}

}

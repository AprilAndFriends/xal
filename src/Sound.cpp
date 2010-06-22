/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <iostream>
#include <hltypes/hstring.h>
#include "Sound.h"
#include "SoundManager.h"

#ifndef __APPLE__
	#include <AL/al.h>
#else
	#include <OpenAL/al.h>
#endif

namespace xal
{
	Sound::Sound(chstr name)
	{
		int slash=name.rfind("/"),backslash=name.rfind("\\");
		hstr new_name=name((slash > backslash) ? slash+1 : backslash+1);
		
		mLoop=false;
		mName=new_name(0,new_name.size()-4);
		mSource=0;
		mPaused=0;
		mGain=1.0f;
		mCategory="sound";
		mPosition.x=mPosition.y=mPosition.z=0.0f;
		mFadeTimer=mFadeSpeed=-1.0f;
		mFadeAction=XAL_FADE_NOTHING;
		mBuffer=0;
	}

	Sound::~Sound()
	{
		SoundManager::getSingleton().logMessage("destroying sound: "+mName);
		stop();
		if (mBuffer) alDeleteBuffers(1,&mBuffer);
		mBuffer=0;
		mSource=0;
		SoundManager::getSingleton()._unregisterSound(this);
	}

	unsigned int Sound::getSource()
	{
		return mSource;
	}

	void Sound::update(float k)
	{
		if (!mBuffer) return;
		if (mFadeTimer >= 0)
		{
			mFadeTimer+=mFadeSpeed*k;
			if (mFadeSpeed > 0 && mFadeTimer > 1)
			{
				alSourcef(mSource,AL_GAIN,mGain*SoundManager::getSingleton().getCategoryGain(mCategory));
				mFadeTimer=mFadeSpeed=-1.0f;
				mFadeAction=XAL_FADE_NOTHING;
			}
			else if (mFadeSpeed < 0 && mFadeTimer < 0)
			{
				if (mFadeAction == XAL_FADE_STOP)
				{
					alSourceStop(mSource);
					mSource=0;
				}
				else
				{
					pause();
				}
				mFadeTimer=mFadeSpeed=-1.0f;
				mFadeAction=XAL_FADE_NOTHING;
			}
			else
			{
				alSourcef(mSource,AL_GAIN,mFadeTimer*mGain*SoundManager::getSingleton().getCategoryGain(mCategory));
			}
		}

		if (!mPaused && mSource != 0)
		{
			int state;
			alGetSourcei(mSource, AL_SOURCE_STATE, &state);
			if (state != AL_PLAYING) mSource=0;
		}
	}

	bool Sound::isPlaying()
	{
		int state;
		if (!mSource) return false;
		alGetSourcei(mSource, AL_SOURCE_STATE, &state);
		return (state == AL_PLAYING);
	}

	void Sound::play(float fade_in_time,float x,float y,float z)
	{
		if (!mBuffer) return;
		if (mPaused)
		{
			SoundManager::getSingleton().lockSource(mSource,0);
			mPaused=0;
		}
		else
		{
			mSource=SoundManager::getSingleton().allocateSource(this);
			alSourcei(mSource,AL_BUFFER,mBuffer);
		}
		alSourcei(mSource,AL_LOOPING,mLoop);
		if (!(x == -1 && y == -1 && z == -1))
		{
			mPosition.x=x; mPosition.y=y; mPosition.z=z;
		}
		alSource3f(mSource,AL_POSITION,mPosition.x,mPosition.y,mPosition.z);
		alSourcei(mSource,AL_SOURCE_RELATIVE,(mPosition.x == 0 && mPosition.y == 0 && mPosition.z == 0));

		if (fade_in_time > 0)
		{
			alSourcef(mSource,AL_GAIN,0);
			mFadeTimer=0;
			mFadeSpeed=1.0f/fade_in_time;
			mFadeAction=XAL_FADE_NOTHING;
		}
		else
			alSourcef(mSource,AL_GAIN,mGain*SoundManager::getSingleton().getCategoryGain(mCategory));


		alSourcePlay(mSource);
		
	}

	void Sound::stop(float fade_out_time)
	{
		if (!mBuffer) return;
		if (fade_out_time > 0)
		{
			mFadeTimer=1;
			mFadeSpeed=-1.0f/fade_out_time;
			mFadeAction=XAL_FADE_STOP;
		}
		else
		{
			SoundManager::getSingleton().stopSourcesWithBuffer(mBuffer);
			mSource=0;
		}
		mPaused=0;
	}

	void Sound::pause(float fade_out_time)
	{
		if (mSource)
		{
			if (fade_out_time > 0)
			{
				mFadeTimer=1;
				mFadeSpeed=-1.0f/fade_out_time;
				mFadeAction=XAL_FADE_PAUSE;
			}
			else
			{
				alSourcePause(mSource);
				SoundManager::getSingleton().lockSource(mSource,1);
				mPaused=1;
			}
		}
	}

	void Sound::setPosition(float x,float y,float z)
	{
		mPosition.x=x; mPosition.y=y; mPosition.z=z;
		if (mSource != 0) alSource3f(mSource,AL_POSITION,x,y,z);
	}

	XALposition Sound::getPosition()
	{
		return mPosition;
	}

	float Sound::getSampleOffset()
	{
		if (!mBuffer) return 0;
		float value;
		alGetSourcef(mSource,AL_SEC_OFFSET,&value);
		return value;
	}

	void Sound::setGain(float gain)
	{
		mGain=gain;
		// this only makes sense for those sounds who have only one instance playing (eg. music)
		if (mSource != 0)
		{
			float cgain=SoundManager::getSingleton().getCategoryGain(mCategory);
			alSourcef(mSource,AL_GAIN,mGain*cgain);
		}
	}

	float Sound::getGain()
	{
		return mGain;
	}

	void Sound::setLoop(bool loop)
	{
		mLoop=loop;
	}

	bool Sound::getLoop()
	{
		return mLoop;
	}
}

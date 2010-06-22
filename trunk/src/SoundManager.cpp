/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/hstring.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#ifndef __APPLE__
  #include <AL/al.h>
  #include <AL/alc.h>
#else
  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>
#endif

#include "SoundManager.h"
#include "Sound.h"
#include "OggSound.h"
#include "DummySound.h"


namespace xal
{
	void xal_writelog(chstr text)
	{
		printf("%s\n",text.c_str());
	}
	
	void (*g_logFunction)(chstr)=xal_writelog;
	
	ALCdevice* gDevice=0;
	ALCcontext* gContext=0;
	bool g_destroying=0;

	SoundManager* g_sm_singleton_ptr;

	SoundManager& SoundManager::getSingleton()
	{
		return *g_sm_singleton_ptr;
	}

	SoundManager* SoundManager::getSingletonPtr()
	{
		return g_sm_singleton_ptr;
	}

	SoundManager::SoundManager(chstr device_name)
	{
		// singleton
		g_sm_singleton_ptr=this;
		
		if (device_name == "Dummy")
		{
			mDeviceName="Dummy";
			return;
		}
		// init OpenAL
		SoundManager::getSingleton().logMessage("Initializing OpenAL");	

		gDevice = alcOpenDevice(device_name.c_str());
		if (alcGetError(gDevice) != ALC_NO_ERROR) goto Fail;
		mDeviceName=alcGetString(gDevice,ALC_DEVICE_SPECIFIER);
		SoundManager::getSingleton().logMessage("Audio device: "+mDeviceName);

		gContext = alcCreateContext(gDevice, NULL);
		if (alcGetError(gDevice) != ALC_NO_ERROR) goto Fail;
		alcMakeContextCurrent(gContext);
		if (alcGetError(gDevice) != ALC_NO_ERROR) goto Fail;

		ALuint sources[XAL_MAX_SOURCES];
		alGenSources(XAL_MAX_SOURCES,sources);
		for (int i=0;i<XAL_MAX_SOURCES;i++)
		{
			mSources[i].id=sources[i];
			mSources[i].locked=0;
		}
		mCategoryGains["sound"]=1.0;
		mCategoryGains["music"]=1.0;

		return;
	Fail:
		gDevice=0;
		gContext=0;
		mDeviceName="";
	}

	SoundManager::~SoundManager()
	{
		SoundManager::getSingleton().logMessage("Destroying OpenAL");
		g_destroying=1;
		if (gDevice)
		{
			for (int i=0;i<XAL_MAX_SOURCES;i++)
				alDeleteSources(1,&mSources[i].id);

			std::map<hstr,Sound*>::iterator it=mSounds.begin();
			for (;it != mSounds.end();it++)
				delete it->second;
			alcMakeContextCurrent(NULL);
			alcDestroyContext(gContext);
			alcCloseDevice(gDevice);
		}
	}
	
	void SoundManager::logMessage(chstr message)
	{
		g_logFunction(message);
	}
	
	void SoundManager::setLogFunction(void (*fnptr)(chstr))
	{
		g_logFunction=fnptr;
	}
	
	hstr SoundManager::getDeviceName()
	{
		return mDeviceName;
	}

	unsigned int SoundManager::allocateSource(Sound* new_owner)
	{
		int i,state;
		// get the first available non-playing source
		for (i=0;i<XAL_MAX_SOURCES;i++)
		{
			alGetSourcei(mSources[i].id, AL_SOURCE_STATE, &state);
			if (state != AL_PLAYING && !mSources[i].locked)
			{
				return mSources[i].id;
			}
		}
		// if we can't find a non playing source, try a random unlocked source
		int n;
		for (i=0;i<10;i++)
		{
			n=rand()%XAL_MAX_SOURCES;
			if (!mSources[n].locked) return mSources[n].id;
		}

		SoundManager::getSingleton().logMessage("SoundManager: unable to allocate audio source!");
		return 0;
	}

	void SoundManager::stopSourcesWithBuffer(unsigned int buffer)
	{
		int i,buf;
		for (i=0;i<XAL_MAX_SOURCES;i++)
		{
			alGetSourcei(mSources[i].id, AL_BUFFER, &buf);
			if (buf == buffer)
			{
				alSourceStop(mSources[i].id);
				mSources[i].locked=0;
			}
		}
	}

	void SoundManager::update(float k)
	{
		std::map<hstr,Sound*>::iterator it=mSounds.begin();
		for (;it != mSounds.end();it++)
			it->second->update(k);
	}

	void SoundManager::setListenerPosition(float x,float y,float z)
	{
		alListener3f(AL_POSITION,x,y,z);
	}

	XALposition SoundManager::getListenerPosition()
	{
		XALposition pos;
		alGetListener3f(AL_POSITION,&pos.x,&pos.y,&pos.z);
		return pos;
	}

	Sound* SoundManager::createSound(chstr filename,chstr category)
	{
		int ogg=filename.find(".ogg");
	
		Sound* s=NULL;
		if (!gDevice) s=new DummySound(filename);
		else if (ogg != hstr::npos)
			s=new OggSound(filename);
		if (!s)	return NULL;

		s->setCategory(category);
		mSounds[s->getName()]=s;
		return s;
	}

	Sound* SoundManager::getSound(chstr name)
	{
		if (mSounds.find(name) == mSounds.end()) return 0;
		else return mSounds[name];
	}
	
	void SoundManager::_unregisterSound(Sound* ptr)
	{
		if (g_destroying) return;
		std::map<hstr,Sound*>::iterator it=mSounds.begin();
		for (;it != mSounds.end();it++)
			if (it->second == ptr)
			{
				mSounds.erase(it);
				break;
			}
	}

	void SoundManager::lockSource(unsigned int source_id,bool lock)
	{
		for (int i=0;i<XAL_MAX_SOURCES;i++)
		{
			if (mSources[i].id == source_id)
			{
				mSources[i].locked=lock;
				return;
			}
		}
	}

	void SoundManager::setCategoryGain(chstr category,float gain)
	{
		int src;
		mCategoryGains[category]=gain;

		std::map<hstr,Sound*>::iterator it=mSounds.begin();
		for (;it != mSounds.end();it++)
		{
			if (it->second->getCategory() == category)
			{
				src=it->second->getSource();
				if (src)
				{
					alSourcef(src,AL_GAIN,it->second->getGain()*gain);
				}
			}
		}
	}

	float SoundManager::getCategoryGain(chstr category)
	{
		if (mCategoryGains.find(category) == mCategoryGains.end()) return 1;
		return mCategoryGains[category];
	}

}

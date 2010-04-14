#include <iostream>
#include <string>
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
using namespace std;

namespace xal
{
	void xal_writelog(const std::string& text)
	{
		printf("%s\n",text.c_str());
	}
	
	void (*g_logFunction)(const std::string&)=xal_writelog;
	
	ALCdevice* gDevice=0;
	ALCcontext* gContext=0;
	
	SoundManager* g_sm_singleton_ptr;

	SoundManager& SoundManager::getSingleton()
	{
		return *g_sm_singleton_ptr;
	}

	SoundManager* SoundManager::getSingletonPtr()
	{
		return g_sm_singleton_ptr;
	}

	SoundManager::SoundManager(string device_name)
	{
		// singleton
		g_sm_singleton_ptr=this;
		// init OpenAL
		SoundManager::getSingleton().logMessage("Initializing OpenAL");	

		gDevice = alcOpenDevice(device_name.c_str());
		if (alcGetError(gDevice) != ALC_NO_ERROR) goto Fail;
		mDeviceName=alcGetString(gDevice,ALC_DEVICE_SPECIFIER);
		SoundManager::getSingleton().logMessage("Choose device: "+mDeviceName);
		
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
		gDevice=NULL;
		gContext=NULL;
		mDeviceName="";
	}

	SoundManager::~SoundManager()
	{
		SoundManager::getSingleton().logMessage("Destroying OpenAL");
		if (gDevice)
		{
			for (int i=0;i<XAL_MAX_SOURCES;i++)
				alDeleteSources(1,&mSources[i].id);

			std::list<Sound*>::iterator it=mSounds.begin();
			for (;it != mSounds.end();it++)
				delete *it;
			alcMakeContextCurrent(NULL);
			alcDestroyContext(gContext);
			alcCloseDevice(gDevice);
		}
	}
	
	void SoundManager::logMessage(const std::string& message)
	{
		g_logFunction(message);
	}
	
	std::string SoundManager::getDeviceName()
	{
		return mDeviceName;
	}

	void SoundManager::destroySound(Sound* s)
	{
		mSounds.remove(s);
		delete s;
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
		for (list<Sound*>::iterator i=mSounds.begin();i!=mSounds.end();i++)
			(*i)->update(k);
	}

	void SoundManager::setListenerPosition(float x,float y,float z)
	{
		alListener3f(AL_POSITION,x,y,z);
	}

	ALposition SoundManager::getListenerPosition()
	{
		ALposition pos;
		alGetListener3f(AL_POSITION,&pos.x,&pos.y,&pos.z);
		return pos;
	}

	Sound* SoundManager::createSound(std::string filename,std::string category)
	{
		int ogg=filename.find(".ogg");
		int speex=filename.find(".spx");
		
		Sound* s=NULL;
		if (ogg != string::npos)
			s=new OggSound(filename);
		if (!s)	return NULL;

		s->mCategory=category;
		mSounds.push_back(s);
		return s;
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

	void SoundManager::setCategoryGain(std::string category,float gain)
	{
		int src;
		mCategoryGains[category]=gain;

		for (list<Sound*>::iterator i=mSounds.begin();i!=mSounds.end();i++)
		{
			if ((*i)->mCategory == category)
			{
				src=(*i)->getSource();
				if (src)
				{
					alSourcef(src,AL_GAIN,(*i)->getGain()*gain);
				}
			}
		}
	}

	float SoundManager::getCategoryGain(std::string category)
	{
		return mCategoryGains[category];
	}

}

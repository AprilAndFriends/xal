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

ALCdevice* gDevice=0;
ALCcontext* gContext=0;

std::string g_logPrefix(".");

static bool s_noLogFile=false;

void setLogPrefix(std::string prefix)
{
	g_logPrefix=prefix;
}

void writelog(std::string text)
{
	std::string s=g_logPrefix+"/audio.log";
	cout << "[pyaudio] " << text << '\n';

	if(!s_noLogFile)
	{
		FILE* f=fopen(s.c_str(),"a");
		if(f) 
			fprintf(f,"%s\n",text.c_str());
		fclose(f);
	}
}

SoundManager* SoundManager::_singleton_ptr;

SoundManager* SoundManager::getSingleton()
{
	return _singleton_ptr;
}

SoundManager::SoundManager(string device_name)
{
	// clear log file
	std::string s=g_logPrefix+"/audio.log";
	FILE* f=fopen(s.c_str(),"w");
	if(!f)
	{
		s_noLogFile=true;
	}
	else
	{
		fclose(f);
	}
	// singleton
	SoundManager::_singleton_ptr=this;
	// init OpenAL
	writelog("Initializing OpenAL");	

	gDevice = alcOpenDevice(device_name.c_str());
	if (alcGetError(gDevice) != ALC_NO_ERROR) goto Fail;
	mDeviceName=alcGetString(gDevice,ALC_DEVICE_SPECIFIER);
	writelog("Choose device: "+mDeviceName);
	
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
	writelog("Destroying OpenAL");
	if (gDevice)
	{
		alcMakeContextCurrent(NULL);
		alcDestroyContext(gContext);
		alcCloseDevice(gDevice);
	}
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

	writelog("SoundManager: unable to allocate audio source!");
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

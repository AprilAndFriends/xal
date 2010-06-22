/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_SOUND_MANAGER_H
#define XAL_SOUND_MANAGER_H

#include <hltypes/hstring.h>
#include <map>
#include "xalExport.h"
#include "Sound.h"


#define XAL_MAX_SOURCES 16

namespace xal
{

	struct xalExport Source
	{
		unsigned int id;
		bool locked; // usually a sound locks a source when it pauses
	};

	class xalExport SoundManager
	{
		Source mSources[XAL_MAX_SOURCES];
		hstr mDeviceName;
		std::map<hstr,float> mCategoryGains;
		std::map<hstr,Sound*> mSounds;
	public:
		SoundManager(chstr device_name);
		~SoundManager();

		void _unregisterSound(Sound* ptr);

		unsigned int allocateSource(Sound* new_owner);

		Sound* createSound(chstr filename,chstr category="sound");
		void update(float k);
		
		Sound* getSound(chstr name);
		

		void stopSourcesWithBuffer(unsigned int buffer);

		void setListenerPosition(float x,float y,float z);
		XALposition getListenerPosition();

		hstr getDeviceName();

		void logMessage(chstr message);
		static void setLogFunction(void (*fnptr)(chstr));

		void lockSource(unsigned source_id,bool lock);
		void setCategoryGain(chstr category,float gain);
		float getCategoryGain(chstr category);

		static SoundManager& getSingleton();
		static SoundManager* getSingletonPtr();
	};
}

#endif

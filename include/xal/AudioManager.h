/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
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
	class Category;
	class Source;

	class xalExport AudioManager
	{
	public:
		AudioManager(chstr deviceName = "");
		~AudioManager();
		
		void logMessage(chstr message);
		hstr getDeviceName() { return this->deviceName; }
		bool isEnabled();
		Category* getCategoryByName(chstr name);
		void createCategory(chstr name);
		void setCategoryGain(chstr category, float gain);
		
		void update(float k);
		
		Source* allocateSource();
		Sound* loadSound(chstr filename, chstr categoryName, chstr prefix = "");
		harray<hstr> loadPath(chstr path, chstr prefix = "");
		harray<hstr> loadPathCategory(chstr path, chstr category, chstr prefix = "");
		void unloadSound(Sound* sound);
		Sound* getSound(chstr name);
		
	protected:
		Source* sources[XAL_MAX_SOURCES];
		hstr deviceName;
		std::map<hstr, Category*> categories;
		std::map<hstr, Sound*> sounds;
		
	};
	
	xalFnExport void init(chstr deviceName = "");
	xalFnExport void destroy();
	xalFnExport void setLogFunction(void (*function)(chstr));
	
}
xalFnExport extern xal::AudioManager* audiomgr;

#endif

#ifndef XAL_SOUND_MANAGER_H
#define XAL_SOUND_MANAGER_H

#include <string>
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
		std::string mDeviceName;
		std::map<std::string,float> mCategoryGains;
		std::map<std::string,Sound*> mSounds;
	public:
		SoundManager(std::string device_name);
		~SoundManager();

		void _unregisterSound(Sound* ptr);

		unsigned int allocateSource(Sound* new_owner);

		Sound* createSound(std::string filename,std::string category="sound");
		void update(float k);
		
		Sound* getSound(const std::string& name);
		

		void stopSourcesWithBuffer(unsigned int buffer);

		void setListenerPosition(float x,float y,float z);
		XALposition getListenerPosition();

		std::string getDeviceName();

		void logMessage(const std::string& message);
		static void setLogFunction(void (*fnptr)(const std::string&));

		void lockSource(unsigned source_id,bool lock);
		void setCategoryGain(std::string category,float gain);
		float getCategoryGain(std::string category);

		static SoundManager& getSingleton();
		static SoundManager* getSingletonPtr();
	};
}

#endif

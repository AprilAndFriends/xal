#ifndef XAL_SOUND_MANAGER_H
#define XAL_SOUND_MANAGER_H

#include <string>
#include <map>
#include <list>
#include "Sound.h"


#define XAL_MAX_SOURCES 16

struct Source
{
	unsigned int id;
	bool locked; // usually a sound locks a source when it pauses
};

class SoundManager
{
	Source mSources[XAL_MAX_SOURCES];
	std::string mDeviceName;
	std::map<std::string,float> mCategoryGains;
	std::list<Sound*> mSounds;
public:
	SoundManager(std::string device_name);
	~SoundManager();

	unsigned int allocateSource(Sound* new_owner);

	Sound* createSound(std::string filename,std::string category="sound");
	void destroySound(Sound* s);
	void update(float k);

	void stopSourcesWithBuffer(unsigned int buffer);

	void setListenerPosition(float x,float y,float z);
	ALposition getListenerPosition();

	std::string getDeviceName();

	void lockSource(unsigned source_id,bool lock);
	void setCategoryGain(std::string category,float gain);
	float getCategoryGain(std::string category);

	static SoundManager* _singleton_ptr;
	static SoundManager* getSingleton();
};

void setLogPrefix(std::string prefix);

#endif

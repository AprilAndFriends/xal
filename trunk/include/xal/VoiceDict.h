#ifndef VOICEDICT_H
#define VOICEDICT_H

#include <map>
#include <string>

#include "Sound.h"

class VoiceSound : public Sound
{
public:
	VoiceSound(std::string name,unsigned char* data,int nBytes,int freq);
	~VoiceSound();
};
class VoiceDict
{
	std::map<std::string,VoiceSound*> mVoices;
	std::string mName;
public:
	VoiceDict(std::string name);
	~VoiceDict();

	void load(std::string base_filename);
	void destroy();

	Sound* getVoice(std::string name);
};

#endif

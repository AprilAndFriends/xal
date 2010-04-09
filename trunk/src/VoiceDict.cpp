#include "VoiceDict.h"
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>


#ifndef __APPLE__
  #include <AL/al.h>
  #include <AL/alc.h>
#else
  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>
#endif


void writelog(std::string text);




VoiceSound::VoiceSound(std::string name,unsigned char* data,int nBytes,int freq) : Sound(name)
{
	writelog("loading voice: "+name);
	alGenBuffers(1,&mBuffer);
	alBufferData(mBuffer, AL_FORMAT_MONO16, data, nBytes, freq);
	mCategory="voice";
	mDuration=float(nBytes)/(freq*2);
}

VoiceSound::~VoiceSound()
{

}


VoiceDict::VoiceDict(std::string name)
{
	mName=name;
	writelog("creating voice dict: "+mName);
}

VoiceDict::~VoiceDict()
{
	destroy();
}

void VoiceDict::destroy()
{
	if (mVoices.size() == 0) return;
	writelog("destroying voice dict: "+mName);

	for (std::map<std::string,VoiceSound*>::iterator it=mVoices.begin();it!=mVoices.end();it++)
		delete it->second;

	mVoices.clear();
}

void VoiceDict::load(std::string base_filename)
{
	writelog(mName+": loading voices from VDF file");
	std::string vdf=base_filename+"/voice.vdf";
	std::string vogg=base_filename+"/voice.ogg";
	FILE* f;
	
    vorbis_info *info;
    OggVorbis_File oggFile;

	f = fopen(vogg.c_str(), "rb");
	if (!f)
	{
		writelog(mName+": Error opening voice.ogg!");
		return;
	}
    ov_open(f, &oggFile, NULL, 0);
	info = ov_info(&oggFile, -1);
	int rate=info->rate;
	if (info->channels > 1)
	{
		writelog(mName+": Error reading voice.ogg, file isn't mono!");
		return;
	}
    unsigned long len = ov_pcm_total(&oggFile, -1) * 2; // always 16 bit data
    unsigned char *data = new unsigned char[len];

    if (data)
    {
        int bs = -1;
        unsigned long todo = len;
        unsigned char *bufpt = data;

        while (todo)
        {
                int read = ov_read(&oggFile, (char *) bufpt, todo, 0, 2, 1, &bs);
                todo -= read;
                bufpt += read;
        }
    }
    else
        writelog("OggSound: couldn't allocate ogg buffer");

    ov_clear(&oggFile);
	fclose(f);

	
	// VDF file
	f=fopen(vdf.c_str(),"r");
	if (f)
	{
		char name[512];
		float start,end;
		VoiceSound* s;
		while (fscanf(f,"%s %f %f",name,&start,&end) == 3)
		{
			start*=rate; end*=rate;
			s=new VoiceSound(name,data+int(start*2),(end-start)*2,rate);
			mVoices[std::string(name)]=s;
		}
		fclose(f);
	}
	else
	{
		writelog("error loading voice dict, VDF file not found!");
	}
	if (data) delete [] data;
}

Sound* VoiceDict::getVoice(std::string name)
{
	return mVoices[name];
}

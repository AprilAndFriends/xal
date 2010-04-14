#include <iostream>
#include <string>
#include "OggSound.h"
#include "SoundManager.h"
#include "Endianess.h"
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

namespace xal
{

	using namespace std;
	void writelog(std::string text);

	OggSound::OggSound(string filename) : Sound(filename)
	{
		writelog("loading ogg sound: "+filename);

		alGenBuffers(1,&mBuffer);

		vorbis_info *info;
		OggVorbis_File oggFile;


		if (ov_fopen((char*) filename.c_str(), &oggFile) != 0)
		{
			writelog("OggSound: Error opening file!");
			return;
		}
		info = ov_info(&oggFile, -1);
		unsigned long len = ov_pcm_total(&oggFile, -1) * info->channels * 2; // always 16 bit data
		unsigned char *data = new unsigned char[len];

		if (data)
		{
			int bs = -1;
			unsigned long todo = len;
			unsigned char *bufpt = data;

			while (todo)
			{
					int read = ov_read(&oggFile, (char *) bufpt, todo, 0, 2, 1, &bs);
			if (!read) 
			{
				len-=todo;
				break;
			}
					todo -= read;
					bufpt += read;
		   }

		#ifdef __BIG_ENDIAN__
		for(uint16_t* p=(uint16_t*)data;(unsigned char*)p<bufpt;p++)
		{
			NORMALIZE_ENDIAN(*p);
		}
		#endif	

			alBufferData(mBuffer, (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data, len, info->rate);
			mDuration=float(len)/(info->rate*info->channels*2);
			delete [] data;
		}
		else
			writelog("OggSound: couldn't allocate ogg buffer");

		ov_clear(&oggFile);

		//fclose(f); // unnecessary! ov_clear also closes the file. also crashes under linux

		
	}

	OggSound::~OggSound()
	{
		writelog("destroying ogg sound: "+mName);
		alDeleteBuffers(1,&mBuffer);
	}
}

#ifndef XAL_OGG_SOUND_H
#define XAL_OGG_SOUND_H

#include "Sound.h"
#include "xalExport.h"

namespace xal
{

	class xalExport OggSound : public Sound
	{
	public:
		OggSound(std::string filename);
		~OggSound();
	};

}
#endif

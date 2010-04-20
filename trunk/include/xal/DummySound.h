#ifndef XAL_DUMMY_SOUND_H
#define XAL_DUMMY_SOUND_H

#include "Sound.h"
#include "xalExport.h"

namespace xal
{

	class xalExport DummySound : public Sound
	{
	public:
		DummySound(std::string filename);
		~DummySound();
	};

}
#endif

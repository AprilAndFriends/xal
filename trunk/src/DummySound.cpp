#include "DummySound.h"
#include "SoundManager.h"

namespace xal
{

	DummySound::DummySound(std::string filename) : Sound(filename)
	{
		SoundManager::getSingleton().logMessage("creating dummy sound: "+filename);
	}

	DummySound::~DummySound()
	{
	}
}

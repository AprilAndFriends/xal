/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include <xal/AudioManager.h>
#include <xal/Sound.h>

#include "ParallelSoundManager.h"

namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	ParallelSoundManager::ParallelSoundManager(float fadeTime) :
		sounds(harray<hstr>())
	{
		this->fadeTime = fadeTime;
	}
	
	ParallelSoundManager::~ParallelSoundManager()
	{
	}
	
/******* CONSTRUCT / DESTRUCT ******************************************/

	void ParallelSoundManager::play(harray<hstr> names)
	{
		harray<hstr> paused = this->sounds / names;
		for (hstr* it = paused.iterate(); it; it = paused.next())
		{
			xal::mgr->getSound(*it)->pause(this->fadeTime);
		}
		harray<hstr> started = names / this->sounds;
		for (hstr* it = started.iterate(); it; it = started.next())
		{
			xal::mgr->getSound(*it)->play(this->fadeTime);
		}
		this->sounds = names;
	}
	
	void ParallelSoundManager::playAll()
	{
		for (hstr* it = this->sounds.iterate(); it; it = this->sounds.next())
		{
			xal::mgr->getSound(*it)->play(this->fadeTime);
		}
	}
	
	void ParallelSoundManager::pauseAll()
	{
		for (hstr* it = this->sounds.iterate(); it; it = this->sounds.next())
		{
			xal::mgr->getSound(*it)->pause(this->fadeTime);
		}
	}
	
	void ParallelSoundManager::stopAll()
	{
		for (hstr* it = this->sounds.iterate(); it; it = this->sounds.next())
		{
			xal::mgr->getSound(*it)->stop(this->fadeTime);
		}
	}
	
	void ParallelSoundManager::clear()
	{
		this->sounds.clear();
	}
	
}

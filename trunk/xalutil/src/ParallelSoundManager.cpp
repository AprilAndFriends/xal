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
#include <hltypes/util.h>
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
	
/******* METHODS *******************************************************/

	void ParallelSoundManager::addSound(chstr name)
	{
		this->buildList += name;
	}
	
	void ParallelSoundManager::updateList()
	{
		this->updateList(this->buildList);
		this->buildList.clear();
	}
		
	void ParallelSoundManager::updateList(harray<hstr> names)
	{
		harray<hstr> paused = this->sounds / names;
		foreach (hstr, it, paused)
		{
			xal::mgr->getSound(*it)->pause(this->fadeTime);
		}
		harray<hstr> started = names / this->sounds;
		foreach (hstr, it, started)
		{
			xal::mgr->getSound(*it)->play(this->fadeTime, true);
		}
		this->sounds = names;
	}
	
	void ParallelSoundManager::playAll()
	{
		foreach (hstr, it, this->sounds)
		{
			xal::mgr->getSound(*it)->play(this->fadeTime, true);
		}
	}
	
	void ParallelSoundManager::pauseAll()
	{
		foreach (hstr, it, this->sounds)
		{
			xal::mgr->getSound(*it)->pause(this->fadeTime);
		}
	}
	
	void ParallelSoundManager::stopAll()
	{
		foreach (hstr, it, this->sounds)
		{
			xal::mgr->getSound(*it)->stop(this->fadeTime);
		}
		this->clear();
	}
	
	void ParallelSoundManager::clear()
	{
		this->sounds.clear();
	}
	
}

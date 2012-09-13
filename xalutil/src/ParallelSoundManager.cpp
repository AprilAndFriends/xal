/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <xal/AudioManager.h>
#include <xal/Player.h>
#include <xal/xal.h>

#include "ParallelSoundManager.h"

namespace xal
{
	ParallelSoundManager::ParallelSoundManager(float fadeTime)
	{
		this->fadeTime = fadeTime;
	}
	
	ParallelSoundManager::~ParallelSoundManager()
	{
		this->clear();
	}
	
	void ParallelSoundManager::queueSound(chstr name)
	{
		this->soundQueue += name;
	}

	void ParallelSoundManager::removeSound(chstr name)
	{
		bool found = false;
		harray<hstr> queue;
		foreach (Player*, it, this->players)
		{
			if ((*it)->getName() == name)
			{
				found = true;
			}
			else if ((*it)->isPlaying()) queue += (*it)->getName();
		}
		if (found) updateList(queue);
	}

	void ParallelSoundManager::updateList()
	{
		this->updateList(this->soundQueue);
		this->soundQueue.clear();
	}
		
	void ParallelSoundManager::updateList(harray<hstr> names)
	{
		foreach (Player*, it, this->players)
		{
			if (names.contains((*it)->getName()))
			{
				names -= (*it)->getName();
				if (!(*it)->isPlaying())
				{
					(*it)->play(this->fadeTime, true);
				}
			}
			else if ((*it)->isPlaying())
			{
				(*it)->pause(this->fadeTime);
			}
		}
		Player* player;
		foreach (hstr, it, names)
		{
			player = xal::mgr->createPlayer(*it);
			player->play(this->fadeTime, true);
			this->players += player;
		}
	}
	
	void ParallelSoundManager::playAll()
	{
		foreach (Player*, it, this->players)
		{
			(*it)->play(this->fadeTime, true);
		}
	}
	
	void ParallelSoundManager::pauseAll()
	{
		foreach (Player*, it, this->players)
		{
			(*it)->pause(this->fadeTime);
		}
	}
	
	void ParallelSoundManager::stopAll()
	{
		this->clear();
	}
	
	void ParallelSoundManager::clear()
	{
		foreach (Player*, it, this->players)
		{
			xal::mgr->destroyPlayer(*it);
		}
		this->players.clear();
	}
	
}

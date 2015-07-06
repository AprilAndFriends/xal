/// @file
/// @version 3.02
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <xal/AudioManager.h>
#include <xal/Player.h>
#include <xal/xal.h>

#include "ParallelSoundManager.h"

namespace xal
{
	static hstr _map_getNames(Player* player)
	{
		return player->getName();
	}

	ParallelSoundManager::ParallelSoundManager(float fadeTime)
	{
		this->fadeTime = fadeTime;
	}
	
	ParallelSoundManager::~ParallelSoundManager()
	{
		this->clear();
	}
	
	harray<hstr> ParallelSoundManager::getPlayingSounds()
	{
		return this->players.mapped(&_map_getNames);
	}

	void ParallelSoundManager::stopSoundsWithPrefix(chstr prefix)
	{
		harray<Player*> removeList;
		foreach (Player*, it, this->players)
		{
			if ((*it)->getName().startsWith(prefix))
			{
				xal::manager->destroyPlayer(*it);
				removeList += *it;
			}
		}
		this->players -= removeList;
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
			else if ((*it)->isPlaying())
			{
				queue += (*it)->getName();
			}
		}
		if (found)
		{
			this->updateList(queue);
		}
	}

	void ParallelSoundManager::updateList()
	{
		this->updateList(this->soundQueue);
		this->soundQueue.clear();
	}

	void ParallelSoundManager::updateList(harray<hstr> names)
	{
		harray<Player*> removeList;
		foreach (Player*, it, this->players)
		{
			if (names.has((*it)->getName()))
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
			else
			{
				xal::manager->destroyPlayer(*it);
				removeList += *it;
			}
		}
		this->players -= removeList;
		Player* player = NULL;
		foreach (hstr, it, names)
		{
			player = xal::manager->createPlayer(*it);
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
	
	void ParallelSoundManager::stopAll()
	{
		this->clear();
	}
	
	void ParallelSoundManager::pauseAll()
	{
		foreach (Player*, it, this->players)
		{
			(*it)->pause(this->fadeTime);
		}
	}

	void ParallelSoundManager::clear()
	{
		foreach (Player*, it, this->players)
		{
			xal::manager->destroyPlayer(*it);
		}
		this->players.clear();
		this->soundQueue.clear();
	}
	
}

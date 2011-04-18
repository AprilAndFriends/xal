/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>
#include <xal/AudioManager.h>
#include <xal/Player.h>

#include "ParallelSoundManager.h"

namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	ParallelSoundManager::ParallelSoundManager(float fadeTime)
	{
		this->fadeTime = fadeTime;
	}
	
	ParallelSoundManager::~ParallelSoundManager()
	{
		this->clear();
	}
	
/******* METHODS *******************************************************/

	void ParallelSoundManager::addSound(chstr name)
	{
		this->sounds += name;
	}
	
	void ParallelSoundManager::updateList()
	{
		this->updateList(this->sounds);
		this->sounds.clear();
	}
		
	void ParallelSoundManager::updateList(harray<hstr> names)
	{
		foreach (Player*, it, this->players)
		{
			if ((*it)->isPlaying())
			{
				if (!names.contains((*it)->getName()))
				{
					(*it)->pause(this->fadeTime);
				}
			}
			else if (names.contains((*it)->getName()))
			{
				(*it)->play(this->fadeTime, true);
				names -= (*it)->getName();
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

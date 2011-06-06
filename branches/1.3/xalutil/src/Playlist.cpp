/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>
#include <xal/AudioManager.h>
#include <xal/Player.h>

#include "Playlist.h"

namespace xal
{
	Playlist::Playlist(bool repeatAll) : enabled(true), playing(false),
		index(-1)
	{
		this->repeatAll = repeatAll;
	}
	
	Playlist::~Playlist()
	{
		this->clear();
	}

	harray<hstr> Playlist::getSoundNames()
	{
		harray<hstr> result;
		foreach (Player*, it, this->players)
		{
			result += (*it)->getName();
		}
		return result;
	}
	
	void Playlist::update()
	{
		if (this->players.size() == 0 || this->index < 0)
		{
			return;
		}
		if (this->repeatAll)
		{
			if (!this->players[this->index]->isPlaying())
			{
				this->index = (this->index + 1) % this->players.size();
				this->players[this->index]->play();
			}
		}
		else if (this->index < this->players.size())
		{
			if (!this->players[this->index]->isPlaying())
			{
				this->index++;
				if (this->index < this->players.size())
				{
					this->players[this->index]->play();
				}
				else
				{
					this->playing = false;
				}
			}
		}
		else
		{
			this->playing = false;
		}
	}
	
	void Playlist::clear()
	{
		this->stop();
		foreach (Player*, it, this->players)
		{
			xal::mgr->destroyPlayer(*it);
		}
		this->players.clear();
		this->index = -1;
	}
	
	void Playlist::queueSound(chstr name)
	{
		this->players += xal::mgr->createPlayer(name);
		this->index = hmax(this->index, 0);
	}
	
	void Playlist::queueSounds(harray<hstr> names)
	{
		foreach (hstr, it, names)
		{
			this->players += xal::mgr->createPlayer(*it);
		}
		this->index = hmax(this->index, 0);
	}
	
	void Playlist::play(float fadeTime)
	{
		if (!this->enabled || this->players.size() == 0 || this->playing)
		{
			return;
		}
		if (this->index >= this->players.size())
		{
			this->index = 0;
		}
		this->players[this->index]->play(fadeTime);
		this->playing = true;
	}
	
	void Playlist::stop(float fadeTime)
	{
		if (this->playing)
		{
			this->players[this->index]->stop(fadeTime);
		}
		this->playing = false;
	}
	
	void Playlist::pause(float fadeTime)
	{
		if (this->playing)
		{
			this->players[this->index]->pause(fadeTime);
		}
		this->playing = false;
	}
	
}

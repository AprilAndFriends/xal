/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <xal/AudioManager.h>
#include <xal/Player.h>

#include "Playlist.h"

namespace xal
{
	Playlist::Playlist(bool repeatAll) : enabled(true), playing(false), index(-1)
	{
		this->repeatAll = repeatAll;
	}
	
	Playlist::~Playlist()
	{
		this->clear();
	}

	bool Playlist::isPaused() const
	{
		return (hbetweenIE(this->index, 0, this->players.size()) && this->players[this->index]->isPaused());
	}
	
	harray<hstr> Playlist::getSoundNames() const
	{
		HL_LAMBDA_CLASS(_soundNames, hstr, ((Player* const& player) { return player->getName(); }));
		return this->players.mapped(&_soundNames::lambda);
	}

	Player* Playlist::getCurrentPlayer() const
	{
		return (this->isPlaying() ? this->players[this->index] : NULL);
	}
	
	void Playlist::update()
	{
		if (this->enabled)
		{
			if (this->players.size() == 0 || !this->playing || this->index < 0)
			{
				return;
			}
			if (this->repeatAll)
			{
				if (!this->players[this->index]->isPlaying())
				{
					this->index = (this->index + 1) % this->players.size();
					this->players[this->index]->play(0.0f, (this->players.size() == 1));
				}
			}
			else if (this->index < this->players.size())
			{
				if (!this->players[this->index]->isPlaying())
				{
					++this->index;
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
	}
	
	void Playlist::clear()
	{
		this->stop();
		foreach (Player*, it, this->players)
		{
			xal::manager->destroyPlayer(*it);
		}
		this->players.clear();
		this->index = -1;
	}
	
	void Playlist::queueSound(chstr name)
	{
		this->players += xal::manager->createPlayer(name);
		this->index = hmax(this->index, 0);
	}
	
	void Playlist::queueSounds(harray<hstr> names)
	{
		HL_LAMBDA_CLASS(_createPlayers, xal::Player*, ((hstr const& name) { return xal::manager->createPlayer(name); }));
		this->players += names.mapped(&_createPlayers::lambda);
		this->index = hmax(this->index, 0);
	}
	
	void Playlist::play(float fadeTime)
	{
		if (this->players.size() == 0 || this->playing)
		{
			return;
		}
		if (this->index >= this->players.size())
		{
			this->index = 0;
		}
		bool looping = (this->players.size() == 1 && this->repeatAll);
		this->playing = true;
		if (this->enabled)
		{
			this->players[this->index]->play(fadeTime, looping);
		}
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

	void Playlist::shuffle()
	{
		if (!this->playing && this->players.size() > 1)
		{
			xal::Player* player = (this->index >= 0 && this->index < this->players.size() ? this->players[index] : NULL);
			this->players.randomize();
			if (player != NULL)
			{
				this->index = this->players.indexOf(player);
			}
		}
	}

	void Playlist::reset()
	{
		this->stop();
		this->index = (this->enabled ? 0 : -1);
	}
	
}

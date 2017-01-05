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
#include <xal/xal.h>

#include "Playlist.h"

namespace xal
{
	Playlist::Playlist(bool repeatAll) : gain(1.0f), pitch(1.0f), enabled(true), playing(false), firstLoop(true), index(-1)
	{
		this->repeatAll = repeatAll;
	}
	
	Playlist::~Playlist()
	{
		this->clear();
	}

	void Playlist::setGain(float value)
	{
		if (this->gain != value)
		{
			this->gain = value;
			this->_updateParameters();
		}
	}

	void Playlist::setPitch(float value)
	{
		if (this->pitch != value)
		{
			this->pitch = value;
			this->_updateParameters();
		}
	}

	void Playlist::setEnabled(bool value)
	{
		if (this->enabled != value)
		{
			this->enabled = value;
			if (this->enabled && this->playing)
			{
				this->players[this->index]->play(0.0f, (this->players.size() == 1 && this->repeatAll));
				this->_updateParameters();
			}
		}
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
		if (!this->enabled || this->players.size() == 0 || !this->playing || this->index < 0)
		{
			return;
		}
		if (this->repeatAll)
		{
			if (!this->players[this->index]->isPlaying())
			{
				if (this->index == this->players.size() - 1)
				{
					this->firstLoop = false;
				}
				this->index = (this->index + 1) % this->players.size();
				bool foundLoopedPlayer = true;
				if (!this->firstLoop)
				{
					foundLoopedPlayer = false;
					for_iter (i, 0, this->players.size())
					{
						if (!this->onlyOncePlayers.has(this->players[this->index]))
						{
							foundLoopedPlayer = true;
							break;
						}
						this->index = (this->index + 1) % this->players.size();
					}
				}
				if (foundLoopedPlayer)
				{
					this->players[this->index]->play(0.0f, (this->players.size() == 1));
				}
				else
				{
					this->playing = false;
				}
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
		this->_updateParameters();
	}
	
	void Playlist::_updateParameters()
	{
		if (!this->enabled || this->players.size() == 0 || this->index < 0)
		{
			return;
		}
		this->players[this->index]->setGain(this->gain);
		this->players[this->index]->setPitch(this->pitch);
	}

	void Playlist::clear()
	{
		this->stop();
		foreach (Player*, it, this->players)
		{
			xal::manager->destroyPlayer(*it);
		}
		this->players.clear();
		this->onlyOncePlayers.clear();
		this->index = -1;
	}
	
	void Playlist::queueSound(chstr name, bool onlyOnce)
	{
		xal::Player* player = xal::manager->createPlayer(name);
		this->players += player;
		if (onlyOnce)
		{
			this->onlyOncePlayers += player;
		}
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
		this->firstLoop = true;
		if (this->onlyOncePlayers.has(this->players))
		{
			hlog::warnf(xal::logTag, "Playlist '%s' has all players set as only-once. It's safer to disable repeatAll in the playlist.", this->getSoundNames().joined(',').cStr());
		}
		if (!hbetweenIE(this->index, 0, this->players.size()))
		{
			this->index = 0;
		}
		bool looping = (this->players.size() == 1 && this->repeatAll);
		this->playing = true;
		if (this->enabled)
		{
			this->_updateParameters();
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

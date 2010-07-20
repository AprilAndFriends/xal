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

#include "Playlist.h"

namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	Playlist::Playlist(bool repeatAll) : playing(false), index(-1),
		sounds(harray<hstr>())
	{
		this->repeatAll = repeatAll;
	}
	
	Playlist::~Playlist()
	{
	}
	
/******* METHODS *******************************************************/

	void Playlist::update()
	{
		if (this->sounds.size() == 0 || this->index < 0)
		{
			return;
		}
		if (this->repeatAll)
		{
			if (!xal::mgr->getSound(this->sounds[this->index])->isPlaying())
			{
				this->index = (this->index + 1) % this->sounds.size();
				xal::mgr->getSound(this->sounds[this->index])->play();
			}
		}
		else if (this->index < this->sounds.size())
		{
			xal::mgr->logMessage(this->index);
			if (!xal::mgr->getSound(this->sounds[this->index])->isPlaying())
			{
				this->index++;
				if (this->index < this->sounds.size())
				{
					xal::mgr->getSound(this->sounds[this->index])->play();
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
	
	void Playlist::play(float fadeTime)
	{
		if (this->sounds.size() == 0 || this->playing)
		{
			return;
		}
		if (this->index >= this->sounds.size())
		{
			this->index = 0;
		}
		xal::mgr->getSound(this->sounds[this->index])->play(fadeTime);
		this->playing = true;
	}
	
	void Playlist::stop(float fadeTime)
	{
		if (this->playing)
		{
			xal::mgr->getSound(this->sounds[this->index])->stop(fadeTime);
		}
		this->playing = false;
	}
	
	void Playlist::pause(float fadeTime)
	{
		if (this->playing)
		{
			xal::mgr->getSound(this->sounds[this->index])->pause(fadeTime);
		}
		this->playing = false;
	}
	
	void Playlist::clear()
	{
		this->stop();
		this->sounds.clear();
		this->index = -1;
	}
	
	void Playlist::queueSound(chstr name)
	{
		this->sounds += name;
		if (this->index < 0)
		{
			this->index = 0;
		}
	}
	
	void Playlist::queueSounds(harray<hstr> names)
	{
		this->sounds += names;
		if (this->index < 0)
		{
			this->index = 0;
		}
	}
	
}

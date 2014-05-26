/// @file
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/exception.h>
#include <hltypes/harray.h>

#include "MultiPlaylist.h"
#include "Playlist.h"

namespace xal
{
	MultiPlaylist::MultiPlaylist()
	{
	}
	
	MultiPlaylist::~MultiPlaylist()
	{
		this->clear();
	}

	bool MultiPlaylist::isEnabled()
	{
		foreach (Playlist*, it, this->playlists)
		{
			if (!(*it)->isEnabled())
			{
				return false;
			}
		}
		return true;
	}
	
	void MultiPlaylist::setEnabled(bool value)
	{
		foreach (Playlist*, it, this->playlists)
		{
			(*it)->setEnabled(value);
		}
	}
	
	bool MultiPlaylist::isPlaying()
	{
		foreach (Playlist*, it, this->playlists)
		{
			if ((*it)->isPlaying())
			{
				return true;
			}
		}
		return false;
	}
	
	bool MultiPlaylist::isPaused()
	{
		foreach (Playlist*, it, this->playlists)
		{
			if (!(*it)->isPaused())
			{
				return false;
			}
		}
		return true;
	}
	
	void MultiPlaylist::registerPlaylist(Playlist* playlist)
	{
		if (this->playlists.contains(playlist))
		{
			throw hl_exception("Playlist was already registered!");
		}
		this->playlists += playlist;
	}

	void MultiPlaylist::unregisterPlaylist(Playlist* playlist)
	{
		if (!this->playlists.contains(playlist))
		{
			throw hl_exception("Playlist has not been registered!");
		}
		this->playlists -= playlist;
	}
		
	void MultiPlaylist::clear()
	{
		this->stop();
		foreach (Playlist*, it, this->playlists)
		{
			delete (*it);
		}
		this->playlists.clear();
	}
	
	void MultiPlaylist::update()
	{
		foreach (Playlist*, it, this->playlists)
		{
			(*it)->update();
		}
	}
		
	void MultiPlaylist::play(float fadeTime)
	{
		foreach (Playlist*, it, this->playlists)
		{
			(*it)->play(fadeTime);
		}
	}
	
	void MultiPlaylist::stop(float fadeTime)
	{
		foreach (Playlist*, it, this->playlists)
		{
			(*it)->stop(fadeTime);
		}
	}
	
	void MultiPlaylist::pause(float fadeTime)
	{
		foreach (Playlist*, it, this->playlists)
		{
			(*it)->pause(fadeTime);
		}
	}

	void MultiPlaylist::shuffle()
	{
		foreach (Playlist*, it, this->playlists)
		{
			(*it)->shuffle();
		}
	}

	void MultiPlaylist::reset()
	{
		foreach (Playlist*, it, this->playlists)
		{
			(*it)->reset();
		}
	}
	
}

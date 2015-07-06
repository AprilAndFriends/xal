/// @file
/// @version 3.03
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>
#include <hltypes/hexception.h>

#include "MultiPlaylist.h"
#include "Playlist.h"

namespace xal
{
	static bool _filter_isEnabled(Playlist* playlist)
	{
		return playlist->isEnabled();
	}

	static bool _filter_isPlaying(Playlist* playlist)
	{
		return playlist->isPlaying();
	}

	static bool _filter_isPaused(Playlist* playlist)
	{
		return playlist->isPaused();
	}

	MultiPlaylist::MultiPlaylist()
	{
	}
	
	MultiPlaylist::~MultiPlaylist()
	{
		this->clear();
	}

	bool MultiPlaylist::isEnabled()
	{
		return this->playlists.matchesAll(&_filter_isEnabled);
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
		return this->playlists.matchesAny(&_filter_isPlaying);
	}

	bool MultiPlaylist::isPaused()
	{
		return this->playlists.matchesAll(&_filter_isPaused);
	}

	void MultiPlaylist::registerPlaylist(Playlist* playlist)
	{
		if (this->playlists.has(playlist))
		{
			throw Exception("Playlist was already registered!");
		}
		this->playlists += playlist;
	}

	void MultiPlaylist::unregisterPlaylist(Playlist* playlist)
	{
		if (!this->playlists.has(playlist))
		{
			throw Exception("Playlist has not been registered!");
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

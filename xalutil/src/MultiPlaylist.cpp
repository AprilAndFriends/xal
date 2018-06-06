/// @file
/// @version 4.0
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
	MultiPlaylist::MultiPlaylist()
	{
	}
	
	MultiPlaylist::~MultiPlaylist()
	{
		this->clear();
	}

	float MultiPlaylist::getGain() const
	{
		return (this->playlists.size() > 0 ? this->playlists.first()->getGain() : 1.0f);
	}

	void MultiPlaylist::setGain(float value)
	{
		foreach (Playlist*, it, this->playlists)
		{
			(*it)->setGain(value);
		}
	}

	float MultiPlaylist::getPitch() const
	{
		return (this->playlists.size() > 0 ? this->playlists.first()->getPitch() : 1.0f);
	}

	void MultiPlaylist::setPitch(float value)
	{
		foreach (Playlist*, it, this->playlists)
		{
			(*it)->setPitch(value);
		}
	}

	bool MultiPlaylist::isEnabled() const
	{
		HL_LAMBDA_CLASS(_isEnabled, bool, ((Playlist* const& playlist) { return playlist->isEnabled(); }));
		return this->playlists.matchesAll(&_isEnabled::lambda);
	}

	void MultiPlaylist::setEnabled(bool value)
	{
		foreach (Playlist*, it, this->playlists)
		{
			(*it)->setEnabled(value);
		}
	}

	bool MultiPlaylist::isPlaying() const
	{
		HL_LAMBDA_CLASS(_isPlaying, bool, ((Playlist* const& playlist) { return playlist->isPlaying(); }));
		return this->playlists.matchesAll(&_isPlaying::lambda);
	}

	bool MultiPlaylist::isPaused() const
	{
		HL_LAMBDA_CLASS(_isPaused, bool, ((Playlist* const& playlist) { return playlist->isPaused(); }));
		return this->playlists.matchesAll(&_isPaused::lambda);
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

/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/exception.h>
#include <hltypes/hdir.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>
#include <hltypes/util.h>

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "Player.h"
#include "Sound.h"
#include "Source.h"
#include "xal.h"

#if HAVE_M4A
#include "M4A_Source.h"
#endif
#if HAVE_MP3
#include "MP3_Source.h"
#endif
#if HAVE_OGG
#include "OGG_Source.h"
#endif
#if HAVE_SPX
#include "SPX_Source.h"
#endif
#if HAVE_WAV
#include "WAV_Source.h"
#endif

namespace xal
{
	AudioManager* mgr;

	AudioManager::AudioManager(chstr systemName, unsigned long backendId, bool threaded, float updateTime, chstr deviceName) :
		enabled(false), paused(false), gain(1.0f), thread(NULL)
	{
		this->name = systemName;
		this->backendId = backendId;
		this->deviceName = deviceName;
		this->updateTime = updateTime;
		if (threaded)
		{
			this->thread = new hthread(&AudioManager::update);
		}
	}

	AudioManager::~AudioManager()
	{
		if (this->thread != NULL)
		{
			xal::log("stopping thread management");
			this->_lock();
			this->thread->stop();
			delete this->thread;
			this->thread = NULL;
			this->_unlock();
		}
	}

	void AudioManager::init()
	{
		if (this->enabled && this->thread != NULL)
		{
			xal::log("starting thread management");
			this->thread->start();
		}
	}

	void AudioManager::clear()
	{
		foreach (Player*, it, this->players)
		{
			(*it)->_stop();
			delete (*it);
		}
		this->players.clear();
		this->managedPlayers.clear();
		foreach_m (Sound*, it, this->sounds)
		{
			delete it->second;
		}
		this->sounds.clear();
		foreach_m (Category*, it, this->categories)
		{
			delete it->second;
		}
		this->categories.clear();
	}
	
	void AudioManager::setGlobalGain(float value)
	{
		this->gain = value;
		foreach (Player*, it, this->players)
		{
			(*it)->setGain((*it)->getGain());
		}
	}

	harray<Player*> AudioManager::getPlayers()
	{
		return (this->players - this->managedPlayers);
	}

	void AudioManager::update()
	{
		while (true)
		{
			xal::mgr->_lock();
			xal::mgr->_update(xal::mgr->updateTime);
			xal::mgr->_unlock();
			hthread::sleep(xal::mgr->updateTime * 1000);
		}
	}
	
	void AudioManager::update(float k)
	{
		this->_lock();
		this->_update(k);
		this->_unlock();
	}

	void AudioManager::_update(float k)
	{
		if (this->enabled)
		{
			foreach (Player*, it, this->players)
			{
				(*it)->_update(k);
			}
			if (!this->paused)
			{
				// creating a copy, because _destroyManagedPlayer alters managedPlayers
				harray<Player*> players(this->managedPlayers);
				foreach (Player*, it, players)
				{
					if (!(*it)->isPlaying() && !(*it)->isFading())
					{
						this->_destroyManagedPlayer(*it);
					}
				}
			}
		}
	}

	void AudioManager::_lock()
	{
		this->mutex.lock();
	}
	
	void AudioManager::_unlock()
	{
		this->mutex.unlock();
	}

	Category* AudioManager::createCategory(chstr name, HandlingMode loadMode, HandlingMode decodeMode)
	{
		if (!this->categories.has_key(name))
		{
			this->categories[name] = new Category(name, loadMode, decodeMode);
		}
		return this->categories[name];
	}

	Category* AudioManager::getCategoryByName(chstr name)
	{
		if (!this->categories.has_key(name))
		{
			throw hl_exception("Audio Manager: Category '" + name + "' does not exist!");
		}
		return this->categories[name];
	}

	float AudioManager::getCategoryGain(chstr name)
	{
		if (!this->categories.has_key(name))
		{
			throw hl_exception("Audio Manager: Category '" + name + "' does not exist!");
		}
		return this->categories[name]->getGain();
	}

	void AudioManager::setCategoryGain(chstr name, float gain)
	{
		this->getCategoryByName(name)->setGain(gain);
		foreach (Player*, it, this->players)
		{
			(*it)->setGain((*it)->getGain());
		}
	}

	Sound* AudioManager::createSound(chstr filename, chstr categoryName, chstr prefix)
	{
		Category* category = this->getCategoryByName(categoryName);
		Sound* sound = new Sound(filename, category, prefix);
		if (this->sounds.has_key(sound->getName()))
		{
			delete sound;
			return NULL;
		}
		this->sounds[sound->getName()] = sound;
		return sound;
	}

	Sound* AudioManager::getSound(chstr name)
	{
		if (!this->sounds.has_key(name))
		{
			throw hl_exception("Audio Manager: Sound '" + name + "' does not exist!");
		}
		return this->sounds[name];
	}

	void AudioManager::destroySound(Sound* sound)
	{
		foreach_m (Sound*, it, this->sounds)
		{
			if (it->second == sound)
			{
				xal::log("destroying sound " + it->first);
				delete it->second;
				this->sounds.erase(it);
				break;
			}
		}
	}
	
	void AudioManager::destroySoundsWithPrefix(chstr prefix)
	{
		xal::log(hsprintf("destroying sounds with prefix '%s'", prefix.c_str()));
		harray<hstr> keys = this->sounds.keys();
		harray<Player*> managedPlayers = this->managedPlayers;
		Sound* sound;
		foreach (hstr, it, keys)
		{
			if ((*it).starts_with(prefix))
			{
				sound = this->sounds[*it];
				foreach (Player*, it2, managedPlayers)
				{
					if ((*it2)->getSound() == sound)
					{
						this->_destroyManagedPlayer(*it2);
					}
				}
				foreach (Player*, it2, this->players)
				{
					if ((*it2)->getSound() == sound)
					{
						throw hl_exception("Audio Manager: Sound " + sound->getName() + " cannot be destroyed, there are one or more manually created players that haven't been destroyed");
					}
				}
				delete sound;
				this->sounds.remove_key(*it);
			}
		}
	}

	harray<hstr> AudioManager::createSoundsFromPath(chstr path, chstr prefix)
	{
		harray<hstr> result;
		hstr category;
		harray<hstr> dirs = hdir::directories(path, true);
		foreach (hstr, it, dirs)
		{
			category = (*it).rsplit("/").pop_back();
			result += this->createSoundsFromPath((*it).c_str(), category, prefix);
		}
		return result;
	}

	harray<hstr> AudioManager::createSoundsFromPath(chstr path, chstr category, chstr prefix)
	{
		this->createCategory(category);
		harray<hstr> result;
		harray<hstr> files = hdir::files(path, true);
		Sound* sound;
		foreach (hstr, it, files)
		{
			sound = this->createSound((*it), category, prefix);
			if (sound != NULL)
			{
				result += sound->getName();
			}
		}
		return result;
	}

	Player* AudioManager::createPlayer(chstr name)
	{
		if (!this->sounds.has_key(name))
		{
			throw hl_exception("Audio Manager: Sound '" + name + "' does not exist!");
		}
		Sound* sound = this->sounds[name];
		Player* player = this->_createAudioPlayer(sound, sound->getBuffer());
		this->players += player;
		return player;
	}

	void AudioManager::destroyPlayer(Player* player)
	{
		this->_lock();
		this->_destroyPlayer(player);
		this->_unlock();
	}

	Player* AudioManager::findPlayer(chstr name)
	{
		harray<Player*> players = this->players - this->managedPlayers;
		foreach (Player*, it, players)
		{
			if ((*it)->getName() == name)
			{
				return (*it);
			}
		}
		return NULL;
	}

	void AudioManager::_destroyPlayer(Player* player)
	{
		this->players -= player;
		player->_stop();
		delete player;
	}

	Player* AudioManager::_createManagedPlayer(chstr name)
	{
		Player* player = this->createPlayer(name);
		this->managedPlayers += player;
		return player;
	}

	void AudioManager::_destroyManagedPlayer(Player* player)
	{
		this->managedPlayers -= player;
		this->_destroyPlayer(player);
	}

	Buffer* AudioManager::_createBuffer(chstr filename, HandlingMode loadMode, HandlingMode decodeMode)
	{
		return new Buffer(filename, loadMode, decodeMode);
	}

	Player* AudioManager::_createAudioPlayer(Sound* sound, Buffer* buffer)
	{
		return new Player(sound, buffer);
	}
	
	Source* AudioManager::_createSource(chstr filename, Format format)
	{
		Source* source;
		switch (format)
		{
#if HAVE_M4A
		case M4A:
			source = new M4A_Source(filename);
			break;
#endif
#if HAVE_MP3
		case MP3:
			source = new MP3_Source(filename);
			break;
#endif
#if HAVE_OGG
		case OGG:
			source = new OGG_Source(filename);
			break;
#endif
#if HAVE_SPX
		case SPX:
			source = new SPX_Source(filename);
			break;
#endif
#if HAVE_WAV
		case WAV:
			source = new WAV_Source(filename);
			break;
#endif
		default:
			source = new Source(filename);
			break;
		}
		return source;
	}

	void AudioManager::play(chstr name, float fadeTime, bool looping, float gain)
	{
		this->_lock();
		Player* player = this->_createManagedPlayer(name);
		player->setGain(gain);
		player->_play(fadeTime, looping);
		this->_unlock();
	}

	void AudioManager::stop(chstr name, float fadeTime)
	{
		this->_lock();
		fadeTime = hmax(fadeTime, 0.0f);
		harray<Player*> players;
		foreach (Player*, it, this->managedPlayers)
		{
			if ((*it)->getSound()->getName() == name)
			{
				if (fadeTime == 0.0f)
				{
					players += (*it);
				}
				else
				{
					(*it)->_stop(fadeTime);
				}
			}
		}
		foreach (Player*, it, players)
		{
			this->_destroyManagedPlayer(*it);
		}
		this->_unlock();
	}

	void AudioManager::stopFirst(chstr name, float fadeTime)
	{
		this->_lock();
		fadeTime = hmax(fadeTime, 0.0f);
		foreach (Player*, it, this->managedPlayers)
		{
			if ((*it)->getSound()->getName() == name)
			{
				if (fadeTime == 0.0f)
				{
					this->_destroyManagedPlayer(*it);
				}
				else
				{
					(*it)->_stop(fadeTime);
				}
				break;
			}
		}
		this->_unlock();
	}

	void AudioManager::stopAll(float fadeTime)
	{
		this->_lock();
		fadeTime = hmax(fadeTime, 0.0f);
		if (fadeTime == 0.0f)
		{
			harray<Player*> players(this->managedPlayers);
			foreach (Player*, it, players)
			{
				this->_destroyManagedPlayer(*it);
			}
			foreach (Player*, it, this->players)
			{
				(*it)->_stop();
			}
		}
		else
		{
			foreach (Player*, it, this->players)
			{
				(*it)->_stop(fadeTime);
			}
		}
		this->_unlock();
	}
	
	void AudioManager::pauseAll(float fadeTime)
	{
		if (!this->paused)
		{
			this->_lock();
			foreach (Player*, it, this->players)
			{
				if ((*it)->isPlaying())
				{
					(*it)->pause();
					this->pausedPlayers += (*it);
				}
				else if ((*it)->isFadingOut())
				{
					(*it)->paused ? (*it)->pause() : (*it)->stop();
				}
			}
			this->paused = true;
			this->_unlock();
		}
	}
	
	void AudioManager::resumeAll(float fadeTime)
	{
		if (this->paused)
		{
			this->_lock();
			foreach (Player*, it, this->pausedPlayers)
			{
				(*it)->play();
			}
			this->paused = false;
			this->pausedPlayers.clear();
			this->_unlock();
		}
	}
	
	void AudioManager::stopCategory(chstr name, float fadeTime)
	{
		this->_lock();
		fadeTime = hmax(fadeTime, 0.0f);
		Category* category = this->getCategoryByName(name);
		if (fadeTime == 0.0f)
		{
			harray<Player*> players(this->managedPlayers);
			foreach (Player*, it, players)
			{
				if ((*it)->getCategory() == category)
				{
					this->_destroyManagedPlayer(*it);
				}
			}
			foreach (Player*, it, this->players)
			{
				if ((*it)->getCategory() == category)
				{
					(*it)->_stop();
				}
			}
		}
		else
		{
			foreach (Player*, it, this->players)
			{
				if ((*it)->getCategory() == category)
				{
					(*it)->_stop(fadeTime);
				}
			}
		}
		this->_unlock();
	}
	
	bool AudioManager::isAnyPlaying(chstr name)
	{
		foreach (Player*, it, this->managedPlayers)
		{
			if ((*it)->getSound()->getName() == name && (*it)->isPlaying())
			{
				return true;
			}
		}
		return false;
	}

	bool AudioManager::isAnyFading(chstr name)
	{
		foreach (Player*, it, this->managedPlayers)
		{
			if ((*it)->getSound()->getName() == name && (*it)->isFading())
			{
				return true;
			}
		}
		return false;
	}

	bool AudioManager::isAnyFadingIn(chstr name)
	{
		foreach (Player*, it, this->managedPlayers)
		{
			if ((*it)->getSound()->getName() == name && (*it)->isFadingIn())
			{
				return true;
			}
		}
		return false;
	}

	bool AudioManager::isAnyFadingOut(chstr name)
	{
		foreach (Player*, it, this->managedPlayers)
		{
			if ((*it)->getSound()->getName() == name && (*it)->isFadingOut())
			{
				return true;
			}
		}
		return false;
	}

}

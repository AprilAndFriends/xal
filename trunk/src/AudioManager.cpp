/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/exception.h>
#include <hltypes/harray.h>
#include <hltypes/hdir.h>
#include <hltypes/hfile.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "Player.h"
#include "Sound.h"
#include "Source.h"
#include "xal.h"

#ifdef HAVE_FLAC
#include "FLAC_Source.h"
#endif
#ifdef HAVE_M4A
#include "M4A_Source.h"
#endif
// TODO
/*
#ifdef HAVE_MIDI
#include "MIDI_Source.h"
#endif
*/
#ifdef HAVE_OGG
#include "OGG_Source.h"
#endif
#ifdef HAVE_SPX
#include "SPX_Source.h"
#endif
#ifdef HAVE_WAV
#include "WAV_Source.h"
#endif

namespace xal
{
	extern void (*gLogFunction)(chstr);
	
	AudioManager* mgr = NULL;

	AudioManager::AudioManager(chstr systemName, unsigned long backendId, bool threaded, float updateTime, chstr deviceName) :
		enabled(false), paused(false), gain(1.0f), thread(NULL)
	{
		this->name = systemName;
		this->backendId = backendId;
		this->deviceName = deviceName;
		this->updateTime = updateTime;
#ifdef HAVE_FLAC
		this->extensions += ".flac";
#endif
#ifdef HAVE_M4A
		this->extensions += ".m4a";
#endif
#ifdef HAVE_MIDI
		this->extensions += ".mid";
#endif
#ifdef HAVE_OGG
		this->extensions += ".ogg";
#endif
#ifdef HAVE_SPX
		this->extensions += ".spx";
#endif
#ifdef HAVE_WAV
		this->extensions += ".wav";
#endif
		if (threaded)
		{
			this->thread = new hthread(&AudioManager::update);
		}
	}

	AudioManager::~AudioManager()
	{
	}

	void AudioManager::init()
	{
		this->_lock();
		if (this->enabled && this->thread != NULL)
		{
			this->_startThreading();
		}
		this->_unlock();
	}

	void AudioManager::_startThreading()
	{
		xal::log("starting audio update thread");
		this->thread->start();
	}

	void AudioManager::clear()
	{
		this->_lock();
		this->_clear();
		this->_unlock();
	}
	
	void AudioManager::_clear()
	{
		if (this->thread != NULL)
		{
			xal::log("stopping audio update thread");
			hthread* t = this->thread;
			this->thread = NULL;
			this->_unlock();
			t->join();
			this->_lock();
			delete t;
		}
		this->_update(0.0f);
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
		this->_lock();
		this->_setGlobalGain(value);
		this->_unlock();
	}

	void AudioManager::_setGlobalGain(float value)
	{
		this->gain = value;
		foreach (Player*, it, this->players)
		{
			(*it)->_systemUpdateGain();
		}
	}

	harray<Player*> AudioManager::getPlayers()
	{
		this->_lock();
		harray<Player*> players = this->_getPlayers();
		this->_unlock();
		return players;
	}

	harray<Player*> AudioManager::_getPlayers()
	{
		return (this->players - this->managedPlayers);
	}

	void AudioManager::update()
	{
		while (xal::mgr->thread != NULL)
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
		this->isThreaded() ? this->_flushQueuedMessages() : this->_update(k);
		this->_unlock();
	}

	void AudioManager::_update(float k)
	{
		if (this->enabled && !this->paused)
		{
			foreach (Player*, it, this->players)
			{
				(*it)->_update(k);
			}
			// creating a copy, because _destroyManagedPlayer alters managedPlayers
			harray<Player*> players = this->managedPlayers;
			foreach (Player*, it, players)
			{
				if (!(*it)->isPlaying() && !(*it)->isFadingOut())
				{
					this->_destroyManagedPlayer(*it);
				}
			}
		}
	}
	
	void AudioManager::_lock()
	{
		if (this->isThreaded())
		{
			this->mutex.lock();
		}
	}
	
	void AudioManager::_unlock()
	{
		if (this->isThreaded())
		{
			this->mutex.unlock();
		}
	}

	Category* AudioManager::createCategory(chstr name, HandlingMode loadMode, HandlingMode decodeMode)
	{
		this->_lock();
		Category* category = this->_createCategory(name, loadMode, decodeMode);
		this->_unlock();
		return category;
	}

	Category* AudioManager::_createCategory(chstr name, HandlingMode loadMode, HandlingMode decodeMode)
	{
		if (!this->categories.has_key(name))
		{
			this->categories[name] = new Category(name, loadMode, decodeMode);
		}
		return this->categories[name];
	}

	Category* AudioManager::getCategoryByName(chstr name)
	{
		this->_lock();
		Category* category = this->_getCategoryByName(name);
		this->_unlock();
		return category;
	}

	Category* AudioManager::_getCategoryByName(chstr name)
	{
		if (!this->categories.has_key(name))
		{
			throw hl_exception("Audio Manager: Category '" + name + "' does not exist!");
		}
		return this->categories[name];
	}

	float AudioManager::getCategoryGain(chstr name)
	{
		this->_lock();
		float gain = this->_getCategoryGain(name);
		this->_unlock();
		return gain;
	}

	float AudioManager::_getCategoryGain(chstr name)
	{
		if (!this->categories.has_key(name))
		{
			throw hl_exception("Audio Manager: Category '" + name + "' does not exist!");
		}
		return this->categories[name]->getGain();
	}

	void AudioManager::setCategoryGain(chstr name, float gain)
	{
		this->_lock();
		this->_setCategoryGain(name, gain);
		this->_unlock();
	}

	void AudioManager::_setCategoryGain(chstr name, float gain)
	{
		this->_getCategoryByName(name)->setGain(gain);
		foreach (Player*, it, this->players)
		{
			(*it)->_systemUpdateGain();
		}
	}

	Sound* AudioManager::createSound(chstr filename, chstr categoryName, chstr prefix)
	{
		this->_lock();
		Sound* sound = this->_createSound(filename, categoryName, prefix);
		this->_unlock();
		return sound;
	}

	Sound* AudioManager::_createSound(chstr filename, chstr categoryName, chstr prefix)
	{
		Category* category = this->_getCategoryByName(categoryName);
		Sound* sound = new Sound(filename, category, prefix);
#ifndef HAVE_NOAUDIO
		if (sound->getFormat() == UNKNOWN || this->sounds.has_key(sound->getName()))
		{
			delete sound;
			return NULL;
		}
#else
		if (this->sounds.has_key(sound->getName()))
		{
			delete sound;
			return NULL;
		}
#endif
		this->sounds[sound->getName()] = sound;
		return sound;
	}

	Sound* AudioManager::getSound(chstr name)
	{
		this->_lock();
		Sound* sound = this->_getSound(name);
		this->_unlock();
		return sound;
	}

	Sound* AudioManager::_getSound(chstr name)
	{
		if (!this->sounds.has_key(name))
		{
			throw hl_exception("Audio Manager: Sound '" + name + "' does not exist!");
		}
		return this->sounds[name];
	}

	void AudioManager::destroySound(Sound* sound)
	{
		this->_lock();
		this->_destroySound(sound);
		this->_unlock();
	}
	
	void AudioManager::_destroySound(Sound* sound)
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
		this->_lock();
		this->_destroySoundsWithPrefix(prefix);
		this->_unlock();
	}

	void AudioManager::_destroySoundsWithPrefix(chstr prefix)
	{
		xal::log(hsprintf("destroying sounds with prefix '%s'", prefix.c_str()));
		harray<hstr> keys = this->sounds.keys();
		// creating a copy, because _destroyManagedPlayer alters managedPlayers
		harray<Sound*> destroySounds;
		foreach (hstr, it, keys)
		{
			if ((*it).starts_with(prefix))
			{
				destroySounds += this->sounds[*it];
			}
		}
		harray<hstr> manualSoundNames;
		harray<Player*> managedPlayers;
		bool manual;
		foreach (Sound*, it, destroySounds)
		{
			managedPlayers = this->managedPlayers;
			foreach (Player*, it2, managedPlayers)
			{
				if ((*it2)->getSound() == (*it))
				{
					this->_destroyManagedPlayer(*it2);
				}
			}
			manual = false;
			foreach (Player*, it2, this->players)
			{
				if ((*it2)->getSound() == (*it))
				{
					manualSoundNames += (*it)->getName();
					manual = true;
					break;
				}
			}
			if (!manual)
			{
				this->sounds.remove_value(*it);
				delete (*it);
			}
		}
		if (manualSoundNames.size() > 0)
		{
			throw hl_exception("Audio Manager: Following sounds cannot be destroyed (there are one or more manually created players that haven't been destroyed): " + manualSoundNames.join(", "));
		}
	}

	harray<hstr> AudioManager::createSoundsFromPath(chstr path, chstr prefix)
	{
		this->_lock();
		harray<hstr> result = this->_createSoundsFromPath(path, prefix);
		this->_unlock();
		return result;
	}

	harray<hstr> AudioManager::_createSoundsFromPath(chstr path, chstr prefix)
	{
		harray<hstr> result;
		hstr category;
		harray<hstr> dirs = hdir::resource_directories(path, true);
		foreach (hstr, it, dirs)
		{
			category = (*it).rsplit("/", -1, true).pop_last();
			result += this->_createSoundsFromPath((*it), category, prefix);
		}
		return result;
	}

	harray<hstr> AudioManager::createSoundsFromPath(chstr path, chstr category, chstr prefix)
	{
		this->_lock();
		harray<hstr> result = this->_createSoundsFromPath(path, category, prefix);
		this->_unlock();
		return result;
	}

	harray<hstr> AudioManager::_createSoundsFromPath(chstr path, chstr category, chstr prefix)
	{
		this->_createCategory(category, xal::FULL, xal::FULL);
		harray<hstr> result;
		harray<hstr> files = hdir::resource_files(path, true);
		Sound* sound;
		foreach (hstr, it, files)
		{
			sound = this->_createSound((*it), category, prefix);
			if (sound != NULL)
			{
				result += sound->getName();
			}
		}
		return result;
	}

	Player* AudioManager::createPlayer(chstr name)
	{
		this->_lock();
		Player* player = this->_createPlayer(name);
		this->_unlock();
		return player;
	}

	Player* AudioManager::_createPlayer(chstr name)
	{
		if (!this->sounds.has_key(name))
		{
			throw hl_exception("Audio Manager: Sound '" + name + "' does not exist!");
		}
		Sound* sound = this->sounds[name];
		Player* player = this->_createSystemPlayer(sound, sound->getBuffer());
		this->players += player;
		return player;
	}

	void AudioManager::destroyPlayer(Player* player)
	{
		this->_lock();
		this->_destroyPlayer(player);
		this->_unlock();
	}

	void AudioManager::_destroyPlayer(Player* player)
	{
		player->_stop();
		this->players -= player;
		if (this->paused && this->pausedPlayers.contains(player))
		{
			this->pausedPlayers -= player;
		}
		delete player;
	}

	Player* AudioManager::findPlayer(chstr name)
	{
		this->_lock();
		Player* player = this->_findPlayer(name);
		this->_unlock();
		return player;
	}

	Player* AudioManager::_findPlayer(chstr name)
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

	Player* AudioManager::_createManagedPlayer(chstr name)
	{
		Player* player = this->_createPlayer(name);
		this->managedPlayers += player;
		return player;
	}

	void AudioManager::_destroyManagedPlayer(Player* player)
	{
		this->managedPlayers -= player;
		this->_destroyPlayer(player);
	}

	Player* AudioManager::_createSystemPlayer(Sound* sound, Buffer* buffer)
	{
		return new Player(sound, buffer);
	}
	
	Source* AudioManager::_createSource(chstr filename, Format format)
	{
		Source* source;
		switch (format)
		{
			// TODO
			/*
#ifdef HAVE_FLAC
		case FLAC:
			source = new FLAC_Source(filename);
			break;
#endif
			*/
#ifdef HAVE_M4A
		case M4A:
			source = new M4A_Source(filename);
			break;
#endif
			// TODO
			/*
#ifdef HAVE_MIDI
		case MIDI:
			source = new MIDI_Source(filename);
			break;
#endif
			*/
#ifdef HAVE_OGG
		case OGG:
			source = new OGG_Source(filename);
			break;
#endif
#ifdef HAVE_SPX
		case SPX:
			source = new SPX_Source(filename);
			break;
#endif
#ifdef HAVE_WAV
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
		this->_play(name, fadeTime, looping, gain);
		this->_unlock();
	}

	void AudioManager::_play(chstr name, float fadeTime, bool looping, float gain)
	{
		if (this->paused)
		{
			return;
		}
		Player* player = this->_createManagedPlayer(name);
		player->_setGain(gain);
		player->_play(fadeTime, looping);
	}

	void AudioManager::stop(chstr name, float fadeTime)
	{
		this->_lock();
		this->_stop(name, fadeTime);
		this->_unlock();
	}

	void AudioManager::_stop(chstr name, float fadeTime)
	{
		fadeTime = hmax(fadeTime, 0.0f);
		// creating a copy, because _destroyManagedPlayer alters managedPlayers
		harray<Player*> players = this->managedPlayers;
		foreach (Player*, it, players)
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
			}
		}
	}

	void AudioManager::stopFirst(chstr name, float fadeTime)
	{
		this->_lock();
		this->_stopFirst(name, fadeTime);
		this->_unlock();
	}

	void AudioManager::_stopFirst(chstr name, float fadeTime)
	{
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
	}

	void AudioManager::stopAll(float fadeTime)
	{
		this->_lock();
		this->_stopAll(fadeTime);
		this->_unlock();
	}
	
	void AudioManager::_stopAll(float fadeTime)
	{
		fadeTime = hmax(fadeTime, 0.0f);
		if (fadeTime == 0.0f)
		{
			// creating a copy, because _destroyManagedPlayer alters managedPlayers
			harray<Player*> players = this->managedPlayers;
			foreach (Player*, it, players)
			{
				this->_destroyManagedPlayer(*it);
			}
		}
		foreach (Player*, it, this->players)
		{
			(*it)->_stop(fadeTime);
		}
	}
	
	void AudioManager::pauseAll(float fadeTime)
	{
		this->_lock();
		this->_pauseAll(fadeTime);
		this->_unlock();
	}
	
	void AudioManager::_pauseAll(float fadeTime)
	{
		if (!this->paused)
		{
			foreach (Player*, it, this->players)
			{
				if ((*it)->isPlaying())
				{
					(*it)->_pause();
					this->pausedPlayers += (*it);
				}
				else if ((*it)->isFadingOut())
				{
					(*it)->paused ? (*it)->_pause() : (*it)->_stop();
				}
			}
			this->paused = true;
		}
	}
	
	void AudioManager::resumeAll(float fadeTime)
	{
		this->_lock();
		this->_resumeAll(fadeTime);
		this->_unlock();
	}
	
	void AudioManager::_resumeAll(float fadeTime)
	{
		if (this->paused)
		{
			this->paused = false;
			foreach (Player*, it, this->pausedPlayers)
			{
				(*it)->_play();
			}
			this->pausedPlayers.clear();
		}
	}
	
	void AudioManager::stopCategory(chstr name, float fadeTime)
	{
		this->_lock();
		this->_stopCategory(name, fadeTime);
		this->_unlock();
	}
	
	void AudioManager::_stopCategory(chstr name, float fadeTime)
	{
		fadeTime = hmax(fadeTime, 0.0f);
		Category* category = this->_getCategoryByName(name);
		if (fadeTime == 0.0f)
		{
			// creating a copy, because _destroyManagedPlayer alters managedPlayers
			harray<Player*> players = this->managedPlayers;
			foreach (Player*, it, players)
			{
				if ((*it)->getCategory() == category)
				{
					this->_destroyManagedPlayer(*it);
				}
			}
		}
		foreach (Player*, it, this->players)
		{
			if ((*it)->getCategory() == category)
			{
				(*it)->_stop(fadeTime);
			}
		}
	}
	
	bool AudioManager::isAnyPlaying(chstr name)
	{
		this->_lock();
		bool result = this->_isAnyPlaying(name);
		this->_unlock();
		return result;
	}

	bool AudioManager::_isAnyPlaying(chstr name)
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
		this->_lock();
		bool result = this->_isAnyFading(name);
		this->_unlock();
		return result;
	}

	bool AudioManager::_isAnyFading(chstr name)
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
		this->_lock();
		bool result = this->_isAnyFadingIn(name);
		this->_unlock();
		return result;
	}

	bool AudioManager::_isAnyFadingIn(chstr name)
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
		this->_lock();
		bool result = this->_isAnyFadingOut(name);
		this->_unlock();
		return result;
	}

	bool AudioManager::_isAnyFadingOut(chstr name)
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

	void AudioManager::queueMessage(chstr message)
	{
		this->_queuedMessages += message;
	}

	void AudioManager::_flushQueuedMessages()
	{
		foreach (hstr, it, this->_queuedMessages)
		{
			gLogFunction(*it);
		}
		this->_queuedMessages.clear();
	}

	void AudioManager::addAudioExtension(chstr extension)
	{
		this->extensions += extension;
	}

	hstr AudioManager::findAudioFile(chstr _filename)
	{
		hstr filename = _filename;
		if (hfile::exists(filename))
		{
			return filename;
		}
		hstr name;
		foreach (hstr, it, this->extensions)
		{
			name = filename + (*it);
			if (hfile::exists(name))
			{
				return name;
			}
		}
		int index = filename.rfind(".");
		if (index >= 0)
		{
			filename = filename.substr(0, index);
			foreach (hstr, it, this->extensions)
			{
				name = filename + (*it);
				if (hfile::exists(name))
				{
					return name;
				}
			}
		}
		return "";
	}
	
}

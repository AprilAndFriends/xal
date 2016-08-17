/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>
#include <hltypes/hexception.h>
#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
#include <hltypes/hrdir.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "AudioManager.h"
#include "Buffer.h"
#include "BufferAsync.h"
#include "Category.h"
#include "NoAudio_AudioManager.h"
#include "Player.h"
#include "Sound.h"
#include "Source.h"
#include "xal.h"

#ifdef _FORMAT_FLAC
#include "FLAC_Source.h"
#endif
#ifdef _FORMAT_OGG
#include "OGG_Source.h"
#endif
#ifdef _FORMAT_SPX
#include "SPX_Source.h"
#endif
#ifdef _FORMAT_WAV
#include "WAV_Source.h"
#endif

namespace xal
{
	extern void (*gLogFunction)(chstr);
	
	HL_ENUM_CLASS_DEFINE(Format,
	(
		HL_ENUM_DEFINE(Format, FLAC);
		HL_ENUM_DEFINE(Format, M4A);
		HL_ENUM_DEFINE(Format, OGG);
		HL_ENUM_DEFINE(Format, WAV);
		HL_ENUM_DEFINE(Format, Unknown);
	));

	HL_ENUM_CLASS_DEFINE(BufferMode,
	(
		HL_ENUM_DEFINE(BufferMode, Full);
		HL_ENUM_DEFINE(BufferMode, Async);
		HL_ENUM_DEFINE(BufferMode, Lazy);
		HL_ENUM_DEFINE(BufferMode, Managed);
		HL_ENUM_DEFINE(BufferMode, OnDemand);
		HL_ENUM_DEFINE(BufferMode, Streamed);
	));

	HL_ENUM_CLASS_DEFINE(SourceMode,
	(
		HL_ENUM_DEFINE(SourceMode, Disk);
		HL_ENUM_DEFINE(SourceMode, Ram);
	));

	AudioManager* manager = NULL;

	AudioManager::AudioManager(void* backendId, bool threaded, float updateTime, chstr deviceName) :
		enabled(false), suspended(false), idlePlayerUnloadTime(60.0f), globalGain(1.0f), thread(NULL), threadRunning(false)
	{
		this->samplingRate = 44100;
		this->channels = 2;
		this->bitsPerSample = 16;
		this->backendId = backendId;
		this->deviceName = deviceName;
		this->updateTime = updateTime;
#ifdef _FORMAT_FLAC
		this->extensions += ".flac";
#endif
#ifdef _FORMAT_OGG
		this->extensions += ".ogg";
#endif
#ifdef _FORMAT_SPX
		this->extensions += ".spx";
#endif
#ifdef _FORMAT_WAV
		this->extensions += ".wav";
#endif
		if (threaded)
		{
			this->thread = new hthread(&AudioManager::_update, "XAL update");
		}
	}

	AudioManager::~AudioManager()
	{
		if (this->thread != NULL)
		{
			delete this->thread;
		}
	}

	void AudioManager::init()
	{
		hmutex::ScopeLock lock(&this->mutex);
		if (this->enabled && this->thread != NULL)
		{
			this->_startThreading();
		}
	}

	void AudioManager::_startThreading()
	{
		hlog::write(logTag, "Starting audio update thread.");
		this->threadRunning = true;
		this->thread->start();
	}

	void AudioManager::clear()
	{
		hmutex::ScopeLock lock(&this->mutex);
		if (this->threadRunning)
		{
			hlog::write(logTag, "Stopping audio update thread.");
			this->threadRunning = false;
			lock.release();
			this->thread->join();
			lock.acquire(&this->mutex);
		}
		if (this->thread != NULL)
		{
			delete this->thread;
			this->thread = NULL;
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
		hmutex::ScopeLock lock(&this->mutex);
		this->_setGlobalGain(value);
	}

	void AudioManager::_setGlobalGain(float value)
	{
		this->globalGain = value;
		foreach (Player*, it, this->players)
		{
			(*it)->_systemUpdateGain();
		}
	}

	harray<Player*> AudioManager::getPlayers()
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_getPlayers();
	}

	harray<Player*> AudioManager::_getPlayers() const
	{
		return (this->players - this->managedPlayers);
	}

	hmap<hstr, Sound*> AudioManager::getSounds()
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_getSounds();
	}

	hmap<hstr, Sound*> AudioManager::_getSounds() const
	{
		return this->sounds;
	}

	void AudioManager::_update(hthread* thread)
	{
		hmutex::ScopeLock lock(&xal::manager->mutex);
		while (xal::manager->thread != NULL && xal::manager->threadRunning)
		{
			xal::manager->_update(xal::manager->updateTime);
			lock.release();
			hthread::sleep(xal::manager->updateTime * 1000);
			lock.acquire(&xal::manager->mutex);
		}
		lock.release();
	}

	void AudioManager::update(float timeDelta)
	{
		hmutex::ScopeLock lock(&this->mutex);
		if (!this->isThreaded())
		{
			this->_update(timeDelta);
		}
	}

	void AudioManager::_update(float timeDelta)
	{
		if (this->enabled && !this->suspended)
		{
			BufferAsync::update();
			foreach (Player*, it, this->players)
			{
				(*it)->_update(timeDelta);
				if ((*it)->_isAsyncPlayQueued())
				{
					(*it)->_play((*it)->fadeTime, (*it)->looping);
				}
			}
			// creating a copy, because _destroyManagedPlayer alters managedPlayers
			harray<Player*> players = this->managedPlayers;
			foreach (Player*, it, players)
			{
				if (!(*it)->_isAsyncPlayQueued() && !(*it)->_isPlaying() && !(*it)->_isFadingOut())
				{
					this->_destroyManagedPlayer(*it);
				}
			}
			foreach (Buffer*, it, this->buffers)
			{
				(*it)->_update(timeDelta);
			}
		}
	}

	Category* AudioManager::createCategory(chstr name, BufferMode bufferMode, SourceMode sourceMode)
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_createCategory(name, bufferMode, sourceMode);
	}

	Category* AudioManager::_createCategory(chstr name, BufferMode bufferMode, SourceMode sourceMode)
	{
		if (!this->categories.hasKey(name))
		{
			this->categories[name] = new Category(name, bufferMode, sourceMode);
		}
		return this->categories[name];
	}

	Category* AudioManager::getCategory(chstr name)
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_getCategory(name);
	}

	Category* AudioManager::_getCategory(chstr name)
	{
		if (!this->categories.hasKey(name))
		{
			throw Exception("Audio Manager: Category '" + name + "' does not exist!");
		}
		return this->categories[name];
	}

	bool AudioManager::hasCategory(chstr category) const
	{
		return this->categories.hasKey(category);
	}

	Sound* AudioManager::createSound(chstr filename, chstr categoryName, chstr prefix)
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_createSound(filename, categoryName, prefix);
	}

	Sound* AudioManager::_createSound(chstr filename, chstr categoryName, chstr prefix)
	{
		Category* category = this->_getCategory(categoryName);
		Sound* sound = new Sound(filename, category, prefix);
		if (sound->getFormat() == Format::Unknown || this->sounds.hasKey(sound->getName()))
		{
			delete sound;
			return NULL;
		}
		this->sounds[sound->getName()] = sound;
		return sound;
	}

	Sound* AudioManager::getSound(chstr name)
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_getSound(name);
	}

	Sound* AudioManager::_getSound(chstr name)
	{
		if (!this->sounds.hasKey(name))
		{
			throw Exception("Audio Manager: Sound '" + name + "' does not exist!");
		}
		return this->sounds[name];
	}

	void AudioManager::destroySound(Sound* sound)
	{
		hmutex::ScopeLock lock(&this->mutex);
		this->_destroySound(sound);
	}
	
	void AudioManager::_destroySound(Sound* sound)
	{
		foreach_m (Sound*, it, this->sounds)
		{
			if (it->second == sound)
			{
				hlog::write(logTag, "Destroying sound: " + it->first);
				delete it->second;
				this->sounds.erase(it);
				break;
			}
		}
	}
	
	void AudioManager::destroySoundsWithPrefix(chstr prefix)
	{
		hmutex::ScopeLock lock(&this->mutex);
		this->_destroySoundsWithPrefix(prefix);
	}

	void AudioManager::_destroySoundsWithPrefix(chstr prefix)
	{
		hlog::write(logTag, "Destroying sounds with prefix: " + prefix);
		harray<hstr> keys = this->sounds.keys();
		// creating a copy, because _destroyManagedPlayer alters managedPlayers
		harray<Sound*> destroySounds;
		foreach (hstr, it, keys)
		{
			if ((*it).startsWith(prefix))
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
				this->sounds.removeValue(*it);
				delete (*it);
			}
		}
		if (manualSoundNames.size() > 0)
		{
			throw Exception("Audio Manager: Following sounds cannot be destroyed (there are one or more manually created players that haven't been destroyed): " + manualSoundNames.joined(", "));
		}
	}

	harray<hstr> AudioManager::createSoundsFromPath(chstr path, chstr prefix)
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_createSoundsFromPath(path, prefix);
	}

	harray<hstr> AudioManager::_createSoundsFromPath(chstr path, chstr prefix)
	{
		harray<hstr> result;
		harray<hstr> dirs = hrdir::directories(path, true);
		foreach (hstr, it, dirs)
		{
			result += this->_createSoundsFromPath((*it), hrdir::baseName(*it), prefix);
		}
		return result;
	}

	harray<hstr> AudioManager::createSoundsFromPath(chstr path, chstr categoryName, chstr prefix)
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_createSoundsFromPath(path, categoryName, prefix);
	}

	harray<hstr> AudioManager::_createSoundsFromPath(chstr path, chstr categoryName, chstr prefix)
	{
		this->_createCategory(categoryName, BufferMode::Full, SourceMode::Disk);
		harray<hstr> result;
		harray<hstr> files = hrdir::files(path, true);
		Sound* sound;
		foreach (hstr, it, files)
		{
			sound = this->_createSound((*it), categoryName, prefix);
			if (sound != NULL)
			{
				result += sound->getName();
			}
		}
		return result;
	}

	bool AudioManager::hasSound(chstr name) const
	{
		return this->sounds.hasKey(name);
	}
	
	Player* AudioManager::createPlayer(chstr soundName)
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_createPlayer(soundName);
	}

	Player* AudioManager::_createPlayer(chstr soundName)
	{
		if (!this->sounds.hasKey(soundName))
		{
			throw Exception("Audio Manager: Sound '" + soundName + "' does not exist!");
		}
		Sound* sound = this->sounds[soundName];
		Player* player = this->_createSystemPlayer(sound);
		this->players += player;
		return player;
	}

	void AudioManager::destroyPlayer(Player* player)
	{
		hmutex::ScopeLock lock(&this->mutex);
		this->_destroyPlayer(player);
	}

	void AudioManager::_destroyPlayer(Player* player)
	{
		player->_stop(); // removes players from suspendedPlayers as well
		this->players -= player;
		delete player;
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

	Buffer* AudioManager::_createBuffer(Sound* sound)
	{
		Buffer* buffer = new Buffer(sound);
		this->buffers += buffer;
		return buffer;
	}

	void AudioManager::_destroyBuffer(Buffer* buffer)
	{
		this->buffers -= buffer;
		delete buffer;
	}

	Source* AudioManager::_createSource(chstr filename, SourceMode sourceMode, BufferMode bufferMode, Format format)
	{
#ifdef _FORMAT_FLAC
		if (format == Format::FLAC)
		{
			return new FLAC_Source(filename, sourceMode, bufferMode);
		}
#endif
#ifdef _FORMAT_OGG
		if (format == Format::OGG)
		{
			return new OGG_Source(filename, sourceMode, bufferMode);
		}
#endif
#ifdef _FORMAT_SPX
		if (format == Format::SPX)
		{
			return new SPX_Source(filename, sourceMode, bufferMode);
		}
#endif
#ifdef _FORMAT_WAV
		if (format == Format::WAV)
		{
			return new WAV_Source(filename, sourceMode, bufferMode);
		}
#endif
		return new Source(filename, sourceMode, bufferMode);
	}

	void AudioManager::play(chstr soundName, float fadeTime, bool looping, float gain)
	{
		hmutex::ScopeLock lock(&this->mutex);
		this->_play(soundName, fadeTime, looping, gain);
	}

	void AudioManager::_play(chstr soundName, float fadeTime, bool looping, float gain)
	{
		if (this->suspended)
		{
			return;
		}
		Player* player = this->_createManagedPlayer(soundName);
		player->_setGain(gain);
		player->_play(fadeTime, looping);
	}

	void AudioManager::playAsync(chstr soundName, float fadeTime, bool looping, float gain)
	{
		hmutex::ScopeLock lock(&this->mutex);
		this->_playAsync(soundName, fadeTime, looping, gain);
	}

	void AudioManager::_playAsync(chstr soundName, float fadeTime, bool looping, float gain)
	{
		if (this->suspended)
		{
			return;
		}
		Player* player = this->_createManagedPlayer(soundName);
		player->_setGain(gain);
		player->_playAsync(fadeTime, looping);
	}

	void AudioManager::stop(chstr soundName, float fadeTime)
	{
		hmutex::ScopeLock lock(&this->mutex);
		this->_stop(soundName, fadeTime);
	}

	void AudioManager::_stop(chstr soundName, float fadeTime)
	{
		if (fadeTime == 0.0f)
		{
			// creating a copy, because _destroyManagedPlayer alters managedPlayers
			harray<Player*> players = this->managedPlayers;
			foreach (Player*, it, players)
			{
				if ((*it)->getSound()->getName() == soundName)
				{
					this->_destroyManagedPlayer(*it);
				}
			}
		}
		else
		{
			foreach (Player*, it, this->managedPlayers)
			{
				if ((*it)->getSound()->getName() == soundName)
				{
					(*it)->_stop(fadeTime);
				}
			}
		}
	}

	void AudioManager::stopFirst(chstr name, float fadeTime)
	{
		hmutex::ScopeLock lock(&this->mutex);
		this->_stopFirst(name, fadeTime);
	}

	void AudioManager::_stopFirst(chstr soundName, float fadeTime)
	{
		foreach (Player*, it, this->managedPlayers)
		{
			if ((*it)->getSound()->getName() == soundName)
			{
				if (fadeTime <= 0.0f)
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
		hmutex::ScopeLock lock(&this->mutex);
		this->_stopAll(fadeTime);
	}
	
	void AudioManager::_stopAll(float fadeTime)
	{
		// managed players can all be destroyed immediately if there is no fade time
		if (fadeTime <= 0.0f)
		{
			// creating a copy, because _destroyManagedPlayer alters managedPlayers
			harray<Player*> players = this->managedPlayers;
			foreach (Player*, it, players)
			{
				this->_destroyManagedPlayer(*it);
			}
		}
		// includes managed players!
		foreach (Player*, it, this->players)
		{
			(*it)->_stop(fadeTime);
		}
	}
	
	void AudioManager::suspendAudio()
	{
		hmutex::ScopeLock lock(&this->mutex);
		this->_suspendAudio();
	}
	
	void AudioManager::_suspendAudio()
	{
		if (!this->suspended)
		{
			hlog::write(logTag, "Suspending XAL.");
			foreach (Player*, it, this->players)
			{
				if ((*it)->_isPlaying())
				{
					(*it)->_pause();
					this->suspendedPlayers += (*it);
				}
				else if ((*it)->_isFadingOut())
				{
					(*it)->paused ? (*it)->_pause() : (*it)->_stop();
				}
			}
			this->_suspendSystem();
			this->suspended = true;
		}
	}
	
	void AudioManager::resumeAudio()
	{
		hmutex::ScopeLock lock(&this->mutex);
		this->_resumeAudio();
	}
	
	void AudioManager::_resumeAudio()
	{
		if (this->suspended)
		{
			hlog::write(logTag, "Resuming XAL.");
			this->suspended = false;
			this->_resumeSystem();
			foreach (Player*, it, this->suspendedPlayers)
			{
				(*it)->_play();
			}
			this->suspendedPlayers.clear();
		}
	}
	
	void AudioManager::stopCategory(chstr categoryName, float fadeTime)
	{
		hmutex::ScopeLock lock(&this->mutex);
		this->_stopCategory(categoryName, fadeTime);
	}
	
	void AudioManager::_stopCategory(chstr categoryName, float fadeTime)
	{
		fadeTime = hmax(fadeTime, 0.0f);
		Category* category = this->_getCategory(categoryName);
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
	
	bool AudioManager::isAnyPlaying(chstr soundName)
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_isAnyPlaying(soundName);
	}

	bool AudioManager::_isAnyPlaying(chstr soundName) const
	{
		foreachc (Player*, it, this->managedPlayers)
		{
			if ((*it)->getSound()->getName() == soundName && (*it)->_isPlaying())
			{
				return true;
			}
		}
		return false;
	}

	bool AudioManager::isAnyFading(chstr soundName)
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_isAnyFading(soundName);
	}

	bool AudioManager::_isAnyFading(chstr soundName) const
	{
		foreachc (Player*, it, this->managedPlayers)
		{
			if ((*it)->getSound()->getName() == soundName && (*it)->_isFading())
			{
				return true;
			}
		}
		return false;
	}

	bool AudioManager::isAnyFadingIn(chstr soundName)
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_isAnyFadingIn(soundName);
	}

	bool AudioManager::_isAnyFadingIn(chstr soundName) const
	{
		foreachc (Player*, it, this->managedPlayers)
		{
			if ((*it)->getSound()->getName() == soundName && (*it)->_isFadingIn())
			{
				return true;
			}
		}
		return false;
	}

	bool AudioManager::isAnyFadingOut(chstr soundName)
	{
		hmutex::ScopeLock lock(&this->mutex);
		return this->_isAnyFadingOut(soundName);
	}

	bool AudioManager::_isAnyFadingOut(chstr soundName) const
	{
		foreachc (Player*, it, this->managedPlayers)
		{
			if ((*it)->getSound()->getName() == soundName && (*it)->_isFadingOut())
			{
				return true;
			}
		}
		return false;
	}

	void AudioManager::clearMemory()
	{
		hmutex::ScopeLock lock(&this->mutex);
		this->_clearMemory();
	}

	void AudioManager::_clearMemory()
	{
		int count = 0;
		foreach (Buffer*, it, this->buffers)
		{
			if ((*it)->_tryClearMemory())
			{
				++count;
			}
		}
		hlog::debugf(logTag, "Found %d buffers for memory clearing.", count);
	}

	void AudioManager::addAudioExtension(chstr extension)
	{
		this->extensions += extension;
	}

	hstr AudioManager::findAudioFile(chstr filename) const
	{
		if (hresource::exists(filename))
		{
			return filename;
		}
		hstr name;
		foreachc (hstr, it, this->extensions)
		{
			name = filename + (*it);
			if (hresource::exists(name))
			{
				return name;
			}
		}
		hstr newFilename = hfile::withoutExtension(filename);
		if (newFilename != filename)
		{
			foreachc (hstr, it, this->extensions)
			{
				name = newFilename + (*it);
				if (hresource::exists(name))
				{
					return name;
				}
			}
		}
		return "";
	}
	
}

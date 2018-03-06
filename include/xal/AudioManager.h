/// @file
/// @version 3.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Provides an interface for the audio manager.

#ifndef XAL_AUDIO_MANAGER_H
#define XAL_AUDIO_MANAGER_H

#include <hltypes/harray.h>
#include <hltypes/henum.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "xalExport.h"

namespace xal
{
	/// @class Format
	/// @brief Defines supported audio formats.
	HL_ENUM_CLASS_PREFIX_DECLARE(xalExport, Format,
	(
		/// @var static const Format Format::FLAC
		/// @brief Free Lossless Audio Codec.
		HL_ENUM_DECLARE(Format, FLAC);
		/// @var static const Format Format::M4A
		/// @brief MPEG 4 Advanced Audio Coding.
		HL_ENUM_DECLARE(Format, M4A);
		/// @var static const Format Format::OGG
		/// @brief OGG by Xiph.Org Foundation.
		HL_ENUM_DECLARE(Format, OGG);
		/// @var static const Format Format::WAV
		/// @brief Waveform Audio File Format.
		HL_ENUM_DECLARE(Format, WAV);
		/// @var static const Format Format::RAW
		/// @brief Raw audio data Format.
		HL_ENUM_DECLARE(Format, Memory);
		/// @var static const Format Format::FLAC
		/// @brief Unknown format, usually indicates errors.
		HL_ENUM_DECLARE(Format, Unknown);
	));

	/// @class BufferMode
	/// @brief Defines when buffers should be created and how they should be handled.
	HL_ENUM_CLASS_PREFIX_DECLARE(xalExport, BufferMode,
	(
		/// @var static const BufferMode BufferMode::Full
		/// @brief Buffers data upon player creation, keeps results in memory.
		HL_ENUM_DECLARE(BufferMode, Full);
		/// @var static const BufferMode BufferMode::Async
		/// @brief Buffers data upon player creation asynchronously, keeps results in memory.
		HL_ENUM_DECLARE(BufferMode, Async);
		/// @var static const BufferMode BufferMode::Lazy
		/// @brief Buffers when first need arises, keeps results in memory.
		HL_ENUM_DECLARE(BufferMode, Lazy);
		/// @var static const BufferMode BufferMode::Managed
		/// @brief Buffers when first need arises, clears memory after a timeout.
		HL_ENUM_DECLARE(BufferMode, Managed);
		/// @var static const BufferMode BufferMode::OnDemand
		/// @brief Buffers when first need arises, clears memory after usage.
		HL_ENUM_DECLARE(BufferMode, OnDemand);
		/// @var static const BufferMode BufferMode::Streamed
		/// @brief Buffers in streamed mode.
		HL_ENUM_DECLARE(BufferMode, Streamed);
	));

	/// @class SourceMode
	/// @brief Defines how audio sources should be handled.
	HL_ENUM_CLASS_PREFIX_DECLARE(xalExport, SourceMode,
	(
		/// @var static const SourceMode SourceMode::Disk
		/// @brief Leaves data on permanent storage device.
		HL_ENUM_DECLARE(SourceMode, Disk);
		/// @var static const SourceMode SourceMode::Ram
		/// @brief Copies data to RAM buffer and accesses it from there.
		HL_ENUM_DECLARE(SourceMode, Ram);
	));

	class Buffer;
	class Category;
	class Player;
	class Sound;
	class Source;

	/// @brief Provides generic functionality regarding audio management.
	class xalExport AudioManager
	{
	public:
		friend class Buffer;
		friend class Category;
		friend class Player;
		friend class Sound;

		/// @brief Destructor.
		virtual ~AudioManager();
		/// @brief Initializes implementation-specific functionality.
		virtual void init();
		/// @brief Uninitializes implementation-specific functionality.
		void clear();
		
		/// @return Gets system backend ID.
		inline void* getBackendId() const { return this->backendId; }
		/// @return Gets name.
		HL_DEFINE_GET(hstr, name, Name);
		/// @return Gets sampling rate.
		HL_DEFINE_GET(int, samplingRate, SamplingRate);
		/// @return Gets channels.
		HL_DEFINE_GET(int, channels, Channels);
		/// @return Gets bits-per-sample value.
		HL_DEFINE_GET(int, bitsPerSample, BitsPerSample);
		/// @return Gets enabled-state.
		HL_DEFINE_IS(enabled, Enabled);
		/// @return Gets suspension-state.
		HL_DEFINE_IS(suspended, Suspended);
		/// @return Gets/sets the time how long Player instances should keep data loaded while idling.
		HL_DEFINE_GETSET(float, idlePlayerUnloadTime, IdlePlayerUnloadTime);
		/// @return Gets the device name.
		HL_DEFINE_GET(hstr, deviceName, DeviceName);
		/// @return Returns true if update is done in a separate thread.
		inline bool isThreaded() const { return (this->thread != NULL); }
		/// @return Gets the time interval between updates.
		HL_DEFINE_GET(float, updateTime, UpdateTime);
		/// @return Gets the global gain.
		float getGlobalGain();
		/// @return Sets the global gain.
		void setGlobalGain(float value);
		/// @return Gets the global gain fade target.
		float getGlobalGainFadeTarget();
		/// @brief Gets and sets the suspend gain fade speed.
		HL_DEFINE_GETSET(float, suspendResumeFadeTime, SuspendResumeFadeTime);
		/// @return Gets a list of all currently existing Player instances.
		harray<Player*> getPlayers();
		/// @return Gets a map of all loaded Sound instances.
		hmap<hstr, Sound*> getSounds();
		/// @return True if global gain is fading.
		bool isGlobalGainFading();

		/// @brief Updates all audio processing.
		/// @param[in] timeDelta Time since the call of this method in seconds.
		/// @note timeDelta is usually the time since the last frame in games. You don't have to call this if threaded update is enabled.
		void update(float timeDelta);

		/// @brief Creates a new audio Category.
		/// @param[in] name Name of the Category.
		/// @param[in] bufferMode How to handle the intermediate Buffer of the Sound in this category.
		/// @param[in] sourceMode How to handle the Source of the Sound in this category.
		/// @return The newly created Category.
		/// @note If the Category exists already, this call will have no effect.
		Category* createCategory(chstr name, BufferMode bufferMode, SourceMode sourceMode);
		/// @brief Gets the audio Category with the given name.
		/// @param[in] name Name of the Category.
		/// @return The Category with the given name.
		Category* getCategory(chstr name);
		/// @brief Checks whether a Category exists.
		/// @param[in] name Name of the Category.
		/// @return True if the Category exists.
		bool hasCategory(chstr name);
		
		/// @brief Creates a new Sound within a Category.
		/// @param[in] filename Filename of the Sound.
		/// @param[in] categoryName Name of the Category where to register the new Sound.
		/// @param[in] prefix Used to differentiate between Sounds that have the same filename (e.g. by using a directory path as prefix).
		/// @return The newly created Sound.
		Sound* createSound(chstr filename, chstr categoryName, chstr prefix = "");
		/// @brief Creates a new Sound within a Category form raw data.
		/// @param[in] name Name of the new Sound.
		/// @param[in] categoryName Name of the Category where to register the new Sound.
		/// @param[in] data Raw audio data.
		/// @param[in] size Raw audio data byte size.
		/// @param[in] channels Number of channels.
		/// @param[in] samplingRate Sampling rate.
		/// @param[in] bitsPerSample Bits per sample.
		/// @return The newly created Sound.
		Sound* createSound(chstr name, chstr categoryName, unsigned char* data, int size, int channels, int samplingRate, int bitsPerSample);
		/// @brief Gets the Sound with the given name.
		/// @param[in] name Name of the Sound.
		/// @return The Sound with the given name.
		Sound* getSound(chstr name);
		/// @brief Checks whether a Sound exists.
		/// @param[in] name Name of the Sound.
		/// @brief True if a Sound exists.
		bool hasSound(chstr name);
		/// @brief Destroys a Sound.
		/// @param[in] sound The Sound instance.
		void destroySound(Sound* sound);
		/// @brief Destroys all Sounds that have the given prefix in their name.
		/// @param[in] prefix Prefix for Sound names to destroy.
		void destroySoundsWithPrefix(chstr prefix);
		/// @brief Creates Sounds from a path and creates a Category for each directory.
		/// @param[in] path Path where the directories are located.
		/// @param[in] prefix Used to differentiate between Sounds that have the same filename (e.g. by using a directory path as prefix).
		/// @return A list of all Sound names that were created.
		/// @note The base-name of each directory located in path is used as Category name and all Sounds within are assigned to that Category.
		harray<hstr> createSoundsFromPath(chstr path, chstr prefix = "");
		/// @brief Creates Sounds from a path and assigns them to a Category.
		/// @param[in] path Path where the audio files are located.
		/// @param[in] categoryName Name for the Category.
		/// @param[in] prefix Used to differentiate between Sounds that have the same filename (e.g. by using a directory path as prefix).
		/// @return A list of all Sound names that were created.
		/// @note If the Category does not exist, it will be created.
		harray<hstr> createSoundsFromPath(chstr path, chstr categoryName, chstr prefix);

		/// @brief Creates a Player for a Sound.
		/// @param[in] soundName Name of the Sound for which the player will be used.
		/// @return The newly created player.
		Player* createPlayer(chstr soundName);
		/// @brief Destroys a Player.
		/// @param[in] player The player to destroy.
		void destroyPlayer(Player* player);

		/// @brief Plays a Sound in a fire-and-forget fashion.
		/// @param[in] soundName Name of the Sound.
		/// @param[in] fadeTime Time how long to fade in the Sound.
		/// @param[in] looping Whether the Sound should be looped.
		/// @param[in] gain The gain of the Sound.
		/// @note If the audio manager is suspended, this does nothing.
		void play(chstr soundName, float fadeTime = 0.0f, bool looping = false, float gain = 1.0f);
		/// @brief Plays a Sound in a fire-and-forget fashion asynchronously.
		/// @param[in] soundName Name of the Sound.
		/// @param[in] fadeTime Time how long to fade in the Sound.
		/// @param[in] looping Whether the Sound should be looped.
		/// @param[in] gain The gain of the Sound.
		/// @note If the audio manager is suspended, this does nothing.
		void playAsync(chstr soundName, float fadeTime = 0.0f, bool looping = false, float gain = 1.0f);
		/// @brief Stops all Sound instances that were played in a fire-and-forget fashion.
		/// @param[in] soundName Name of the Sound.
		/// @param[in] fadeTime Time how long to fade out the Sounds.
		void stop(chstr soundName, float fadeTime = 0.0f);
		/// @brief Stops only the first Sound instance that was played in a fire-and-forget fashion.
		/// @param[in] soundName Name of the Sound.
		/// @param[in] fadeTime Time how long to fade out the Sound.
		void stopFirst(chstr soundName, float fadeTime = 0.0f);
		/// @brief Stops all Sounds that are currently playing.
		/// @param[in] fadeTime Time how long to fade out the Sounds.
		/// @note This method also stops manually created Players.
		void stopAll(float fadeTime = 0.0f);
		/// @brief Stops all Sounds that belong to a certain Category.
		/// @param[in] categoryName Name of the Category.
		/// @param[in] fadeTime Time how long to fade out the Sounds.
		/// @note This method also stops manually created Players.
		void stopCategory(chstr categoryName, float fadeTime = 0.0f);
		/// @brief Checks if a Sound is playing.
		/// @param[in] soundName Name of the Sound.
		/// @return True if there is any Sound playing.
		/// @note This method only checks managed Sounds that were played in a fire-and-forget fashion.
		bool isAnyPlaying(chstr soundName);
		/// @brief Checks if a Sound is fading.
		/// @param[in] soundName Name of the Sound.
		/// @return True if there is any Sound fading.
		/// @note This method only checks managed Sounds that were played in a fire-and-forget fashion.
		bool isAnyFading(chstr soundName);
		/// @brief Checks if a Sound is fading in.
		/// @param[in] soundName Name of the Sound.
		/// @return True if there is any Sound fading in.
		/// @note This method only checks managed Sounds that were played in a fire-and-forget fashion.
		bool isAnyFadingIn(chstr soundName);
		/// @brief Checks if a Sound is fading out.
		/// @param[in] soundName Name of the Sound.
		/// @return True if there is any Sound fading out.
		/// @note This method only checks managed Sounds that were played in a fire-and-forget fashion.
		bool isAnyFadingOut(chstr soundName);

		/// @brief Fades the global gain to another value.
		/// @param[in] globalGainTarget The value to which the global gain should be changed.
		/// @param[in] fadeTime Time how long the global gain fade should take.
		void fadeGlobalGain(float globalGainTarget, float fadeTime = 1.0f);

		/// @brief Frees up unused memory.
		/// @note This can be useful if the operating system is low on memory.
		void clearMemory();
		/// @brief Suspends the entire audio processing.
		/// @note This is useful when the app goes out of focus. It does nothing if the system has already been suspended.
		void suspendAudio();
		/// @brief Resumes the previously suspended audio processing.
		/// @note This is useful when the app goes out of focus. It does nothing if the system hasn't been suspended previously.
		void resumeAudio();

		/// @brief Adds a custom audio file extension.
		/// @param[in] extension File extension to add.
		void addAudioExtension(chstr extension);
		/// @brief Finds an actual audio filename.
		/// @param[in] filename Reference filename for the audio file.
		/// @return The actual audio filename.
		virtual hstr findAudioFile(chstr filename) const;

	protected:
		/// @brief Name of the audio system.
		hstr name;
		/// @brief Back-end ID.
		void* backendId;
		/// @brief Sampling rate of the audio device.
		int samplingRate;
		/// @brief Number of channels of the audio device.
		int channels;
		/// @brief Bites per sample of the audio device.
		int bitsPerSample;
		/// @brief Whether any audio system is present.
		bool enabled;
		/// @brief Whether the audio system is suspended temporarily.
		/// @note Usually should be true when the app is suspended or out of focus.
		bool suspended;
		/// @brief Whether the audio system uses threaded updating.
		bool threaded;
		/// @brief How long a Player needs to remain inactive for its data to be cleared.
		float idlePlayerUnloadTime;
		/// @brief Device name which is required for some audio systems.
		hstr deviceName;
		/// @brief How much time should pass between updates when "threaded" is enabled.
		float updateTime;
		/// @brief Global gain.
		float globalGain;
		/// @brief Global gain fade target.
		float globalGainFadeTarget;
		/// @brief Global gain fade speed.
		float globalGainFadeSpeed;
		/// @brief Global gain fade time.
		float globalGainFadeTime;
		/// @brief Fade time when suspending and resuming audio system.
		float suspendResumeFadeTime;
		/// @brief List of registered audio categories.
		hmap<hstr, Category*> categories;
		/// @brief Currently existing Player instances.
		harray<Player*> players;
		/// @brief List of Player instances that are managed solely by the audio system.
		/// @note Managed players are usually created when a Sound is played through a call to the AudioManager instance and destroyed when they aren't needed anymore (using a fire-and-forget mechanism).
		harray<Player*> managedPlayers;
		/// @brief List of Player instances that need to resume once the audio system exits suspension.
		harray<Player*> suspendedPlayers;
		/// @brief List of loaded Sounds.
		hmap<hstr, Sound*> sounds;
		/// @brief List Buffer instances.
		harray<Buffer*> buffers;
		/// @brief List of file extensions supported.
		harray<hstr> extensions;
		/// @brief Thread instance handling the threaded update.
		hthread* thread;
		/// @brief Whether the threaded update is running.
		bool threadRunning;
		/// @brief Mutex for data access when threaded updating is used.
		hmutex mutex;

		/// @brief Constructor.
		/// @param[in] backendId Special ID needed by some audio systems.
		/// @param[in] threaded Whether update should be handled in a separate thread.
		/// @param[in] updateTime How much time should pass between updates when "threaded" is enabled.
		/// @param[in] deviceName Required by some audio systems.
		/// @note On Win32, backendId is the window handle. On Android, backendId is a pointer to the JavaVM. On iOS backendId is a pointer to the UIViewController.
		AudioManager(void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");

		/// @note Starts the thread for threaded update.
		void _startThreading();
		
		/// @note This method is not thread-safe and is for internal usage only.
		float _getGlobalGain() const;
		/// @note This method is not thread-safe and is for internal usage only.
		void _setGlobalGain(float value);
		/// @note This method is not thread-safe and is for internal usage only.
		float _getGlobalGainFadeTarget() const;
		/// @note This method is not thread-safe and is for internal usage only.
		harray<Player*> _getPlayers() const;
		/// @note This method is not thread-safe and is for internal usage only.
		hmap<hstr, Sound*> _getSounds() const;
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isGlobalGainFading() const;
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isAnyFading() const;

		/// @note This method is not thread-safe and is for internal usage only.
		virtual void _update(float timeDelta);

		/// @note This method is not thread-safe and is for internal usage only.
		Category* _createCategory(chstr name, BufferMode bufferMode, SourceMode sourceMode);
		/// @note This method is not thread-safe and is for internal usage only.
		Category* _getCategory(chstr name);
		/// @note This method is not thread-safe and is for internal usage only.
		bool _hasCategory(chstr name) const;

		/// @note This method is not thread-safe and is for internal usage only.
		virtual Sound* _createSound(chstr filename, chstr categoryName, chstr prefix);
		/// @note This method is not thread-safe and is for internal usage only.
		virtual Sound* _createSound(chstr name, chstr categoryName, unsigned char* data, int size, int channels, int samplingRate, int bitsPerSample);
		/// @note This method is not thread-safe and is for internal usage only.
		Sound* _getSound(chstr name);
		/// @note This method is not thread-safe and is for internal usage only.
		bool _hasSound(chstr name) const;
		/// @note This method is not thread-safe and is for internal usage only.
		void _destroySound(Sound* sound);
		/// @note This method is not thread-safe and is for internal usage only.
		void _destroySoundsWithPrefix(chstr prefix);
		/// @note This method is not thread-safe and is for internal usage only.
		harray<hstr> _createSoundsFromPath(chstr path, chstr prefix);
		/// @note This method is not thread-safe and is for internal usage only.
		harray<hstr> _createSoundsFromPath(chstr path, chstr category, chstr prefix);

		/// @note This method is not thread-safe and is for internal usage only.
		Player* _createPlayer(chstr name);
		/// @note This method is not thread-safe and is for internal usage only.
		void _destroyPlayer(Player* player);

		/// @brief Creates an internally managed Player.
		/// @param[in] name Name of the Sound.
		/// @return The newly created Player.
		Player* _createManagedPlayer(chstr name);
		/// @brief Destroys an internally managed Player.
		/// @param[in] player The Player to destroy.
		void _destroyManagedPlayer(Player* player);

		/// @note This method is not thread-safe and is for internal usage only.
		Buffer* _createBuffer(Sound* sound);
		/// @note This method is not thread-safe and is for internal usage only.
		Buffer* _createBuffer(Category* category, unsigned char* data, int size, int channels, int samplingRate, int bitsPerSample);
		/// @note This method is not thread-safe and is for internal usage only.
		void _destroyBuffer(Buffer* buffer);

		/// @note This method is not thread-safe and is for internal usage only.
		virtual Player* _createSystemPlayer(Sound* sound) = 0;
		/// @note This method is not thread-safe and is for internal usage only.
		virtual Source* _createSource(chstr filename, SourceMode sourceMode, BufferMode bufferMode, Format format);

		/// @note This method is not thread-safe and is for internal usage only.
		void _play(chstr soundName, float fadeTime, bool looping, float gain);
		/// @note This method is not thread-safe and is for internal usage only.
		void _playAsync(chstr soundName, float fadeTime, bool looping, float gain);
		/// @note This method is not thread-safe and is for internal usage only.
		void _stop(chstr soundName, float fadeTime);
		/// @note This method is not thread-safe and is for internal usage only.
		void _stopFirst(chstr soundName, float fadeTime);
		/// @note This method is not thread-safe and is for internal usage only.
		void _stopAll(float fadeTime);
		/// @note This method is not thread-safe and is for internal usage only.
		void _stopCategory(chstr categoryName, float fadeTime);
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isAnyPlaying(chstr soundName) const;
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isAnyFading(chstr soundName) const;
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isAnyFadingIn(chstr soundName) const;
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isAnyFadingOut(chstr soundName) const;

		/// @note This method is not thread-safe and is for internal usage only.
		void _fadeGlobalGain(float globalGainTarget, float fadeTime);

		/// @note This method is not thread-safe and is for internal usage only.
		void _clearMemory();
		/// @note This method is not thread-safe and is for internal usage only.
		virtual void _suspendAudio();
		/// @note This method is not thread-safe and is for internal usage only.
		virtual void _resumeAudio();

		/// @brief Depending on the audio manager implementation, this method may convert audio data to the appropriate format.
		/// @param[in] source Source object that holds the data.
		/// @param[in,out] stream The data stream buffer.
		void _convertStream(Source* source, hstream& stream);
		/// @brief Depending on the audio manager implementation, this method may convert audio data to the appropriate format.
		/// @param[in] logicalName Logical name of the data stream.
		/// @param[in] channels Number of channels.
		/// @param[in] samplingRate Sampling rate.
		/// @param[in] bitsPerSample Bits per sample.
		/// @param[in,out] stream The data stream buffer.
		virtual void _convertStream(chstr logicalName, int channels, int samplingRate, int bitsPerSample, hstream& stream);

		/// @brief Special additional processing for suspension, required for some implementations.
		/// @note This method is not thread-safe and is for internal usage only.
		virtual void _suspendSystem();
		/// @brief Special additional processing for suspension, required for some implementations.
		/// @note This method is not thread-safe and is for internal usage only.
		virtual void _resumeSystem();

		/// @brief Threaded update call.
		/// @param[in] thread The Thread instance calling.
		static void _update(hthread* thread);

	};
	
	/// @brief The AudioManager instance.
	xalExport extern xal::AudioManager* manager;

}

#endif

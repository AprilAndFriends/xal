/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Provides an interface to play and control audio data.

#ifndef XAL_PLAYER_H
#define XAL_PLAYER_H

#include <hltypes/hmutex.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <hltypes/hmutex.h>

#include "xalExport.h"

namespace xal
{
	class AudioManager;
	class Buffer;
	class Category;
	class Sound;

	/// @brief Provides audio playback functionality.
	class xalExport Player
	{
	public:
		friend class AudioManager;

		/// @return Gets gain at which the Sound is played.
		float getGain();
		/// @return Sets gain at which the Sound is played.
		void setGain(float value);
		/// @return Gets pitch multiplier.
		float getPitch();
		/// @return Sets pitch multiplier.
		void setPitch(float value);
		/// @return Gets associated Sound object.
		HL_DEFINE_GET(Sound*, sound, Sound);
		/// @return Gets name of the Sound.
		hstr getName();
		/// @return Gets filename of the Sound.
		hstr getFilename();
		/// @return Gets duration of the Sound.
		float getDuration();
		/// @return Gets byte-size of the Sound.
		int getSize();
		/// @return Gets buffer-size in bytes of the Sound.
		int getBufferSize();
		/// @return Gets byte-size of the Sound's source file.
		int getSourceSize();
		/// @return Gets time position of the playback.
		float getTimePosition();
		/// @return Gets sample position of the playback.
		unsigned int getSamplePosition();
		/// @return Gets Sound's category.
		Category* getCategory();
		
		/// @return True if the Sound is playing.
		/// @note This is false if the Sound is fading out even tough it is still "playing".
		bool isPlaying();
		/// @return True if the Sound is queued to play asynchronously.
		bool isAsyncPlayQueued();
		/// @return True if the Sound is paused.
		/// @note This is only true if the Sound isn't playing at all (and not fading).
		bool isPaused();
		/// @return True if the Sound is fading in or out.
		bool isFading();
		/// @return True if the Sound is fading in.
		bool isFadingIn();
		/// @return True if the Sound is fading out.
		bool isFadingOut();
		/// @return True if the Sound is looping.
		HL_DEFINE_IS(looping, Looping);

		/// @brief Starts playing the Sound.
		/// @param[in] fadeTime How long to fade-in the Sound.
		/// @param[in] looping Whether the Sound should be looped once it is done playing.
		/// @note Ignored if the Sound is already playing. Prevents pause/stop without pausing/stopping the Sound if called during fade-out.
		void play(float fadeTime = 0.0f, bool looping = false);
		/// @brief Starts playing the Sound asynchronously.
		/// @param[in] fadeTime How long to fade-in the Sound.
		/// @param[in] looping Whether the Sound should be looped once it is done playing.
		/// @note Ignored if the Sound is already playing. Prevents pause/stop without pausing/stopping the Sound if called during fade-out.
		void playAsync(float fadeTime = 0.0f, bool looping = false);
		/// @brief Stops the Sound completely.
		/// @param[in] fadeTime How long to fade-out the Sound.
		void stop(float fadeTime = 0.0f);
		/// @brief Pauses the Sound completely.
		/// @param[in] fadeTime How long to fade-out the Sound.
		/// @note Unpause the Sound with play().
		/// @see play
		void pause(float fadeTime = 0.0f);

	protected:
		/// @brief The gain at which the Sound is played.
		/// @note In general "gain" is not the same as "volume".
		float gain;
		/// @brief The pitch multiplier.
		float pitch;
		/// @brief Whether the Sound is paused.
		/// @note This is true while the Sound is still fading out.
		bool paused;
		/// @brief Whether the Sound is played in a looped manner.
		bool looping;
		/// @brief How quickly fading is done.
		float fadeSpeed;
		/// @brief How long fading is progressing.
		float fadeTime;
		/// @brief How far the buffer is offset.
		/// @note Different meaning and behavior depending on implementation.
		float offset; // TODO - should be removed?
		/// @brief The Sound to be played.
		Sound* sound;
		/// @brief The Buffer used for feeding the audio-system with audio data.
		Buffer* buffer;
		/// @brief The index of the current stream-buffer.
		/// @note Used usually only with streamed Sounds.
		int bufferIndex;
		/// @brief How many bytes have been processed during the last update.
		int processedByteCount;
		/// @brief How long this Player has been idle.
		/// @note Used for memory cleaning.
		float idleTime;
		/// @brief Flag whether async playing was queued.
		bool asyncPlayQueued;
		/// @brief Mutex for access of async playing flag.
		hmutex asyncPlayMutex;

		/// @brief Constructor.
		/// @param[in] sound The Sound to play.
		Player(Sound* sound);
		/// @brief Destructor.
		virtual ~Player();

		/// @note This method is not thread-safe and is for internal usage only.
		float _getGain();
		/// @note This method is not thread-safe and is for internal usage only.
		void _setGain(float value);
		/// @note This method is not thread-safe and is for internal usage only.
		float _getPitch();
		/// @note This method is not thread-safe and is for internal usage only.
		void _setPitch(float value);
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isPlaying();
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isPaused();
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isFading();
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isFadingIn();
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isFadingOut();
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isAsyncPlayQueued();

		/// @brief Updates the Player.
		/// @param[in] timeDelta Time since the last update.
		/// @note This method is not thread-safe and is for internal usage only.
		virtual void _update(float timeDelta);

		/// @note This method is not thread-safe and is for internal usage only.
		void _play(float fadeTime = 0.0f, bool looping = false);
		/// @note This method is not thread-safe and is for internal usage only.
		void _playAsync(float fadeTime = 0.0f, bool looping = false);
		/// @note This method is not thread-safe and is for internal usage only.
		void _stop(float fadeTime = 0.0f);
		/// @note This method is not thread-safe and is for internal usage only.
		void _pause(float fadeTime = 0.0f);

		/// @return The current gain for the Sound depending on global gain, category gain and Player gain.
		float _calcGain();

		/// @brief Whether the Sound is actually playing.
		/// @note This is implemented by the audio-system.
		inline virtual bool _systemIsPlaying() { return false; }
		/// @brief Position of the playback buffer.
		/// @note This is implemented by the audio-system.
		inline virtual unsigned int _systemGetBufferPosition() { return 0; }
		/// @brief Whether this implementation needs to correct the streamed buffer position.
		/// @note This is implemented by the audio-system.
		inline virtual bool _systemNeedsStreamedBufferPositionCorrection() { return true; }
		/// @brief Offset within the buffer.
		/// @note This is implemented by the audio-system.
		inline virtual float _systemGetOffset() { return 0.0f; }
		/// @brief Sets offset within the buffer.
		/// @note This is implemented by the audio-system.
		inline virtual void _systemSetOffset(float value) { }
		/// @brief Prepares the Player for playback.
		/// @return True if successful.
		/// @note This is implemented by the audio-system.
		inline virtual bool _systemPreparePlay() { return true; }
		/// @brief Prepares the underlying Buffer for playback.
		/// @note This is implemented by the audio-system.
		inline virtual void _systemPrepareBuffer() { }
		/// @brief Updates the current gain of the Player in the audio-system.
		/// @note This is implemented by the audio-system.
		inline virtual void _systemUpdateGain() { }
		/// @brief Updates the current pitch of the Player in the audio-system.
		/// @note This is implemented by the audio-system.
		inline virtual void _systemUpdatePitch() { }
		/// @brief Starts playback in the audio-system.
		/// @note This is implemented by the audio-system.
		inline virtual void _systemPlay() { }
		/// @brief Stops playback in the audio-system.
		/// @return How many bytes have been played since the last update.
		/// @note This is implemented by the audio-system.
		inline virtual int _systemStop() { return 0; }
		/// @brief Updates non-streaming processing in non-streamed Sounds.
		/// @note This is implemented by the audio-system.
		inline virtual void _systemUpdateNormal() { }
		/// @brief Updates streaming processing in streamed Sounds.
		/// @return How many bytes have been played since the last update.
		/// @note This is implemented by the audio-system.
		inline virtual int _systemUpdateStream() { return 0; }

	private:
		/// @brief Internal implementation for actually stopping the playback.
		void _stopSound(float fadeTime = 0.0f);

	};

}
#endif

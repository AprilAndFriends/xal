/// @file
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Provides an interface to play and control audio data.

#if 1


#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
#ifndef XAL_AVFOUNDATION_PLAYER_H
#define XAL_AVFOUNDATION_PLAYER_H

#include "Player.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;

	class xalExport AVFoundation_Player : public Player
	{
	public:
		AVFoundation_Player(Sound* sound, Buffer* buffer);
		virtual ~AVFoundation_Player();

	protected:

		bool _sysIsPlaying();
		float _sysGetOffset();
		void _sysSetOffset(float value);
		bool _sysPreparePlay();
		void _sysPrepareBuffer();
		void _sysUpdateGain();
		void _sysUpdateFadeGain();
		void _sysPlay();
		void _sysStop();
		void _sysUpdateStream();

		void *avAudioPlayer_void;
		
	};

}
#endif
#endif

#endif

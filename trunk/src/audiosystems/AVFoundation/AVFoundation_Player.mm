/// @file
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if 1

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
#include <string.h>
#include <AVFoundation/AVFoundation.h>

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "AVFoundation_AudioManager.h"
#include "AVFoundation_Player.h"
#include "Sound.h"

#define avAudioPlayer ((AVAudioPlayer*)avAudioPlayer_void)

namespace xal
{
	AVFoundation_Player::AVFoundation_Player(Sound* sound, Buffer* buffer) :
		Player(sound, buffer), avAudioPlayer_void(NULL)
	{
		NSLog(@"%s\n", __PRETTY_FUNCTION__);

	}

	AVFoundation_Player::~AVFoundation_Player()
	{
		NSLog(@"%s\n", __PRETTY_FUNCTION__);
		[avAudioPlayer release];
		avAudioPlayer_void = NULL;
	}

	bool AVFoundation_Player::_sysIsPlaying()
	{
		//NSLog(@"%s\n", __PRETTY_FUNCTION__);

		if(avAudioPlayer)
			return [avAudioPlayer isPlaying] ? true : false;
		else
			return false;
	}

	float AVFoundation_Player::_sysGetOffset()
	{
		NSLog(@"%s\n", __PRETTY_FUNCTION__);

		if(avAudioPlayer)
			return [avAudioPlayer currentTime];
		else
			NSLog(@"%s: ERROR: no audio player", __PRETTY_FUNCTION__);
		return 0;
	}

	void AVFoundation_Player::_sysSetOffset(float value)
	{
		NSLog(@"%s\n", __PRETTY_FUNCTION__);

		if(avAudioPlayer)
			[avAudioPlayer setCurrentTime:value];
		else
			NSLog(@"%s: ERROR: no audio player", __PRETTY_FUNCTION__);
	}

	bool AVFoundation_Player::_sysPreparePlay()
	{
		NSLog(@"%s\n", __PRETTY_FUNCTION__);
		NSString* realFilename = [NSString stringWithUTF8String:sound->getRealFilename().c_str()];
		NSString* filePath = [[NSBundle mainBundle] pathForResource:[realFilename stringByDeletingPathExtension] 
															 ofType:[realFilename pathExtension]];
		NSURL *url = [NSURL fileURLWithPath:filePath];
		NSError *error = nil;
		avAudioPlayer_void = [AVAudioPlayer alloc];
		[avAudioPlayer initWithContentsOfURL:url error:&error];
		NSLog(@"-- file %@, address %p", filePath, avAudioPlayer_void);
		if (avAudioPlayer_void && !error) 
		{
			return true;
		}
		else
		{
			if(error)
				NSLog(@"%s: ERROR: %@", __PRETTY_FUNCTION__, error);
			else
				NSLog(@"%s: ERROR: unknown error", __PRETTY_FUNCTION__);
			
			//[avAudioPlayer release];
			avAudioPlayer_void = nil;
			
			return false;

		}
	}

	void AVFoundation_Player::_sysPrepareBuffer()
	{
		NSLog(@"%s\n", __PRETTY_FUNCTION__);

		if(avAudioPlayer)
			[avAudioPlayer prepareToPlay];
		else
			NSLog(@"%s: ERROR: no audio player", __PRETTY_FUNCTION__);
	}

	void AVFoundation_Player::_sysUpdateGain()
	{
		NSLog(@"%s\n", __PRETTY_FUNCTION__);

		if(avAudioPlayer)
			[avAudioPlayer setVolume:this->_calcGain()];
		else
			NSLog(@"%s: ERROR: no audio player", __PRETTY_FUNCTION__);
	}

	void AVFoundation_Player::_sysUpdateFadeGain()
	{
		NSLog(@"%s\n", __PRETTY_FUNCTION__);

		if(avAudioPlayer)
			[avAudioPlayer setVolume:this->_calcFadeGain()];
		else
			NSLog(@"%s: ERROR: no audio player", __PRETTY_FUNCTION__);
	}

	void AVFoundation_Player::_sysPlay()
	{
		NSLog(@"%s\n", __PRETTY_FUNCTION__);

		if(avAudioPlayer) 
			[avAudioPlayer play];
		else
			NSLog(@"%s: ERROR: no audio player", __PRETTY_FUNCTION__);
	}

	void AVFoundation_Player::_sysStop()
	{
		NSLog(@"%s\n", __PRETTY_FUNCTION__);

		if(avAudioPlayer)
			[avAudioPlayer stop];
		else
			NSLog(@"%s: ERROR: no audio player", __PRETTY_FUNCTION__);
	}

	void AVFoundation_Player::_sysUpdateStream()
	{
		NSLog(@"%s\n", __PRETTY_FUNCTION__);

	}


}
#endif

#endif

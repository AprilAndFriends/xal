/// @file
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// iOS specific OpenAL code.

#if defined(_OPENAL) && defined(_IOS)

#import <AVFoundation/AVFoundation.h>

#include <hltypes/hlog.h>

#include "OpenAL_AudioManager.h"
#include "xal.h"

static bool active = true, restoreSessionFailed = false;
static int restoreAttempts = 0;

// on iOS, interruptions such as receiving a call and cancelling it or receiving an alarm cause problems
// with OpenAL. So this situation needs to be handled properly. 
// reference article: http://benbritten.com/2009/02/02/restarting-openal-after-application-interruption-on-the-iphone/
// modifed to use newer iOS apis then the ones in the article and added exceptions for problematic devices
// such as iPhone3GS

bool restoreiOSAudioSession()
{
	NSError* err = nil;
	[[AVAudioSession sharedInstance] setActive:YES error:&err];

	if (err != nil)
	{
		restoreSessionFailed = true;
		restoreAttempts++;
		if (restoreAttempts % 20 == 0)
			hlog::writef(xal::logTag, "Failed restoring iOS Audio Session after %d attempts. Will keep trying...", restoreAttempts);
		return 0;
	}
	if (((xal::OpenAL_AudioManager*) xal::mgr)->resumeOpenALContext())
	{
		if (restoreAttempts > 0)
		{
			hlog::writef(xal::logTag, "Succeded restoring iOS Audio Session after %d attempts.", restoreAttempts);
		}
		active = true;
		restoreSessionFailed = false;
		restoreAttempts = 0;
		return 1;
	}
	else return 0;
}

void suspendiOSAudioSession()
{
	[[AVAudioSession sharedInstance] setActive:NO error:NULL];
	active = false;
	((xal::OpenAL_AudioManager*) xal::mgr)->suspendOpenALContext();
}

bool hasiOSAudioSessionRestoreFailed()
{
	return restoreSessionFailed;
}

@interface OpenALAudioSessionDelegate : NSObject<AVAudioPlayerDelegate, AVAudioSessionDelegate>
{

}
@end

@implementation OpenALAudioSessionDelegate

- (void)beginInterruption
{
	hlog::writef(xal::logTag, "iOS audio interruption began.");
	suspendiOSAudioSession();
}

- (void)endInterruption
{
	hlog::writef(xal::logTag, "iOS audio interruption ended.");
	if (!restoreiOSAudioSession())
	{
		hlog::writef(xal::logTag, "Error resuming Audio session, try again later.");
	}
}

@end

static OpenALAudioSessionDelegate* _audio_delegate = 0;

bool OpenAL_iOS_isAudioSessionActive()
{
	return active;
}

void OpenAL_iOS_init()
{
	_audio_delegate = [OpenALAudioSessionDelegate alloc];
	[[AVAudioSession sharedInstance] setDelegate:_audio_delegate];
}

void OpenAL_iOS_destroy()
{
	if (_audio_delegate)
	{
		[_audio_delegate release];
		_audio_delegate = NULL;
	}
}

#endif

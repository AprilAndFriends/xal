/// @file
/// @author  Kresimir Spes
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// iOS specific OpenAL code

#if defined(HAVE_OPENAL) && defined(_IOS)

#import <AVFoundation/AVFoundation.h>
#include "OpenAL_AudioManager.h"

static bool active = true, restoreSessionFailed = false;
static int restoreAttempts = 0;

// on iOS, interruptions such as receiving a call and cancelling it or receiving an alarm cause problems
// with openAL. So this situation needs to be handled properly. 
// reference article: http://benbritten.com/2009/02/02/restarting-openal-after-application-interruption-on-the-iphone/
// modifed to use never iOS apis then the ones in the article

bool restoreiOSAudioSession()
{
	NSError* err = nil;
	[[AVAudioSession sharedInstance] setActive:YES error:&err];

	if (err != nil)
	{
		restoreSessionFailed = true;
		restoreAttempts++;
		return 0;
	}
	else if (restoreAttempts > 0)
	{
		NSLog(@"Succeded restoring iOS Audio Session after %d attempts.", restoreAttempts);
	}
	((xal::OpenAL_AudioManager*) xal::mgr)->resumeOpenALContext();
	active = true;
	restoreSessionFailed = false;
	restoreAttempts = 0;
	return 1;
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
	NSLog(@"iOS audio interruption began.");
	suspendiOSAudioSession();
}

- (void)endInterruption
{
	NSLog(@"iOS audio interruption ended.");
	if (!restoreiOSAudioSession())
	{
		NSLog(@"Error resuming Audio session, try again later.");
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

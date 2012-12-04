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

// on iOS, interruptions such as receiving a call and cancelling it or receiving an alarm cause problems
// with openAL. So this situation needs to be handled properly. 
// reference article: http://benbritten.com/2009/02/02/restarting-openal-after-application-interruption-on-the-iphone/
// modifed to use never iOS apis then the ones in the article
	
@interface OpenALAudioSessionDelegate : NSObject<AVAudioPlayerDelegate, AVAudioSessionDelegate>
{

}
@end

@implementation OpenALAudioSessionDelegate

- (void)beginInterruption
{
	NSLog(@"iOS audio interruption began.");
	[[AVAudioSession sharedInstance] setActive:NO error:NULL];
	((xal::OpenAL_AudioManager*) xal::mgr)->suspendOpenALContext();
}

- (void)endInterruption
{
	NSLog(@"iOS audio interruption ended.");
	[[AVAudioSession sharedInstance] setActive:YES error:NULL];
	((xal::OpenAL_AudioManager*) xal::mgr)->resumeOpenALContext();
}

@end

static OpenALAudioSessionDelegate* _audio_delegate = 0;

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

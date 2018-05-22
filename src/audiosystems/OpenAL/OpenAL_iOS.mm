/// @file
/// @version 3.6
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
#import <UIKit/UIKit.h>

#include <hltypes/hlog.h>

#include "OpenAL_AudioManager.h"
#include "xal.h"

static bool active = true;
bool restoreSessionFailed = false;
static int restoreAttempts = 0;

// on iOS, interruptions such as receiving a call and cancelling it or receiving an alarm cause problems
// with OpenAL. So this situation needs to be handled properly. 
// reference article: http://benbritten.com/2009/02/02/restarting-openal-after-application-interruption-on-the-iphone/
// modifed to use newer iOS apis then the ones in the article and added exceptions for problematic devices
// such as iPhone3GS

@interface xaliOSAppDelegate : NSObject

@end

@implementation xaliOSAppDelegate

+ (void)load
{
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didFinishLaunching:) name:UIApplicationDidFinishLaunchingNotification object:nil];
}

+ (void)didFinishLaunching:(NSNotification*)notification
{
	[[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryAmbient error:NULL];
}
@end

bool tryActivateAudioSession()
{
	NSError* error = nil;
	[[AVAudioSession sharedInstance] setActive:YES error:&error];
	if (error != nil)
	{
		restoreSessionFailed = true;
		++restoreAttempts;
		if (restoreAttempts % 20 == 0)
		{
			hlog::writef(xal::logTag, "Failed restoring iOS Audio Session after %d attempts. Will keep trying...", restoreAttempts);
		}
		return false;
	}
	return true;
}

void finishRestore()
{
	if (restoreAttempts > 0)
	{
		hlog::writef(xal::logTag, "Succeeded restoring iOS Audio Session after %d attempts.", restoreAttempts);
	}
	active = true;
	restoreSessionFailed = false;
	restoreAttempts = 0;
}

bool restoreiOSAudioSession() // mutexed version
{
	if (!tryActivateAudioSession())
	{
		return false;
	}
	if (((xal::OpenAL_AudioManager*)xal::manager)->resumeOpenALContext())
	{
		finishRestore();
		return true;
	}
	return false;
}

bool _restoreiOSAudioSession() // unmutexed version to avoid deadlocks
{
	if (!tryActivateAudioSession())
	{
		return false;
	}
	if (((xal::OpenAL_AudioManager*)xal::manager)->_resumeOpenALContext())
	{
		finishRestore();
		return true;
	}
	return false;
}

void suspendiOSAudioSession()
{
	[[AVAudioSession sharedInstance] setActive:NO error:NULL];
	active = false;
	((xal::OpenAL_AudioManager*)xal::manager)->suspendOpenALContext();
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

- (void)handleInterruption:(NSNotification*)notification
{
	NSDictionary* interuptionDict = notification.userInfo;
	// get the AVAudioSessionInterruptionTypeKey enum from the dictionary
	NSInteger interuptionType = [[interuptionDict valueForKey:AVAudioSessionInterruptionTypeKey] integerValue];
	NSNumber* seccondReason = [[notification userInfo] objectForKey:@"AVAudioSessionInterruptionOptionKey"] ;
	// decide what to do based on interruption type here...
	switch (interuptionType)
	{
	case AVAudioSessionInterruptionTypeBegan:
		hlog::write(xal::logTag, "iOS audio interruption began.");
		suspendiOSAudioSession();
		break;
	case AVAudioSessionInterruptionTypeEnded:
		hlog::write(xal::logTag, "iOS audio interruption ended.");
		break;
	}
	switch ([seccondReason integerValue])
	{
	case AVAudioSessionInterruptionOptionShouldResume:
		hlog::write(xal::logTag, "resuming audio playback");
		if (!restoreiOSAudioSession())
		{
			hlog::writef(xal::logTag, "Error resuming Audio session, try again later.");
		}
		break;
	default:
		hlog::write(xal::logTag, "iOS audio interruption ended, but iOS says playback shouldn't be resumed yet.");
		break;
	}
	
}
@end

static OpenALAudioSessionDelegate* _audioDelegate = NULL;

bool OpenAL_iOS_isAudioSessionActive()
{
	return active;
}

void OpenAL_iOS_init()
{
	_audioDelegate = [OpenALAudioSessionDelegate alloc];
	[[NSNotificationCenter defaultCenter] addObserver:_audioDelegate selector:@selector(handleInterruption:) name:AVAudioSessionInterruptionNotification object:[AVAudioSession sharedInstance]];
}

void OpenAL_iOS_destroy()
{
	if (_audioDelegate != NULL)
	{
		[_audioDelegate release];
		_audioDelegate = NULL;
	}
}
#endif

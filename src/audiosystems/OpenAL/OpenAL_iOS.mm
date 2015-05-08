/// @file
/// @version 3.4
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

static bool active = true, restoreSessionFailed = false;
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

+ (void)didFinishLaunching:(NSNotification *)notification
{
	if ([[[UIDevice currentDevice] systemVersion] compare:@"5.0" options:NSNumericSearch] == NSOrderedAscending)
	{
		// less than iOS 5.0 - workarround for an apple bug where the audio sesion get's interrupted while using AVAssetReader and similar
		AVAudioSession *audioSession = [AVAudioSession sharedInstance];
		[audioSession setActive: NO error: nil];
		[audioSession setCategory:AVAudioSessionCategoryPlayback error:nil];
		
		// Modifying Playback Mixing Behavior, allow playing music in other apps
		UInt32 allowMixing = true;
		AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryMixWithOthers, sizeof(allowMixing), &allowMixing);
		[audioSession setActive: YES error: nil];
	}
	else
	{
		[[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryAmbient error:NULL];
	}
}
@end

bool restoreiOSAudioSession()
{
	NSError* err = nil;
	[[AVAudioSession sharedInstance] setActive:YES error:&err];

	if (err != nil)
	{
		restoreSessionFailed = true;
		restoreAttempts++;
		if (restoreAttempts % 20 == 0)
			hlog::writef(logTag, "Failed restoring iOS Audio Session after %d attempts. Will keep trying...", restoreAttempts);
		return 0;
	}
	if (((xal::OpenAL_AudioManager*) xal::manager)->resumeOpenALContext())
	{
		if (restoreAttempts > 0)
		{
			hlog::writef(logTag, "Succeded restoring iOS Audio Session after %d attempts.", restoreAttempts);
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
	((xal::OpenAL_AudioManager*) xal::manager)->suspendOpenALContext();
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
#if  __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_6_0
	NSDictionary *interuptionDict = notification.userInfo;
	// get the AVAudioSessionInterruptionTypeKey enum from the dictionary
	NSInteger interuptionType = [[interuptionDict     valueForKey:AVAudioSessionInterruptionTypeKey] integerValue];
 NSNumber* seccondReason = [[notification userInfo] objectForKey:@"AVAudioSessionInterruptionOptionKey"] ;
	// decide what to do based on interruption type here...
	switch (interuptionType)
	{
		case AVAudioSessionInterruptionTypeBegan:
			hlog::write(logTag, "iOS audio interruption began.");
			suspendiOSAudioSession();
			break;
		case AVAudioSessionInterruptionTypeEnded:
			hlog::write(logTag, "iOS audio interruption ended.");
			break;
	}
	switch ([seccondReason integerValue])
	{
		case AVAudioSessionInterruptionOptionShouldResume:
			hlog::write(logTag, "resuming audio playback");
			if (!restoreiOSAudioSession())
			{
				hlog::writef(logTag, "Error resuming Audio session, try again later.");
			}
			break;
		default:
			hlog::write(logTag, "iOS audio interruption ended, but iOS says playback shouldn't be resumed yet.");
			break;
	}
	
}
#else
- (void)beginInterruption
{
	hlog::write(logTag, "iOS audio interruption began.");
	suspendiOSAudioSession();
}

- (void)endInterruption
{
	hlog::write(logTag, "iOS audio interruption ended.");
	if (!restoreiOSAudioSession())
	{
		hlog::writef(logTag, "Error resuming Audio session, try again later.");
	}
}
#endif
@end

static OpenALAudioSessionDelegate* _audio_delegate = 0;

bool OpenAL_iOS_isAudioSessionActive()
{
	return active;
}

void OpenAL_iOS_init()
{
	_audio_delegate = [OpenALAudioSessionDelegate alloc];
#if  __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_6_0
	[[NSNotificationCenter defaultCenter] addObserver: _audio_delegate
											 selector: @selector(handleInterruption:)
												 name: AVAudioSessionInterruptionNotification
											   object: [AVAudioSession sharedInstance]];
#else
	[[AVAudioSession sharedInstance] setDelegate:_audio_delegate];
#endif
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

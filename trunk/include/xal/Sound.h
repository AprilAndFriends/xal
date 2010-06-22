/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_SOUND_H
#define XAL_SOUND_H

#include <hltypes/hstring.h>
#include "xalExport.h"

struct XALposition
{
	float x,y,z;
};

#define XAL_FADE_NOTHING 0
#define XAL_FADE_STOP 1
#define XAL_FADE_PAUSE 2
#define XAL_FADE_PLAY 3

namespace xal
{
	class xalExport Sound
	{
	protected:
		XALposition mPosition;
		float mGain;
		bool mLoop;
		unsigned int mSource;
		float mFadeTimer;
		float mFadeSpeed;
		int mFadeAction;
		bool mPaused;
		float mDuration;
		hstr mName;
		hstr mCategory;
		unsigned int mBuffer;
	public:


		Sound(chstr name);
		virtual ~Sound();

		void play(float fade_in_time=0.0f,float x=0,float y=0,float z=0);
		void stop(float fade_out_time=0.0f);
		void pause(float fade_out_time=0.0f);
		
		bool isPlaying();

		void setGain(float gain);
		float getGain();
		
		void setCategory(chstr category) { mCategory=category; }
		chstr getCategory() { return mCategory; }
		
		chstr getName() { return mName; }

		float getSampleOffset();

		unsigned int getSource();

		float getDuration() { return mDuration; }

		void setPosition(float x,float y,float z);
		XALposition getPosition();

		void update(float k);

		void setLoop(bool loop);
		bool getLoop();
	};

}

#endif

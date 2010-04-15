#ifndef XAL_SOUND_H
#define XAL_SOUND_H

#include <string>
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
		std::string mName;
		std::string mCategory;
		unsigned int mBuffer;
	public:


		Sound(std::string name);
		virtual ~Sound();

		void play(float fade_in_time=0.0f,float x=0,float y=0,float z=0);
		void stop(float fade_out_time=0.0f);
		void pause(float fade_out_time=0.0f);
		
		bool isPlaying();

		void setGain(float gain);
		float getGain();
		
		void setCategory(const std::string& category) { mCategory=category; }
		const std::string& getCategory() { return mCategory; }
		
		const std::string& getName() { return mName; }

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

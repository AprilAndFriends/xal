#ifndef XAL_SOUND_H
#define XAL_SOUND_H

#include <string>

struct ALposition
{
	float x,y,z;
};

#define FADE_NOTHING 0
#define FADE_STOP 1
#define FADE_PAUSE 2
#define FADE_PLAY 3

class Sound
{
private:
	ALposition mPosition;
	float mGain;
	bool mLoop;
	unsigned int mSource;
	float mFadeTimer;
	float mFadeSpeed;
	int mFadeAction;
	bool mPaused;
protected:
	float mDuration;
public:
	std::string mName;
	std::string mCategory;
	unsigned int mBuffer;

	Sound(std::string name);
	~Sound();

	void play(float fade_in_time=0.0f,float x=0,float y=0,float z=0);
	void stop(float fade_out_time=0.0f);
	void pause(float fade_out_time=0.0f);
	
	bool isPlaying();

	void setGain(float gain);
	float getGain();

	float getSampleOffset();

	unsigned int getSource();

	float getDuration() { return mDuration; }

	void setPosition(float x,float y,float z);
	ALposition getPosition();

	void update(float k);

	void setLoop(bool loop);
	bool getLoop();
};


#endif

#include <stdio.h>
#include <xal/SoundManager.h>

int main(int argc, char **argv)
{
	xal::SoundManager* mgr=new xal::SoundManager("");
	
	xal::Sound* s=mgr->createSound("../media/bark.ogg");
	
	s->play();
	
	while (s->isPlaying())
	{
		
	}
	
	s->play();
	
	while (s->isPlaying())
	{
		
	}
	
	s->play();
	
	while (s->isPlaying())
	{
		
	}
	
	delete mgr;
	return 0;
}

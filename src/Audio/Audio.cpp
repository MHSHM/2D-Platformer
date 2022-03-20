#include <Audio.h>
#include <thread>

bool AudioPlayer::Init()
{
	sound_engine = irrklang::createIrrKlangDevice();
	if (!sound_engine) 
	{
		return false; 
	}

	return true;
}

void AudioPlayer::Play_Sound(const std::string&& path, float volume, bool is_looped = true)
{
	sound_engine->setSoundVolume(volume);
	sound_engine->play2D(path.c_str(), is_looped);
}

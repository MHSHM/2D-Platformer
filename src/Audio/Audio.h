#pragma once

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

#include <string>
#include <irrKlang.h>


class AudioPlayer
{
public:
	bool Init();
	void Play_Sound(const std::string&& path, float voluem, bool is_looped);

public:
	irrklang::ISoundSource* source;
	irrklang::ISoundEngine* sound_engine;
};

#pragma once

#include "../Dependencies/GLM/mat4x4.hpp"
#include "../Dependencies/GLM/gtx/transform.hpp"

#include <Texture.h>

class Renderer 
{
public:
	Renderer(class Game* _game);
	void Draw(class Transform* transform, class Render* render);
public:
	class Game* game;

	~Renderer();

};
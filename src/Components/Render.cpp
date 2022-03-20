#include <Render.h>
#include <iostream>
#include <Texture.h>
#include <Game.h>
#include <Scene.h>

Render::Render(Actor* _owner) :
	Component(_owner),
	texture(nullptr)
{
	quad.Init();
}

void Render::Update(float deltatime)
{
}

void Render::Clear()
{
	Animation* animation = owner->Get_Component<Animation>();

	// if the owner has an animation cmp
	// then the texture should be deleted with it
	if (animation != nullptr) 
	{
		texture = nullptr; 
		return;
	}
	
	owner->scene_node->scene->game->Remove_Texture(owner->scene_node->scene, texture);
	texture = nullptr;
}

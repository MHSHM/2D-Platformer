#include <Game.h>
#include <Scene.h>
#include <thread>

#include <glfw3.h>

Scene::Scene():
	game(nullptr)
{

}

void Scene::Init(Game* _game)
{
	game = _game;
	scene_nodes.reserve(MAX_SCENE_NODES);
	dead_scene_nodes.reserve(MAX_SCENE_NODES);
	actors.reserve(MAX_ACTOR);
	renderable_cmps.reserve(MAX_RENDER);
	input_cmps.reserve(MAX_INPUT);
	rigidbody_cmps.reserve(MAX_Rigidbody);
	collider_cmps.reserve(MAX_COLLIDER);
	camera_cmps.reserve(MAX_CAMERA);
	animation_cmps.reserve(MAX_ANIMATION);
	textures.reserve(MAX_TEXTURES);
}

void Scene::Update_Components(float deltatime)
{
	for(auto& animation : animation_cmps)
	{
		animation.Update(deltatime);
	}

	for (auto& renderable : renderable_cmps)
	{
		renderable.first.Update(deltatime);
		renderable.second.Update(deltatime);
	}

	for (auto& input : input_cmps)
	{
		input.Update(deltatime);
	}

	for (auto& camera : camera_cmps)
	{
		camera.Update(deltatime);
	}

	for (auto& rigidbody : rigidbody_cmps)
	{
		rigidbody.Update(deltatime);
	}

	for (auto& collider : collider_cmps)
	{
		collider.Update(deltatime);
	}
}


void Scene::Clear()
{
	while (!scene_nodes.empty())
	{
		game->Remove_Scene_Node(this, &scene_nodes.back());
	}

	scene_nodes_map.clear();
}

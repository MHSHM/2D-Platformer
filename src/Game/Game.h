#pragma once

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define MAX_SCENES 10

#include <Component.h>
#include <Renderer.h>
#include <Shader.h>
#include <Audio.h>

#include <unordered_map>

class Game
{
public:
	Game();
	bool Init();
	void Run_Game();
	void Shutdown();
	void Process_Input();
	void Update();
	void Generate_Output();
	void Load_Data(); 
	void Load_Shaders();

public:
	class Scene* Create_Scene(const std::string&& scene_tag);
	void Set_Active_Scene(const std::string&& scene_tag);
	class SceneNode* Create_Scene_Node(class Scene* scene, std::string&& _tag); 
	void Remove_Scene_Node(class Scene* scene, SceneNode* scene_node);
	void Remove_Actor(class Scene*scene, class Actor* actor);
	void Add_Component(class SceneNode* scene_node, ComponentType type);
	void Remove_Component(Scene* scene, Component* cmp);
	Texture* Load_Texture(class Scene* scene, std::string&& texture_path);
	void Remove_Texture(class Scene* scene, class Texture* texture);
	float Get_Deltatime();

public:
	class GLFWwindow* window;

	Renderer renderer;
	AudioPlayer audio_player;
	class Scene* active_scene;

	bool is_game_running; 
	float time_since_last_frame;

	std::vector<Scene> scenes;

	std::unordered_map<std::string, Shader> shaders_table;
	std::unordered_map<std::string, Scene*> scenes_map;

	~Game();
};


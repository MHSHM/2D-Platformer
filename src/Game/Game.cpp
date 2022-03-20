#include "Game.h"
#include <Scene.h>
#include <SceneNode.h>
#include <Component.h>
#include <Transform.h>
#include <Render.h>
#include <Input.h>
#include <Rigidbody.h>
#include <Collider.h>
#include <Camera.h>
#include <Animation.h>


#define GLEW_STATIC

#include <iostream>
#include <sstream>
#include <fstream>
#include <glew.h>
#include <glfw3.h>

Game::Game() :
	is_game_running(true),
	window(nullptr),
	time_since_last_frame(0.0f),
	renderer(this)
{
	
}

bool Game::Init()
{

	scenes.reserve(MAX_SCENES);

	if (!glfwInit())
	{
		std::cerr << "failed to Initialize GLFW!\n";
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Mario", NULL, NULL);
	if (!window)
	{
		std::cerr << "failed to create window!\n";
		return false;
	}

	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return false;
	}

	if (!audio_player.Init()) 
	{
		std::cerr << "failed to initialize the audio player\n";
		return false; 
	}

	Load_Data();

	Load_Shaders();

	return true;
}

void Game::Run_Game()
{
	while (is_game_running)
	{
		Process_Input();
		Update();
		Generate_Output();
	}
}

void Game::Shutdown()
{
	for (auto& scene : scenes) 
	{
		scene.Clear(); 
	}

	scenes_map.clear();

	for (auto shader : shaders_table) 
	{
		shader.second.Clear(); 
	}

	shaders_table.clear();

	glfwTerminate();
}

void Game::Process_Input()
{
	glfwPollEvents();

	is_game_running = !glfwWindowShouldClose(window);
}


void Game::Update()
{
	float deltatime = Get_Deltatime();
	active_scene->Update_Components(deltatime);

	if (active_scene->tag == "main_menu") 
	{
		SceneNode* start = active_scene->scene_nodes_map["start_button"]; 
		SceneNode* exit = active_scene->scene_nodes_map["exit_button"]; 

		Transform* start_transform = start->actor->Get_Component<Transform>();
		Render* start_render = start->actor->Get_Component<Render>(); 
		Transform* exit_transform = exit->actor->Get_Component<Transform>(); 
		Render* exit_render = exit->actor->Get_Component<Render>(); 

		int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		if (state == GLFW_PRESS)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			float x_offset = 20.0f;

			float start_width = ((start_render->quad.width * start_transform->scale_x)
				+ start_transform->translation.x) - x_offset;

			bool start_check_x_axis = ((float)xpos >= (start_transform->translation.x + x_offset) &&
				((float)xpos <= start_width));

			float y_offset = 40.0f;
			float start_screen_space = (WINDOW_HEIGHT - start_transform->translation.y) - y_offset;

			bool start_check_y_axis = ((float)ypos <= start_screen_space &&
				(float)ypos >= start_screen_space - (start_render->quad.height * start_transform->scale_y) + (y_offset * 2.0f));

			if (start_check_x_axis & start_check_y_axis)
			{
				Set_Active_Scene("level_1");
				audio_player.sound_engine->stopAllSounds();
				audio_player.Play_Sound("Assets/Sound/Grasslands Theme.mp3", 1.0f, true);
				return;
			}

			float exit_y_offset = 10.0f;
			float exit_width = (exit_render->quad.width * exit_transform->scale_x) + exit_transform->translation.x;
			// from view space to screen space
			float exit_screen_space = (WINDOW_HEIGHT - exit_transform->translation.y) - exit_y_offset;
			float exit_height = exit_screen_space - (exit_render->quad.height * exit_transform->scale_y) + exit_y_offset;

			bool exit_check_x_axis = (xpos >= exit_transform->translation.x && xpos <= exit_width);
			bool exit_check_y_axis = (ypos <= exit_screen_space && ypos >= exit_height);

			if (exit_check_x_axis && exit_check_y_axis)
			{
				is_game_running = false;
			}
		}
	}

	if (active_scene->tag == "level_1")
	{
		static bool is_lost = false;
		Scene* level_1 = scenes_map["level_1"];
		SceneNode* player = level_1->scene_nodes_map["player"];

		SceneNode* ground_container = level_1->scene_nodes_map["ground_container"];
		SceneNode* floating_tiles_container = level_1->scene_nodes_map["floating_tiles_container"];
		SceneNode* gems_container = level_1->scene_nodes_map["gems_container"];
		SceneNode* wall1 = level_1->scene_nodes_map["wall1"];
		SceneNode* camera = level_1->scene_nodes_map["level_1_camera"];
		SceneNode* wall2 = level_1->scene_nodes_map["wall2"];
		SceneNode* finish_line = level_1->scene_nodes_map["finish_line"];

		Collider* player_collider = player->actor->Get_Component<Collider>();
		const Collider* wall1_collider = wall1->actor->Get_Component<Collider>();
		const Collider* cam_collider = camera->actor->Get_Component<Collider>();
		const Collider* wall2_collider = wall2->actor->Get_Component<Collider>();
		const Collider* finish_line_collider = finish_line->actor->Get_Component<Collider>();

		Transform* player_transform = player->actor->Get_Component<Transform>();
		Rigidbody* r = player->actor->Get_Component<Rigidbody>();
		Animation* player_animation = player->actor->Get_Component<Animation>();

		r->Add_Force(glm::vec2(0.0f, 1.f), -200.0f);

		// fraction
		float fraction = 90.0f;

		if (r->velocity.x > 0.0f)
		{
			r->Add_Force(glm::vec2(-1.0f, 0.0f), std::min(fraction, r->velocity.x));
		}

		if (r->velocity.x < 0.0f)
		{
			r->Add_Force(glm::vec2(1.0f, 0.0f), std::min(fraction, abs(r->velocity.x)));
		}


		bool is_colliding = false;
		for (auto ground : ground_container->children)
		{
			Collider* collider = ground->actor->Get_Component<Collider>();
			if (Is_Colliding(*player_collider, *collider) && !is_lost)
			{
				Transform* ground_transform = ground->actor->Get_Component<Transform>();
				Render* render = ground->actor->Get_Component<Render>();

				player_transform->translation.y = ground_transform->translation.y +
					(render->quad.height * ground_transform->scale_y) - player_collider->aabb.min_y_offset;

				// cancel gravity
				r->Add_Force(glm::vec2(0.0f, 1.0f), 200.0f);
				r->velocity.y = 0.f;

				player_animation->Set_State("idle");

				is_colliding = true;
			}
		}


		for (auto& floating_tile : floating_tiles_container->children)
		{
			Transform* floating_tile_transform = floating_tile->actor->Get_Component<Transform>();

			const Collider* tile_c = floating_tile->actor->Get_Component<Collider>();

			Render* render = floating_tile->actor->Get_Component<Render>();

			if (Is_Colliding(*player_collider, *tile_c))
			{
				player_transform->translation.y = floating_tile_transform->translation.y +
					(render->quad.height * floating_tile_transform->scale_y) - player_collider->aabb.min_y_offset;

				r->Add_Force(glm::vec2(0.0f, 1.0f), 200.0f);
				r->velocity.y = 0.f;

				player_animation->Set_State("idle");

				is_colliding = true;
			}
		}

		if (Is_Colliding(*player_collider, *wall1_collider) && !is_lost)
		{
			Transform* invisible_wall_transform = wall1->actor->Get_Component<Transform>();
			Render* invisible_wall_render = wall1->actor->Get_Component<Render>();

			player_transform->translation.x = invisible_wall_transform->translation.x +
				(invisible_wall_transform->scale_x * invisible_wall_render->quad.width) - player_collider->aabb.min_x_offset;
		}

		if (Is_Colliding(*player_collider, *wall2_collider) && !is_lost)
		{
			float cam_offset = 400.0f;
			float wall1_offset = 400.0f;

			Transform* wall1_transform = wall1->actor->Get_Component <Transform>();
			Transform* wall2_transform = wall2->actor->Get_Component<Transform>();
			Transform* cam_transform = camera->actor->Get_Component<Transform>();

			wall2_transform->translation.x = player_transform->translation.x + (player_transform->scale_x * 320.0f);
			wall1_transform->translation.x = player_transform->translation.x - (wall1_offset);

			cam_transform->translation.x = player_transform->translation.x - cam_offset;
		}

		// player movement
		float player_speed = 350.0f;
		float y_threshold = 231.0f;

		if (player_transform->translation.y > y_threshold && !is_lost)
		{

			int right = glfwGetKey(window, GLFW_KEY_RIGHT);

			if (right == GLFW_PRESS)
			{
				r->Add_Force(glm::vec2(1.0f, 0.0f), player_speed);
				if (is_colliding == true)
				{
					player_animation->Set_State("running");
				}
			}

			int left = glfwGetKey(window, GLFW_KEY_LEFT);

			if (left == GLFW_PRESS)
			{
				r->Add_Force(glm::vec2(1.0f, 0.0f), -player_speed);
				if (is_colliding == true)
				{
					player_animation->Set_State("running");
				}
			}

			float jump_force = 800.0f;
			int jump = glfwGetKey(window, GLFW_KEY_SPACE);

			if (jump == GLFW_PRESS)
			{
				if (r->prev_velocity.y < r->velocity.y)
				{
					r->Add_Force(glm::vec2(0.0f, 1.0f), jump_force);
					player_animation->Set_State("jump");
					if (!audio_player.sound_engine->isCurrentlyPlaying("Assets/Sound/jumping/jump_01.wav")) 
					{
						audio_player.Play_Sound("Assets/Sound/jumping/jump_01.wav", 0.3f, false);
					}
				}
				else if (r->velocity.y == 0.0f)
				{
					r->Add_Force(glm::vec2(0.0f, 1.0f), jump_force);
					player_animation->Set_State("jump");
					if (!audio_player.sound_engine->isCurrentlyPlaying("Assets/Sound/jumping/jump_01.wav"))
					{
						audio_player.Play_Sound("Assets/Sound/jumping/jump_01.wav", 0.3f, false);
					}
				}
			}
		}

		for (auto& gem : gems_container->children)
		{
			const Collider* collider = gem->actor->Get_Component<Collider>();

			if (Is_Colliding(*player_collider, *collider))
			{
				active_scene->dead_scene_nodes.push_back(gem);
				audio_player.Play_Sound("Assets/Sound/coin_picking.wav", 0.3f, false);
			}
		}

		SceneNode* eagle = level_1->scene_nodes_map["eagle"];
		Transform* eagle_transform = eagle->actor->Get_Component<Transform>();
		Collider* eagle_collider = eagle->actor->Get_Component<Collider>(); 

		float offset = 400.0f;
		float eagle_speed = 100.0f;
		
		if (glm::distance(player_transform->translation, eagle_transform->translation) < offset)
		{
			if (!Is_Colliding(*player_collider, *eagle_collider) && is_lost == false) 
			{
				glm::vec3 dir = glm::normalize(player_transform->translation - eagle_transform->translation);

				eagle_transform->translation += dir * deltatime * eagle_speed;
			}
			else 
			{
				if (!audio_player.sound_engine->isCurrentlyPlaying("Assets/Sound/lose.wav")) 
				{
					audio_player.sound_engine->stopAllSounds(); 
					audio_player.Play_Sound("Assets/Sound/lose.wav", 1.0f, false);
					player_animation->Set_State("hurt");
					r->Add_Force(glm::vec2(0.0f, 1.0f), 6000.0f);
					is_lost = true; 
				}
			}
		}

		// Game over
		SceneNode* game_over = level_1->scene_nodes_map["game_over"];

		Collider* game_over_collider = game_over->actor->Get_Component<Collider>(); 

		if (Is_Colliding(*player_collider, *game_over_collider) && !is_lost) 
		{
			audio_player.sound_engine->stopAllSounds();
			audio_player.Play_Sound("Assets/Sound/lose.wav", 1.0f, false);
			player_animation->Set_State("hurt");
			r->Add_Force(glm::vec2(0.0f, 1.0f), 20000.0f);
			is_lost = true;
		}

		SceneNode* hidden_gameover = level_1->scene_nodes_map["hidden_game_over"]; 

		Collider* h_game_over_c = hidden_gameover->actor->Get_Component<Collider>(); 

		if (Is_Colliding(*player_collider, *h_game_over_c))
		{
			audio_player.sound_engine->stopAllSounds();
			Set_Active_Scene("game_over");
			audio_player.Play_Sound("Assets/Sound/game over.mp3", 1.0f, true);
		}

		// winner
		if (Is_Colliding(*player_collider, *finish_line_collider)) 
		{
			audio_player.sound_engine->stopAllSounds(); 
			Set_Active_Scene("winner"); 
			audio_player.Play_Sound("Assets/Sound/Well Done.ogg", 1.0f, false);
		}
	}

	if (active_scene->tag == "game_over")
	{
		SceneNode* exit = active_scene->scene_nodes_map["exit_button"];

		Transform* exit_transform = exit->actor->Get_Component<Transform>();
		Render* exit_render = exit->actor->Get_Component<Render>();

		int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		if (state == GLFW_PRESS)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			float exit_y_offset = 10.0f;
			float exit_width = (exit_render->quad.width * exit_transform->scale_x) + exit_transform->translation.x;
			
			// from view space to screen space
			float exit_screen_space = (WINDOW_HEIGHT - exit_transform->translation.y) - exit_y_offset;
			float exit_height = exit_screen_space - (exit_render->quad.height * exit_transform->scale_y) + exit_y_offset;

			bool exit_check_x_axis = (xpos >= exit_transform->translation.x && xpos <= exit_width);
			bool exit_check_y_axis = (ypos <= exit_screen_space && ypos >= exit_height);

			if (exit_check_x_axis && exit_check_y_axis)
			{
				is_game_running = false;
			}
		}
	}

	if (active_scene->tag == "winner") 
	{
		SceneNode* exit = active_scene->scene_nodes_map["exit_button"];

		Transform* exit_transform = exit->actor->Get_Component<Transform>();
		Render* exit_render = exit->actor->Get_Component<Render>();

		int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		if (state == GLFW_PRESS)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			float exit_y_offset = 10.0f;
			float exit_width = (exit_render->quad.width * exit_transform->scale_x) + exit_transform->translation.x;

			// from view space to screen space
			float exit_screen_space = (WINDOW_HEIGHT - exit_transform->translation.y) - exit_y_offset;
			float exit_height = exit_screen_space - (exit_render->quad.height * exit_transform->scale_y) + exit_y_offset;

			bool exit_check_x_axis = (xpos >= exit_transform->translation.x && xpos <= exit_width);
			bool exit_check_y_axis = (ypos <= exit_screen_space && ypos >= exit_height);

			if (exit_check_x_axis && exit_check_y_axis)
			{
				is_game_running = false;
			}
		}
	}

	// clear scene
	while (!active_scene->dead_scene_nodes.empty())
	{
		Remove_Scene_Node(active_scene, active_scene->dead_scene_nodes.back());
		active_scene->dead_scene_nodes.pop_back();
	}
}

void Game::Generate_Output()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& renderable_cmp : active_scene->renderable_cmps)
	{
		renderer.Draw(&(renderable_cmp.first), &(renderable_cmp.second));
	}

	glfwSwapBuffers(window);
}

void Game::Load_Data()
{
	// main menu scene
	Scene* main_menu = Create_Scene("main_menu");
	
	// background
	SceneNode* background = Create_Scene_Node(main_menu, "background");
	Add_Component(background, ComponentType::TransformRender); 
	
	Transform* background_transform = background->actor->Get_Component<Transform>();
	background_transform->scale_x = 4.0f;
	background_transform->scale_y = 4.0f;
	
	Render* background_render = background->actor->Get_Component<Render>();
	background_render->texture = Load_Texture(main_menu, "Assets/BG/BG.png");

	// camera
	SceneNode* main_menu_camera = Create_Scene_Node(main_menu, "main_menu_camera");

	Add_Component(main_menu_camera, ComponentType::TransformRender);
	Add_Component(main_menu_camera, ComponentType::Camera);

	Transform* mm_camera_trasnform = main_menu_camera->actor->Get_Component<Transform>();
	mm_camera_trasnform->translation.x = 0.0f;
	mm_camera_trasnform->translation.y = 0.0f;
	mm_camera_trasnform->scale_x = 0.0f;
	mm_camera_trasnform->scale_y = 0.0f;

	// start
	SceneNode* start_button = Create_Scene_Node(main_menu, "start_button"); 

	Add_Component(start_button, ComponentType::TransformRender);

	Transform* start_transform = start_button->actor->Get_Component<Transform>(); 
	start_transform->translation = glm::vec3(530.0f, 300.0f, 0.f);
	start_transform->scale_x = 0.5f; 
	start_transform->scale_y = 0.7f; 

	Render* start_render = start_button->actor->Get_Component<Render>(); 
	start_render->texture = Load_Texture(main_menu, "Assets/UI/start.png");

	// exit
	SceneNode* exit_button = Create_Scene_Node(main_menu, "exit_button"); 

	Add_Component(exit_button, ComponentType::TransformRender);

	Transform* exit_transform = exit_button->actor->Get_Component<Transform>();
	exit_transform->translation = glm::vec3(553.0f, 250.0f, 0.f);
	exit_transform->scale_x = 0.35f;
	exit_transform->scale_y = 0.4f;

	Render* exit_render = exit_button->actor->Get_Component<Render>();
	exit_render->texture = Load_Texture(main_menu, "Assets/UI/exit.png");

	// level 1
	Scene* level_1 = Create_Scene("level_1"); 
	background = Create_Scene_Node(level_1, "background");

	Add_Component(background, ComponentType::TransformRender);

	background_transform = background->actor->Get_Component<Transform>();
	background_transform->translation = glm::vec3(0.0f, 0.0f, -9.0f);
	background_transform->scale_x = 4.0f;
	background_transform->scale_y = 4.0f;

	background_render = background->actor->Get_Component<Render>();
	background_render->texture = Load_Texture(level_1, "Assets/BG/BG.png");

	// tree
	SceneNode* tree = Create_Scene_Node(level_1, "tree_1");
	Add_Component(tree, ComponentType::TransformRender);

	Transform* tree_transform = tree->actor->Get_Component<Transform>();
	tree_transform->translation = glm::vec3(-380.0f, 230.0f, 0.0);
	tree_transform->scale_x = 0.5f;

	Render* tree_render = tree->actor->Get_Component<Render>();

	tree_render->texture = Load_Texture(level_1, "Assets/Tree_2.png");
	tree = Create_Scene_Node(level_1, "tree_2");
	Add_Component(tree, ComponentType::TransformRender);

	tree_transform = tree->actor->Get_Component<Transform>();
	tree_transform->translation = glm::vec3(-60.0f, 230.0f, 0.0);
	tree_transform->scale_x = 0.5f;

	tree_render = tree->actor->Get_Component<Render>();

	tree_render->texture = Load_Texture(level_1, "Assets/Tree_3.png");

	tree = Create_Scene_Node(level_1, "tree_3");
	Add_Component(tree, ComponentType::TransformRender);

	tree_transform = tree->actor->Get_Component<Transform>();
	tree_transform->translation = glm::vec3(1100.0f, 230.0f, 0.0);
	tree_transform->scale_x = 0.5f;

	tree_render = tree->actor->Get_Component<Render>();

	tree_render->texture = Load_Texture(level_1, "Assets/Tree_2.png");
	tree = Create_Scene_Node(level_1, "tree_4");
	Add_Component(tree, ComponentType::TransformRender);

	tree_transform = tree->actor->Get_Component<Transform>();
	tree_transform->translation = glm::vec3(1480.0f, 230.0f, 0.0);
	tree_transform->scale_x = 0.5f;

	tree_render = tree->actor->Get_Component<Render>();

	tree_render->texture = Load_Texture(level_1, "Assets/Tree_3.png");

	// Bush
	SceneNode* bush = Create_Scene_Node(level_1, "bush");

	Add_Component(bush, ComponentType::TransformRender);

	Transform* bush_transform = bush->actor->Get_Component<Transform>();
	bush_transform->translation = glm::vec3(-180.0f, 230.0f, 0.0f);
	bush_transform->scale_x = 0.2f;
	bush_transform->scale_y = 0.2f;

	Render* bush_render = bush->actor->Get_Component<Render>();
	bush_render->texture = Load_Texture(level_1, "Assets/Bush (3).png");

	// ground
	SceneNode* ground_container = Create_Scene_Node(level_1, "ground_container");

	SceneNode* ground = Create_Scene_Node(level_1, "ground_1");
	Add_Component(ground, ComponentType::TransformRender);
	Add_Component(ground, ComponentType::Collider);

	Transform* ground_transform = ground->actor->Get_Component<Transform>();
	ground_transform->translation = glm::vec3(-410.0f, 0.0f, 0.0f);
	ground_transform->scale_y = 1.3f;
	ground_transform->scale_x = 1.6f;

	Render* ground_render = ground->actor->Get_Component<Render>();
	ground_render->texture = Load_Texture(level_1, "Assets/ground.png");

	Collider* ground_collider = ground->actor->Get_Component<Collider>();

	ground_collider->aabb.min_x_offset = 10.0f;
	ground_collider->aabb.min_y_offset = ground_render->quad.height * ground_transform->scale_y;

	ground_container->Add_Child(ground);


	ground = Create_Scene_Node(level_1, "ground_2");
	Add_Component(ground, ComponentType::TransformRender);
	Add_Component(ground, ComponentType::Collider);

	ground_transform = ground->actor->Get_Component<Transform>();
	ground_transform->translation = glm::vec3(1100.0f, 0.0f, 0.0f);
	ground_transform->scale_y = 1.3f;
	ground_transform->scale_x = 1.6f;

	ground_render = ground->actor->Get_Component<Render>();
	ground_render->texture = Load_Texture(level_1, "Assets/ground.png");

	ground_collider = ground->actor->Get_Component<Collider>();

	ground_collider->aabb.min_x_offset = 18.0f;
	ground_collider->aabb.min_y_offset = ground_render->quad.height * ground_transform->scale_y;

	ground_container->Add_Child(ground);

	// sign
	SceneNode* sign = Create_Scene_Node(level_1, "sign");

	Add_Component(sign, ComponentType::TransformRender);

	Transform* sign_transform = sign->actor->Get_Component<Transform>();
	sign_transform->scale_x = 0.18f;
	sign_transform->scale_y = 0.28f;
	sign_transform->translation = glm::vec3(1380.0f, 233.0f, 0.0f);

	Render* sign_render = sign->actor->Get_Component<Render>();
	sign_render->texture = Load_Texture(level_1, "Assets/Sign_2.png");

	// mushrooms
	SceneNode* mushroom = Create_Scene_Node(level_1, "mushroom_0");

	Add_Component(mushroom, ComponentType::TransformRender);

	Transform* mushroom_transform = mushroom->actor->Get_Component<Transform>();
	mushroom_transform->scale_x = 0.1f;
	mushroom_transform->scale_y = 0.15f;
	mushroom_transform->translation = glm::vec3(1250.0f, 233.0f, 0.0f);


	Render* mushroom_render = mushroom->actor->Get_Component<Render>();
	mushroom_render->texture = Load_Texture(level_1, "Assets/Mushroom_1.png");

	mushroom = Create_Scene_Node(level_1, "mushroom_1");

	Add_Component(mushroom, ComponentType::TransformRender);

	mushroom_transform = mushroom->actor->Get_Component<Transform>();
	mushroom_transform->scale_x = 0.1f;
	mushroom_transform->scale_y = 0.15f;
	mushroom_transform->translation = glm::vec3(1450.0f, 233.0f, 0.0f);


	mushroom_render = mushroom->actor->Get_Component<Render>();
	mushroom_render->texture = Load_Texture(level_1, "Assets/Mushroom_2.png");

	// floating tiles
	SceneNode* floating_tiles_container = Create_Scene_Node(level_1, "floating_tiles_container");
	SceneNode* gems_container = Create_Scene_Node(level_1, "gems_container");

	glm::vec3 transflation = glm::vec3(200.0f, 210.0f, 0.0f);
	float scale_x = 0.4f;
	float scale_y = 0.15f;
	float x_offset = 250.0f;

	for (uint16_t i = 0; i < 3; ++i)
	{
		SceneNode* floating_tile = Create_Scene_Node(level_1, "floating_tile_" + std::to_string(i));
		Add_Component(floating_tile, ComponentType::TransformRender);
		Add_Component(floating_tile, ComponentType::Collider);

		SceneNode* gem = Create_Scene_Node(level_1, "gem_" + std::to_string(i));
		Add_Component(gem, ComponentType::TransformRender);
		Add_Component(gem, ComponentType::Animation);
		Add_Component(gem, ComponentType::Collider);

		Transform* floating_tile_transform = floating_tile->actor->Get_Component<Transform>();
		floating_tile_transform->translation = glm::vec3(230.0f + (float)(x_offset * i), 210.0f, 0.0f);
		floating_tile_transform->scale_x = scale_x;
		floating_tile_transform->scale_y = scale_y;

		Render* floating_tile_render = floating_tile->actor->Get_Component<Render>();
		floating_tile_render->texture = Load_Texture(level_1, "Assets/tile.png");

		Collider* collider = floating_tile->actor->Get_Component<Collider>();
		collider->aabb.min_x_offset = 17.0f;
		collider->aabb.max_x_offset = -6.0f;

		Transform* gem_transform = gem->actor->Get_Component<Transform>();
		gem_transform->translation = floating_tile_transform->translation + glm::vec3(50.0f, 30.0f, 0.0f);
		gem_transform->scale_x = 0.07f;
		gem_transform->scale_y = 0.08f;

		Animation* gem_animation = gem->actor->Get_Component<Animation>();

		State* default_state = gem_animation->Create_State("default");
		default_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/gem/gem-1.png"));
		default_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/gem/gem-2.png"));
		default_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/gem/gem-3.png"));
		default_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/gem/gem-4.png"));
		default_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/gem/gem-5.png"));

		default_state->speed = 5.0f;

		gem_animation->Set_State("default");

		floating_tiles_container->Add_Child(floating_tile);
		gems_container->Add_Child(gem);
	}

	// Eagle
	SceneNode* eagle = Create_Scene_Node(level_1, "eagle"); 

	Add_Component(eagle, ComponentType::TransformRender);
	Add_Component(eagle, ComponentType::Collider); 
	Add_Component(eagle, ComponentType::Animation); 

	Transform* eagle_transform = eagle->actor->Get_Component<Transform>();
	eagle_transform->translation = glm::vec3(1550.0f, 233.0f, 0.0f);
	eagle_transform->scale_x = 0.20f;
	eagle_transform->scale_y = 0.30f;

	Animation* eagle_animation = eagle->actor->Get_Component<Animation>(); 
	State* attack = eagle_animation->Create_State("attack"); 
	attack->frames.push_back(Load_Texture(level_1, "Assets/Animation/eagle/eagle-attack-1.png"));
	attack->frames.push_back(Load_Texture(level_1, "Assets/Animation/eagle/eagle-attack-2.png"));
	attack->frames.push_back(Load_Texture(level_1, "Assets/Animation/eagle/eagle-attack-3.png"));
	attack->frames.push_back(Load_Texture(level_1, "Assets/Animation/eagle/eagle-attack-4.png"));
	attack->speed = 5.0f; 

	eagle_animation->Set_State("attack");

	Collider* eagle_collider = eagle->actor->Get_Component<Collider>(); 
	eagle_collider->aabb.min_x_offset = 15.0f;
	eagle_collider->aabb.max_y_offset = -15.0f;

	// player
	SceneNode* player = Create_Scene_Node(level_1, "player");
	Add_Component(player, ComponentType::TransformRender);
	Add_Component(player, ComponentType::Rigidbody);
	Add_Component(player, ComponentType::Collider);
	Add_Component(player, ComponentType::Animation);


	Animation* player_animation = player->actor->Get_Component<Animation>();
	Transform* player_transform = player->actor->Get_Component<Transform>();
	Render* player_render = player->actor->Get_Component<Render>();
	Collider* player_collider = player->actor->Get_Component<Collider>();
	Rigidbody* r = player->actor->Get_Component<Rigidbody>();

	State* idle_state = player_animation->Create_State("idle");
	idle_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/idle/player-idle-1.png"));
	idle_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/idle/player-idle-2.png"));
	idle_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/idle/player-idle-3.png"));
	idle_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/idle/player-idle-4.png"));
	idle_state->speed = 5.0f;

	State* jump_state = player_animation->Create_State("jump");
	jump_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/jump/player-jump-1.png"));
	jump_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/jump/player-jump-2.png"));
	jump_state->loop = false;
	jump_state->speed = 5.0f;

	State* running = player_animation->Create_State("running");
	running->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/running/player-run-1.png"));
	running->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/running/player-run-2.png"));
	running->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/running/player-run-3.png"));
	running->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/running/player-run-4.png"));
	running->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/running/player-run-5.png"));
	running->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/running/player-run-6.png"));
	running->speed = 5.0f;

	State* hurt_state = player_animation->Create_State("hurt"); 
	hurt_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/hurt/player-hurt-1.png"));
	hurt_state->frames.push_back(Load_Texture(level_1, "Assets/Animation/player/hurt/player-hurt-2.png"));
	hurt_state->speed = 5.0f; 
	hurt_state->loop = false;

	player_animation->Set_State("idle");

	player_transform->scale_x = 0.15f;
	player_transform->scale_y = 0.25f;
	player_transform->translation = glm::vec3(0.0f, 300.0f, 0.0f);

	player_collider->aabb.min_y_offset = 2.0f;
	player_collider->aabb.min_x_offset = 10.0f;

	// wall 1
	SceneNode* wall_1 = Create_Scene_Node(level_1, "wall1");
	Add_Component(wall_1, ComponentType::TransformRender);
	Add_Component(wall_1, ComponentType::Collider);

	float wall_offset = 500.0f;
	Transform* wall_transform = wall_1->actor->Get_Component<Transform>();
	wall_transform->translation = glm::vec3(player_transform->translation.x - wall_offset, 0.0f, 0.0f);
	wall_transform->scale_y = 5.0f;
	wall_transform->scale_x = 0.0f;

	// wall 2
	SceneNode* wall_2 = Create_Scene_Node(level_1, "wall2");
	Add_Component(wall_2, ComponentType::TransformRender);
	Add_Component(wall_2, ComponentType::Collider);

	float wall2_offset = 40.0f;
	Transform* wall2_transform = wall_2->actor->Get_Component<Transform>();
	wall2_transform->translation = glm::vec3(player_transform->translation.x + (player_transform->scale_x * 320.0f), -10.0f, 0.0f);
	wall2_transform->scale_y = 5.0f;
	wall2_transform->scale_x = 0.0f;

	// Game over Collider
	SceneNode* game_over = Create_Scene_Node(level_1, "game_over"); 

	Add_Component(game_over, ComponentType::TransformRender);
	Add_Component(game_over, ComponentType::Collider);

	Transform* game_over_transform = game_over->actor->Get_Component<Transform>(); 
	game_over_transform->scale_y = 0.0f;
	game_over_transform->scale_x = 10.0f;

	Render* ren = game_over->actor->Get_Component<Render>(); 

	ren->texture = Load_Texture(level_1, "Assets/default.png");

	// hidden game over collider
	game_over = Create_Scene_Node(level_1, "hidden_game_over");

	Add_Component(game_over, ComponentType::TransformRender);
	Add_Component(game_over, ComponentType::Collider);

	game_over_transform = game_over->actor->Get_Component<Transform>();
	game_over_transform->translation = glm::vec3(0.0f, -100.0f, 0.0f);
	game_over_transform->scale_y = 0.1f;
	game_over_transform->scale_x = 10.0f;

	ren = game_over->actor->Get_Component<Render>();

	ren->texture = Load_Texture(level_1, "Assets/default.png");


	// camera
	SceneNode* level_1_camera = Create_Scene_Node(level_1, "level_1_camera");
	Add_Component(level_1_camera, ComponentType::TransformRender);
	Add_Component(level_1_camera, ComponentType::Collider);
	Add_Component(level_1_camera, ComponentType::Camera);

	float camera_offset = 500.0f;
	Transform* camera_trasnform = level_1_camera->actor->Get_Component<Transform>();
	camera_trasnform->translation.x = player_transform->translation.x - camera_offset;
	camera_trasnform->translation.y *= 0.0f;
	camera_trasnform->scale_x = 0.0f;
	camera_trasnform->scale_y = 0.0f;

	// finish line
	SceneNode* finish_line = Create_Scene_Node(level_1, "finish_line"); 
	Add_Component(finish_line, ComponentType::TransformRender);
	Add_Component(finish_line, ComponentType::Collider);

	Transform* finishline_transform = finish_line->actor->Get_Component<Transform>(); 
	finishline_transform->translation = glm::vec3(1600.0f, 100.0f, 0.0f); 
	finishline_transform->scale_x = 0.0f;
	finishline_transform->scale_y = 5.0f;

	// Game Over
	Scene* game_over_scene = Create_Scene("game_over"); 

	// background
	SceneNode* game_over_background = Create_Scene_Node(game_over_scene, "background");
	Add_Component(game_over_background, ComponentType::TransformRender);

	Transform* go_background_transform = game_over_background->actor->Get_Component<Transform>();
	go_background_transform->scale_x = 4.0f;
	go_background_transform->scale_y = 4.0f;

	Render* go_background_render = game_over_background->actor->Get_Component<Render>();
	go_background_render->texture = Load_Texture(game_over_scene, "Assets/BG/BG.png");

	// Game over 
	SceneNode* game_over_text = Create_Scene_Node(game_over_scene, "game_over_text"); 

	Add_Component(game_over_text, ComponentType::TransformRender);

	Transform* gm_text_trans = game_over_text->actor->Get_Component<Transform>(); 
	gm_text_trans->translation = glm::vec3(465.0f, 300.0f, 0.0f);

	Render* gm_text_render = game_over_text->actor->Get_Component<Render>();
	gm_text_render->texture = Load_Texture(game_over_scene, "Assets/UI/game_over_.png");

	// camera
	SceneNode* game_over_camera = Create_Scene_Node(game_over_scene, "camera");

	Add_Component(game_over_camera, ComponentType::TransformRender);
	Add_Component(game_over_camera, ComponentType::Camera);

	Transform* go_camera_trasnform = game_over_camera->actor->Get_Component<Transform>();
	go_camera_trasnform->translation.x = 0.0f;
	go_camera_trasnform->translation.y = 0.0f;
	go_camera_trasnform->scale_x = 0.0f;
	go_camera_trasnform->scale_y = 0.0f;

	// exit
	exit_button = Create_Scene_Node(game_over_scene, "exit_button");

	Add_Component(exit_button, ComponentType::TransformRender);

	exit_transform = exit_button->actor->Get_Component<Transform>();
	exit_transform->translation = glm::vec3(553.0f, 200.0f, 0.f);
	exit_transform->scale_x = 0.35f;
	exit_transform->scale_y = 0.4f;

	exit_render = exit_button->actor->Get_Component<Render>();
	exit_render->texture = Load_Texture(game_over_scene, "Assets/UI/exit.png");

	// Winner
	Scene* winner = Create_Scene("winner"); 

	// background
	background = Create_Scene_Node(winner, "background");
	Add_Component(background, ComponentType::TransformRender);

	background_transform = background->actor->Get_Component<Transform>();
	background_transform->scale_x = 4.0f;
	background_transform->scale_y = 4.0f;

	background_render = background->actor->Get_Component<Render>();
	background_render->texture = Load_Texture(winner, "Assets/BG/BG.png");

	// camera
	SceneNode* camera = Create_Scene_Node(winner, "main_menu_camera");

	Add_Component(camera, ComponentType::TransformRender);
	Add_Component(camera, ComponentType::Camera);

	camera_trasnform = camera->actor->Get_Component<Transform>();
	camera_trasnform->translation.x = 0.0f;
	camera_trasnform->translation.y = 0.0f;
	camera_trasnform->scale_x = 0.0f;
	camera_trasnform->scale_y = 0.0f;

	// trophy
	SceneNode* trophy = Create_Scene_Node(winner, "trophy");
	Add_Component(trophy, ComponentType::TransformRender); 

	Transform* trophy_transform = trophy->actor->Get_Component<Transform>();
	trophy_transform->scale_x = 0.8f; 
	trophy_transform->translation = glm::vec3(480.0f, 300.0f, 0.0f);

	Render* trophy_render = trophy->actor->Get_Component<Render>();

	trophy_render->texture = Load_Texture(winner, "Assets/UI/trophy.png");

	// exit
	exit_button = Create_Scene_Node(winner, "exit_button");

	Add_Component(exit_button, ComponentType::TransformRender);

	exit_transform = exit_button->actor->Get_Component<Transform>();
	exit_transform->translation = glm::vec3(553.0f, 200.0f, 0.f);
	exit_transform->scale_x = 0.35f;
	exit_transform->scale_y = 0.4f;

	exit_render = exit_button->actor->Get_Component<Render>();
	exit_render->texture = Load_Texture(winner, "Assets/UI/exit.png");

	Set_Active_Scene("main_menu");
	audio_player.Play_Sound("Assets/Sound/main menu music.mp3", 1.0f, true);
}

void Game::Load_Shaders()
{
	Shader basic;
	basic.Create_Shader_Program("Shaders/basic.vert", "Shaders/basic.frag");
	shaders_table["basic"] = std::move(basic);

	Shader background;
	basic.Create_Shader_Program("Shaders/background.vert", "Shaders/background.frag");
	shaders_table["background"] = std::move(basic);
}

Scene* Game::Create_Scene(const std::string&& scene_tag)
{
	scenes.push_back(Scene());
	scenes.back().tag = scene_tag;
	scenes.back().Init(this);
	scenes_map[scene_tag] = &scenes.back();
	return &scenes.back(); 
}

void Game::Set_Active_Scene(const std::string&& scene_tag)
{
	active_scene = scenes_map[scene_tag];
}


Texture* Game::Load_Texture(Scene* scene, std::string&& texture_path)
{
	for (auto& texture : scene->textures)
	{
		if (texture.first == texture_path) 
		{
			texture.second.counter++;
			return &texture.second;
		}
	}

	Texture texture;
	if (texture.Init(texture_path))
	{
		scene->textures.push_back({texture_path, std::move(texture)});
		return &(scene->textures.back().second);
	}

	return nullptr;
}

void Game::Remove_Texture(Scene* scene, Texture* texture)
{

	if (texture == nullptr) 
	{
		return;
	}

	if (texture->counter > 1)
	{
		texture->counter -= 1;
		return;
	}

	texture->counter = 0;

	std::pair<std::string, Texture>* target = nullptr; 
	for (auto& pair : scene->textures) 
	{
		if (&pair.second == texture) 
		{
			target = &pair;
		}
	}

	if (target == nullptr) 
	{
		return;
	}

	// find which render component has a pointer to the back texture
	// make it point to the correct slot
	Texture* back_texture = &scene->textures.back().second; 

	for (auto& pair : active_scene->renderable_cmps) 
	{
		if (pair.second.texture == back_texture) 
		{
			pair.second.texture = texture;
			break; 
		}
	}

	// do the same for animation components
	// it is not as bad as it looks
	for (auto& animation : active_scene->animation_cmps)
	{
		for (auto& state : animation.states) 
		{
			for (auto& frame : state.frames) 
			{
				if (frame == back_texture) 
				{
					frame = texture;
				}
			}
		}
	}

	std::iter_swap(target, scene->textures.end() - 1);

	scene->textures.back().second.Clear();
	scene->textures.pop_back();
}


SceneNode* Game::Create_Scene_Node(Scene* scene, std::string&& _tag)
{
	scene->scene_nodes.push_back(SceneNode(scene));
	scene->scene_nodes.back().tag = _tag;
	scene->scene_nodes.back().children.reserve(MAX_CHILDREN);
	scene->actors.emplace_back(Actor(&scene->scene_nodes.back()));
	scene->scene_nodes.back().actor = &(scene->actors.back());
	scene->scene_nodes_map[_tag] = &scene->scene_nodes.back();
	return &scene->scene_nodes.back();
}

void Game::Remove_Scene_Node(Scene* scene, SceneNode* scene_node)
{

	for (auto &child : scene_node->children)
	{
		Remove_Scene_Node(scene, child);
	}

	// Here to delete a scene node we swap the node with the back of the vector
	// to avoid shifting when using erase
	// we still need to fix the children of the back as they will be pointing
	// to different parent after swapping
	SceneNode* back_scene_node = &scene->scene_nodes.back();
	scene->scene_nodes_map[back_scene_node->tag] = scene_node;
	scene->scene_nodes_map[scene_node->tag] = nullptr;


	if (back_scene_node->parent) 
	{
		for (auto& child : back_scene_node->parent->children) 
		{
			if (child == back_scene_node) 
			{
				child = scene_node;
			}
		}
	}

	if (scene_node->parent)
	{
		scene_node->parent->Remove_Child(scene_node);
	}

	for (auto& actor : scene->actors) 
	{
		if (actor.scene_node == back_scene_node) 
		{
			actor.scene_node = scene_node;
		}
	}

	for (auto& child : back_scene_node->children)
	{
		child->parent = scene_node;
	}
	
	Remove_Actor(scene, scene_node->actor);
	
	std::iter_swap(scene_node, scene->scene_nodes.end() - 1);

	scene->scene_nodes.pop_back();
}

void Game::Remove_Actor(Scene* scene, Actor* actor)
{
	auto iter = std::find(scene->actors.begin(), scene->actors.end(), *actor);

	if (iter != scene->actors.end()) 
	{
		for (auto& cmp : actor->components) 
		{
			cmp->Clear();
		}

		for (auto& cmp : actor->components) 
		{
			Remove_Component(scene, cmp);
		}

		for (auto& scene_node : scene->scene_nodes) 
		{
			if (scene_node.actor == &scene->actors.back()) 
			{
				scene_node.actor = actor;
			}
		}

		for (auto& cmp : scene->actors.back().components) 
		{
			cmp->owner = actor;
		}

		std::iter_swap(actor, scene->actors.end() - 1);
		scene->actors.pop_back();
	}
}

void Game::Add_Component(SceneNode* scene_node, ComponentType type)
{
	switch (type)
	{
	case ComponentType::TransformRender:
	{
		scene_node->scene->renderable_cmps.push_back({ Transform(scene_node->actor), Render(scene_node->actor) });
		scene_node->actor->components.push_back(&(scene_node->scene->renderable_cmps.back().first));
		scene_node->actor->components.push_back(&(scene_node->scene->renderable_cmps.back().second));
	}
	break;

	case ComponentType::Rigidbody: 
	{
		scene_node->scene->rigidbody_cmps.push_back(Rigidbody(scene_node->actor));
		scene_node->actor->components.push_back(&(scene_node->scene->rigidbody_cmps.back()));
	}
	break;

	case ComponentType::Collider: 
	{
		scene_node->scene->collider_cmps.push_back(Collider(scene_node->actor));
		scene_node->actor->components.push_back(&(scene_node->scene->collider_cmps.back()));
	}
	break;

	case ComponentType::Camera: 
	{
		scene_node->scene->camera_cmps.push_back(Camera(scene_node->actor));
		scene_node->actor->components.push_back(&(scene_node->scene->camera_cmps.back()));
	}
	break;

	case ComponentType::Animation: 
	{
		scene_node->scene->animation_cmps.push_back(Animation(scene_node->actor));
		scene_node->actor->components.push_back(&(scene_node->scene->animation_cmps.back()));
	}
	break;

	default:
		break;
	}
}

void Game::Remove_Component(Scene* scene, Component* cmp)
{
	Transform* transform = dynamic_cast<Transform*>(cmp);

	if (transform != nullptr) 
	{
		Transform* back_transform = &(scene->renderable_cmps.back().first);
		Render* back_render = &(scene->renderable_cmps.back().second);

		Actor* owner = back_transform->owner;

		for (auto& ptr : owner->components)
		{
			if (ptr == back_transform) 
			{
				ptr = transform; 
			}

			if (ptr == back_render)
			{
				ptr = (Render*)(transform + 1);
			}
		}

		std::iter_swap((std::pair<Transform, Render>*)cmp, scene->renderable_cmps.end() - 1);
		scene->renderable_cmps.pop_back();

		return;
	}

	Rigidbody* r = dynamic_cast<Rigidbody*>(cmp);

	if (r != nullptr) 
	{
		Rigidbody* r_back = &scene->rigidbody_cmps.back();

		Actor* owner = r_back->owner;

		for (auto& ptr : owner->components) 
		{
			if (ptr == r_back) 
			{
				ptr = r;
			}
		}

		std::iter_swap(r, scene->rigidbody_cmps.end() - 1);
		scene->rigidbody_cmps.pop_back();

		return;
	}

	Collider* c = dynamic_cast<Collider*>(cmp);

	if (c != nullptr) 
	{
		Collider* c_back = &scene->collider_cmps.back();

		Actor* owner = c_back->owner;

		for (auto& ptr : owner->components) 
		{
			if (ptr == c_back) 
			{
				ptr = c;
			}
		}

		std::iter_swap(c, scene->collider_cmps.end() - 1);
		scene->collider_cmps.pop_back();

		return;
	}

	Camera* cam = dynamic_cast<Camera*>(cmp);

	if (cam != nullptr) 
	{
		Camera* cam_back = &scene->camera_cmps.back();

		Actor* owner = cam_back->owner;

		for (auto& ptr : owner->components) 
		{
			if (ptr == cam_back) 
			{
				ptr = cam;
			}
		}

		std::iter_swap(cam, scene->camera_cmps.end() - 1);
		scene->camera_cmps.pop_back();

		return;
	}

	Animation* anim = dynamic_cast<Animation*>(cmp);

	if (anim != nullptr)
	{
		Animation* anim_back = &scene->animation_cmps.back();

		Actor* owner = anim_back->owner;

		for (auto& ptr : owner->components) 
		{
			if (ptr == anim_back)
			{
				ptr = anim;
			}
		}

		std::iter_swap(anim, scene->animation_cmps.end() - 1);
		scene->animation_cmps.pop_back();

		return;
	}
}

float Game::Get_Deltatime()
{
	float delta_time = glfwGetTime() - time_since_last_frame;
	time_since_last_frame = glfwGetTime();
	return delta_time;
}

Game::~Game()
{
}

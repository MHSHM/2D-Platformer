#include <vector>
#include <string>
#include <unordered_map>

#include <SceneNode.h>
#include <Actor.h>
#include <Transform.h>
#include <Render.h>
#include <Input.h>
#include <Rigidbody.h>
#include <Collider.h>
#include <Camera.h>
#include <Animation.h>
#include <Audio.h>

#define MAX_SCENE_NODES 100
#define MAX_ACTOR 100
#define MAX_TRANSFORM 100
#define MAX_RENDER 100
#define MAX_INPUT 100
#define MAX_Rigidbody 100
#define MAX_COLLIDER 100
#define MAX_CAMERA 100
#define MAX_TEXTURES 200
#define MAX_ANIMATION 100


class Scene 
{
public:
	Scene();

	void Init(class Game* _game);
	void Update_Components(float deltatime);
	void Clear(); 

public:

	std::string tag; 

	std::unordered_map<std::string, class SceneNode*> scene_nodes_map;
	std::vector<SceneNode> scene_nodes;
	std::vector<SceneNode*> dead_scene_nodes;

	std::vector<std::pair<std::string, Texture> > textures;

	std::vector<Actor> actors;

	std::vector<std::pair<Transform, Render> > renderable_cmps;
	std::vector<Input> input_cmps;
	std::vector<Rigidbody> rigidbody_cmps;
	std::vector<Collider> collider_cmps;
	std::vector<Camera> camera_cmps;
	std::vector<Animation> animation_cmps;

	class Game* game;
};
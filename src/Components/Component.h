#pragma once

enum class ComponentType
{
	TransformRender = 0,
	Rigidbody = 1,
	Collider = 2,
	Camera = 3,
	Animation = 4
};

class Component
{
public:

	Component(class Actor* _owner);

	virtual void Update(float deltatime);
	virtual void Clear(); 

public:
	class Actor* owner;

	virtual ~Component();

};
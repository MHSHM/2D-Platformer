#pragma once

#include <VertexArrayObject.h>

class Quad
{
public:
	void Init();
	Quad(); 

public:
	VertexArrayObject vao;
	float width; 
	float height; 

	glm::vec3 color; 

};
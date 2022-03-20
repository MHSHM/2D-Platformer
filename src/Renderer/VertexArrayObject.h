#pragma once

#include <vector>

#include "../Dependencies/GLM/vec2.hpp"
#include "../Dependencies/GLM/vec3.hpp"

struct Vertex 
{

	Vertex():
		position(glm::vec3(0.0f, 0.0f, 1.0f)),
		uv_coord(glm::vec2(0.0f, 0.0f)) {}

	glm::vec3 position; 
	glm::vec2 uv_coord;
};

enum class VertexLayout
{
	Position = 0,
	PositionUv = 1,
};

class VertexArrayObject
{
public:

	VertexArrayObject() = default;

	void Init(std::vector<struct Vertex>& vertices, std::vector<unsigned int>& indices, const VertexLayout& layout);

	void Bind();
	void Un_Bind();

	unsigned int Get_VAO() { return vertex_array_obj_id; }
	unsigned int Get_Element_Buffer_Size() { return element_buffer_size; }

	void Clear();

private:
	unsigned int vertex_array_obj_id;
	unsigned int array_buffer_id;
	unsigned int element_buffer_id;
	unsigned int element_buffer_size;
};
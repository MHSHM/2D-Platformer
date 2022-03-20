#include "Quad.h"

#include <vector>


Quad::Quad() :
	width(320.0f),
	height(180.0f),
	color(0.0f, 0.0f, 1.0f)
{
	
}

void Quad::Init()
{
	std::vector<Vertex> quad_vertices(4);

	quad_vertices[0].position = { 0.0f, height, 0.0f };
	quad_vertices[0].uv_coord = { 0.0f, 1.0f };

	quad_vertices[1].position = { width, height, 0.0f };
	quad_vertices[1].uv_coord = { 1.0f, 1.0f };

	quad_vertices[2].position = { width, 0.0f, 0.0f };
	quad_vertices[2].uv_coord = { 1.0f, 0.0f };

	quad_vertices[3].position = { 0.0f,  0.0f, 0.0f };
	quad_vertices[3].uv_coord = { 0.0f, 0.0f };

	std::vector<unsigned int> indices = { 0, 1, 3, 1, 2, 3 };

	vao.Init(quad_vertices, indices, VertexLayout::PositionUv);
}
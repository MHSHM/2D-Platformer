#include <Renderer.h>
#include <Game.h>
#include <Scene.h>
#include <glew.h>

Renderer::Renderer(Game* _game) :
	game(_game) 
{
}

void Renderer::Draw(Transform* transform, Render* render)
{

	if (!transform || !render)
	{
		return;
	}

	Camera* camera = &game->active_scene->camera_cmps[0];

	if (camera == nullptr)
	{
		return;
	}

	Shader* shader;

	if (render->owner->scene_node->tag == "background")
	{
		shader = &(game->shaders_table["background"]);
	}
	else
	{
		shader = &(game->shaders_table["basic"]);
	}

	shader->Bind();

	shader->Set_Matrix4_Uniform("in_world", transform->world_transform);
	shader->Set_Matrix4_Uniform("in_orthoproj", camera->ortho_proj);
	shader->Set_Matrix4_Uniform("in_view", camera->view);

	if (render->texture)
	{
		render->texture->Bind(0);
		shader->Set_Int_Uniform("texture_map", 0);

		render->quad.vao.Bind();

		glDrawElements(GL_TRIANGLES, render->quad.vao.Get_Element_Buffer_Size(), GL_UNSIGNED_INT, nullptr);

		render->quad.vao.Un_Bind();
	}

	shader->Un_Bind();

}

Renderer::~Renderer() 
{
}
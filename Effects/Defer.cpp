#include "Defer.h"

#include <assimploader.h>
#include <Bindables/shaderprogram.h>

namespace RTREffects
{
	DeferRenderer::DeferRenderer(const std::string& name)
		:Utils(name)
	{
		globalSettings::mainCamera.set_perspective(60.0f, globalSettings::screen_width, globalSettings::screen_height, 0.1f, 512.0f);
		globalSettings::mainCamera.set_position(glm::vec3(0.0f, 0.0f, 4.0f));
	}

	void DeferRenderer::prepare()
	{
		prepare_scene();
		prepare_defer();
		prepare_compute();
	}

	void DeferRenderer::render()
	{

	}

	void DeferRenderer::prepare_ui(const std::string& name)
	{

	}

	void DeferRenderer::prepare_scene()
	{
		m_proxy = AssimpLoader::LoadModel("Sponza", "sponza.obj", m_main_scene);

		std::vector<DrawItems::VertexType> instruction{ DrawItems::VertexType::Position,DrawItems::VertexType::Normal,DrawItems::VertexType::Tangent,
			DrawItems::VertexType::Texcoord };
		m_proxy->AddRootVertexRule(instruction);
		m_proxy->AddRootTextureRule("diffuse_tex", 0, SceneGraph::Material::TextureCategory::DIFFUSE).
			AddRootTextureRule("specular_tex", 1, SceneGraph::Material::TextureCategory::SPECULAR).
			AddRootTextureRule("normal_tex", 2, SceneGraph::Material::TextureCategory::NORMAL);
		m_proxy->ScaleModel(glm::vec3(0.2f));
	}
}
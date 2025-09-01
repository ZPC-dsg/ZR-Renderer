#include "OIT.h"

#include <SceneGraph/modelproxy.h>
#include <SceneGraph/drawableproxy.h>
#include <Bindables/imagetexture2D.h>
#include <assimploader.h>
#include <drawableloader.h>
#include <Bindables/shaderprogram.h>

namespace OITEffects
{
	OIT::OIT(const std::string& name)
		:Utils(name)
	{
		globalSettings::mainCamera.set_perspective(60.0f, globalSettings::screen_width, globalSettings::screen_height, 0.1f, 512.0f);
		globalSettings::mainCamera.set_position(glm::vec3(0.0f, 0.0f, 4.0f));
	}

	void OIT::render()
	{
		render_opaque();
		render_OIT();
		composite();
	}

	void OIT::prepare()
	{
		prepare_opaque();
		prepare_transparent();
		prepare_OITdata();
	}

	void OIT::prepare_ui(const std::string& name)
	{
		
	}

	void OIT::prepare_opaque()
	{
		m_proxy = AssimpLoader::LoadModel("Sponza", "sponza.obj", m_main_scene);
		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "vertex", "test", "model_test.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "fragment", "test", "model_test.frag");
		m_opaque_shader = Bind::ShaderProgram::Resolve("shader", std::vector<GLuint>{vertex, fragment});
		m_proxy->AddRootBindable(m_opaque_shader);
		std::vector<DrawItems::VertexType> instruction{ DrawItems::VertexType::Position,DrawItems::VertexType::Normal,DrawItems::VertexType::Texcoord };
		m_proxy->AddRootVertexRule(instruction);
		m_proxy->AddRootTextureRule("texture_diffuse1", 0, SceneGraph::Material::TextureCategory::DIFFUSE);
		m_proxy->AddRootTextureRule("texture_specular1", 1, SceneGraph::Material::TextureCategory::SPECULAR);
		m_proxy->AddRootUniformRule<SceneGraph::ConfigurationType::Transformation>(m_opaque_shader->EditUniform("model").GetLeafUniform(),
			[](glm::mat4 model)->glm::mat4 {return model; });
		m_proxy->Cook();
		m_proxy->ScaleModel(glm::vec3(0.02f));
	}

	void OIT::prepare_transparent()
	{

	}

	void OIT::prepare_OITdata()
	{

	}

	void OIT::render_opaque()
	{

	}

	void OIT::render_OIT()
	{

	}

	void OIT::composite()
	{

	}
}
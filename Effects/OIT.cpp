#include "OIT.h"

#include <SceneGraph/modelproxy.h>
#include <SceneGraph/drawableproxy.h>
#include <Bindables/rendertarget.h>
#include <assimploader.h>
#include <drawableloader.h>
#include <Bindables/shaderprogram.h>
#include <Common/random_generator.h>

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
		ImGui::Begin(name.c_str());

		ImGui::SliderInt("sample count", &m_sample_count, 1, 16);
		m_sample_side_count = std::sqrt(m_sample_count);
		m_sample_count = std::pow(m_sample_side_count, 2);
		ImGui::End();
	}

	void OIT::prepare_opaque()
	{
		m_proxy = AssimpLoader::LoadModel("Sponza", "sponza.obj", m_main_scene);

		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "vertex", "test", "model_test.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "fragment", "test", "model_test.frag");
		m_opaque_shader = Bind::ShaderProgram::Resolve("shader", std::vector<GLuint>{vertex, fragment});
		m_proxy->AddRootBindable(m_opaque_shader);

		// 现在不考虑屏幕尺寸变化
		auto opaque_render_target = Bind::RenderTarget::Resolve("opaque_rendertarget", globalSettings::screen_width, globalSettings::screen_height);
		opaque_render_target->AppendTexture<GL_TEXTURE_2D>("opaque_texture").AppendDepthComponent<GL_RENDERBUFFER>("opaque_depth").CheckCompleteness();
		m_proxy->AddRootBindable(opaque_render_target);

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
		GeometryParameter param;
		param.m_name = "transparent_cubes";
		
		auto& instance_data = param.m_instance_datas;

		auto translate_data = Common::UniformGenerator::Generate({ -30.0f, -30.0f, -30.0f }, { 30.0f, 30.0f, 30.0f }, 100);
		auto scale_data = Common::UniformGenerator::Generate({ 0.2f, 0.2f, 0.2f }, { 5.0f, 5.0f, 5.0f }, 100);
		auto color_data = Common::UniformGenerator::Generate({ 0.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 100);

		std::vector<AvailableType> translates(translate_data.size());
		std::vector<AvailableType> scales(scale_data.size());
		std::vector<AvailableType> colors(color_data.size());
		auto transform_lambda = [&]()
			{
				for (size_t i = 0; i < translates.size(); i++)
				{
					translates[i] = translate_data[i];
					scales[i] = scale_data[i];
					colors[i] = color_data[i];
				}
			};
		transform_lambda();
		instance_data.push_back(std::make_pair(LeafType::Float3, std::move(translates)));
		instance_data.push_back(std::make_pair(LeafType::Float3, std::move(scales)));
		instance_data.push_back(std::make_pair(LeafType::Float4, std::move(colors)));

		m_transparent_proxy = DrawableLoader::LoadGeometry<GeometryType::Sphere>(m_main_scene, param);

		// auto sample_texture = 
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
#include "OIT.h"

#include <SceneGraph/modelproxy.h>
#include <SceneGraph/drawableproxy.h>
#include <Bindables/rendertarget.h>
#include <Bindables/storagetexture2D.h>
#include <Bindables/storagetexture2Darray.h>
#include <Bindables/texturebuffer.h>
#include <Bindables/atomicounter.h>
#include <assimploader.h>
#include <drawableloader.h>
#include <Bindables/shaderprogram.h>
#include <Common/random_generator.h>
#include <Common/render_helper.h>

#include <glm/gtc/matrix_transform.hpp>

namespace RTREffects
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

		// render_to_screen();
		// Common::RenderHelper::RenderTextureToScreen(m_test_opaque);
		// render_instance_test();
	}

	void OIT::prepare()
	{
		prepare_opaque();
		prepare_transparent();
		// prepare_OITdata();
		prepare_blend();
		prepare_rectangle();

		// prepare_instance_test();
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

		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "opaque_vertex", "test", "model_test.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "opaque_fragment", "test", "model_test.frag");
		m_opaque_shader = Bind::ShaderProgram::Resolve("opaque_shader", std::vector<GLuint>{vertex, fragment});
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
		m_proxy->ScaleModel(glm::vec3(0.2f));

		m_test_opaque = opaque_render_target->get_texture_image<Bind::ImageTexture2D>(0, {}, 0);
	}

	void OIT::prepare_transparent()
	{
		GeometryParameter param;
		param.m_name = "transparent_cubes";

		auto& instance_data = param.m_instance_datas;

		// auto translate_data = Common::UniformGenerator::Generate({ -12.0f, -12.0f, 0.0f }, { 12.0f, 12.0f, 12.0f }, 50);
		// auto scale_data = Common::UniformGenerator::Generate(1.0f, 10.0f, 50);
		// auto color_data = Common::UniformGenerator::Generate({ 0.2f, 0.2f, 0.2f, 0.4f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 50);

		std::vector<glm::vec3> translate_data = { {1.0,3.0,-2.0},{1.0,3.0,0.0},{1.0,3.0,2.0} };
		std::vector<glm::vec3> scale_data = { {8.0,8.0,8.0},{8.0,8.0,8.0},{8.0,8.0,8.0} };
		std::vector<glm::vec4> color_data = { {1.0,0.5,0.7,0.1},{0.6,0.9,0.8,0.1},{0.7,0.7,0.5,0.1} };

		std::vector<AvailableType> transforms(translate_data.size());
		std::vector<AvailableType> colors(color_data.size());
		auto transform_lambda = [&]()
			{
				for (size_t i = 0; i < transforms.size(); i++)
				{
					glm::mat4 trans = glm::mat4(1.0f);
					trans = glm::scale(trans, glm::vec3(scale_data[i]));
					trans = glm::translate(trans, translate_data[i]);
					transforms[i] = trans;

					colors[i] = color_data[i];
				}
			};
		transform_lambda();
		instance_data.push_back(std::make_pair(LeafType::FMat4, std::move(transforms)));
		instance_data.push_back(std::make_pair(LeafType::Float4, std::move(colors)));

		m_transparent_proxy = DrawableLoader::LoadGeometry<GeometryType::Sphere>(m_main_scene, param);

		OGL_TEXTURE2D_DESC desc;
		desc.target = GL_TEXTURE_2D;
		desc.width = (size_t)globalSettings::screen_width;
		desc.height = (size_t)globalSettings::screen_height;
		desc.internal_format = GL_R32UI;
		desc.cpu_format = GL_RED_INTEGER;
		desc.data_type = GL_UNSIGNED_INT;
		std::vector<uint32_t> initial_val(desc.width * desc.height, 0xFFFFFFFF);
		auto start_offset_image = Bind::StorageTexture2D::Resolve("head_offset_image", desc, 0, (void*)initial_val.data());

		size_t list_size = desc.width * desc.height * 2 * sizeof(uint32_t) * 4;
		auto list_buffer = Bind::TextureBuffer::Resolve("list_texture_buffer", list_size, GL_RGBA32UI, 1, 1);

		std::vector<GLuint> data(1, 0);
		auto counter = Bind::AtomicCounter::Resolve("list_counter", data, 0);

		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "transparent_vertex", "OIT", "transparent.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "transparent_fragment", "OIT", "transparent.frag");
		m_transparent_shader = Bind::ShaderProgram::Resolve("transparent_shader", std::vector<GLuint>{vertex, fragment});

		auto opaque_render_target = Bind::RenderTarget::Resolve("opaque_rendertarget", globalSettings::screen_width, globalSettings::screen_height);
		auto opaque_depth = opaque_render_target->get_depth_stencil();
		auto trans_render_target = Bind::RenderTarget::Resolve("transparent_render_target", {}, opaque_depth);
		
		m_transparent_proxy->AddRootBindable(start_offset_image).AddRootBindable(list_buffer).AddRootBindable(counter).AddRootBindable(m_transparent_shader).
			AddRootBindable(trans_render_target);

		std::vector<DrawItems::VertexType> instruction{ DrawItems::VertexType::Position,DrawItems::VertexType::InstanceData,DrawItems::VertexType::InstanceData };
		m_transparent_proxy->AddRootVertexRule(instruction);

		m_transparent_proxy->Cook();
	}

	void OIT::prepare_blend()
	{
		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "blend_vertex", "OIT", "blend.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "blend_fragment", "OIT", "blend.frag");
		m_blend_shader = Bind::ShaderProgram::Resolve("blend_shader", std::vector<GLuint>{vertex, fragment});

		auto opaque_framebuffer = Bind::RenderTarget::Resolve("opaque_rendertarget", globalSettings::screen_width, globalSettings::screen_height);
		m_opaque_texture = opaque_framebuffer->get_texture_image<Bind::ImageTexture2D>(0, {}, 2);
	}

	void OIT::prepare_rectangle()
	{
		m_rectangle = std::make_shared<DrawItems::Plane>("blend_rectangle");

		Dynamic::Dsr::VertexAttrib attrib;
		attrib.location = 0;
		attrib.name = "aPos";
		attrib.size = 1;
		attrib.type = GL_FLOAT_VEC3;

		m_rectangle->GenerateVAO({ attrib }, { DrawItems::VertexType::Position });
	}

	/*
	void OIT::prepare_test()
	{
		m_test_texture = Bind::ImageTexture2D::Resolve("test_texture", "girl.jpg", {}, 0);
	}
	*/

	void OIT::prepare_instance_test()
	{
		GeometryParameter param;
		param.m_name = "transparent_cubes";

		auto& instance_data = param.m_instance_datas;

		// auto translate_data = Common::UniformGenerator::Generate({ -4.0f, 0.0f, -4.0f }, { 4.0f, 6.0f, 4.0f }, 60);
		// auto scale_data = Common::UniformGenerator::Generate(0.2f, 5.0f, 60);
		// auto color_data = Common::UniformGenerator::Generate({ 0.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 60);

		std::vector<glm::vec3> translate_data = { {1.0,3.0,-2.0},{1.0,3.0,0.0},{1.0,3.0,2.0} };
		std::vector<glm::vec3> scale_data = { {8.0,8.0,8.0},{8.0,8.0,8.0},{8.0,8.0,8.0} };
		std::vector<glm::vec4> color_data = { {1.0,0.5,0.7,0.8},{0.6,0.9,0.8,0.7},{0.7,0.7,0.5,0.9} };

		std::vector<AvailableType> transforms(translate_data.size());
		std::vector<AvailableType> colors(color_data.size());
		auto transform_lambda = [&]()
			{
				for (size_t i = 0; i < transforms.size(); i++)
				{
					glm::mat4 trans = glm::mat4(1.0f);
					trans = glm::scale(trans, glm::vec3(scale_data[i]));
					trans = glm::translate(trans, translate_data[i]);
					transforms[i] = trans;

					colors[i] = color_data[i];
				}
			};
		transform_lambda();
		instance_data.push_back(std::make_pair(LeafType::FMat4, std::move(transforms)));
		instance_data.push_back(std::make_pair(LeafType::Float4, std::move(colors)));

		m_transparent_proxy = DrawableLoader::LoadGeometry<GeometryType::Sphere>(m_main_scene, param);

		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "instance_test_vertex", "test", "instance_test.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "instance_test_fragment", "test", "instance_test.frag");
		m_instance_test_shader = Bind::ShaderProgram::Resolve("instance_test_shader", std::vector<GLuint>{vertex, fragment});

		m_transparent_proxy->AddRootBindable(m_instance_test_shader);
		std::vector<DrawItems::VertexType> instruction{ DrawItems::VertexType::Position,DrawItems::VertexType::InstanceData,DrawItems::VertexType::InstanceData };
		m_transparent_proxy->AddRootVertexRule(instruction);

		m_transparent_proxy->Cook();
	}

	void OIT::prepare_OITdata()
	{

	}

	void OIT::render_opaque()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);

		m_opaque_shader->EditUniform("view") = globalSettings::mainCamera.get_view();
		m_opaque_shader->EditUniform("projection") = globalSettings::mainCamera.get_perspective();
		m_opaque_shader->EditUniform("camera_pos") = globalSettings::mainCamera.get_position();
		
		m_proxy->Render();
	}

	void OIT::render_OIT()
	{
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);

		m_transparent_shader->EditUniform("view") = globalSettings::mainCamera.get_view();
		m_transparent_shader->EditUniform("projection") = globalSettings::mainCamera.get_perspective();

		m_transparent_proxy->Render(false, false);
	}

	void OIT::composite()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_blend_shader->BindWithoutUpdate();
		m_opaque_texture->Bind();
		auto list_buffer = Bind::TextureBuffer::Resolve("list_texture_buffer", 0, GL_RGBA32UI, 1, 1);
		list_buffer->SetUsage(true);
		list_buffer->Bind();
		auto start_offset_image = Bind::StorageTexture2D::Resolve("head_offset_image", {}, 0, (void*)0);
		start_offset_image->SetInitialize(false);
		start_offset_image->Bind();

		glDisable(GL_DEPTH_TEST);

		m_rectangle->Draw();

		list_buffer->SetUsage(false);
		list_buffer->UnBind();
		start_offset_image->UnBind(); 
		start_offset_image->SetInitialize(true);
		m_blend_shader->UnBind();
		m_opaque_texture->UnBind();
	}

	/*
	void OIT::render_to_screen()
	{
		Common::RenderHelper::RenderTextureToScreen(m_blend_texture);
	}
	*/

	void OIT::render_instance_test()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_instance_test_shader->EditUniform("view") = globalSettings::mainCamera.get_view();
		m_instance_test_shader->EditUniform("projection") = globalSettings::mainCamera.get_perspective();
		m_transparent_proxy->Render();
	}
}
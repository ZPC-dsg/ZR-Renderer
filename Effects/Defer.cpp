#include "Defer.h"

#include <assimploader.h>
#include <Bindables/shaderprogram.h>
#include <Bindables/rendertarget.h>
#include <Bindables/storagetexture2D.h>
#include <Bindables/storagebuffer.h>
#include <Common/render_helper.h>
#include <Common/computeProxy.h>
#include <Common/random_generator.h>

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
		render_defer();
		switch (m_debug_mode)
		{
			case -1:
			{
				render_compute();
				render_screen(); 
				break;
			}
			case 0:
			{
				render_diff_spec(true);
				break;
			}
			case 1:
			{
				render_diff_spec(false);
				break;
			}
			case 2:
			{
				render_normal();
				break;
			}
			case 3:
			{
				render_depth();
				break;
			}
		}
	}

	void DeferRenderer::prepare_ui(const std::string& name)
	{
		ImGui::Begin(name.c_str());

		int item_count = sizeof(m_debug_items) / sizeof(m_debug_items[0]);
		int debug_index = (item_count + m_debug_mode) % item_count;

		ImGui::Combo("Debug Mode", &debug_index, m_debug_items, item_count);

		m_debug_mode = (debug_index == item_count - 1) ? -1 : debug_index;

		ImGui::End();
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

	void DeferRenderer::prepare_defer()
	{
		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "defer_vertex", "Defer", "defer.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "defer_fragment", "Defer", "defer.frag");
		m_defer_shader = Bind::ShaderProgram::Resolve("defer_shader", std::vector<GLuint>{vertex, fragment});

		auto defer_framebuffer = Bind::RenderTarget::Resolve("defer_framebuffer", globalSettings::screen_width, globalSettings::screen_height);
		defer_framebuffer->AppendTexture<GL_TEXTURE_2D>("diffuse_specular_texture", {}, 1, 1, GL_RGBA8)
			.AppendTexture<GL_TEXTURE_2D>("normal_shininess_texture", {}, 1, 1, GL_RGBA16F)
			.AppendDepthComponent<GL_TEXTURE_2D>("depth_texture", 1, GL_DEPTH_COMPONENT32F);

		m_diff_spec_texture = defer_framebuffer->get_texture_image<Bind::ImageTexture2D>("diff_spec", "diffuse_specular_texture", {}, 0);
		m_norm_shin_texture = defer_framebuffer->get_texture_image<Bind::ImageTexture2D>("norm_shini", "normal_shininess_texture", {}, 0);
		m_depth_texture = defer_framebuffer->get_texture_depthstencil<Bind::ImageTexture2D>("depth", {}, 0);

		m_proxy->AddRootBindable(m_defer_shader).AddRootBindable(defer_framebuffer);

		m_proxy->AddRootUniformRule<SceneGraph::ConfigurationType::Transformation>(m_defer_shader->EditUniform("model").GetLeafUniform(),
			[](glm::mat4 model)->glm::mat4 {return model; })
			.AddRootUniformRule<SceneGraph::ConfigurationType::MaterialShiniStrength>(m_defer_shader->EditUniform("specular_strength").GetLeafUniform(),
				[](float strength)->float {return strength; })
			.AddRootUniformRule<SceneGraph::ConfigurationType::MaterialShininess>(m_defer_shader->EditUniform("shininess").GetLeafUniform(),
				[](float shininess)->float {return shininess; });

		m_proxy->Cook();
	}

	void DeferRenderer::prepare_compute()
	{
		GLuint comp = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Compute, "render_compute", "Defer", "cull_and_render.comp");
		m_compute_shader = Bind::ShaderProgram::Resolve("cull_render_shader", { comp });

		OGL_TEXTURE2D_DESC desc;
		desc.target = GL_TEXTURE_2D;
		desc.width = globalSettings::screen_width;
		desc.height = globalSettings::screen_height;
		desc.internal_format = GL_RGBA16F;
		desc.cpu_format = GL_RGBA;
		desc.data_type = GL_FLOAT;
		m_output_image = Bind::StorageTexture2D::Resolve("output_image", desc, 0);

		// 准备点光源数据
		struct PointLight
		{
			glm::vec3 position;
			float range;
			glm::vec3 color;
			unsigned int _padding;
		};

		std::vector<PointLight> lights(100);
		std::vector<glm::vec3> positions = Common::UniformGenerator::Generate({ -50.0f, 0.0f, -50.0f }, { 50.0f, 50.0f, 50.0f }, lights.size());
		std::vector<float> ranges = Common::UniformGenerator::Generate(10.0f, 40.0f, lights.size());
		std::vector<glm::vec3> colors = Common::UniformGenerator::Generate({ 0.1f, 0.1f, 0.1f }, { 1.0f, 1.0f, 1.0f }, lights.size());
		for (size_t i = 0; i < lights.size(); i++)
		{
			lights[i].position = positions[i];
			lights[i].range = ranges[i];
			lights[i].color = colors[i];
		}

		auto light_buffer = Bind::StorageBuffer::Resolve("light_buffer", "light_ssbo", lights.size() * sizeof(PointLight), 0, GL_DYNAMIC_STORAGE_BIT);
		light_buffer->Update((void*)lights.data());

		// 暂时不考虑屏幕大小变化，同时屏幕大小一定是16的倍数
		GLuint sizeX = globalSettings::screen_width / 16;
		GLuint sizeY = globalSettings::screen_height / 16;
		m_compute_proxy = std::make_shared<Common::ComputeProxy>(sizeX, sizeY);

		m_compute_proxy->AddBindable(m_compute_shader).AddBindable(m_output_image).AddBindable(m_diff_spec_texture, 1).AddBindable(m_norm_shin_texture, 2)
			.AddBindable(m_depth_texture, 3).AddBindable(light_buffer);

		// 更新计算着色器中的uniform数据
		m_compute_proxy->EditUniform("light_count", (unsigned int)lights.size()).EditUniform("z_near", globalSettings::mainCamera.zNear)
			.EditUniform("z_far", globalSettings::mainCamera.zFar).EditUniform("screenWidth", (float)globalSettings::screen_width)
			.EditUniform("screenHeight", (float)globalSettings::screen_height);// 更新计算着色器中的uniform数据

		m_compute_proxy->Finalize();
	}

	void DeferRenderer::render_defer()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);

		m_defer_shader->EditUniform("view") = globalSettings::mainCamera.get_view();
		m_defer_shader->EditUniform("projection") = globalSettings::mainCamera.get_perspective();

		m_proxy->Render();
	}

	void DeferRenderer::render_compute()
	{
		m_compute_proxy->EditUniform("view", globalSettings::mainCamera.get_view());
		m_compute_proxy->EditUniform("proj", globalSettings::mainCamera.get_perspective());

		m_compute_proxy->Dispatch();
		// 内存屏障：确保计算着色器的image写入对后续渲染可见以及后续纹理采样能采样到最新数据
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	void DeferRenderer::render_screen()
	{
		glDisable(GL_CULL_FACE);
		m_output_image->SetUsage(false);
		Common::RenderHelper::RenderTextureToScreen(m_output_image);
		m_output_image->SetUsage(true);
	}

	void DeferRenderer::render_diff_spec(bool diffuse)
	{
		if (diffuse)
		{
			Common::RenderHelper::RenderTextureToScreen(m_diff_spec_texture);
		}
		else
		{
			Common::RenderHelper::RenderTextureToScreen(m_diff_spec_texture, 1);
		}
	}

	void DeferRenderer::render_normal()
	{
		Common::RenderHelper::RenderUnitVectorToScreen(m_norm_shin_texture);
	}

	void DeferRenderer::render_depth()
	{
		Common::RenderHelper::RenderLinearDepthToScreen(m_depth_texture, globalSettings::mainCamera.zNear, globalSettings::mainCamera.zFar);
	}
}
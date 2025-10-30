#include <Pipeline/defer.h>
#include <Pipeline/postprocess.h>
#include <Macros/gfxdebug_helper.h>
#include <Bindables/rendertarget.h>
#include <Bindables/shaderprogram.h>
#include <Bindables/constantbuffer.h>
#include <Bindables/Sampler.h>
#include <Common/render_helper.h>
#include <Common/math_const.h>
#include <Common/random_generator.h>
#include <assimploader.h>

#define SIMPLE_SSAO_RANDOM_SAMPLE_NUM 64

namespace OGLPipeline
{
	const char* DeferRenderer::scene_names[] =
	{
		"Sponza",
		"Feiji Cup",
	};

	DeferRenderer::DeferRenderer(const std::string& scene_name, const std::string ui_name)
		:Utils(scene_name), m_ui_name(ui_name)
	{
		globalSettings::mainCamera.set_perspective(60.0f, globalSettings::screen_width, globalSettings::screen_height, 0.1f, 512.0f);
		globalSettings::mainCamera.set_position(glm::vec3(0.0f, 0.0f, 4.0f));
	}

	void DeferRenderer::render()
	{
		RenderPreZ();
		RenderHiZ();
		RenderDefer();
		RenderAO();
		if (!m_should_display_ao)
		{
			DeferLighting();
			g_post_processor.MainProcessor();
			DisplayDefer();
		}
	}

	// TODO ：屏幕尺寸变化处理逻辑后续补充
	void DeferRenderer::prepare()
	{
		g_post_processor.Accept(this);

		PreparePreZ();
		PrepareHiZ();
		PrepareDeferBuffers();
		PrepareScene();
		PrepareSamplers();
		PrepareDeferLighting();
		PrepareLightBuffer();
		PrepareAO();

		g_post_processor.PreparePostProcess();
	}

	void DeferRenderer::prepare_ui(const std::string& name)
	{
		ImGui::Begin(name.c_str());

		int current_scene = static_cast<int>(m_scene_index);
		ImGui::Combo("Scene List", &current_scene, scene_names, IM_ARRAYSIZE(scene_names));
		if (current_scene != m_scene_index)
		{
			globalSettings::mainCamera.set_perspective(60.0f, globalSettings::screen_width, globalSettings::screen_height, 0.1f, 512.0f);
			globalSettings::mainCamera.set_position(glm::vec3(0.0f, 0.0f, 4.0f));
		}
		m_scene_index = current_scene;

		// Defer RenderTargets Display
		if (ImGui::CollapsingHeader("Defer Display Mode", false))
		{
			if (m_should_display_ao || g_post_processor.ShouldDisplayBloom())
			{
				ImGui::BeginDisabled();
			}
			ImGui::PushID(1); // 作用域1，防止发生按钮名称冲突
			ImGui::Indent();
			int display_mode = static_cast<int>(m_defer_display_mode);
			ImGui::RadioButton("Albedo", &display_mode, DEFER_DISPLAY_MODE_ALBEDO);
			ImGui::RadioButton("Diffuse", &display_mode, DEFER_DISPLAY_MODE_DIFFUSE);
			ImGui::RadioButton("Specular", &display_mode, DEFER_DISPLAY_MODE_SPECULAR);
			ImGui::RadioButton("World Position", &display_mode, DEFER_DISPLAY_MODE_WORLD_POSITION);
			ImGui::RadioButton("World Normal", &display_mode, DEFER_DISPLAY_MODE_WORLD_NORMAL);
			ImGui::RadioButton("Metallic", &display_mode, DEFER_DISPLAY_MODE_METALLIC);
			ImGui::RadioButton("Roughness", &display_mode, DEFER_DISPLAY_MODE_ROUGHNESS);
			ImGui::RadioButton("Ndc Depth", &display_mode, DEFER_DISPLAY_MODE_NDC_DEPTH);
			ImGui::RadioButton("Linear Depth", &display_mode, DEFER_DISPLAY_MODE_LINEAR_DEPTH);
			ImGui::RadioButton("Anisotrophy", &display_mode, DEFER_DISPLAY_MODE_ANISOTROPHY);
			ImGui::RadioButton("PBR Lighting", &display_mode, DEFER_DISPLAY_MODE_PBR_LIGHTING);
			ImGui::RadioButton("None", &display_mode, DEFER_DISPLAY_MODE_NUM);
			m_defer_display_mode = static_cast<uint16_t>(display_mode);
			ImGui::Unindent();
			ImGui::PopID();
			if (m_should_display_ao || g_post_processor.ShouldDisplayBloom())
			{
				ImGui::EndDisabled();
			}
		}

		PrepareAOUI();

		g_post_processor.PrepareUI();

		PrepareLightUI();

		ImGui::End();
	}

	void DeferRenderer::resize()
	{
		m_prez_framebuffer->DestroyAndCreateNew(globalSettings::screen_width, globalSettings::screen_height);
		m_rt_depthbuffer->UpdateNewResource(m_defer_framebuffer->get_depth_stencil(), m_rt_depthbuffer->get_parameter());

		m_defer_framebuffer->DestroyAndCreateNew(globalSettings::screen_width, globalSettings::screen_height);
		OGL_TEXTURE_PARAMETER params;
		m_rt_albedo_specular->UpdateNewResource(m_defer_framebuffer->get_render_target(1), params);
		params.min_filter = GL_NEAREST;
		params.mag_filter = GL_NEAREST;
		m_rt_position_anisotrophy->UpdateNewResource(m_defer_framebuffer->get_render_target(0), params);
		m_rt_normal_metallic_roughness->UpdateNewResource(m_defer_framebuffer->get_render_target(2), params);
		m_rt_depthbuffer->UpdateNewResource(m_defer_framebuffer->get_depth_stencil(), params);

		m_defer_lighting_framebuffer->DestroyAndCreateNew(globalSettings::screen_width, globalSettings::screen_height);
		params.min_filter = GL_LINEAR;
		params.mag_filter = GL_LINEAR;
		m_lighting_texture->UpdateNewResource(m_defer_lighting_framebuffer->get_render_target(0), params);

		m_AO_framebuffer->DestroyAndCreateNew(globalSettings::screen_width, globalSettings::screen_height);
		m_AO_texture->UpdateNewResource(m_AO_framebuffer->get_render_target(0), params);
		OGL_TEXTURE2D_DESC desc = m_AO_filtered_texture->get_description();
		desc.width = globalSettings::screen_width;
		desc.height = globalSettings::screen_height;
		m_AO_filtered_texture->DestroyAndCreateNew(desc);

		PrepareDefaultTextures();

		g_post_processor.OnResize();
	}

	void DeferRenderer::PrepareScene()
	{
		// Sponza
		m_scenes.push_back(AssimpLoader::LoadModel("Sponza", "sponza.obj", m_main_scene));
		m_scenes[0]->BeginRange("main_defer");
		m_scenes[0]->AddRootBindable(m_defer_framebuffer);

		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "defer_vertex", "Common", "defer.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "defer_fragment", "Common", "defer.frag");
		auto defer_shader = Bind::ShaderProgram::Resolve("defer_shader", std::vector<GLuint>{vertex, fragment});
		m_scenes[0]->AddRootBindable(defer_shader);

		m_scenes[0]->AddRootUniformRule<SceneGraph::ConfigurationType::Transformation>(defer_shader->EditUniform("model").GetLeafUniform(),
			[](glm::mat4 model)->glm::mat4 {return model; })
			.AddRootUniformRule<SceneGraph::ConfigurationType::MaterialMetallic>(defer_shader->EditUniform("metallic").GetLeafUniform(),
				[](float metallic)->float {return metallic; })
			.AddRootUniformRule<SceneGraph::ConfigurationType::MaterialRoughness>(defer_shader->EditUniform("roughness").GetLeafUniform(),
				[](float roughness)->float {return roughness; });
		std::vector<DrawItems::VertexType> instruction{ DrawItems::VertexType::Position,DrawItems::VertexType::Normal,DrawItems::VertexType::Tangent,
			DrawItems::VertexType::Texcoord };
		m_scenes[0]->AddRootVertexRule(instruction);
		m_scenes[0]->AddRootTextureRule("diffuse_tex", 0, SceneGraph::Material::TextureCategory::DIFFUSE).
			AddRootTextureRule("specular_tex", 1, SceneGraph::Material::TextureCategory::SPECULAR).
			AddRootTextureRule("normal_tex", 2, SceneGraph::Material::TextureCategory::NORMAL);
		m_scenes[0]->ScaleModel(glm::vec3(0.2f));
		m_scenes[0]->EndRange();

		m_scenes[0]->Cook();

		// Feiji Cup
		m_scenes.push_back(AssimpLoader::LoadModel("FeijiCup", "Cup_Handle.obj", m_main_scene));
		m_scenes[1]->BeginRange("main_defer");
		m_scenes[1]->AddRootBindable(m_defer_framebuffer);
		m_scenes[1]->AddRootBindable(defer_shader);
		m_scenes[1]->AddRootUniformRule<SceneGraph::ConfigurationType::Transformation>(defer_shader->EditUniform("model").GetLeafUniform(),
			[](glm::mat4 model)->glm::mat4 {return model; })
			.AddRootUniformRule<SceneGraph::ConfigurationType::MaterialMetallic>(defer_shader->EditUniform("metallic").GetLeafUniform(),
				[](float metallic)->float {return metallic; })
			.AddRootUniformRule<SceneGraph::ConfigurationType::MaterialRoughness>(defer_shader->EditUniform("roughness").GetLeafUniform(),
				[](float roughness)->float {return roughness; });
		m_scenes[1]->AddRootVertexRule(instruction);
		m_scenes[1]->AddRootTextureRule("diffuse_tex", 0, SceneGraph::Material::TextureCategory::DIFFUSE).
			AddRootTextureRule("specular_tex", 1, SceneGraph::Material::TextureCategory::SPECULAR).
			AddRootTextureRule("normal_tex", 2, SceneGraph::Material::TextureCategory::NORMAL);
		m_scenes[1]->EndRange();
		m_scenes[1]->Cook();
	}

	void DeferRenderer::PrepareDeferBuffers()
	{
		m_defer_framebuffer = Bind::RenderTarget::Resolve("defer_framebuffer", globalSettings::screen_width, globalSettings::screen_height);
		m_defer_framebuffer->AppendTexture<GL_TEXTURE_2D>("rt_postion_anisotrophy", {}, 1, 1, GL_RGBA16F)
			.AppendTexture<GL_TEXTURE_2D>("rt_albedo_specular", {}, 1, 1, GL_RGBA8)
			.AppendTexture<GL_TEXTURE_2D>("rt_normal_metallic_roughness", {}, 1, 1, GL_RGBA16F)
			.AppendDepthComponent<GL_TEXTURE_2D>("rt_depth", 1, GL_DEPTH_COMPONENT32F).CheckCompleteness();

		OGL_TEXTURE_PARAMETER params;
		m_rt_albedo_specular = m_defer_framebuffer->get_texture_image<Bind::ImageTexture2D>("albe_spec_texture", "rt_albedo_specular", params, 1);
		params.min_filter = GL_NEAREST;
		params.mag_filter = GL_NEAREST;
		m_rt_position_anisotrophy = m_defer_framebuffer->get_texture_image<Bind::ImageTexture2D>("pos_aniso_texture", "rt_postion_anisotrophy", params, 0);
		m_rt_normal_metallic_roughness = m_defer_framebuffer->get_texture_image<Bind::ImageTexture2D>("norm_mr_texture", "rt_normal_metallic_roughness", params, 2);
		m_rt_depthbuffer = m_defer_framebuffer->get_texture_depthstencil<Bind::ImageTexture2D>("depth_tex", params, 0);
	}

	void DeferRenderer::PrepareSamplers()
	{
		OGL_TEXTURE_PARAMETER param;
		param.wrap_x = GL_CLAMP_TO_EDGE;
		param.wrap_y = GL_CLAMP_TO_EDGE;
		param.wrap_z = GL_CLAMP_TO_EDGE;
		m_bilinear_sampler = Bind::Sampler::Resolve("bilinear_sampler", 0, param);

		param.min_filter = GL_LINEAR_MIPMAP_LINEAR;
		m_trilinear_sampler = Bind::Sampler::Resolve("trilinear_sampler", 1, param);
		
		param.min_filter = GL_NEAREST;
		param.mag_filter = GL_NEAREST;
		m_point_sampler = Bind::Sampler::Resolve("point_sampler", 2, param);
	}

	void DeferRenderer::PrepareDefaultTextures()
	{
		OGL_TEXTURE2D_DESC desc;
		desc.target = GL_TEXTURE_2D;
		desc.width = globalSettings::screen_width;
		desc.height = globalSettings::screen_height;
		desc.internal_format = GL_R8;
		desc.cpu_format = GL_RED;
		desc.data_type = GL_UNSIGNED_BYTE;

		std::vector<uint8_t> data(globalSettings::screen_width * globalSettings::screen_height, 255);
		m_default_white_texture = Bind::ImageTexture2D::Resolve("default_white", desc, {}, 3, (void*)data.data());
	}

	void DeferRenderer::PrepareDeferLighting()
	{
		m_defer_lighting_framebuffer = Bind::RenderTarget::Resolve("defer_lighting_framebuffer", globalSettings::screen_width, globalSettings::screen_height);
		m_defer_lighting_framebuffer->AppendTexture<GL_TEXTURE_2D>("defer_lighting_texture", {}, 1, 1, GL_RGBA16F).CheckCompleteness();

		OGL_TEXTURE_PARAMETER params;
		params.wrap_x = GL_CLAMP_TO_EDGE;
		params.wrap_y = GL_CLAMP_TO_EDGE;
		m_lighting_texture = m_defer_lighting_framebuffer->get_texture_image<Bind::ImageTexture2D>("defer_shading", "defer_lighting_texture", params, 0);

		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "defer_lighting_vertex", "Common", "defer_shading.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "defer_lighting_fragment", "Common", "defer_shading.frag");
		auto defer_lighting_shader = Bind::ShaderProgram::Resolve("defer_lighting_shader", std::vector<GLuint>{vertex, fragment});
	}

	void DeferRenderer::PrepareLightBuffer()
	{
		auto defer_lighting_shader = Bind::ShaderProgram::Resolve("defer_lighting_shader", { 0,0 });
		m_light_buffer = Bind::ConstantBuffer::Resolve("light_buffer", defer_lighting_shader, "LightBuffer", 0);

		m_main_light = Common::LightCommon::ConstructDefaultDirection();
		auto ref = m_light_buffer->EditConstant("lights");
		auto main_light_slot = ref[0];
		using namespace std::string_literals;
		main_light_slot["position"s] = m_main_light.m_position;
		main_light_slot["flags"s] = m_main_light.m_flags;
		main_light_slot["color"s] = m_main_light.m_color;
		main_light_slot["intensity"s] = m_main_light.m_intensity;
		for (int i = 1; i < MAX_LIGHTS_SUPPORTED; i++)
		{
			auto light_slot = ref[i];
			light_slot["flags"s] = LIGHT_STATUS_INVALID;
		}

		m_light_buffer->Bind();
		m_light_buffer->Update();
		m_light_buffer->UnBind();

		m_available_indexes = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	}

	void DeferRenderer::PreparePreZ()
	{
		m_prez_framebuffer = Bind::RenderTarget::Resolve("prez_framebuffer", globalSettings::screen_width, globalSettings::screen_height);
		m_prez_framebuffer->AppendDepthComponent<GL_TEXTURE_2D>("rt_depth", 1, GL_DEPTH_COMPONENT32F).CheckCompleteness();
		OGL_TEXTURE_PARAMETER params;
		params.min_filter = GL_NEAREST;
		params.mag_filter = GL_NEAREST;
		m_rt_depthbuffer = m_prez_framebuffer->get_texture_depthstencil<Bind::ImageTexture2D>("depth_tex", params, 0);

		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "prez_vertex", "Common", "defer_shading.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "prez_fragment", "Common", "empty.frag");
		auto prez_shader = Bind::ShaderProgram::Resolve("prez_shader", { vertex,fragment });
	}

	void DeferRenderer::PrepareHiZ()
	{

	}

	void DeferRenderer::PrepareAO()
	{
		m_AO_framebuffer = Bind::RenderTarget::Resolve("AO_framebuffer", globalSettings::screen_width, globalSettings::screen_height);
		m_AO_framebuffer->AppendTexture<GL_TEXTURE_2D>("AO_texture", {}, 1, 1, GL_R16F);
		m_AO_texture = m_AO_framebuffer->get_texture_image<Bind::ImageTexture2D>("AO_image", 0, {}, 0);
		OGL_TEXTURE2D_DESC desc;
		desc.target = GL_TEXTURE_2D;
		desc.width = globalSettings::screen_width;
		desc.height = globalSettings::screen_height;
		desc.internal_format = GL_R16F;
		desc.cpu_format = GL_RED;
		desc.data_type = GL_FLOAT;
		m_AO_filtered_texture = Bind::ImageTexture2D::Resolve("AO_filtered_texture", desc, {}, 3);

		// SSAO
		GenerateSSAORandomTexture();
		GenerateSimpleSSAORandomPos();
		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "simple_SSAO_vertex", "Common", "defer_shading.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "simple_SSAO_fragment", "AO", "SSAO_simple.frag");
		auto SSAO_simple_shader = Bind::ShaderProgram::Resolve("simple_SSAO_shader", { vertex,fragment });
		GLuint filter_vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "simple_SSAO_filter_vertex", "Common", "defer_shading.vert");
		GLuint filter_fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "simple_SSAO_filter_fragment", "AO", "SSAO_simple_filter.frag");
		auto SSAO_simple_filter_shader = Bind::ShaderProgram::Resolve("simple_SSAO_filter_shader", { filter_vertex,filter_fragment });
	}

	void DeferRenderer::GenerateSSAORandomTexture()
	{
		const float angle_off1 = 127;
		const float angle_off2 = 198;
		const float angle_off3 = 23;

		std::vector<uint8_t> bases(32);

		auto Quantize8SignedByte = [](float f)->uint8_t
			{
				float uf = f * 0.5f + 0.5f;
				int i = (int)(uf * 255.0f + 0.5f);
				return (uint8_t)i;
			};

		for (int pos = 0; pos < 16; ++pos)
		{
			// distribute rotations over 4x4 pattern
			int reorder[16] = { 0, 11, 7, 3, 10, 4, 15, 12, 6, 8, 1, 14, 13, 2, 9, 5 };
			int w = reorder[pos];

			// ordered sampling of the rotation basis (*2 is missing as we use mirrored samples)
			float ww = w / 16.0f * PI;

			// randomize base scale
			float lenm = 1.0f - (std::sin(angle_off2 * w * 0.01f) * 0.5f + 0.5f) * angle_off3 * 0.01f;
			float s = std::sin(ww) * lenm;
			float c = std::cos(ww) * lenm;

			bases[2 * pos] = Quantize8SignedByte(c);
			bases[2 * pos + 1] = Quantize8SignedByte(s);
		}

		const unsigned int extent = 64;
		const unsigned int block_size = 4 * 4 * 2;
		const unsigned int row_size = extent * 2;
		std::vector<uint8_t> real_data(extent * extent * 2);
		for (int i = 0; i < 16; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				size_t start = (i * 4) * row_size + (j * 4) * 2;

				std::copy(bases.begin(), bases.begin() + 8, real_data.begin() + start);
				std::copy(bases.begin() + 8, bases.begin() + 16, real_data.begin() + start + row_size);
				std::copy(bases.begin() + 16, bases.begin() + 24, real_data.begin() + start + 2 * row_size);
				std::copy(bases.begin() + 24, bases.end(), real_data.begin() + start + 3 * row_size);
			}
		}

		OGL_TEXTURE2D_DESC desc;
		desc.target = GL_TEXTURE_2D;
		desc.width = extent;
		desc.height = extent;
		desc.internal_format = GL_RG8;
		desc.cpu_format = GL_RG;
		OGL_TEXTURE_PARAMETER param;
		param.wrap_x = GL_REPEAT;
		param.wrap_y = GL_REPEAT;
		m_SSAO_random_texture = Bind::ImageTexture2D::Resolve("SSAO_random_texture", desc, param, 1, (void*)real_data.data());
	}

	void DeferRenderer::GenerateSimpleSSAORandomPos()
	{
		m_simple_SSAO_random_positions.reserve(SIMPLE_SSAO_RANDOM_SAMPLE_NUM);

		for (int i = 0; i < SIMPLE_SSAO_RANDOM_SAMPLE_NUM; i++)
		{
			m_simple_SSAO_random_positions.push_back(Common::UniformGenerator::Generate(glm::vec3(0.0f), glm::vec3(1.0f)));
			m_simple_SSAO_random_positions[i].x = 2.0f * m_simple_SSAO_random_positions[i].x - 1.0f;
			m_simple_SSAO_random_positions[i].y = 2.0f * m_simple_SSAO_random_positions[i].y - 1.0f;
			m_simple_SSAO_random_positions[i] = glm::normalize(m_simple_SSAO_random_positions[i]);

			float scale = Common::UniformGenerator::Generate(0.0f, 1.0f);
			scale *= std::pow(float(i) / (float)SIMPLE_SSAO_RANDOM_SAMPLE_NUM, 2.0f);
			m_simple_SSAO_random_positions[i] *= scale;
		}
	}

	void DeferRenderer::PrepareLightUI()
	{
		using namespace std::string_literals;

		if (ImGui::Button("Show Light Parameters"))
		{
			m_show_light_window = true;
		}

		if (m_show_light_window)
		{
			ImGui::Begin("Light Parameters", &m_show_light_window);

			bool should_update = false;

			// Directional Light
			if (ImGui::CollapsingHeader("Main Light"))
			{
				auto ref = m_light_buffer->EditConstant("lights")[0];

				ImGui::PushID(3);
				ImGui::Text("Direction:");
				ImGui::NewLine();
				should_update |= ImGui::InputFloat("x:", &m_main_light.m_position.x, 0.1f, 1.0f);
				ImGui::NewLine();
				should_update |= ImGui::InputFloat("y:", &m_main_light.m_position.y, 0.1f, 1.0f);
				ImGui::NewLine();
				should_update |= ImGui::InputFloat("z:", &m_main_light.m_position.z, 0.1f, 1.0f);
				if (should_update)
				{
					m_main_light.m_position = glm::normalize(m_main_light.m_position);
					ref["position"s] = m_main_light.m_position;
				}

				bool update_color = false;
				ImGui::Text("Color:");
				float color[3] = { m_main_light.m_color.x,m_main_light.m_color.y,m_main_light.m_color.z };
				should_update |= update_color = ImGui::ColorEdit3("RGB", color);
				if (update_color)
				{
					m_main_light.m_color = { color[0], color[1], color[2] };
					ref["color"s] = m_main_light.m_color;
				}

				bool update_intensity = false;
				should_update |= update_intensity = ImGui::DragFloat("Intensity", &m_main_light.m_intensity, 0.1f, 0.0f, 100.0f);
				if (update_intensity)
				{
					ref["intensity"s] = m_main_light.m_intensity;
				}
				ImGui::PopID();
			}

			// Point Light
			if (ImGui::CollapsingHeader("Point Lights"))
			{
				ImGui::PushID(4);
				for (size_t i = 0; i < m_point_lights.size(); i++)
				{
					auto ref = m_light_buffer->EditConstant("lights")[m_point_lights[i].second];

					std::string item_name = "Point Light " + std::to_string(i);
					if (ImGui::CollapsingHeader(item_name.c_str()))
					{
						ImGui::PushID(100 + i);
						bool update_position = false;
						ImGui::Text("Position:");
						ImGui::NewLine();
						should_update |= update_position |= ImGui::InputFloat("x:", &m_point_lights[i].first.m_position.x, 0.1f, 1.0f);
						ImGui::NewLine();
						should_update |= update_position |= ImGui::InputFloat("y:", &m_point_lights[i].first.m_position.y, 0.1f, 1.0f);
						ImGui::NewLine();
						should_update |= update_position |= ImGui::InputFloat("z:", &m_point_lights[i].first.m_position.z, 0.1f, 1.0f);
						if (update_position)
						{
							ref["position"s] = m_point_lights[i].first.m_position;
						}

						bool update_color = false;
						ImGui::Text("Color:");
						float color[3] = { m_point_lights[i].first.m_color.x,m_point_lights[i].first.m_color.y,m_point_lights[i].first.m_color.z };
						should_update |= update_color = ImGui::ColorEdit3("RGB", color);
						if (update_color)
						{
							m_point_lights[i].first.m_color = { color[0], color[1], color[2] };
							ref["color"s] = m_point_lights[i].first.m_color;
						}

						bool update_intensity = false;
						should_update |= update_intensity = ImGui::DragFloat("Intensity", &m_point_lights[i].first.m_intensity, 0.1f, 0.0f, 10.0f);
						if (update_intensity)
						{
							ref["intensity"s] = m_point_lights[i].first.m_intensity;
						}

						if (ImGui::Button("Delete"))
						{
							should_update = true;

							size_t index = m_point_lights[i].second;
							m_available_indexes.insert(index);
							m_point_lights.erase(m_point_lights.begin() + i);

							ref["flags"s] = LIGHT_STATUS_INVALID;
						}
					}
					ImGui::PopID();
				}

				if (ImGui::Button("Add") && m_available_indexes.size())
				{
					should_update = true;

					size_t index = *(m_available_indexes.begin());
					Common::LightCommon point = Common::LightCommon::ConstructDefaultPoint();
					m_point_lights.push_back({ point,index });
					m_available_indexes.erase(index);

					auto constant = m_light_buffer->EditConstant("lights")[index];
					constant["position"s] = point.m_position;
					constant["flags"s] = point.m_flags;
					constant["color"s] = point.m_color;
					constant["intensity"s] = point.m_intensity;
				}
				ImGui::PopID();
			}

			// Spot Light
			if (ImGui::CollapsingHeader("Spot Lights"))
			{
				ImGui::PushID(5);
				for (size_t i = 0; i < m_spot_lights.size(); i++)
				{
					auto ref = m_light_buffer->EditConstant("lights")[m_spot_lights[i].second];

					std::string item_name = "Spot Light " + std::to_string(i);
					if (ImGui::CollapsingHeader(item_name.c_str()))
					{
						ImGui::PushID(120 + i);
						bool update_position = false;
						ImGui::Text("Position:");
						ImGui::NewLine();
						should_update |= update_position |= ImGui::InputFloat("x:", &m_spot_lights[i].first.m_position.x, 0.1f, 1.0f);
						ImGui::NewLine();
						should_update |= update_position |= ImGui::InputFloat("y:", &m_spot_lights[i].first.m_position.y, 0.1f, 1.0f);
						ImGui::NewLine();
						should_update |= update_position |= ImGui::InputFloat("z:", &m_spot_lights[i].first.m_position.z, 0.1f, 1.0f);
						if (update_position)
						{
							ref["position"s] = m_spot_lights[i].first.m_position;
						}

						bool update_color = false;
						ImGui::Text("Color:");
						float color[3] = { m_spot_lights[i].first.m_color.x,m_spot_lights[i].first.m_color.y,m_spot_lights[i].first.m_color.z };
						should_update |= update_color = ImGui::ColorEdit3("RGB", color);
						if (update_color)
						{
							m_spot_lights[i].first.m_color = { color[0], color[1], color[2] };
							ref["color"s] = m_spot_lights[i].first.m_color;
						}

						bool update_intensity = false;
						float intensity = std::abs(m_spot_lights[i].first.m_intensity);
						should_update |= update_intensity = ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 10.0f);
						if (update_intensity)
						{
							m_spot_lights[i].first.m_intensity = m_spot_lights[i].first.m_intensity > 0 ? intensity : -intensity;
							ref["intensity"s] = m_spot_lights[i].first.m_intensity;
						}

						bool update_direction = false;
						float z = std::sqrt(1.0f - std::pow(m_spot_lights[i].first.m_customs.x, 2) - std::pow(m_spot_lights[i].first.m_customs.y, 2));
						z = m_spot_lights[i].first.m_intensity > 0 ? z : -z;
						glm::vec3 direction = { m_spot_lights[i].first.m_customs.x,m_spot_lights[i].first.m_customs.y,z };
						glm::vec3 old_direction = direction;
						ImGui::Text("Direction:");
						ImGui::NewLine();
						should_update |= update_direction |= ImGui::InputFloat("x:", &direction.x, 0.1f, 1.0f);
						ImGui::NewLine();
						should_update |= update_direction |= ImGui::InputFloat("y:", &direction.y, 0.1f, 1.0f);
						ImGui::NewLine();
						should_update |= update_direction |= ImGui::InputFloat("z:", &direction.z, 0.1f, 1.0f);
						if (direction.x == 0.0 && direction.y == 0.0 && direction.z == 0.0)
						{
							direction = glm::vec3(1.0);
						}
						direction = glm::normalize(direction);
						if (update_direction)
						{
							m_spot_lights[i].first.m_customs.x = direction.x;
							m_spot_lights[i].first.m_customs.y = direction.y;
							if (old_direction.z * direction.z < 0)
							{
								m_spot_lights[i].first.m_intensity = -m_spot_lights[i].first.m_intensity;
								ref["intensity"s] = m_spot_lights[i].first.m_intensity;
							}
						}

						bool update_inner = false;
						float inner_angle = m_spot_lights[i].first.m_customs.z;
						should_update |= update_inner = ImGui::InputFloat("Inner Angle:", &inner_angle, 0.1f, 1.0f);
						ImGui::NewLine();
						bool update_outer = false;
						float outer_angle = m_spot_lights[i].first.m_customs.w;
						should_update |= update_outer = ImGui::InputFloat("Outer Angle:", &outer_angle, 0.1f, 1.0f);
						inner_angle = std::min(std::max(inner_angle, 0.0f), 180.0f);
						outer_angle = std::min(std::max(outer_angle, inner_angle), 180.0f);
						if (update_inner || update_outer)
						{
							m_spot_lights[i].first.m_customs.z = glm::cos(glm::radians(outer_angle));
							m_spot_lights[i].first.m_customs.w = 1.0f / (glm::cos(glm::radians(inner_angle)) - m_spot_lights[i].first.m_customs.z);
						}

						if (update_direction || update_inner || update_outer)
						{
							ref["customs"s] = m_spot_lights[i].first.m_customs;
						}

						if (ImGui::Button("Delete"))
						{
							should_update = true;

							size_t index = m_spot_lights[i].second;
							m_available_indexes.insert(index);
							m_spot_lights.erase(m_spot_lights.begin() + i);

							ref["flags"s] = LIGHT_STATUS_INVALID;
						}
					}
					ImGui::PopID();
				}

				if (ImGui::Button("Add") && m_available_indexes.size())
				{
					should_update = true;

					size_t index = *(m_available_indexes.begin());
					Common::LightCommon spot = Common::LightCommon::ConstructDefaultSpot();
					m_point_lights.push_back({ spot,index });
					m_available_indexes.erase(index);

					auto constant = m_light_buffer->EditConstant("lights")[index];
					constant["position"s] = spot.m_position;
					constant["flags"s] = spot.m_flags;
					constant["color"s] = spot.m_color;
					constant["intensity"s] = spot.m_intensity;
					constant["customs"s] = spot.m_customs;
				}
				ImGui::PopID();
			}

			// Rect Light
			if (ImGui::CollapsingHeader("Rect Lights"))
			{
				// TODO
			}

			if (should_update)
			{
				m_light_buffer->Bind();
				m_light_buffer->Update();
				m_light_buffer->UnBind();
			}

			ImGui::End();
		}
	}

	void DeferRenderer::PrepareAOUI()
	{
		if (ImGui::CollapsingHeader("AO Method", false))
		{
			ImGui::PushID(5);
			ImGui::Indent();
			int ao_method = static_cast<int>(m_ao_method);
			ImGui::RadioButton("Simple SSAO", &ao_method, AO_METHOD_SSAO_SIMPLE);
			ImGui::RadioButton("UE SSAO", &ao_method, AO_METHOD_SSAO_UE);
			ImGui::RadioButton("SSDO", &ao_method, AO_METHOD_SSDO);
			ImGui::RadioButton("SSDO", &ao_method, AO_METHOD_HBAO);
			ImGui::RadioButton("GTAO", &ao_method, AO_METHOD_GTAO);
			ImGui::RadioButton("None", &ao_method, AO_METHOD_NUM);
			m_ao_method = static_cast<uint16_t>(ao_method);
			ImGui::Unindent();
			ImGui::PopID();
		}

		if (g_post_processor.ShouldDisplayBloom())
		{
			ImGui::BeginDisabled();
		}
		ImGui::Checkbox("Display AO", &m_should_display_ao);
		if (m_should_display_ao)
		{
			m_defer_display_mode = DEFER_DISPLAY_MODE_NUM;
		}
		if (g_post_processor.ShouldDisplayBloom())
		{
			ImGui::EndDisabled();
		}

		if (ImGui::CollapsingHeader("AO Extra Parameters", false))
		{
			if (m_ao_method == AO_METHOD_SSAO_SIMPLE)
			{
				ImGui::DragFloat("Simple SSAO Kernel Radius:", &m_ao_extra_block.simple_SSAO_kernel_radius, 0.01f, 0.1f, 1.0f);
				ImGui::NewLine();
				ImGui::DragFloat("Simple SSAO Bias:", &m_ao_extra_block.simple_SSAO_bias, 0.001f, 0.005f, 0.05f);
			}
			else
			{
				// TODO
			}
		}
	}

	void DeferRenderer::RenderPreZ()
	{
		APP_RANGE_BEGIN("PreZ Pass");

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);



		APP_RANGE_END();
	}

	void DeferRenderer::RenderHiZ()
	{

	}

	void DeferRenderer::RenderDefer()
	{
		APP_RANGE_BEGIN("Defer Generation Pass");

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);

		m_scenes[0]->ChangeSet("main_defer");

		auto defer_shader = Bind::ShaderProgram::Resolve("defer_shader", { 0,0 });
		defer_shader->EditUniform("view") = globalSettings::mainCamera.get_view();
		defer_shader->EditUniform("projection") = globalSettings::mainCamera.get_perspective();

		m_scenes[m_scene_index]->Render();

		APP_RANGE_END();
	}

	void DeferRenderer::RenderAO()
	{
		APP_RANGE_BEGIN("AO Pass");
		m_AO_framebuffer->Bind();
		AOGeneration();
		AOFilter();
		m_AO_framebuffer->UnBind();
		RenderAOToScreen();
		APP_RANGE_END();
	}

	void DeferRenderer::AOGeneration()
	{
		if (m_ao_method != AO_METHOD_NUM)
		{
			APP_RANGE_BEGIN("AO Generation Pass");
		}

		m_AO_framebuffer->ChangeTexture(m_AO_texture);

		switch (m_ao_method)
		{
		case AO_METHOD_SSAO_SIMPLE:
		{
			auto simple_SSAO_shader = Bind::ShaderProgram::Resolve("simple_SSAO_shader", { 0,0 });
			simple_SSAO_shader->BindWithoutUpdate();

			m_rt_depthbuffer->Bind();
			m_rt_normal_metallic_roughness->Bind();
			m_SSAO_random_texture->Bind();

			simple_SSAO_shader->EditUniform("view") = globalSettings::mainCamera.get_view();
			simple_SSAO_shader->EditUniform("projection") = globalSettings::mainCamera.get_perspective();
			auto desc = m_AO_texture->get_description();
			simple_SSAO_shader->EditUniform("AO_texture_size") = glm::uvec2(desc.width, desc.height);
			
			auto& handle = simple_SSAO_shader->EditUniform("random_samples");
			for (int i = 0; i < SIMPLE_SSAO_RANDOM_SAMPLE_NUM; i++)
			{
				handle[i] = m_simple_SSAO_random_positions[i];
			}

			simple_SSAO_shader->EditUniform("kernel_radius") = m_ao_extra_block.simple_SSAO_kernel_radius;
			simple_SSAO_shader->EditUniform("bias") = m_ao_extra_block.simple_SSAO_bias;

			simple_SSAO_shader->UpdateOnly();

			Common::RenderHelper::RenderSimpleQuad();

			simple_SSAO_shader->UnBind();
			m_rt_depthbuffer->UnBind();
			m_rt_normal_metallic_roughness->UnBind();
			m_SSAO_random_texture->UnBind();
			break;
		}
		case AO_METHOD_SSAO_UE:
		{

		}
		case AO_METHOD_HBAO:
		{

		}
		case AO_METHOD_GTAO:
		{

		}
		default:
		{
			break;
		}
		}

		if (m_ao_method != AO_METHOD_NUM)
		{
			APP_RANGE_END();
		}
	}

	void DeferRenderer::AOFilter()
	{
		if (m_ao_method != AO_METHOD_NUM)
		{
			APP_RANGE_BEGIN("AO Filter Pass");
		}

		m_AO_framebuffer->ChangeTexture(m_AO_filtered_texture);

		switch (m_ao_method)
		{
		case AO_METHOD_SSAO_SIMPLE:
		{
			auto shader = Bind::ShaderProgram::Resolve("simple_SSAO_filter_shader", { 0,0 });
			shader->Bind();
			m_AO_texture->Bind();

			Common::RenderHelper::RenderSimpleQuad();

			shader->UnBind();
			m_AO_texture->UnBind();
			break;
		}
		case AO_METHOD_SSAO_UE:
		{

		}
		case AO_METHOD_HBAO:
		{

		}
		case AO_METHOD_GTAO:
		{

		}
		default:
		{
			break;
		}
		}

		if (m_ao_method != AO_METHOD_NUM)
		{
			APP_RANGE_END();
		}
	}

	void DeferRenderer::RenderAOToScreen()
	{
		if (m_should_display_ao)
		{
			Common::RenderHelper::RenderTextureToScreen(m_AO_filtered_texture, 8);
		}
	}

	void DeferRenderer::DisplayDefer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);

		switch (m_defer_display_mode)
		{
		case DEFER_DISPLAY_MODE_ALBEDO:
		{
			Common::RenderHelper::RenderTextureToScreen(m_rt_albedo_specular);
			break;
		}
		case DEFER_DISPLAY_MODE_DIFFUSE:
		{
			// TODO
			break;
		}
		case DEFER_DISPLAY_MODE_SPECULAR:
		{
			Common::RenderHelper::RenderTextureToScreen(m_rt_albedo_specular, 1);
			break;
		}
		case DEFER_DISPLAY_MODE_WORLD_POSITION:
		{
			// TODO
			break;
		}
		case DEFER_DISPLAY_MODE_WORLD_NORMAL:
		{
			Common::RenderHelper::RenderUnitVectorToScreen(m_rt_normal_metallic_roughness);
			break;
		}
		case DEFER_DISPLAY_MODE_METALLIC:
		{
			Common::RenderHelper::RenderTextureToScreen(m_rt_normal_metallic_roughness, 2);
			break;
		}
		case DEFER_DISPLAY_MODE_ROUGHNESS:
		{
			Common::RenderHelper::RenderTextureToScreen(m_rt_normal_metallic_roughness, 1);
			break;
		}
		case DEFER_DISPLAY_MODE_NDC_DEPTH:
		{
			Common::RenderHelper::RenderTextureToScreen(m_rt_depthbuffer, 8);
			break;
		}
		case DEFER_DISPLAY_MODE_LINEAR_DEPTH:
		{
			Common::RenderHelper::RenderLinearDepthToScreen(m_rt_depthbuffer, globalSettings::mainCamera.zNear, globalSettings::mainCamera.zFar);
			break;
		}
		case DEFER_DISPLAY_MODE_ANISOTROPHY:
		{
			Common::RenderHelper::RenderTextureToScreen(m_rt_position_anisotrophy, 1);
			break;
		}
		case DEFER_DISPLAY_MODE_PBR_LIGHTING:
		{
			Common::RenderHelper::RenderTextureToScreen(m_lighting_texture);
			break;
		}
		default:
		{
			Common::RenderHelper::RenderTextureToScreen(g_post_processor.m_output_texture);
		}
		}

		return;
	}

	void DeferRenderer::DeferLighting()
	{
		APP_RANGE_BEGIN("defer_lighting");

		m_defer_lighting_framebuffer->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glDisable(GL_CULL_FACE);

		auto defer_lighting_shader = Bind::ShaderProgram::Resolve("defer_lighting_shader", { 0,0 });
		defer_lighting_shader->BindWithoutUpdate();
		defer_lighting_shader->EditUniform("cameraPos") = globalSettings::mainCamera.get_position();
		defer_lighting_shader->UpdateOnly();

		m_rt_position_anisotrophy->Bind();
		m_rt_albedo_specular->Bind();
		m_rt_normal_metallic_roughness->Bind();
		std::shared_ptr<Bind::ImageTexture2D> ao_input;
		if (m_ao_method != AO_METHOD_NUM)
		{
			ao_input = m_AO_filtered_texture;
		}
		else
		{
			ao_input = m_default_white_texture;
		}
		ao_input->Bind();

		m_light_buffer->Bind();

		Common::RenderHelper::RenderSimpleQuad();

		m_light_buffer->UnBind();
		m_rt_position_anisotrophy->UnBind();
		m_rt_albedo_specular->UnBind();
		m_rt_normal_metallic_roughness->UnBind();
		ao_input->UnBind();
		defer_lighting_shader->UnBind();
		m_defer_lighting_framebuffer->UnBind();

		APP_RANGE_END();

		return;
	}
}
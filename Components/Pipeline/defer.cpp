#include <Pipeline/defer.h>
#include <Pipeline/postprocess.h>
#include <Macros/gfxdebug_helper.h>
#include <Bindables/rendertarget.h>
#include <Bindables/shaderprogram.h>
#include <Bindables/constantbuffer.h>
#include <Common/render_helper.h>
#include <assimploader.h>

namespace OGLPipeline
{
	DeferRenderer::DeferRenderer(const std::string& scene_name, const std::string ui_name)
		:Utils(scene_name), m_ui_name(ui_name)
	{
		globalSettings::mainCamera.set_perspective(60.0f, globalSettings::screen_width, globalSettings::screen_height, 0.1f, 512.0f);
		globalSettings::mainCamera.set_position(glm::vec3(0.0f, 0.0f, 4.0f));
	}

	void DeferRenderer::render()
	{
		RenderDefer();
		DeferLighting();
		DisplayDefer();
		g_post_processor.MainProcessor(this);
	}

	// TODO ：屏幕尺寸变化处理逻辑后续补充
	void DeferRenderer::prepare()
	{
		m_scene = AssimpLoader::LoadModel("Sponza", "sponza.obj", m_main_scene);

		m_defer_framebuffer = Bind::RenderTarget::Resolve("defer_framebuffer", globalSettings::screen_width, globalSettings::screen_height);
		m_defer_framebuffer->AppendTexture<GL_TEXTURE_2D>("rt_postion_anisotrophy", {}, 1, 1, GL_RGBA16F)
			.AppendTexture<GL_TEXTURE_2D>("rt_albedo_specular", {}, 1, 1, GL_RGBA8)
			.AppendTexture<GL_TEXTURE_2D>("rt_normal_metallic_roughness", {}, 1, 1, GL_RGBA16F)
			.AppendDepthComponent<GL_TEXTURE_2D>("rt_depth", 1, GL_DEPTH_COMPONENT32F).CheckCompleteness();
		m_scene->AddRootBindable(m_defer_framebuffer);

		m_rt_albedo_specular = m_defer_framebuffer->get_texture_image<Bind::ImageTexture2D>("albe_spec_texture", "rt_albedo_specular", {}, 1);
		OGL_TEXTURE_PARAMETER params;
		params.min_filter = GL_NEAREST;
		params.mag_filter = GL_NEAREST;
		m_rt_position_anisotrophy = m_defer_framebuffer->get_texture_image<Bind::ImageTexture2D>("pos_aniso_texture", "rt_postion_anisotrophy", params, 0);
		m_rt_normal_metallic_roughness = m_defer_framebuffer->get_texture_image<Bind::ImageTexture2D>("norm_mr_texture", "rt_normal_metallic_roughness", params, 2);
		m_rt_depthbuffer = m_defer_framebuffer->get_texture_depthstencil<Bind::ImageTexture2D>("depth_tex", {}, 0);

		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "defer_vertex", "Common", "defer.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "defer_fragment", "Common", "defer.frag");
		auto defer_shader = Bind::ShaderProgram::Resolve("defer_shader", std::vector<GLuint>{vertex, fragment});
		m_scene->AddRootBindable(defer_shader);

		m_scene->AddRootUniformRule<SceneGraph::ConfigurationType::Transformation>(defer_shader->EditUniform("model").GetLeafUniform(),
			[](glm::mat4 model)->glm::mat4 {return model; })
			.AddRootUniformRule<SceneGraph::ConfigurationType::MaterialMetallic>(defer_shader->EditUniform("metallic").GetLeafUniform(),
				[](float metallic)->float {return metallic; })
			.AddRootUniformRule<SceneGraph::ConfigurationType::MaterialRoughness>(defer_shader->EditUniform("roughness").GetLeafUniform(),
				[](float roughness)->float {return roughness; });
		std::vector<DrawItems::VertexType> instruction{ DrawItems::VertexType::Position,DrawItems::VertexType::Normal,DrawItems::VertexType::Tangent,
			DrawItems::VertexType::Texcoord };
		m_scene->AddRootVertexRule(instruction);
		m_scene->AddRootTextureRule("diffuse_tex", 0, SceneGraph::Material::TextureCategory::DIFFUSE).
			AddRootTextureRule("specular_tex", 1, SceneGraph::Material::TextureCategory::SPECULAR).
			AddRootTextureRule("normal_tex", 2, SceneGraph::Material::TextureCategory::NORMAL);
		m_scene->ScaleModel(glm::vec3(0.2f));

		m_scene->Cook();

		PrepareDeferLighting();
		PrepareLightBuffer();
		g_post_processor.PrepareAA();
	}

	void DeferRenderer::prepare_ui(const std::string& name)
	{
		ImGui::Begin(name.c_str());

		// Defer RenderTargets Display
		if (ImGui::CollapsingHeader("Defer Display Mode", false))
		{
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
		}

		// Anti Aliasing
		if (ImGui::CollapsingHeader("Anti Aliasing", false))
		{
			ImGui::PushID(2);
			ImGui::Indent();
			int aamethod = static_cast<int>(g_post_processor.m_gui_block.anti_aliasing_method);
			ImGui::RadioButton("MSAA", &aamethod, ANTI_ALIASING_METHOD_MSAA);
			ImGui::RadioButton("FXAA", &aamethod, ANTI_ALIASING_METHOD_FXAA);
			ImGui::RadioButton("TAA", &aamethod, ANTI_ALIASING_METHOD_TAA);
			ImGui::RadioButton("None", &aamethod, ANTI_ALIASING_METHOD_NUM);
			g_post_processor.m_gui_block.anti_aliasing_method = static_cast<uint16_t>(aamethod);
			ImGui::Unindent();
			ImGui::PopID();
		}

		PrepareLightUI();

		ImGui::End();
	}

	void DeferRenderer::PrepareDeferLighting()
	{
		m_defer_lighting_framebuffer = std::make_shared<Bind::RenderTarget>("defer_lighting_framebuffer", globalSettings::screen_width, globalSettings::screen_height);
		m_defer_lighting_framebuffer->AppendTexture<GL_TEXTURE_2D>("defer_lighting_texture", {}, 1, 1, GL_RGBA16F).CheckCompleteness();

		m_lighting_texture = m_defer_lighting_framebuffer->get_texture_image<Bind::ImageTexture2D>("defer_shading", "defer_lighting_texture", {}, 0);

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
				should_update |= update_intensity = ImGui::DragFloat("Intensity", &m_main_light.m_intensity, 0.1f, 0.0f, 10.0f);
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

	void DeferRenderer::RenderDefer()
	{
		APP_RANGE_BEGIN("generate_defer");

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);

		auto defer_shader = Bind::ShaderProgram::Resolve("defer_shader", { 0,0 });
		defer_shader->EditUniform("view") = globalSettings::mainCamera.get_view();
		defer_shader->EditUniform("projection") = globalSettings::mainCamera.get_perspective();

		m_scene->Render();

		APP_RANGE_END();
	}

	void DeferRenderer::DisplayDefer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0, 0, 0, 1);
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
			break;
		}
		}

		return;
	}

	void DeferRenderer::DeferLighting()
	{
		APP_RANGE_BEGIN("defer_lighting");

		m_defer_lighting_framebuffer->Bind();
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glDisable(GL_CULL_FACE);

		auto defer_lighting_shader = Bind::ShaderProgram::Resolve("defer_lighting_shader", { 0,0 });
		defer_lighting_shader->BindWithoutUpdate();
		defer_lighting_shader->EditUniform("cameraPos") = globalSettings::mainCamera.get_position();
		defer_lighting_shader->UpdateOnly();

		m_rt_position_anisotrophy->Bind();
		m_rt_albedo_specular->Bind();
		m_rt_normal_metallic_roughness->Bind();

		m_light_buffer->Bind();

		Common::RenderHelper::RenderSimpleQuad();

		m_light_buffer->UnBind();
		m_rt_position_anisotrophy->UnBind();
		m_rt_albedo_specular->UnBind();
		m_rt_normal_metallic_roughness->UnBind();
		defer_lighting_shader->UnBind();
		m_defer_lighting_framebuffer->UnBind();

		APP_RANGE_END();

		return;
	}
}
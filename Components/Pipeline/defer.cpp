#include <Pipeline/defer.h>
#include <Pipeline/postprocess.h>
#include <Macros/gfxdebug_helper.h>
#include <Bindables/rendertarget.h>
#include <Bindables/shaderprogram.h>
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
		DisplayDefer();
		g_post_processor.MainProcessor();
	}

	// TODO ：屏幕尺寸变化处理逻辑后续补充
	void DeferRenderer::prepare()
	{
		m_scene = AssimpLoader::LoadModel("Sponza", "sponza.obj", m_main_scene);

		m_defer_framebuffer = std::make_shared<Bind::RenderTarget>("defer_framebuffer", globalSettings::screen_width, globalSettings::screen_height);
		m_defer_framebuffer->AppendTexture<GL_TEXTURE_2D>("rt_postion_anisotrophy", {}, 1, 1, GL_RGBA16F)
			.AppendTexture<GL_TEXTURE_2D>("rt_albedo_specular", {}, 1, 1, GL_RGBA8)
			.AppendTexture<GL_TEXTURE_2D>("rt_normal_metallic_roughness", {}, 1, 1, GL_RGBA16F)
			.AppendDepthComponent<GL_TEXTURE_2D>("rt_depth", 1, GL_DEPTH_COMPONENT32F).CheckCompleteness();
		m_scene->AddRootBindable(m_defer_framebuffer);

		m_rt_position_anisotrophy = m_defer_framebuffer->get_texture_image<Bind::ImageTexture2D>("rt_postion_anisotrophy", {}, 0);
		m_rt_normal_metallic_roughness = m_defer_framebuffer->get_texture_image<Bind::ImageTexture2D>("rt_normal_metallic_roughness", {}, 0);
		m_rt_albedo_specular = m_defer_framebuffer->get_texture_image<Bind::ImageTexture2D>("rt_albedo_specular", {}, 0);
		m_rt_depthbuffer = m_defer_framebuffer->get_texture_depthstencil<Bind::ImageTexture2D>({}, 0);

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

		ImGui::End();
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
		default:
		{
			break;
		}
		}

		return;
	}
}
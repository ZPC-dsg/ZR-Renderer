#include <Pipeline/postprocess.h>
#include <Macros/gfxdebug_helper.h>
#include <Bindables/rendertarget.h>
#include <Bindables/shaderprogram.h>
#include <Pipeline/defer.h>
#include <Common/render_helper.h>
#include <init.h>

namespace OGLPipeline
{
	PostProcessor g_post_processor = PostProcessor::Get();

	void PostProcessor::MainProcessor(DeferRenderer* main_renderer)
	{
		APP_RANGE_BEGIN("post_process");

		glDisable(GL_CULL_FACE);
		AntiAliasing(main_renderer->m_lighting_texture);

		APP_RANGE_END();
	}

	void PostProcessor::SetOption(PostProcessOptions category, uint16_t option)
	{
		switch (category)
		{
		case AntiAliasingMethod: m_gui_block.anti_aliasing_method = option; break;
		}
	}
	
	void PostProcessor::PrepareUI()
	{
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

		if (g_post_processor.m_gui_block.anti_aliasing_method != ANTI_ALIASING_METHOD_FXAA)
		{
			ImGui::BeginDisabled();
		}
		ImGui::DragFloat("FXAA Threshold Min", &g_post_processor.m_gui_block.FXAA_threshold_min, 0.001f, 0.03f, 0.1f);
		ImGui::NewLine();
		ImGui::DragFloat("FXAA Threshold", &g_post_processor.m_gui_block.FXAA_threshold, 0.001f, 0.05f, 0.4f);
		if (g_post_processor.m_gui_block.anti_aliasing_method != ANTI_ALIASING_METHOD_FXAA)
		{
			ImGui::EndDisabled();
		}
	}

	void PostProcessor::PrepareAA()
	{
		m_AA_framebuffer = Bind::RenderTarget::Resolve("AA_framebuffer", globalSettings::screen_width, globalSettings::screen_height);
		m_AA_framebuffer->AppendTexture<GL_TEXTURE_2D>("AA_texture", {}, 1, 1, GL_RGBA16F).CheckCompleteness();

		// FXAA
		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "FXAA_vertex", "Common", "defer_shading.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "FXAA_fragment", "AA", "FXAA.frag");
		auto FXAA_shader = Bind::ShaderProgram::Resolve("FXAA_shader", std::vector<GLuint>{vertex, fragment});
	}

	void PostProcessor::AntiAliasing(std::shared_ptr<Bind::ImageTexture2D> scene_color)
	{
		switch (m_gui_block.anti_aliasing_method)
		{
		case 0:
		case 1:
		{
			m_AA_framebuffer->Bind();
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			auto shader = Bind::ShaderProgram::Resolve("FXAA_shader", { 0,0 });
			shader->BindWithoutUpdate();
			shader->UpdateOnly();

			GLuint binding = scene_color->GetBindingPoint();
			scene_color->ChangeBindingPoint(0);
			scene_color->Bind();

			Common::RenderHelper::RenderSimpleQuad();

			scene_color->UnBind();
			scene_color->ChangeBindingPoint(binding);
			shader->UnBind();
			m_AA_framebuffer->UnBind();
		}
		case 2:
		default:
			break;
		}

		return;
	}
}
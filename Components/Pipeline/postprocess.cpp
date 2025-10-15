#include <Pipeline/postprocess.h>
#include <Macros/gfxdebug_helper.h>
#include <Bindables/rendertarget.h>
#include <Bindables/shaderprogram.h>
#include <Bindables/Sampler.h>
#include <Pipeline/defer.h>
#include <Common/render_helper.h>
#include <init.h>

namespace OGLPipeline
{
	PostProcessor g_post_processor = PostProcessor::Get();

	void PostProcessor::Accept(DeferRenderer* renderer)
	{
		m_renderer = renderer;
	}

	void PostProcessor::PreparePostProcess()
	{
		PrepareAA();
		PrepareBloom();
		PrepareToneGamma();
	}

	void PostProcessor::MainProcessor()
	{
		APP_RANGE_BEGIN("post_process");
		glDisable(GL_CULL_FACE);

		AntiAliasing();
		Bloom();
		ToneMappingAndGammaCorrection();

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
		ImGui::NewLine();
		ImGui::DragFloat("FXAA Subpixel Quality", &g_post_processor.m_gui_block.FXAA_subpixel_quality, 0.01f, 0.0f, 1.0f);
		if (g_post_processor.m_gui_block.anti_aliasing_method != ANTI_ALIASING_METHOD_FXAA)
		{
			ImGui::EndDisabled();
		}
	}

	void PostProcessor::PrepareAA()
	{
		m_postprocess_framebuffer = Bind::RenderTarget::Resolve("postprocess_framebuffer", globalSettings::screen_width, globalSettings::screen_height);
		m_postprocess_framebuffer->AppendTexture<GL_TEXTURE_2D>("AA_texture", {}, 1, 1, GL_RGBA16F).CheckCompleteness();
		m_AA_texture = m_postprocess_framebuffer->get_texture_image<Bind::ImageTexture2D>("AA_result_texture", 0, {}, 0);

		// FXAA
		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "FXAA_vertex", "Common", "defer_shading.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "FXAA_fragment", "AA", "FXAA.frag");
		auto FXAA_shader = Bind::ShaderProgram::Resolve("FXAA_shader", std::vector<GLuint>{vertex, fragment});
	}

	void PostProcessor::AntiAliasing()
	{
		APP_RANGE_BEGIN("Anti Aliasing");
		m_postprocess_framebuffer->Bind();
		m_postprocess_framebuffer->ChangeTexture(m_AA_texture);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		switch (m_gui_block.anti_aliasing_method)
		{
		case 0:
			break;
		case 1:
		{
			auto shader = Bind::ShaderProgram::Resolve("FXAA_shader", { 0,0 });
			shader->BindWithoutUpdate();
			shader->EditUniform("luma_threshold") = m_gui_block.FXAA_threshold;
			shader->EditUniform("luma_threshold_min") = m_gui_block.FXAA_threshold_min;
			shader->EditUniform("subpixel_quality") = m_gui_block.FXAA_subpixel_quality;
			shader->UpdateOnly();

			GLuint binding = m_renderer->m_lighting_texture->GetBindingPoint();
			m_renderer->m_lighting_texture->ChangeBindingPoint(0);
			m_renderer->m_lighting_texture->Bind();
			m_renderer->m_bilinear_sampler->Bind();
			m_renderer->m_lighting_texture->ChangeBindingPoint(2);
			m_renderer->m_lighting_texture->Bind();
			m_renderer->m_point_sampler->Bind();

			Common::RenderHelper::RenderSimpleQuad();

			m_renderer->m_lighting_texture->UnBind();
			m_renderer->m_lighting_texture->ChangeBindingPoint(binding);
			m_renderer->m_point_sampler->UnBind();
			m_renderer->m_bilinear_sampler->UnBind();
			shader->UnBind();
			break;
		}
		case 2:
		default:
			break;
		}

		APP_RANGE_END();
		return;
	}

	void PostProcessor::PrepareBloom()
	{

	}

	void PostProcessor::Bloom()
	{

	}

	void PostProcessor::PrepareToneGamma()
	{
		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "tone_gamma_vertex", "Common", "defer_shading.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "tone_gamma_fragment", "Common", "tone_gamma.frag");
		auto tone_gamma_shader = Bind::ShaderProgram::Resolve("tone_gamma_shader", std::vector<GLuint>{vertex, fragment});

		OGL_TEXTURE2D_DESC desc = m_AA_texture->get_description();
		m_tone_gamma_texture = Bind::ImageTexture2D::Resolve("tone_gamma_texture", desc, {}, 0);
	}

	void PostProcessor::ToneMappingAndGammaCorrection()
	{
		APP_RANGE_BEGIN("Tone Mapping & Gamma Correction");

		m_postprocess_framebuffer->ChangeTexture(m_tone_gamma_texture);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto tone_gamma_shader = Bind::ShaderProgram::Resolve("tone_gamma_shader", { 0,0 });
		tone_gamma_shader->Bind();
		m_AA_texture->Bind();

		Common::RenderHelper::RenderSimpleQuad();

		m_AA_texture->UnBind();
		tone_gamma_shader->UnBind();

		APP_RANGE_END();
		return;
	}
}
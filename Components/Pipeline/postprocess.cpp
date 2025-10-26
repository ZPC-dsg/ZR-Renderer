#include <Pipeline/postprocess.h>
#include <Macros/gfxdebug_helper.h>
#include <Bindables/rendertarget.h>
#include <Bindables/shaderprogram.h>
#include <Bindables/Sampler.h>
#include <Pipeline/defer.h>
#include <Common/render_helper.h>
#include <init.h>

#include <algorithm>

#define BLOOM_MAX_FILTER_SAMPLES 32

namespace OGLPipeline
{
	PostProcessor g_post_processor = PostProcessor::Get();

	void PostProcessor::Accept(DeferRenderer* renderer)
	{
		m_renderer = renderer;
	}

	void PostProcessor::PreparePostProcess()
	{
		m_postprocess_framebuffer = Bind::RenderTarget::Resolve("postprocess_framebuffer", globalSettings::screen_width, globalSettings::screen_height);
		PrepareAA();
		PrepareBloom();
		PrepareToneGamma();
	}

	void PostProcessor::MainProcessor()
	{
		APP_RANGE_BEGIN("post_process");

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		m_postprocess_framebuffer->Bind();
		TAA();
		Bloom();

		if (!m_gui_block.bloom_filter_display_level && !m_gui_block.bloom_downsample_display_level)
		{
			ToneMapping();
			FXAA();
			GammaCorrection();
		}

		m_postprocess_framebuffer->UnBind();
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
			int aamethod = static_cast<int>(m_gui_block.anti_aliasing_method);
			ImGui::RadioButton("MSAA", &aamethod, ANTI_ALIASING_METHOD_MSAA);
			ImGui::RadioButton("FXAA", &aamethod, ANTI_ALIASING_METHOD_FXAA);
			ImGui::RadioButton("TAA", &aamethod, ANTI_ALIASING_METHOD_TAA);
			ImGui::RadioButton("None", &aamethod, ANTI_ALIASING_METHOD_NUM);
			m_gui_block.anti_aliasing_method = static_cast<uint16_t>(aamethod);
			ImGui::Unindent();
			ImGui::PopID();
		}

		// FXAA
		if (m_gui_block.anti_aliasing_method != ANTI_ALIASING_METHOD_FXAA)
		{
			ImGui::BeginDisabled();
		}
		ImGui::DragFloat("FXAA Threshold Min", &m_gui_block.FXAA_threshold_min, 0.001f, 0.03f, 0.1f);
		ImGui::NewLine();
		ImGui::DragFloat("FXAA Threshold", &m_gui_block.FXAA_threshold, 0.001f, 0.05f, 0.4f);
		ImGui::NewLine();
		ImGui::DragFloat("FXAA Subpixel Quality", &m_gui_block.FXAA_subpixel_quality, 0.01f, 0.0f, 1.0f);
		if (m_gui_block.anti_aliasing_method != ANTI_ALIASING_METHOD_FXAA)
		{
			ImGui::EndDisabled();
		}

		// Bloom
		if (ImGui::CollapsingHeader("Bloom", false))
		{
			ImGui::PushID(3);
			ImGui::Indent();
			ImGui::Text("Downsample Level:");
			ImGui::NewLine();
			int downsample_level = static_cast<int>(m_gui_block.bloom_downsample_display_level);
			ImGui::RadioButton("None", &downsample_level, 0);
			for (int i = 1; i <= 6; i++)
			{
				std::string tag = "Level " + std::to_string(i);
				ImGui::RadioButton(tag.c_str(), &downsample_level, i);
			}
			m_gui_block.bloom_downsample_display_level = downsample_level;
			ImGui::Unindent();
			ImGui::PopID();

			ImGui::NewLine();

			ImGui::PushID(4);
			ImGui::Indent();
			ImGui::Text("Filter Level:");
			ImGui::NewLine();
			int filter_level = static_cast<int>(m_gui_block.bloom_filter_display_level);
			ImGui::RadioButton("None", &filter_level, 0);
			for (int i = 1; i <= 6; i++)
			{
				std::string tag = "Level " + std::to_string(i);
				ImGui::RadioButton(tag.c_str(), &filter_level, i);
			}
			m_gui_block.bloom_filter_display_level = filter_level;
			ImGui::Unindent();
			ImGui::PopID();

			if (downsample_level || filter_level)
			{
				m_renderer->m_defer_display_mode = DEFER_DISPLAY_MODE_NUM;
				m_renderer->m_should_display_ao = false;
			}

			if (ImGui::CollapsingHeader("Bloom Kernel Sizes(Percentage)", false))
			{
				for (int i = 0; i < 6; i++)
				{
					std::string tag = "Stage " + std::to_string(i + 1);
					ImGui::DragFloat(tag.c_str(), &m_gui_block.bloom_filter_kernel_sizes[i], 0.01f, 0.1f, 75.0f);
					ImGui::NewLine();
				}
			}
			if (ImGui::CollapsingHeader("Bloom Tint Weights", false))
			{
				for (int i = 0; i < 6; i++)
				{
					float colors[3] = { m_gui_block.bloom_tint_weights[i].x,m_gui_block.bloom_tint_weights[i].y,m_gui_block.bloom_tint_weights[i].z };
					std::string tag = "Stage " + std::to_string(i + 1);
					ImGui::ColorEdit3(tag.c_str(), colors);
					ImGui::NewLine();
					m_gui_block.bloom_tint_weights[i] = { colors[0],colors[1],colors[2] };
				}
			}
		}

		// Tone Mapping
		if (ImGui::CollapsingHeader("Tone Mapping", false))
		{
			ImGui::DragFloat("Exposure", &m_gui_block.tone_mapping_exposure, 0.1f, 0.1f, 10.0f);
		}
	}

	void PostProcessor::OnResize()
	{
		OGL_TEXTURE2D_DESC desc;
		desc.width = globalSettings::screen_width;
		desc.height = globalSettings::screen_height;
		desc.target = GL_TEXTURE_2D;
		desc.internal_format = GL_RGBA16F;
		desc.cpu_format = GL_RGBA;
		desc.data_type = GL_FLOAT;

		m_AA_texture->DestroyAndCreateNew(desc);
		m_tonemapping_texture->DestroyAndCreateNew(desc);
		m_gammacorrection_texture->DestroyAndCreateNew(desc);
		m_output_texture->DestroyAndCreateNew(desc);

		std::array<std::pair<unsigned int, unsigned int>, 6> tex_sizes;
		tex_sizes[0].first = (globalSettings::screen_width + 1) / 2;
		tex_sizes[0].second = (globalSettings::screen_height + 1) / 2;
		for (int i = 1; i < tex_sizes.size(); i++)
		{
			tex_sizes[i].first = (tex_sizes[i - 1].first + 1) / 2;
			tex_sizes[i].second = (tex_sizes[i - 1].second + 1) / 2;
		}
		for (int i = 0; i < 6; i++)
		{
			desc.width = tex_sizes[i].first;
			desc.height = tex_sizes[i].second;
			m_bloom_downsample_chain[i]->DestroyAndCreateNew(desc);
			m_bloom_horizon_filter_chain[i]->DestroyAndCreateNew(desc);
		}
	}

	bool PostProcessor::ShouldDisplayBloom()
	{
		return m_gui_block.bloom_downsample_display_level > 0 || m_gui_block.bloom_filter_display_level > 0;
	}

	void PostProcessor::PrepareAA()
	{
		OGL_TEXTURE2D_DESC desc;
		desc.width = globalSettings::screen_width;
		desc.height = globalSettings::screen_height;
		desc.target = GL_TEXTURE_2D;
		desc.internal_format = GL_RGBA16F;
		desc.cpu_format = GL_RGBA;
		desc.data_type = GL_FLOAT;
		OGL_TEXTURE_PARAMETER param;
		param.wrap_x = GL_CLAMP_TO_EDGE;
		param.wrap_y = GL_CLAMP_TO_EDGE;
		m_AA_texture = Bind::ImageTexture2D::Resolve("FXAA_texture", desc, param, 0);

		// FXAA
		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "FXAA_vertex", "Common", "defer_shading.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "FXAA_fragment", "AA", "FXAA.frag");
		auto FXAA_shader = Bind::ShaderProgram::Resolve("FXAA_shader", std::vector<GLuint>{vertex, fragment});
	}

	void PostProcessor::TAA()
	{
		if (m_gui_block.anti_aliasing_method == ANTI_ALIASING_METHOD_TAA)
		{
			APP_RANGE_BEGIN("TAA Pass");
			m_postprocess_framebuffer->ChangeTexture(m_AA_texture);

			// TODO : 还没有实现TAA逻辑，暂时将其设置为原来的defer lighting 纹理
			m_AA_texture->CopyImage(m_renderer->m_lighting_texture);

			APP_RANGE_END();
		}
	}

	void PostProcessor::FXAA()
	{
		if (m_gui_block.anti_aliasing_method == ANTI_ALIASING_METHOD_FXAA)
		{
			APP_RANGE_BEGIN("FXAA Pass");
			m_postprocess_framebuffer->ChangeTexture(m_AA_texture);

			auto shader = Bind::ShaderProgram::Resolve("FXAA_shader", { 0,0 });
			shader->BindWithoutUpdate();
			shader->EditUniform("luma_threshold") = m_gui_block.FXAA_threshold;
			shader->EditUniform("luma_threshold_min") = m_gui_block.FXAA_threshold_min;
			shader->EditUniform("subpixel_quality") = m_gui_block.FXAA_subpixel_quality;
			shader->UpdateOnly();

			m_tonemapping_texture->Bind();
			m_renderer->m_bilinear_sampler->Bind();
			m_tonemapping_texture->ChangeBindingPoint(2);
			m_tonemapping_texture->Bind();
			m_renderer->m_point_sampler->Bind();

			Common::RenderHelper::RenderSimpleQuad();

			m_tonemapping_texture->UnBind();
			m_tonemapping_texture->ChangeBindingPoint(0);
			m_renderer->m_point_sampler->UnBind();
			m_renderer->m_bilinear_sampler->UnBind();
			shader->UnBind();

			APP_RANGE_END();
		}
	}

	void PostProcessor::PrepareBloom()
	{
		PrepareBloomDownSample();
		PrepareBloomFilter();
	}
	
	void PostProcessor::PrepareBloomDownSample()
	{
		std::array<std::pair<unsigned int, unsigned int>, 6> tex_sizes;
		tex_sizes[0].first = (globalSettings::screen_width + 1) / 2;
		tex_sizes[0].second = (globalSettings::screen_height + 1) / 2;
		for (int i = 1; i < tex_sizes.size(); i++)
		{
			tex_sizes[i].first = (tex_sizes[i - 1].first + 1) / 2;
			tex_sizes[i].second= (tex_sizes[i - 1].second + 1) / 2;
		}

		OGL_TEXTURE2D_DESC desc;
		desc.target = GL_TEXTURE_2D;
		desc.internal_format = GL_RGBA16F;
		desc.cpu_format = GL_RGBA;
		desc.data_type = GL_FLOAT;
		OGL_TEXTURE_PARAMETER param;
		param.wrap_x = GL_CLAMP_TO_EDGE;
		param.wrap_y = GL_CLAMP_TO_EDGE;
		for (int i = 0; i < 6; i++)
		{
			desc.width = tex_sizes[i].first;
			desc.height = tex_sizes[i].second;
			std::string tag = "bloom_downsample_" + std::to_string(i);
			m_bloom_downsample_chain[i] = Bind::ImageTexture2D::Resolve(tag, desc, param, 0);
		}

		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "bloom_downsample_vertex", "Common", "defer_shading.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "bloom_downsample_fragment", "Bloom", "downsamplebloom.frag");
		auto bloom_downsample_shader = Bind::ShaderProgram::Resolve("bloom_downsample_shader", { vertex,fragment });
	}

	void PostProcessor::PrepareBloomFilter()
	{
		GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "bloom_filter_vertex", "Bloom", "bloom_filter.vert");
		GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "bloom_filter_fragment", "Bloom", "bloom_filter.frag");
		auto bloom_filter_shader = Bind::ShaderProgram::Resolve("bloom_filter_shader", { vertex,fragment });

		for (int i = 0; i < 6; i++)
		{
			auto desc = m_bloom_downsample_chain[i]->get_description();
			auto param = m_bloom_downsample_chain[i]->get_parameter();
			std::string tag = "bloom_horizon_filter_" + std::to_string(i);
			m_bloom_horizon_filter_chain[i] = Bind::ImageTexture2D::Resolve(tag, desc, param, 0);
		}
	}

	void PostProcessor::Bloom()
	{
		APP_RANGE_BEGIN("Bloom Pass");

		APP_RANGE_BEGIN("Bloom Downsample Pass")
		BloomDownSample();
		APP_RANGE_END();
		if (m_gui_block.bloom_downsample_display_level > 0)
		{
			m_output_texture = m_bloom_downsample_chain[m_gui_block.bloom_downsample_display_level - 1];
			glViewport(0, 0, globalSettings::screen_width, globalSettings::screen_height);
			APP_RANGE_END();
			return;
		}

		APP_RANGE_BEGIN("Bloom Filter Pass");
		BloomFilter();
		APP_RANGE_END();
		if (m_gui_block.bloom_filter_display_level > 0)
		{
			m_output_texture = m_bloom_downsample_chain[m_gui_block.bloom_filter_display_level - 1];
			glViewport(0, 0, globalSettings::screen_width, globalSettings::screen_height);
			APP_RANGE_END();
			return;
		}

		glViewport(0, 0, globalSettings::screen_width, globalSettings::screen_height);
		APP_RANGE_END();
	}

	void PostProcessor::BloomDownSample()
	{
		auto bloom_downsample_shader = Bind::ShaderProgram::Resolve("bloom_downsample_shader", { 0,0 });
		bloom_downsample_shader->Bind();

		m_postprocess_framebuffer->ChangeTexture(m_bloom_downsample_chain[0]);
		auto desc = m_bloom_downsample_chain[0]->get_description();
		glViewport(0, 0, desc.width, desc.height);
		std::shared_ptr<Bind::ImageTexture2D> input_texture;
		/*
		if (m_gui_block.anti_aliasing_method == ANTI_ALIASING_METHOD_FXAA)
		{
			input_texture = m_renderer->m_lighting_texture;
		}
		else
		{
			input_texture = m_AA_texture;
		}
		*/
		// TODO : 暂时没有实现TAA，用下面代码暂时代替
		input_texture = m_renderer->m_lighting_texture;
		input_texture->Bind();

		Common::RenderHelper::RenderSimpleQuad();

		input_texture->UnBind();

		for (int i = 1; i < 6; i++)
		{
			m_postprocess_framebuffer->ChangeTexture(m_bloom_downsample_chain[i]);
			desc = m_bloom_downsample_chain[i]->get_description();
			glViewport(0, 0, desc.width, desc.height);
			m_bloom_downsample_chain[i - 1]->Bind();

			Common::RenderHelper::RenderSimpleQuad();

			m_bloom_downsample_chain[i - 1]->UnBind();
		}
	
		bloom_downsample_shader->UnBind();
	}

	void PostProcessor::BloomFilter()
	{
		std::shared_ptr<Bind::ImageTexture2D> additive = nullptr;

		auto GetBlurRadius = [](unsigned int screen_width, float kernel_percent)->float
			{
				const float percent_to_scale = 0.01;
				const float diameter_to_radius = 0.5;
				return static_cast<float>(screen_width) * kernel_percent * percent_to_scale * diameter_to_radius;
			};
		
		glm::vec2 offsets_and_weights[BLOOM_MAX_FILTER_SAMPLES];

		auto ComputeGaussianWeight = [](float X, float sigma)->float
			{
				X = std::abs(X);
				const float k = -16.7f;

				return std::exp(k * std::pow(X / sigma, 2.0f));
			};

		auto ComputeOffsetsAndWeights = [&](float kernel_size)->int
			{
				const float clamped_kernel_size = std::clamp(kernel_size, 0.1f, (float)BLOOM_MAX_FILTER_SAMPLES - 1.0f);
				int clamped_integer_radius = std::min(static_cast<int>(std::ceil(clamped_kernel_size)), BLOOM_MAX_FILTER_SAMPLES - 1);

				int sample_count = 0;
				float weights = 0;

				for (int i = -clamped_integer_radius; i <= clamped_integer_radius; i += 2)
				{
					float weight0 = ComputeGaussianWeight((float)i, clamped_kernel_size);
					float weight1 = 0.0f;
					if (i != clamped_integer_radius)
					{
						weight1 = ComputeGaussianWeight((float)(i + 1), clamped_kernel_size);
					}

					float total_weight = weight0 + weight1;
					offsets_and_weights[sample_count].x = i + weight1 / total_weight;
					offsets_and_weights[sample_count].y = total_weight;

					weights += total_weight;
					sample_count++;
				}

				float weight_inverse = 1.0f / weights;
				for (int i = 0; i < sample_count; i++)
				{
					offsets_and_weights[i].y *= weight_inverse;
				}

				return sample_count;
			};

		auto filter_shader = Bind::ShaderProgram::Resolve("bloom_filter_shader", { 0,0 });
		filter_shader->BindWithoutUpdate();
		auto GaussianBloom = [&, this](glm::vec4 tint_color, float kernel_size_percent, int stage)->void
			{
				const float blur_radius = GetBlurRadius(globalSettings::screen_width, kernel_size_percent);
				const glm::vec2 inverse_filter_extent = glm::vec2(1.0 / static_cast<float>(globalSettings::screen_width), 1.0 / static_cast<float>(globalSettings::screen_height));

				auto desc = m_bloom_horizon_filter_chain[5 - stage]->get_description();
				glViewport(0, 0, desc.width, desc.height);
				auto& offsets_handle = filter_shader->EditUniform("offsets");
				auto& weights_handle = filter_shader->EditUniform("sample_weights");

				int sample_count = ComputeOffsetsAndWeights(blur_radius);
				for (int i = 0; i < sample_count; i++)
				{
					weights_handle[i] = glm::vec4(offsets_and_weights[i].y);
				}

				// Horizon Pass
				{
					for (int i = 0; i < sample_count; i += 2)
					{
						offsets_handle[i / 2] = glm::vec4(offsets_and_weights[i].x * inverse_filter_extent.x, 0.0f, 
							i + 1 < sample_count ? offsets_and_weights[i + 1].x * inverse_filter_extent.x : 0.0f, 0.0f);
					}

					m_postprocess_framebuffer->ChangeTexture(m_bloom_horizon_filter_chain[5 - stage]);
					m_bloom_downsample_chain[5 - stage]->Bind();

					filter_shader->EditUniform("sample_count") = static_cast<unsigned int>(sample_count);
					filter_shader->EditUniform("use_additive") = false;
					filter_shader->UpdateOnly();
					
					Common::RenderHelper::RenderSimpleQuad();

					m_bloom_downsample_chain[5 - stage]->UnBind();
				}

				// Vertical Pass
				{
					for (int i = 0; i < sample_count; i++)
					{
						weights_handle[i] = glm::vec4(offsets_and_weights[i].y * tint_color);
					}
					for (int i = 0; i < sample_count; i += 2)
					{
						offsets_handle[i / 2] = glm::vec4(0.0f, offsets_and_weights[i].x * inverse_filter_extent.y, 0.0f, 
							i + 1 < sample_count ? offsets_and_weights[i + 1].x * inverse_filter_extent.y : 0.0f);
					}

					m_postprocess_framebuffer->ChangeTexture(m_bloom_downsample_chain[5 - stage]);
					m_bloom_horizon_filter_chain[5 - stage]->Bind();

					if (stage == 0)
					{
						filter_shader->EditUniform("use_additive") = false;
					}
					else
					{
						filter_shader->EditUniform("use_additive") = true;
						additive = m_bloom_downsample_chain[6 - stage];
						additive->ChangeBindingPoint(1);
						additive->Bind();
					}

					filter_shader->UpdateOnly();
					Common::RenderHelper::RenderSimpleQuad();

					if (additive)
					{
						additive->ChangeBindingPoint(0);
						additive->UnBind();
					}
					
					m_bloom_horizon_filter_chain[5 - stage]->UnBind();
				}
			};

		const float tint_scale = 0.113f;
		for (int i = 0; i < 6; i++)
		{
			float t = m_gui_block.bloom_tint_weights[5 - i].x * tint_scale;
			glm::vec4 input_tint = glm::vec4(t);
			GaussianBloom(input_tint, m_gui_block.bloom_filter_kernel_sizes[5 - i] / 100.0f, i);
		}
	}

	void PostProcessor::PrepareToneGamma()
	{
		GLuint tone_vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "tone_mapping_vertex", "Common", "defer_shading.vert");
		GLuint tone_fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "tone_mapping_fragment", "Common", "tone_mapping.frag");
		auto tone_shader = Bind::ShaderProgram::Resolve("tone_mapping_shader", std::vector<GLuint>{ tone_vertex,tone_fragment});

		GLuint gamma_vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "gamma_correction_vertex", "Common", "defer_shading.vert");
		GLuint gamma_fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "gamma_correction_fragment", "Common", "gamma_correction.frag");
		auto gamma_shader = Bind::ShaderProgram::Resolve("gamma_correction_shader", std::vector<GLuint>{ gamma_vertex, gamma_fragment});

		OGL_TEXTURE2D_DESC desc = m_AA_texture->get_description();
		OGL_TEXTURE_PARAMETER param = m_AA_texture->get_parameter();
		m_tonemapping_texture = Bind::ImageTexture2D::Resolve("tone_mapping_texture", desc, param, 0);
		m_gammacorrection_texture = Bind::ImageTexture2D::Resolve("gamma_correction_texture", desc, param, 0);
	}

	void PostProcessor::ToneMapping()
	{
		APP_RANGE_BEGIN("Tone Mapping Pass");

		m_postprocess_framebuffer->ChangeTexture(m_tonemapping_texture);

		auto tone_shader = Bind::ShaderProgram::Resolve("tone_mapping_shader", { 0,0 });
		tone_shader->BindWithoutUpdate();
		tone_shader->EditUniform("exposure") = m_gui_block.tone_mapping_exposure;
		tone_shader->UpdateOnly();
		// TODO : 目前没有TAA效果，暂时绑定defer lighting纹理
		m_renderer->m_lighting_texture->Bind();
		m_bloom_downsample_chain[0]->ChangeBindingPoint(1);
		m_bloom_downsample_chain[0]->Bind();

		Common::RenderHelper::RenderSimpleQuad();

		m_bloom_downsample_chain[0]->UnBind();
		m_bloom_downsample_chain[0]->ChangeBindingPoint(0);
		tone_shader->UnBind();

		APP_RANGE_END();
	}

	void PostProcessor::GammaCorrection()
	{
		APP_RANGE_BEGIN("Gamma Correction Pass");

		m_postprocess_framebuffer->ChangeTexture(m_gammacorrection_texture);

		auto gamma_shader = Bind::ShaderProgram::Resolve("gamma_correction_shader", { 0,0 });
		gamma_shader->Bind();
		std::shared_ptr<Bind::ImageTexture2D> input_texture;
		if (m_gui_block.anti_aliasing_method == ANTI_ALIASING_METHOD_FXAA)
		{
			input_texture = m_AA_texture;
		}
		else
		{
			input_texture = m_tonemapping_texture;
		}
		input_texture->Bind();

		Common::RenderHelper::RenderSimpleQuad();

		input_texture->UnBind();
		gamma_shader->UnBind();

		m_output_texture = m_gammacorrection_texture;

		APP_RANGE_END();
	}
}
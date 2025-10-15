#pragma once

#include <cstdint>
#include <memory>

#define ANTI_ALIASING_METHOD_MSAA 0
#define ANTI_ALIASING_METHOD_FXAA 1
#define ANTI_ALIASING_METHOD_TAA  2
#define ANTI_ALIASING_METHOD_NUM  3

namespace Bind
{
	class ImageTexture2D;
	class RenderTarget;
}

namespace OGLPipeline
{
	enum PostProcessOptions
	{
		AntiAliasingMethod = 0,
	};

	struct PostProcessGuiBlock
	{
		uint16_t anti_aliasing_method = ANTI_ALIASING_METHOD_NUM;
		float FXAA_threshold_min = 0.0833;
		float FXAA_threshold = 0.166;
		float FXAA_subpixel_quality = 0.75;
	};

	class DeferRenderer;

	class PostProcessor
	{
		friend class DeferRenderer;

	public:
		PostProcessor() = default;
		~PostProcessor() = default;

		static PostProcessor& Get()
		{
			static PostProcessor processor;
			return processor;
		}

		void MainProcessor();
		void PreparePostProcess();
		void SetOption(PostProcessOptions category, uint16_t option);

		void Accept(DeferRenderer* renderer);
		
	private:
		void PrepareUI();

		void PrepareAA();
		void AntiAliasing();

		void PrepareBloom();
		void Bloom();

		void PrepareToneGamma();
		void ToneMappingAndGammaCorrection();

	private:
		PostProcessGuiBlock m_gui_block;
		DeferRenderer* m_renderer = nullptr;

	private:
		std::shared_ptr<Bind::RenderTarget> m_postprocess_framebuffer;

		std::shared_ptr<Bind::ImageTexture2D> m_AA_texture;
		std::shared_ptr<Bind::ImageTexture2D> m_tone_gamma_texture;
	};

	extern PostProcessor g_post_processor;
}
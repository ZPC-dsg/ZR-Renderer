#pragma once

#include <cstdint>
#include <memory>
#include <array>

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

		uint16_t bloom_downsample_display_level = 0; // 默认不显示bloom downsample结果
		uint16_t bloom_filter_display_level = 0; // 默认不显示bloom filter结果
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
		void TAA();
		void FXAA();

		void PrepareBloom();
		void PrepareBloomDownSample();
		void PrepareBloomFilter();
		void Bloom();
		void BloomDownSample();
		void BloomFilter();

		void PrepareToneGamma();
		void ToneMapping();
		void GammaCorrection();

	private:
		PostProcessGuiBlock m_gui_block;
		DeferRenderer* m_renderer = nullptr;

	private:
		std::shared_ptr<Bind::RenderTarget> m_postprocess_framebuffer;

		std::shared_ptr<Bind::ImageTexture2D> m_AA_texture;
		std::shared_ptr<Bind::ImageTexture2D> m_tonemapping_texture;
		std::shared_ptr<Bind::ImageTexture2D> m_gammacorrection_texture;
		std::shared_ptr<Bind::ImageTexture2D> m_tone_gamma_texture;

		std::array<std::shared_ptr<Bind::ImageTexture2D>, 6> m_bloom_downsample_chain;
		std::array<std::shared_ptr<Bind::ImageTexture2D>, 6> m_bloom_horizon_filter_chain;
		std::shared_ptr<Bind::ImageTexture2D> m_bloom_texture;

		std::shared_ptr<Bind::ImageTexture2D> m_output_texture;
	};

	extern PostProcessor g_post_processor;
}
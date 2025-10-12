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

		void MainProcessor(DeferRenderer* main_renderer);
		void SetOption(PostProcessOptions category, uint16_t option);
		
	private:
		void PrepareUI();

		void PrepareAA();
		void AntiAliasing(std::shared_ptr<Bind::ImageTexture2D> scene_color);

	private:
		PostProcessGuiBlock m_gui_block;

	private:
		std::shared_ptr<Bind::RenderTarget> m_AA_framebuffer;
	};

	extern PostProcessor g_post_processor;
}
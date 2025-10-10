#pragma once

#include <cstdint>

#define ANTI_ALIASING_METHOD_MSAA 0
#define ANTI_ALIASING_METHOD_FXAA 1
#define ANTI_ALIASING_METHOD_TAA  2
#define ANTI_ALIASING_METHOD_NUM  3


namespace OGLPipeline
{
	enum PostProcessOptions
	{
		AntiAliasingMethod = 0,
	};

	struct PostProcessGuiBlock
	{
		uint16_t anti_aliasing_method = ANTI_ALIASING_METHOD_NUM;
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
		void SetOption(PostProcessOptions category, uint16_t option);

	private:
		void AntiAliasing();

	private:
		PostProcessGuiBlock m_gui_block;
	};

	extern PostProcessor g_post_processor;
}
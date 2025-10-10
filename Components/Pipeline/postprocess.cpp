#include <Pipeline/postprocess.h>
#include <Macros/gfxdebug_helper.h>

namespace OGLPipeline
{
	PostProcessor g_post_processor = PostProcessor::Get();

	void PostProcessor::MainProcessor()
	{
		APP_RANGE_BEGIN("post_process");

		APP_RANGE_END();
	}

	void PostProcessor::SetOption(PostProcessOptions category, uint16_t option)
	{
		switch (category)
		{
		case AntiAliasingMethod: m_gui_block.anti_aliasing_method = option; break;
		}
	}

	void PostProcessor::AntiAliasing()
	{
		switch (m_gui_block.anti_aliasing_method)
		{
		case 0:
		case 1:

		case 2:
		default:
			break;
		}

		return;
	}
}
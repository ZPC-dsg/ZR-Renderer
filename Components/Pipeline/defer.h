#pragma once

#define DEFER_DISPLAY_MODE_WORLD_POSITION 0
#define DEFER_DISPLAY_MODE_WORLD_NORMAL   1
#define DEFER_DISPLAY_MODE_NDC_DEPTH      2
#define DEFER_DISPLAY_MODE_LINEAR_DEPTH   3
#define DEFER_DISPLAY_MODE_ALBEDO         4
#define DEFER_DISPLAY_MODE_DIFFUSE        5
#define DEFER_DISPLAY_MODE_SPECULAR       6
#define DEFER_DISPLAY_MODE_METALLIC       7
#define DEFER_DISPLAY_MODE_ROUGHNESS      8
#define DEFER_DISPLAY_MODE_ANISOTROPHY    9
#define DEFER_DISPLAY_MODE_NUM            10

#include <utils.h>

namespace Bind
{
	class RenderTarget;
	class ImageTexture2D;
}

namespace SceneGraph
{
	class ModelProxy;
}

namespace OGLPipeline
{
	class DeferRenderer : public Utils
	{
	public:
		DeferRenderer(const std::string& scene_name, const std::string ui_name = "DeferOptions");
		~DeferRenderer() = default;

		void render() override;
		void prepare() override;
		void prepare_ui(const std::string& name) override;

	private:
		void RenderDefer();
		void DisplayDefer();

	private:
		std::string m_ui_name;

		uint16_t m_defer_display_mode = DEFER_DISPLAY_MODE_NUM;

	private:
		std::shared_ptr<Bind::RenderTarget> m_defer_framebuffer;
		std::shared_ptr<Bind::ImageTexture2D> m_rt_position_anisotrophy;
		std::shared_ptr<Bind::ImageTexture2D> m_rt_normal_metallic_roughness;
		std::shared_ptr<Bind::ImageTexture2D> m_rt_albedo_specular;
		std::shared_ptr<Bind::ImageTexture2D> m_rt_depthbuffer;

		std::shared_ptr<SceneGraph::ModelProxy> m_scene;
	};
}

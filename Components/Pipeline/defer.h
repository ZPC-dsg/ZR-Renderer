#pragma once

#define MAX_LIGHTS_SUPPORTED 16

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
#define DEFER_DISPLAY_MODE_PBR_LIGHTING   10
#define DEFER_DISPLAY_MODE_NUM            11

#define AO_METHOD_SSAO_SIMPLE  0
#define AO_METHOD_SSAO_UE      1
#define AO_METHOD_SSDO         2
#define AO_METHOD_HBAO         3
#define AO_METHOD_GTAO         4
#define AO_METHOD_NUM          5

#include <utils.h>
#include <Common/light_common.h>

#include <set>
#include <string>

namespace Bind
{
	class RenderTarget;
	class ImageTexture2D;
	class ConstantBuffer;
	class Sampler;
}

namespace SceneGraph
{
	class ModelProxy;
}

namespace OGLPipeline
{
	class PostProcessor;

	class DeferRenderer : public Utils
	{
		friend class PostProcessor;

		struct AOParamBlock
		{
			// Simple SSAO
			float simple_SSAO_kernel_radius = 0.5f;
			float simple_SSAO_bias = 0.025f;
		};

	public:
		DeferRenderer(const std::string& scene_name, const std::string ui_name = "DeferOptions");
		~DeferRenderer() = default;

		void render() override;
		void prepare() override;
		void prepare_ui(const std::string& name) override;
		void resize() override;

	private:
		void PrepareScene();
		void PrepareDefaultTextures();
		void PrepareSamplers();

		void PreparePreZ();
		void PrepareHiZ();

		void PrepareDeferBuffers();
		void PrepareDeferLighting();
		void PrepareLightBuffer();

		void PrepareAO();
		void GenerateSSAORandomTexture();
		void GenerateSimpleSSAORandomPos();

		void PrepareLightUI();
		void PrepareAOUI();

		void RenderPreZ();
		void RenderHiZ();

		void RenderDefer();
		void DisplayDefer();
		void DeferLighting();

		void RenderAO();
		void AOGeneration();
		void AOFilter();
		void RenderAOToScreen();

	private:
		size_t m_scene_index = 0; // 默认渲染Sponza
		static const char* scene_names[];

		std::string m_ui_name;

		AOParamBlock m_ao_extra_block;

		uint16_t m_defer_display_mode = DEFER_DISPLAY_MODE_NUM;
		bool m_show_light_window = false;
		uint16_t m_ao_method = AO_METHOD_SSAO_SIMPLE;
		bool m_should_display_ao = false;

	private:
		// 场景唯一方向光
		Common::LightCommon m_main_light;
		std::vector<std::pair<Common::LightCommon, size_t>> m_point_lights;
		std::vector<std::pair<Common::LightCommon, size_t>> m_spot_lights;
		std::vector<std::pair<Common::LightCommon, size_t>> m_rect_lights;
		std::set<size_t> m_available_indexes;

		std::shared_ptr<Bind::ConstantBuffer> m_light_buffer;

	private:
		std::shared_ptr<Bind::RenderTarget> m_prez_framebuffer;
		std::shared_ptr<Bind::ImageTexture2D> m_rt_depthbuffer;

		std::shared_ptr<Bind::RenderTarget> m_hiz_framebuffer;
		std::shared_ptr<Bind::ImageTexture2D> m_hiz_texture;

		std::shared_ptr<Bind::RenderTarget> m_defer_framebuffer;
		std::shared_ptr<Bind::ImageTexture2D> m_rt_position_anisotrophy;
		std::shared_ptr<Bind::ImageTexture2D> m_rt_normal_metallic_roughness;
		std::shared_ptr<Bind::ImageTexture2D> m_rt_albedo_specular;

		std::shared_ptr<Bind::RenderTarget> m_AO_framebuffer;
		std::shared_ptr<Bind::ImageTexture2D> m_AO_texture;
		std::shared_ptr<Bind::ImageTexture2D> m_AO_filtered_texture;
		std::shared_ptr<Bind::ImageTexture2D> m_SSAO_random_texture;
		std::vector<glm::vec3> m_simple_SSAO_random_positions;

		std::shared_ptr<Bind::RenderTarget> m_defer_lighting_framebuffer;
		std::shared_ptr<Bind::ImageTexture2D> m_lighting_texture;

		std::shared_ptr<Bind::Sampler> m_point_sampler;
		std::shared_ptr<Bind::Sampler> m_bilinear_sampler;
		std::shared_ptr<Bind::Sampler> m_trilinear_sampler;

		std::shared_ptr<Bind::ImageTexture2D> m_default_white_texture;

		std::vector<std::shared_ptr<SceneGraph::ModelProxy>> m_scenes;
	};
}

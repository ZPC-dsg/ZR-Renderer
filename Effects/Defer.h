#pragma once

#include <utils.h>

namespace SceneGraph
{
	class ModelProxy;
}

namespace Bind
{
	class ShaderProgram;
	class ImageTexture2D;
}

namespace RTREffects
{
	class DeferRenderer :public Utils
	{
	public:
		DeferRenderer(const std::string& name);
		~DeferRenderer() = default;

		void prepare() override;
		void prepare_ui(const std::string& name) override;
		void render() override;

	private:
		void prepare_scene();
		void prepare_defer();
		void prepare_compute();

		void render_defer();
		void render_compute();
		void render_screen();

		void render_diff_spec(bool is_diffuse);
		void render_normal();
		void render_depth();

	private:
		std::shared_ptr<SceneGraph::ModelProxy> m_proxy;

		std::shared_ptr<Bind::ShaderProgram> m_defer_shader;
		std::shared_ptr<Bind::ShaderProgram> m_compute_shader;

		std::shared_ptr<Bind::ImageTexture2D> m_diff_spec_texture;
		std::shared_ptr<Bind::ImageTexture2D> m_norm_shin_texture;
		std::shared_ptr<Bind::ImageTexture2D> m_depth_texture;

	private:
		int m_debug_mode = -1;

		const char* m_debug_items[5] =
		{
			"Diffuse Color",
			"Specular Color",
			"World space Normal",
			"Linear Depth",
			"None"
		};
	};
}
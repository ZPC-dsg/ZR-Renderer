#pragma once

#include <utils.h>

namespace SceneGraph
{
	class ModelProxy;
	class DrawableProxy;
}

namespace Bind
{
	class ShaderProgram;
	class ImageTexture2D;
}

namespace RTREffects
{
	class OIT :public Utils
	{
	public:
		OIT(const std::string& name);
		~OIT() = default;

		void render() override;
		void prepare() override;
		void prepare_ui(const std::string& name) override;

	private:
		void prepare_opaque();
		void prepare_transparent();
		void prepare_OITdata();

		void render_opaque();
		void render_OIT();
		void composite();

		void prepare_test();

	private:
		std::shared_ptr<SceneGraph::ModelProxy> m_proxy;
		std::shared_ptr<SceneGraph::DrawableProxy> m_transparent_proxy;

		std::shared_ptr<Bind::ShaderProgram> m_opaque_shader;
		std::shared_ptr<Bind::ShaderProgram> m_transparent_shader;

		std::shared_ptr<Bind::ImageTexture2D> m_test_texture;

	private:
		int m_sample_count = 4;
		int m_sample_side_count = 2;
	};
}
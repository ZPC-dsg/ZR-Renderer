#pragma once

#include <utils.h>

namespace SceneGraph
{
	class ModelProxy;
}

namespace Bind
{
	class ShaderProgram;
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

	private:
		std::shared_ptr<SceneGraph::ModelProxy> m_proxy;

		std::shared_ptr<Bind::ShaderProgram> m_defer_shader;
		std::shared_ptr<Bind::ShaderProgram> m_compute_shader;
	};
}
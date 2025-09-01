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

namespace OITEffects
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

	private:
		std::shared_ptr<SceneGraph::ModelProxy> m_proxy;

		std::shared_ptr<Bind::ShaderProgram> m_opaque_shader;
	};
}
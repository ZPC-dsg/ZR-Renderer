#pragma once

#include <utils.h>

namespace SceneGraph
{
	class ModelProxy;
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

	private:
		std::shared_ptr<SceneGraph::ModelProxy> m_proxy;

	};
}
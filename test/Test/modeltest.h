#pragma once

#include <utils.h>
#include <SceneGraph/modelproxy.h>
#include <SceneGraph/drawableproxy.h>
#include <Bindables/imagetexture2D.h>

namespace Bind {
	class ShaderProgram;
}

class ModelTest :public Utils {
public:
	ModelTest(const std::string& scene_name);
	~ModelTest() = default;

	void prepare() override;
	void prepare_ui(const std::string& name) override;
	void render() override;

private:
	std::shared_ptr<Bind::ShaderProgram> m_shader;
	std::shared_ptr<SceneGraph::ModelProxy> m_proxy;
	std::vector<std::shared_ptr<SceneGraph::DrawableProxy>> m_drawables;
	std::shared_ptr<Bind::ImageTexture2D> m_diffuse;
};
#include <SceneGraph/modelproxy.h>
#include <SceneGraph/node.h>
#include <Bindables/headers.h>
#include <SceneGraph/scene.h>

namespace SceneGraph {
	ModelProxy::ModelProxy(Scene* scene, ControlNode& root, size_t count, const std::string& rel_path, const std::string& name)
		:DrawableProxy(scene, root, name, count), m_relative_path(rel_path)
	{
	}

	ModelProxy& ModelProxy::ScaleModel(const glm::vec3& scaling) noexcept {
		m_root->SetWorldMatrix(glm::scale(glm::mat4(1.0f), scaling));
		return *this;
	}

	void ModelProxy::Cook() {
		//必须要有一个shader
		if (!m_root->HasShader()) {
			assert("In order to render the model, you need to add a shader program before cooking!" && false);
			return;
		}
		if (!m_root->HasVertexConfiguration()) {
			assert("In order to render the model, you need to add a vertex configuration before cooking!" && false);
			return;
		}

		LOGI("Start cooking model: {}...", m_name.c_str());
		m_root->StartCooking(m_relative_path);
		LOGI("Finished cooking model: {}!", m_name.c_str());
	}
}
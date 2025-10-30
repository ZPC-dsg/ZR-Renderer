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
		m_current_set = 0;
		for (; m_current_set < m_render_sets.size(); m_current_set++)
		{
			LOGI("Start cooking model: {} for configuration set: {}...", m_name.c_str(), m_render_sets[m_current_set].c_str());
			m_root->StartCooking(m_relative_path);
			LOGI("Finished cooking model: {} for configuration set: {}!", m_name.c_str(), m_render_sets[m_current_set].c_str());
		}
		m_current_set = 0;

		m_root->ClearTextureConfig();
		m_root->ClearVertexConfig();
		m_generated_textures = {};
	}
}
#include <SceneGraph/drawableproxy.h>
#include <SceneGraph/node.h>
#include <Bindables/headers.h>
#include <SceneGraph/scene.h>

namespace SceneGraph {
	DrawableProxy::DrawableProxy(Scene* scene, ControlNode& root, const std::string& name, size_t nodes)
		:m_scene(scene), m_root(&root), m_name(name), m_nodecount(nodes), m_current_set(-1)
	{
		// 预留足够的空间，这样在addrule的时候这两个向量不会因为容量不足而发生复制，如果发生了复制，那么之前加入的rule对其中的ref的引用将会失效
		// TODO : 去掉这段逻辑，将uniform和constant rule中的element ref改为proxy中的index
		m_uniform_refs.reserve(100);
		m_constant_refs.reserve(100);
	}

	DrawableProxy& DrawableProxy::AddRootBindable(std::shared_ptr<Bind::Bindable> bindable) {
		m_root->SetBindable(m_bindables.size());
		m_bindables.push_back(bindable);

		return *this;
	}

	DrawableProxy& DrawableProxy::AddControlBindable(const std::string& name, std::shared_ptr<Bind::Bindable> bindable) {
		if (!m_controls.contains(name)) {
			LOG_WARNING("Control node with name: {} does not exist!", name.c_str());
			return *this;
		}

		m_controls[name]->SetBindable(m_bindables.size());
		m_bindables.push_back(bindable);

		return *this;
	}

	DrawableProxy& DrawableProxy::AddRootTextureRule(const std::string& shader_name, unsigned int binding, Material::TextureCategory type) {
		m_root->AddTextureConfig(shader_name, binding, type);
		return *this;
	}

	DrawableProxy& DrawableProxy::AddRootVertexRule(std::vector<DrawItems::VertexType> instruction) {
		m_root->AddVertexConfig(instruction);
		return *this;
	}

	DrawableProxy& DrawableProxy::AddControlTextureRule(const std::string& name, const std::string& shader_name, unsigned int binding, Material::TextureCategory type) {
		if (!m_controls.contains(name)) {
			LOG_WARNING("Control node with name: {} does not exist!", name.c_str());
			return *this;
		}

		m_controls[name]->AddTextureConfig(shader_name, binding, type);
		return *this;
	}

	DrawableProxy& DrawableProxy::AddControlVertexRule(const std::string& name, std::vector<DrawItems::VertexType> instruction) {
		if (!m_controls.contains(name)) {
			LOG_WARNING("Control node with name: {} does not exist!", name.c_str());
			return *this;
		}

		m_controls[name]->AddVertexConfig(instruction);
		return *this;
	}

	void DrawableProxy::AddControlNode(const std::string& name, const std::string& father_name) {
		assert("Control node name already exists!" && m_controls.contains(name));

		Node* father = m_root->FindNodeWithName(father_name);
		if (!father) {
			LOG_WARNING("Node with name: {} does not exist!", father_name.c_str())
				return;
		}

		std::unique_ptr<Node> control = std::make_unique<ControlNode>(m_nodecount, name);
		for (Node* child : father->GetChildren()) {
			child->SetParent(*control);
		}
		father->ResetChildren(std::vector<Node*>{control.get()});
		control->SetParent(*father);

		m_controls[name] = static_cast<ControlNode*>(control.get());
		m_scene->AddNode(std::move(control));
	}

	void DrawableProxy::Cook() {
		m_current_set = 0;
		for (; m_current_set < m_render_sets.size(); m_current_set++)
		{
			LOGI("Start cooking drawable: {} for configuration set: {}...", m_name.c_str(), m_render_sets[m_current_set].c_str());
			m_root->StartCooking("");//空字符串代表渲染的不是模型，纹理需要到textures文件夹而不是model自己的文件夹中寻找
			LOGI("Finished cooking drawable: {} for configuration set: {}!", m_name.c_str(), m_render_sets[m_current_set].c_str());
		}
		m_current_set = 0;

		m_root->ClearTextureConfig();
		m_root->ClearVertexConfig();
		m_generated_textures = {};
	}

	void DrawableProxy::Render(bool clear_texture, bool clear_depth, bool clear_stencil) {
		m_root->BindAll();
		
		if (clear_texture)
		{
			glClear(GL_COLOR_BUFFER_BIT);
		}
		if (clear_depth)
		{
			glClear(GL_DEPTH_BUFFER_BIT);
		}
		if (clear_stencil)
		{
			glClear(GL_STENCIL_BUFFER_BIT);
		}

		m_root->StartRender();
		m_root->UnBindAll();
	}

	void DrawableProxy::Bind() {
		m_root->BindAll();
	}

	void DrawableProxy::BeginRange(const std::string& name)
	{
		if (std::find(m_render_sets.begin(), m_render_sets.end(), name) != m_render_sets.end())
		{
			LOGE("Configuration Set :{} already exists!", name.c_str());
			assert(false);
		}
		else
		{
			m_current_set = m_render_sets.size();
			m_render_sets.push_back(name);
		}
	}

	void DrawableProxy::EndRange()
	{
		m_current_set = -1;
	}

	void DrawableProxy::ChangeSet(const std::string& name)
	{
		auto it = std::find(m_render_sets.begin(), m_render_sets.end(), name);
		if (it == m_render_sets.end())
		{
			LOGE("Configuration Set named {} does not exist!", name.c_str());
			assert(false);
		}
		else
		{
			m_current_set = it - m_render_sets.begin();
		}
	}
}
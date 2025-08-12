#include <SceneGraph/drawableproxy.h>
#include <SceneGraph/node.h>
#include <Bindables/headers.h>
#include <SceneGraph/scene.h>

namespace SceneGraph {
	DrawableProxy::DrawableProxy(Scene* scene, ControlNode& root, const std::string& name, size_t nodes)
		:m_scene(scene), m_root(&root), m_name(name), m_nodecount(nodes)
	{
		m_uniform_refs.reserve(100);
		m_constant_refs.reserve(100);
		//预留足够的空间，这样在addrule的时候这两个向量不会因为容量不足而发生复制，如果发生了复制，那么之前加入的rule对其中的ref的引用将会失效
	}

	DrawableProxy& DrawableProxy::AddRootBindable(std::shared_ptr<Bind::Bindable> bindable) {
		const std::type_index& type_info = bindable->GetTypeInfo();
		if (type_info == typeid(Bind::ConstantBuffer) || type_info == typeid(Bind::CubeMap) || type_info == typeid(Bind::ImageTexture2D)
			|| type_info == typeid(Bind::ImageTexture2DArray)) {
			m_root->SetBindable(bindable);
		}
		else {
			m_root->SetBindableUnique(bindable);
		}

		return *this;
	}

	DrawableProxy& DrawableProxy::AddControlBindable(const std::string& name, std::shared_ptr<Bind::Bindable> bindable) {
		if (!m_controls.contains(name)) {
			LOG_WARNING("Control node with name: {} does not exist!", name.c_str());
			return *this;
		}

		const std::type_index& type_info = bindable->GetTypeInfo();
		if (type_info == typeid(Bind::ConstantBuffer) || type_info == typeid(Bind::CubeMap) || type_info == typeid(Bind::ImageTexture2D)
			|| type_info == typeid(Bind::ImageTexture2DArray)) {
			m_controls[name]->SetBindable(bindable);
		}
		else {
			m_controls[name]->SetBindableUnique(bindable);
		}

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
		//必须要有一个shader
		if (!m_root->HasShader()) {
			assert("In order to render drawables, you need to add a shader program before cooking!" && false);
			return;
		}
		if (!m_root->HasVertexConfiguration()) {
			assert("In order to render drawables, you need to add a vertex configuration before cooking!" && false);
			return;
		}

		LOGI("Start cooking drawable: {}...", m_name.c_str());
		m_root->StartCooking("");//空字符串代表渲染的不是模型，纹理需要到textures文件夹而不是model自己的文件夹中寻找
		LOGI("Finished cooking drawable: {}!", m_name.c_str());
	}

	void DrawableProxy::Render() {
		m_root->StartRender();
	}

	void DrawableProxy::Bind() {
		m_root->BindAll();
	}
}
#include <SceneGraph/node.h>
#include <Bindables/shaderprogram.h>
#include <Bindables/imagetexture2D.h>
#include <Bindables/constantbuffer.h>
#include <SceneGraph/drawableproxy.h>
#include <logging.h>

namespace SceneGraph {
	//Node
	Node::Node(const size_t id, const std::string& name)
		:m_id(id), m_name(name)
	{
	}

	//不设置父亲节点的子节点
	void Node::SetParent(Node& node) {
		m_parent = &node;
	}

	//不设置子节点的父亲节点
	void Node::AddChild(Node& child) {
		m_children.push_back(&child);
	}

	void Node::ResetChildren(std::vector<Node*> nodes) {
		m_children = nodes;
	}

	void Node::SetBindable(size_t bindable_index, size_t index) {
		m_bindable_sets.resize(m_proxy->m_render_sets.size());

		if (m_bindable_sets[m_proxy->m_current_set].size() == 0)
		{
			m_bindable_sets[m_proxy->m_current_set].resize(this->GetDrawableCount());
		}

		if (index >= m_bindable_sets[m_proxy->m_current_set].size())
		{
			LOGE("Index {} bigger than primitive amount contained in this node!Primitive count:{}.", std::to_string(index).c_str(), std::to_string(m_bindable_sets[m_proxy->m_current_set].size()).c_str());
			assert(false);
		}
		m_bindable_sets[m_proxy->m_current_set][index].push_back(bindable_index);
	}

	void Node::SetProxy(DrawableProxy* proxy)
	{
		m_proxy = proxy;
		for (auto c : m_children)
		{
			c->SetProxy(proxy);
		}
	}

	Node* Node::FindNodeWithName(const std::string& name) {
		if (m_name == name) {
			return this;
		}
		else {
			for (Node* child : m_children) {
				if (Node* target = child->FindNodeWithName(name); target) {
					return target;
				}
			}
			return nullptr;
		}
	}

	Node* Node::FindNodeWithID(unsigned int ID) {
		if (m_id == ID) {
			return this;
		}
		else {
			for (Node* child : m_children) {
				if (Node* target = child->FindNodeWithID(ID); target) {
					return target;
				}
			}
			return nullptr;
		}
	}

	void Node::BindAll() {
		for (auto& prim : m_bindable_sets[m_proxy->m_current_set]) {
			for (size_t ind : prim) {
				auto b = m_proxy->m_bindables[ind];
				if (b->GetTypeInfo() == typeid(Bind::ShaderProgram))
				{
					std::static_pointer_cast<Bind::ShaderProgram>(b)->BindWithoutUpdate();
					continue;
				}

				b->Bind();
			}
		}
	}

	void Node::BindIndex(size_t index) {
		if (index >= m_bindable_sets[m_proxy->m_current_set].size())
		{
			LOGE("Index to bind bigger than primitive amount!");
			assert(false);
		}

		for (size_t ind : m_bindable_sets[m_proxy->m_current_set][index]) {
			auto b = m_proxy->m_bindables[ind];
			if (b->GetTypeInfo() == typeid(Bind::ShaderProgram))
			{
				std::static_pointer_cast<Bind::ShaderProgram>(b)->BindWithoutUpdate();
				continue;
			}

			b->Bind();
		}
	}

	void Node::UnBindAll() {
		for (auto& prim : m_bindable_sets[m_proxy->m_current_set]) {
			for (size_t ind : prim) {
				m_proxy->m_bindables[ind]->UnBind();
			}
		}
	}

	void Node::UnBindIndex(size_t index) {
		if (index >= m_bindable_sets[m_proxy->m_current_set].size())
		{
			LOGE("Index to unbind bigger than primitive amount!");
			assert(false);
		}

		for (size_t ind : m_bindable_sets[m_proxy->m_current_set][index]) {
			m_proxy->m_bindables[ind]->UnBind();
		}
	}

	//EntityNode
	EntityNode::EntityNode(const size_t id, const std::string& name, unsigned int drawable_count)
		:Node(id, name)
	{

	}

	size_t EntityNode::GetDrawableCount() const
	{
		return m_drawables.size();
	}

	void EntityNode::SetParent(Node& node) {
		m_parent = &node;
		m_transform.SetDirtyFlag(true);
	}

	const glm::mat4& EntityNode::CalculateWorldMatrix() const noexcept {
		return m_transform.GetWorldMatrix();
	}

	void EntityNode::SetTranslation(const glm::vec3& translate) noexcept {
		m_transform.SetTranslation(translate);
	}

	void EntityNode::SetRotation(const glm::vec3& axis, float angle) noexcept {
		m_transform.SetRotation(axis, angle);
	}

	void EntityNode::SetRotation(const glm::vec3& angles) noexcept {
		m_transform.SetRotation(angles);
	}

	void EntityNode::SetRotation(const glm::vec4& quat) noexcept {
		m_transform.SetRotation(quat);
	}

	void EntityNode::SetScaling(const glm::vec3& scale) noexcept {
		m_transform.SetScale(scale);
	}

	void EntityNode::CookNode(std::vector<Dynamic::Dsr::VertexAttrib>& attribs, std::vector<DrawItems::VertexType>& instruction,
		std::vector<std::tuple<Material::TextureCategory, std::string, GLuint>>& textures, const std::string& rel_path) {
		CookVertex(attribs, instruction);
		CookTexture(textures, rel_path);
		
		m_transform.SetDirtyFlag(true);//第一帧渲染的时候需要更新世界矩阵

		for (Node* child : m_children) {
			child->CookNode(attribs, instruction, textures, rel_path);
		}
	}

	void EntityNode::CookVertex(const std::vector<Dynamic::Dsr::VertexAttrib>& attribs, const std::vector<DrawItems::VertexType>& instruction) {
		for (auto m : m_drawables) {
			m->GenerateVAO(attribs, instruction);
		}
	}

	//多个同一纹理类型的纹理按照在material中从前往后的存储顺序来决定生成的纹理是哪一个
	void EntityNode::CookTexture(const std::vector<std::tuple<Material::TextureCategory, std::string, GLuint>>& textures, const std::string& rel_path) {
		m_binding_sets.resize(m_proxy->m_render_sets.size());
		m_binding_sets.back().resize(m_materials.size());

		for (size_t ind = 0; ind < m_materials.size(); ind++)
		{
			auto& m = m_materials[ind];

			std::vector<std::vector<SceneGraph::TextureInfo>> tex_infos(Material::TextureCategory::NUM);
			for (int i = 0; i < tex_infos.size(); i++)
			{
				tex_infos[i] = m->GetTextures(static_cast<Material::TextureCategory>(i));
			}

			std::vector<size_t> tex_nums(Material::TextureCategory::NUM);
			for (const auto& [type, name, binding] : textures)
			{
				int pos = static_cast<int>(type);

				if (tex_nums[pos] >= tex_infos[pos].size())
				{
					size_t index = LoadDefault(type, binding, ind);
					SetBindable(index, ind);
				}
				else
				{
					const auto& tex_info = tex_infos[pos][tex_nums[pos]];
					size_t index = LoadTexture(type, m->GetName(), rel_path, tex_info, binding, ind);
					SetBindable(index, ind);
					tex_nums[pos]++;
				}
			}
		}
	}

	size_t EntityNode::LoadDefault(Material::TextureCategory type, GLuint unit, size_t index)
	{
		size_t ind;
		if (m_proxy->m_generated_textures.contains("Default_" + Material::TexTypeToString(type)))
		{
			ind = m_proxy->m_generated_textures["Default_" + Material::TexTypeToString(type)];
		}
		else
		{
			ind = m_proxy->m_bindables.size();
			std::shared_ptr<Bind::ImageTexture2D> tex = Bind::ImageTexture2D::Resolve("Default_" + Material::TexTypeToString(type),
				Material::DefaultTexture(type), OGL_TEXTURE_PARAMETER(), unit, false);
			m_proxy->m_bindables.push_back(tex);
			m_proxy->m_generated_textures["Default_" + Material::TexTypeToString(type)] = ind;
		}

		m_binding_sets[m_proxy->m_current_set][index].push_back({ind,unit});
		return ind;
	}

	size_t EntityNode::LoadTexture(Material::TextureCategory type, const std::string& mat_name, const std::string& rel_path, const SceneGraph::TextureInfo& info, GLuint unit, size_t index)
	{
		std::string real_path = rel_path.length() ? rel_path + "/" + info.m_path : info.m_path;
		
		size_t ind;
		if (m_proxy->m_generated_textures.contains(real_path))
		{
			ind = m_proxy->m_generated_textures[real_path];
		}
		else
		{
			OGL_TEXTURE_PARAMETER param;
			param.wrap_x = TextureInfo::GLWrapMode(info.m_wrapmode[0]);
			param.wrap_y = TextureInfo::GLWrapMode(info.m_wrapmode[1]);

			ind = m_proxy->m_bindables.size();
			m_proxy->m_generated_textures[real_path] = ind;
			std::shared_ptr<Bind::ImageTexture2D> tex = Bind::ImageTexture2D::Resolve(mat_name + "_" + Material::TexTypeToString(type) +
				"_ " + info.m_path, real_path, param, unit, rel_path.length());
			m_proxy->m_bindables.push_back(tex);
		}

		m_binding_sets[m_proxy->m_current_set][index].push_back({ind,unit});
		return ind;
	}

	void EntityNode::Render(ControlNode* node, bool force_update) {
		bool child_force = false;

		if (force_update) {
			m_transform.SetDirtyFlag(false);
			m_world_matrix = m_parent->GetWorldMatrix() * CalculateWorldMatrix();
		}
		else {
			if (m_transform.IsDirty()) {
				m_transform.SetDirtyFlag(false);
				m_world_matrix = m_parent ? m_parent->GetWorldMatrix() * CalculateWorldMatrix() : CalculateWorldMatrix();
				child_force = true;
			}
		}

		if (m_drawables.size()) {
			if (node->m_uniform_function_indexes[m_proxy->m_current_set].contains(ConfigurationType::Transformation)) {
				for (size_t ind : node->m_uniform_function_indexes[m_proxy->m_current_set][ConfigurationType::Transformation]) {
					auto& p = node->m_uniform_functions[ConfigurationType::Transformation][ind];
					(*p)(*this);
				}
			}
			if (node->m_constant_functions.contains(ConfigurationType::Transformation)) {
				for (size_t ind : node->m_constant_function_indexes[m_proxy->m_current_set][ConfigurationType::Transformation]) {
					auto& p = node->m_constant_functions[ConfigurationType::Transformation][ind];
					(*p)(*this);
				}
			}

			for (size_t i = 0; i < m_drawables.size(); i++) {
				Update(node, i);
				BindIndex(i);
				m_drawables[i]->Draw(m_proxy->m_current_set);
			}
		}

		for (Node* child : m_children) {
			child->Render(node, force_update || child_force);
		}
	}

	void EntityNode::Update(ControlNode* node, size_t index) {
		if (node->m_uniform_function_indexes.size())
		{
			for (auto& uniform : node->m_uniform_function_indexes[m_proxy->m_current_set]) {
				if (uniform.first != ConfigurationType::Transformation) {
					for (size_t ind : uniform.second) {
						auto& p = node->m_uniform_functions[uniform.first][ind];
						(*p)(*this, index);
					}
				}
			}
		}
		
		if (node->m_constant_function_indexes.size())
		{
			for (auto& constant : node->m_constant_function_indexes[m_proxy->m_current_set]) {
				if (constant.first != ConfigurationType::Transformation) {
					for (size_t ind : constant.second) {
						auto& p = node->m_constant_functions[constant.first][ind];
						(*p)(*this, index);
					}
				}
			}
		}

		for (const auto& [ind, binding] : m_binding_sets[m_proxy->m_current_set][index])
		{
			m_proxy->m_bindables[ind]->ChangeBindingPoint(binding);
		}

		node->Update(nullptr);//Control node的Update不需要node参数
	}

	//ControlNode
	ControlNode::ControlNode(const size_t id, const std::string& name)
		:Node(id, name)
	{
		assert(name.length());
	}

	void ControlNode::AddFunctionIndex(bool is_uniform, ConfigurationType Config, size_t index)
	{
		if (is_uniform)
		{
			m_uniform_function_indexes[m_proxy->m_current_set][Config].push_back(index);
		}
		else
		{
			m_constant_function_indexes[m_proxy->m_current_set][Config].push_back(index);
		}
	}

	void ControlNode::ResizeIndexMap(bool is_uniform)
	{
		if (is_uniform && m_uniform_function_indexes.size() < m_proxy->m_render_sets.size())
		{
			m_uniform_function_indexes.resize(m_proxy->m_render_sets.size());
		}
		else if (!is_uniform && m_constant_function_indexes.size() < m_proxy->m_render_sets.size())
		{
			m_constant_function_indexes.resize(m_proxy->m_render_sets.size());
		}
	}

	size_t ControlNode::GetDrawableCount() const
	{
		return 1;
	}

	void ControlNode::AddTextureConfig(const std::string& name, GLuint binding, Material::TextureCategory type) {
		if(m_texture_vector.size() < m_proxy->m_render_sets.size())
			m_texture_vector.resize(m_proxy->m_render_sets.size());
		m_texture_vector[m_proxy->m_current_set].emplace_back(type, name, binding);
	}

	void ControlNode::AddVertexConfig(std::vector<DrawItems::VertexType> instruction) {
		m_vertex_instruction.resize(m_proxy->m_render_sets.size());
		m_vertex_instruction[m_proxy->m_current_set] = instruction;
	}

	void ControlNode::StartCooking(const std::string& rel_path) {
		std::vector<Dynamic::Dsr::VertexAttrib> attribs;
		std::shared_ptr<Bind::ShaderProgram> shader = nullptr;
		for (size_t ind = 0; ind < m_bindable_sets[m_proxy->m_current_set][0].size(); ind++)
		{
			auto b = m_proxy->m_bindables[ind];
			if (b->GetTypeInfo() == typeid(Bind::ShaderProgram))
			{
				shader = std::static_pointer_cast<Bind::ShaderProgram>(b);
				attribs = Dynamic::Dsr::ShaderReflection::GetVertexAttribs(shader->get_program());
				break;
			}
		}
		if (shader == nullptr)
		{
			LOGE("There must be a shader for cooking in a control node!");
			assert(false);
		}
		if (attribs.size() == 0)
		{
			LOGE("Vertex attribution amount must be bigger than 0!");
			assert(false);
		}

		if (attribs.size() != m_vertex_instruction[m_proxy->m_current_set].size())
		{
			LOGE("Vertex attribs specified do not match the amount of vertex attribs in vertex shader!Attrib count specified: {}.Attrib count in vertex shader: {}.",
				std::to_string(m_vertex_instruction[m_proxy->m_current_set].size()).c_str(), std::to_string(attribs.size()).c_str());
			assert(false);
		}

		for (Node* child : m_children) {
			child->CookNode(attribs, m_vertex_instruction[m_proxy->m_current_set], m_texture_vector[m_proxy->m_current_set], rel_path);
		}

		shader->BindWithoutUpdate();
		for (const auto& [_, name, binding] : m_texture_vector[m_proxy->m_current_set]) {
			shader->SetTexture(name, binding);
		}
		shader->UnBind();
	}

	// TODO
	void ControlNode::CookNode(std::vector<Dynamic::Dsr::VertexAttrib>& attribs, std::vector<DrawItems::VertexType>& instruction,
		std::vector<std::tuple<Material::TextureCategory, std::string, GLuint>>& textures, const std::string& rel_path) {
		/*
		if (HasVertexConfiguration()) {
			auto new_attribs = Dynamic::Dsr::ShaderReflection::GetVertexAttribs(
				std::static_pointer_cast<Bind::ShaderProgram>(GetBindableUnique(typeid(Bind::ShaderProgram)))->get_program());
			std::string error_code = "Vertex configuration does not match properly!Adjust for control node " + m_name + " needed.";
			assert(error_code.c_str() && attribs.size() == m_vertex_instruction.size());
			for (Node* child : m_children) {
				child->CookNode(new_attribs, m_vertex_instruction, m_texture_vector.size() ? m_texture_vector : textures, rel_path);
			}
			
			auto shader = std::static_pointer_cast<Bind::ShaderProgram>(GetBindableUnique(typeid(Bind::ShaderProgram)));
			shader->BindWithoutUpdate();
			for (const auto& t : m_texture_vector) {
				for (const auto& p : t.second) {
					shader->SetTexture(p.first, p.second);
				}
			}
			shader->UnBind();
		}
		else {
			for (Node* child : m_children) {
				child->CookNode(attribs, instruction, m_texture_vector.size() ? m_texture_vector : textures, rel_path);
			}
		}
		*/
	}

	void ControlNode::StartRender() {
		for (Node* child : m_children) {
			child->Render(this, false);
		}
	}

	//TODO : 暂且假设每个control node不会从上一个control node继承bindable等信息，每个control node都含有完整的信息，日后可能会更新这个功能
	void ControlNode::Render(ControlNode* node, bool force_update) {
		if (node) {
			node->UnBindAll();
		}

		if (force_update) {
			m_world_matrix = m_parent->GetWorldMatrix();
		}

		BindAll();
		for (Node* child : m_children) {
			child->Render(this, force_update);
		}
	}

	void ControlNode::Update(ControlNode* node, size_t index) {
		for (size_t ind : m_bindable_sets[m_proxy->m_current_set][0])
		{
			auto b = m_proxy->m_bindables[ind];
			if (b->GetTypeInfo() == typeid(Bind::ShaderProgram))
			{
				std::static_pointer_cast<Bind::ShaderProgram>(b)->UpdateOnly();
			}
			else if (b->GetTypeInfo() == typeid(Bind::ConstantBuffer))
			{
				std::static_pointer_cast<Bind::ConstantBuffer>(b)->Update();
			}
		}
	}

	void ControlNode::ClearTextureConfig()
	{
		m_texture_vector = {};
	}

	void ControlNode::ClearVertexConfig()
	{
		m_vertex_instruction = {};
	}
}
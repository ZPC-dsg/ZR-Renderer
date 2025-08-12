#pragma once

#include <SceneGraph/node.h>
#include <Drawables/drawable.h>

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <typeindex>
#include <algorithm>

namespace Bind {
	class Bindable;
}

namespace SceneGraph {
	class Scene {
	public:
		Scene() = default;
		Scene(const std::string& name);
		~Scene() = default;

		inline void SetName(const std::string& name) noexcept { m_name = name; };
		const std::string& GetName(const std::string& name) noexcept { return m_name; };

		void AddMaterial(std::unique_ptr<Material>&& mat);
		void AddDrawable(const std::type_index& type, std::unique_ptr<DrawItems::Drawable>&& item);

		void SetNodes(std::vector<std::unique_ptr<Node>>&& nodes);
		void AddNode(std::unique_ptr<Node>&& node);
		void AddChild(Node& child);

		Node* FindNode(const std::string& name);
		inline Node& GetRootNode() { return *m_root; };
		inline const std::vector<std::unique_ptr<Node>>& GetNodes() const noexcept { return m_nodes; };

	private:
		void SetupRootNode();

	private:
		std::string m_name;
		std::vector<std::unique_ptr<Node>> m_nodes;
		ControlNode* m_root = nullptr;//根节点一定是一个控制节点
		std::unordered_map<std::type_index, std::vector<std::unique_ptr<DrawItems::Drawable>>> m_drawables;
		std::vector<std::unique_ptr<Material>> m_materials;
	};
}
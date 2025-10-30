#pragma once

#include <SceneGraph/material.h>
#include <SceneGraph/node.h>
#include <logging.h>

namespace Bind {
	class Bindable;
	class RenderTarget;
}

class DrawableLoader;

namespace SceneGraph {
	class Scene;

	class DrawableProxy {
		friend class DrawableLoader;
		friend class Node;
		friend class EntityNode;
		friend class ControlNode;

	public:
		virtual ~DrawableProxy() = default;

		inline ControlNode& GetRoot() const noexcept { return *m_root; };
		inline std::string GetRootName() const noexcept { return m_root->GetName(); };

		DrawableProxy& AddRootBindable(std::shared_ptr<Bind::Bindable> bindable);
		DrawableProxy& AddControlBindable(const std::string& name, std::shared_ptr<Bind::Bindable> bindable);

		DrawableProxy& AddRootTextureRule(const std::string& shader_name, unsigned int binding, Material::TextureCategory type);
		DrawableProxy& AddRootVertexRule(std::vector<DrawItems::VertexType> instruction);
		template <ConfigurationType Type, typename Func, typename... Args>
		DrawableProxy& AddRootUniformRule(Dynamic::Dcb::UniformElementRef ref, Func f, Args&&... args) {
			m_uniform_refs.push_back(ref);
			m_root->RegisterUniform<Type>(m_uniform_refs.back(), f, std::forward<Args>(args)...);
			return *this;
		}
		template <ConfigurationType Type, typename Func, typename... Args>
		DrawableProxy& AddRootConstantRule(Dynamic::Dcb::ConstantElementRef ref, Func f, Args&&... args) {
			m_constant_refs[""][Type].push_back(ref);
			m_root->RegisterConstant<Type>(m_constant_refs.back(), f, std::forward<Args>(args)...);
			return *this;
		}

		DrawableProxy& AddControlTextureRule(const std::string& name, const std::string& shader_name, unsigned int binding, Material::TextureCategory type);
		DrawableProxy& AddControlVertexRule(const std::string& name, std::vector<DrawItems::VertexType> instruction);
		template <ConfigurationType Type, typename Func, typename... Args>
		DrawableProxy& AddControlUniformRule(const std::string& name, Dynamic::Dcb::UniformElementRef ref, Func f, Args&&... args) {
			if (!m_controls.contains(name)) {
				LOG_WARNING("Control node with name: {} does not exist!", name.c_str());
				return *this;
			}

			m_uniform_refs[name][Type].push_back(ref);
			m_controls[name]->RegisterUniform<Type>(m_uniform_refs.back(), f, std::forward<Args>(args)...);
			return *this;
		}
		template <ConfigurationType Type, typename Func, typename... Args>
		DrawableProxy& AddControlConstantRule(const std::string& name, Dynamic::Dcb::ConstantElementRef ref, Func f, Args&&... args) {
			if (!m_controls.contains(name)) {
				LOG_WARNING("Control node with name: {} does not exist!", name.c_str());
				return *this;
			}

			m_constant_refs[name][Type].push_back(ref);
			m_controls[name]->RegisterConstant<Type>(m_constant_refs.back(), f, std::forward<Args>(args)...);
			return *this;
		}

		virtual void Cook();

		void AddControlNode(const std::string& name, const std::string& father_name);

		// Render之前一定要确保current set正确
		void Render(bool clear_texture = true, bool clear_depth = true, bool clear_stencil = false);
		void Bind();//绑定根节点所有bindables避免在渲染的时候重复绑定

		// 开始准备某一个set所需的资源
		void BeginRange(const std::string& name);
		void EndRange();
		void ChangeSet(const std::string& name);

	protected:
		DrawableProxy(Scene* scene, ControlNode& root, const std::string& name, size_t nodes = 0);

	protected:
		ControlNode* m_root;
		std::unordered_map<std::string, ControlNode*> m_controls;

		size_t m_nodecount = 1;
		std::string m_name;

		Scene* m_scene;

		// 假设不同的render set使用不同的shader
		std::vector<std::string> m_render_sets;
		size_t m_current_set;

	protected:
		//保存所有的proxy，因为需要在程序运行中将这些proxy的引用传递给node，它们不能在中途离开作用域
		std::vector<Dynamic::Dcb::UniformElementRef> m_uniform_refs;
		std::vector<Dynamic::Dcb::ConstantElementRef> m_constant_refs;

		// 场景下所有的bindable
		std::vector<std::shared_ptr<Bind::Bindable>> m_bindables;

	protected:
		// 暂时记录由texture rule生成的texture以避免重复生成，在Cook结束后清空
		std::unordered_map<std::string, size_t> m_generated_textures;
	};
}
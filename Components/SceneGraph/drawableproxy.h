#pragma once

#include <SceneGraph/material.h>
#include <SceneGraph/node.h>
#include <logging.h>

namespace Bind {
	class Bindable;
}

class DrawableLoader;

namespace SceneGraph {
	class Scene;

	class DrawableProxy {
		friend class DrawableLoader;

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

		void Render();
		void Bind();//�󶨸��ڵ�����bindables��������Ⱦ��ʱ���ظ���

	protected:
		DrawableProxy(Scene* scene, ControlNode& root, const std::string& name, size_t nodes = 0);

	protected:
		ControlNode* m_root;
		std::unordered_map<std::string, ControlNode*> m_controls;

		size_t m_nodecount = 1;
		std::string m_name;

		Scene* m_scene;

	protected:
		//�������е�proxy����Ϊ��Ҫ�ڳ��������н���Щproxy�����ô��ݸ�node�����ǲ�������;�뿪������
		std::vector<Dynamic::Dcb::UniformElementRef> m_uniform_refs;
		std::vector<Dynamic::Dcb::ConstantElementRef> m_constant_refs;
	};
}
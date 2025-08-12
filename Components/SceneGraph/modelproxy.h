#pragma once

#include <SceneGraph/drawableproxy.h>

#include <vector>
#include <typeindex>
#include <memory>
#include <algorithm>

class AssimpLoader;

namespace SceneGraph {
	class Scene;

	//ģ�ʹ����࣬���ڵ���ģ����Ҫ�ĸ���bindable�Լ��󶨹���
	//����ֱ�ӹ�����������Ҫʹ��AssimpLoader�ྲ̬����LoadModel����
	class ModelProxy :public DrawableProxy {
		friend class AssimpLoader;

	public:
		~ModelProxy() = default;

		ModelProxy& ScaleModel(const glm::vec3& scaling) noexcept;

		void Cook() override;//�����ú�һ��ģ���й�����֮�󣬵������������������ģ�������bindable

	private:
		ModelProxy(Scene* scene, ControlNode& root, size_t count, const std::string& rel_path, const std::string& name);

	private:
		std::string m_relative_path;//�����models�ļ��е�·��
	};
}
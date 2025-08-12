#pragma once

#include <SceneGraph/drawableproxy.h>

#include <vector>
#include <typeindex>
#include <memory>
#include <algorithm>

class AssimpLoader;

namespace SceneGraph {
	class Scene;

	//模型代理类，用于调控模型需要的各种bindable以及绑定规则
	//不能直接构造该类对象，需要使用AssimpLoader类静态函数LoadModel构造
	class ModelProxy :public DrawableProxy {
		friend class AssimpLoader;

	public:
		~ModelProxy() = default;

		ModelProxy& ScaleModel(const glm::vec3& scaling) noexcept;

		void Cook() override;//在设置好一切模型有关数据之后，调用这个函数真正生成模型所需的bindable

	private:
		ModelProxy(Scene* scene, ControlNode& root, size_t count, const std::string& rel_path, const std::string& name);

	private:
		std::string m_relative_path;//相对于models文件夹的路径
	};
}
#pragma once

#include <Bindables/shaderprogram.h>
#include <Bindables/constantbuffer.h>

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <glad/glad.h>

namespace Bind
{
	class Bindable;
	class ShaderProgram;
	class ConstantBuffer;
}

namespace Common
{
	class ComputeProxy
	{
	public:
		ComputeProxy(GLuint dispathX, GLuint dispatchY, GLuint dispatchZ = 1)
			: m_dimensions({ dispathX,dispatchY,dispatchZ })
		{ }
		~ComputeProxy() = default;

		// binding为负代表不改变绑定点。只有纹理或缓冲去有关资源指定binding参数才是有效的
		ComputeProxy& AddBindable(std::shared_ptr<Bind::Bindable> bindable, GLint binding = -1);
		template<typename T>
		ComputeProxy& EditUniform(const std::string& uniform_name, T&& value);
		template<typename T>
		ComputeProxy& EditConstant(const std::string& buffer_name, const std::string& element_name, T&& value);

		// 在添加完所有的bindable之后，并且将所有在渲染循环之前的uniform或constant更新好之后调用
		void Finalize();
		inline bool IsFinalized() const noexcept { return m_is_finalized; }

		void Dispatch();

	private:
		void BindAll();
		void UnBindAll();

	private:
		std::vector<std::shared_ptr<Bind::Bindable>> m_bindables;
		std::shared_ptr<Bind::ShaderProgram> m_compute_shader;
		std::unordered_map<std::string, std::shared_ptr<Bind::ConstantBuffer>> m_constants;

		// 记录下m_bindables中会在执行计算任务时改变binding point的资源的原有binding point，在执行完毕之后恢复binding point
		std::unordered_map<size_t, GLuint> m_binding_change_list;
		std::unordered_map<std::string, GLuint> m_constants_binding_change_list;

		std::vector<GLuint> m_dimensions;

		bool m_is_finalized = false;
	};

	template<typename T>
	ComputeProxy& ComputeProxy::EditUniform(const std::string& uniform, T&& value)
	{
		using type = std::decay_t<T>;
		static_assert(IsVariantMember<type, AvailableType>::value, "Value type not supported!");

		m_compute_shader->EditUniform(uniform) = std::forward<T>(value);
		return *this;
	}

	template<typename T>
	ComputeProxy& ComputeProxy::EditConstant(const std::string& buffer_name, const std::string& element_name, T&& value)
	{
		static_assert(IsVariantMember<T, AvailableType>::value, "Value type not supported!");
		assert(m_constants.contains(buffer_name));

		m_constants[buffer_name]->EditConstant(element_name) = std::forward<T>(value);
		return *this;
	}
}
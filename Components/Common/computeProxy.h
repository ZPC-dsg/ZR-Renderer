#pragma once

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

		ComputeProxy& AddBindable(std::shared_ptr<Bind::Bindable> bindable);
		ComputeProxy& EditUniform(const std::string& uniform_name);
		ComputeProxy& EditConstant(const std::string& buffer_name, const std::string& element_name);

		// 在添加完所有的bindable之后调用
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

		std::vector<GLuint> m_dimensions;

		bool m_is_finalized = false;
	};
}
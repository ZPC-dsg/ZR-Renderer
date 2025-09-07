#pragma once

#include <Bindables/bindable.h>
#include <resourcefactory.h>

namespace Bind
{
	class AtomicCounter :public Bindable
	{
	public:
		AtomicCounter(const std::string& counter_name, const std::vector<GLuint>& initial_value, GLuint binding);
		~AtomicCounter() = default;

		void Bind() noxnd override;
		void UnBind() noxnd override;

		static std::shared_ptr<AtomicCounter> Resolve(const std::string& counter_name, const std::vector<GLuint>& initial_value, GLuint binding);
		static std::string GenerateUID(const std::string& counter_name, const std::vector<GLuint>& initial_value, GLuint binding);

		std::string GetUID() const noexcept override;
		std::type_index GetTypeInfo() const noexcept override;

		AtomicCounter& UpdateInitialData(const std::vector<GLuint>& initial_value, size_t start = 0);

	private:
		GLuint m_binding;
		std::string m_counter_name;
		std::vector<GLuint> m_initial_value;

		std::shared_ptr<RawBuffer> m_buffer;
	};
}
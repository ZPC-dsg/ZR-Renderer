#include <Bindables/atomicounter.h>
#include <Bindables/bindable_resolver.h>

namespace Bind
{
	AtomicCounter::AtomicCounter(const std::string& counter_name, const std::string& buffer_name, const std::vector<GLuint>& initial_value, GLuint binding)
		:m_counter_name(counter_name), m_binding(binding), m_initial_value(initial_value)
	{
		m_buffer = std::static_pointer_cast<RawBuffer>(ResourceFactory::CreateBuffer(buffer_name, initial_value.size() * sizeof(GLuint), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT));
		m_buffer->BindBase(GL_ATOMIC_COUNTER_BUFFER, binding);

		m_buffer->UpdateMap(initial_value.size() * sizeof(GLuint), 0, initial_value.data());
	}

	void AtomicCounter::Bind() noxnd
	{
		m_buffer->BindBase(GL_ATOMIC_COUNTER_BUFFER, m_binding);
		m_buffer->UpdateMap(m_initial_value.size() * sizeof(GLuint), 0, m_initial_value.data());
	}

	void AtomicCounter::UnBind() noxnd
	{
		m_buffer->BindBase(GL_ATOMIC_COUNTER_BUFFER, 0);
	}

	std::shared_ptr<AtomicCounter> AtomicCounter::Resolve(const std::string& counter_name, const std::string& buffer_name, const std::vector<GLuint>& initial_value, GLuint binding)
	{
		return BindableResolver::Resolve<AtomicCounter>(counter_name, buffer_name, initial_value, binding);
	}

	std::string AtomicCounter::GenerateUID(const std::string& counter_name, const std::string& buffer_name, const std::vector<GLuint>& initial_value, GLuint binding)
	{
		using namespace std::string_literals;
		return typeid(AtomicCounter).name() + "#"s + counter_name + "#"s + buffer_name;
	}

	std::string AtomicCounter::GetUID() const noexcept
	{
		return GenerateUID(m_counter_name, m_buffer->ResourceName(), {}, m_binding);
	}

	std::type_index AtomicCounter::GetTypeInfo() const noexcept
	{
		return typeid(AtomicCounter);
	}

	AtomicCounter& AtomicCounter::UpdateInitialData(const std::vector<GLuint>& initial_value, size_t start)
	{
		assert(start + initial_value.size() <= m_initial_value.size());
		std::copy(initial_value.begin(), initial_value.end(), m_initial_value.begin() + start);

		return *this;
	}
}
#include <Bindables/storagebuffer.h>
#include <Bindables/bindable_resolver.h>

namespace Bind
{
	StorageBuffer::StorageBuffer(const std::string& name, const std::string& buffer_name, size_t size, GLuint binding, GLbitfield flags)
		:m_name(name), m_binding(binding), m_size(size)
	{
		m_buffer = std::static_pointer_cast<RawBuffer>(ResourceFactory::CreateBuffer(buffer_name, size, flags));
		m_buffer->BindBase(GL_SHADER_STORAGE_BUFFER, binding);
	}

	void StorageBuffer::Bind() noxnd
	{
		m_buffer->BindBase(GL_SHADER_STORAGE_BUFFER, m_binding);
	}

	void StorageBuffer::UnBind() noxnd
	{
		m_buffer->BindBase(GL_SHADER_STORAGE_BUFFER, 0);
	}

	std::shared_ptr<StorageBuffer> StorageBuffer::Resolve(const std::string& name, const std::string& buffer_name, size_t size, GLuint binding, GLbitfield flags)
	{
		return BindableResolver::Resolve<StorageBuffer>(name, buffer_name, size, binding, flags);
	}

	std::string StorageBuffer::GenerateUID(const std::string& name, const std::string& buffer_name, size_t size, GLuint binding, GLbitfield flags)
	{
		using namespace std::string_literals;
		return typeid(StorageBuffer).name() + "#"s + name + "#"s + buffer_name;
	}

	std::string StorageBuffer::GetUID() const noexcept
	{
		return GenerateUID(m_name, m_buffer->ResourceName(), m_size, m_binding, m_buffer->StorageFlags());
	}

	std::type_index StorageBuffer::GetTypeInfo() const noexcept
	{
		return typeid(StorageBuffer);
	}
}
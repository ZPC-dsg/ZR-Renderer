#include <Bindables/pixelunpackbuffer.h>
#include <Bindables/bindable_resolver.h>

namespace Bind
{
	PixelUnpackBuffer::PixelUnpackBuffer(const std::string& name, const std::string& buffer_name, size_t size, GLenum data_type, void* data)
		:m_name(name), m_data_type(data_type)
	{
		m_buffer = std::static_pointer_cast<RawBuffer>(ResourceFactory::CreateBuffer(buffer_name, size, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT));
		m_buffer->UpdateCopy(size, 0, data);
	}

	void PixelUnpackBuffer::Bind() noxnd
	{
		m_buffer->Bind(GL_PIXEL_UNPACK_BUFFER);
	}

	void PixelUnpackBuffer::UnBind() noxnd
	{
		m_buffer->UnBind(GL_PIXEL_UNPACK_BUFFER);
	}

	std::shared_ptr<PixelUnpackBuffer> PixelUnpackBuffer::Resolve(const std::string& name, const std::string& buffer_name, size_t size, GLenum data_type, void* data)
	{
		return BindableResolver::Resolve<PixelUnpackBuffer>(name, buffer_name, size, data_type, data);
	}

	std::string PixelUnpackBuffer::GenerateUID(const std::string& name, const std::string& buffer_name, size_t size, GLenum data_type, void* data)
	{
		using namespace std::string_literals;
		return typeid(PixelUnpackBuffer).name() + "#"s + name + "#"s + buffer_name;
	}

	std::string PixelUnpackBuffer::GetUID() const noexcept
	{
		return GenerateUID(m_name, m_buffer->ResourceName(), 0, m_data_type, nullptr);
	}

	std::type_index PixelUnpackBuffer::GetTypeInfo() const noexcept
	{
		return typeid(PixelUnpackBuffer);
	}

	void PixelUnpackBuffer::Update(void* data, size_t offset, size_t size) noxnd
	{
		m_buffer->UpdateCopy(size == 0 ? m_size : (size_t)size, offset, data);
	}
}
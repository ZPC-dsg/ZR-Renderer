#include <Bindables/texturebuffer.h>
#include <Bindables/bindable_resolver.h>

namespace Bind
{
	TextureBuffer::TextureBuffer(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit)
		:AbstractTexture({}, texture_unit), m_name(name), m_size(size), m_buffer_unit(buffer_unit)
	{
		std::string buffer_name = name + "_buffer#";
		m_buffer = std::static_pointer_cast<RawBuffer>(ResourceFactory::CreateBuffer(buffer_name, size, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT));

		std::string texture_name = name + "_texture#";
		m_resource = std::static_pointer_cast<RawTexture2D>(ResourceFactory::CreateTexture2D(texture_name, format));

		glTextureBuffer(m_resource->GetResource(), format, m_buffer->GetResource());
	}

	TextureBuffer::TextureBuffer(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, void* data, bool is_texture)
		:AbstractTexture({}, texture_unit), m_name(name), m_size(size), m_buffer_unit(buffer_unit), m_is_texture(is_texture)
	{
		std::string buffer_name = name + "_buffer#";
		m_buffer = std::static_pointer_cast<RawBuffer>(ResourceFactory::CreateBuffer(buffer_name, size, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT));
		m_buffer->UpdateCopy(size, 0, data);

		std::string texture_name = name + "_texture#";
		m_resource = std::static_pointer_cast<RawTexture2D>(ResourceFactory::CreateTexture2D(texture_name, format));

		glTextureBuffer(m_resource->GetResource(), format, m_buffer->GetResource());
	}

	TextureBuffer::TextureBuffer(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, std::shared_ptr<RawBuffer> buffer, size_t start_offset, size_t map_size, bool is_texture)
		:AbstractTexture({}, texture_unit), m_name(name), m_size(size), m_buffer_unit(buffer_unit), m_buffer(buffer), m_is_texture(is_texture)
	{
		std::string texture_name = name + "_texture#";
		m_resource = std::static_pointer_cast<RawTexture2D>(ResourceFactory::CreateTexture2D(texture_name, format));

		glTextureBufferRange(m_resource->GetResource(), format, m_buffer->GetResource(), start_offset, map_size == 0 ? m_size : map_size);
	}

	void TextureBuffer::Bind() noxnd
	{
		m_is_texture ? m_resource->Bind(m_unit) : m_resource->BindAsStorage(m_buffer_unit);
	}

	void TextureBuffer::UnBind() noxnd
	{
		m_is_texture ? m_resource->UnBind(m_unit) : m_resource->UnBindAsStorage(m_buffer_unit);
	}

	std::shared_ptr<TextureBuffer> TextureBuffer::Resolve(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit)
	{
		return BindableResolver::Resolve<TextureBuffer>(name, size, format, texture_unit, buffer_unit);
	}

	std::shared_ptr<TextureBuffer> TextureBuffer::Resolve(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, void* data, bool is_texture)
	{
		return BindableResolver::Resolve<TextureBuffer>(name, size, format, texture_unit, buffer_unit, data, is_texture);
	}

	std::shared_ptr<TextureBuffer> TextureBuffer::Resolve(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, std::shared_ptr<RawBuffer> buffer, size_t start_offset, size_t map_size, bool is_texture)
	{
		return BindableResolver::Resolve<TextureBuffer>(name, size, format, texture_unit, buffer_unit, buffer, start_offset, map_size, is_texture);
	}

	std::string TextureBuffer::GenerateUID(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit)
	{
		using namespace std::string_literals;
		return typeid(TextureBuffer).name() + "#"s + std::to_string(texture_unit) + "#"s + std::to_string(buffer_unit);
	}

	std::string TextureBuffer::GenerateUID(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, void* data, bool is_texture)
	{
		using namespace std::string_literals;
		return typeid(TextureBuffer).name() + "#"s + std::to_string(texture_unit) + "#"s + std::to_string(buffer_unit);
	}

	std::string TextureBuffer::GenerateUID(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, std::shared_ptr<RawBuffer> buffer, size_t start_offset, size_t map_size, bool is_texture)
	{
		using namespace std::string_literals;
		return typeid(TextureBuffer).name() + "#"s + std::to_string(texture_unit) + "#"s + std::to_string(buffer_unit);
	}

	std::string TextureBuffer::GetUID() const noexcept
	{
		return GenerateUID(m_name, m_size, GetDataFormat(), m_unit, m_buffer_unit);
	}

	std::type_index TextureBuffer::GetTypeInfo() const noexcept
	{
		return typeid(TextureBuffer);
	}

	GLenum TextureBuffer::GetDataFormat() const noexcept
	{
		return m_resource->GetDescription().internal_format;
	}

	void TextureBuffer::UpdateBuffer(void* data, size_t offset, size_t size)
	{
		m_buffer->UpdateCopy(size == 0 ? m_size : size, offset, data);
	}
}
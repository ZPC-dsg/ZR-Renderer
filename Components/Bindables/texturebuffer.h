#pragma once

#include <Bindables/abstracttexture.h>
#include <resourcefactory.h>

namespace Bind
{
	class TextureBuffer :public AbstractTexture
	{
	public:
		// 所拥有的纹理和缓冲的名字自动生成，不用用户设置
		// TODO : 以后所有的缓冲区和纹理等实际opengl资源名称都不用用户设置
		TextureBuffer(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit);
		TextureBuffer(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, void* data, bool is_texture = false);
		TextureBuffer(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, std::shared_ptr<RawBuffer> buffer, size_t start_offset = 0, size_t map_size = 0, bool is_texture = false);
		~TextureBuffer() = default;

		void Bind() noxnd override;
		void UnBind() noxnd override;

		static std::shared_ptr<TextureBuffer> Resolve(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit);
		static std::shared_ptr<TextureBuffer> Resolve(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, void* data, bool is_texture = false);
		static std::shared_ptr<TextureBuffer> Resolve(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, std::shared_ptr<RawBuffer> buffer, size_t start_offset = 0, size_t map_size = 0, bool is_texture = false);

		static std::string GenerateUID(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit);
		static std::string GenerateUID(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, void* data, bool is_texture = false);
		static std::string GenerateUID(const std::string& name, size_t size, GLenum format, GLuint texture_unit, GLuint buffer_unit, std::shared_ptr<RawBuffer> buffer, size_t start_offset = 0, size_t map_size = 0, bool is_texture = false);
		std::string GetUID() const noexcept override;
		std::type_index GetTypeInfo() const noexcept override;
		GLenum GetDataFormat() const noexcept;
		
		inline bool UsedAsTexture() const noexcept { return m_is_texture; }
		inline void SetUsage(bool texture) noexcept { m_is_texture = texture; }

		void UpdateBuffer(void* data, size_t offset = 0, size_t size = 0) noxnd;

	private:
		std::string m_name;
		size_t m_size;

		size_t m_start_offset = 0;
		size_t m_map_size = 0;

		GLuint m_buffer_unit;
		std::shared_ptr<RawBuffer> m_buffer;

		bool m_is_texture = false;
	};
}
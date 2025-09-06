#pragma once

#include <Bindables/abstracttexture.h>

namespace Bind
{
	class PixelUnpackBuffer;

	class StorageTexture2D :public AbstractTexture
	{
	public:
		StorageTexture2D(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, std::shared_ptr<PixelUnpackBuffer> pub = nullptr);
		StorageTexture2D(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, void* data = nullptr);
		~StorageTexture2D() = default;

		void Bind() noxnd override;
		void UnBind() noxnd override;

		static std::shared_ptr<StorageTexture2D> Resolve(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, std::shared_ptr<PixelUnpackBuffer> pub = nullptr);
		static std::shared_ptr<StorageTexture2D> Resolve(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, void* data = nullptr);
		static std::string GenerateUID(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, std::shared_ptr<PixelUnpackBuffer> pub = nullptr);
		static std::string GenerateUID(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, void* data = nullptr);

		std::string GetUID() const noexcept override;
		std::type_index GetTypeInfo() const noexcept override;

		void UpdatePUBData(void* data, size_t offset = 0, size_t size = 0) noxnd;

	private:
		std::string m_image_name;

		std::shared_ptr<PixelUnpackBuffer> m_pub;
	};
}
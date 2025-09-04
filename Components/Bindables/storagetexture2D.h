#pragma once

#include <Bindables/abstracttexture.h>

namespace Bind
{
	class StorageTexture2D :public AbstractTexture
	{
	public:
		StorageTexture2D(const std::string& name, const OGL_TEXTURE2D_DESC& desc, GLuint unit);
		~StorageTexture2D() = default;

		void Bind() noxnd override;
		void UnBind() noxnd override;

		static std::shared_ptr<StorageTexture2D> Resolve(const std::string& name, const OGL_TEXTURE2D_DESC& desc, GLuint unit);
		static std::string GenerateUID(const std::string& name, const OGL_TEXTURE2D_DESC& desc, GLuint unit);

		std::string GetUID() const noexcept override;
		std::type_index GetTypeInfo() const noexcept override;

	private:
	};
}
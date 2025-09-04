#pragma once

#include <Bindables/abstracttexture.h>

namespace Bind
{
	class StorageTexture2DArray :public AbstractTexture
	{
	public:
		StorageTexture2DArray(const std::string& name, const OGL_TEXTURE2D_DESC& desc, GLuint unit);
		~StorageTexture2DArray() = default;

		void Bind() noxnd override;
		void UnBind() noxnd override;

		static std::shared_ptr<StorageTexture2DArray> Resolve(const std::string& name, const OGL_TEXTURE2D_DESC& desc, GLuint unit);
		static std::string GenerateUID(const std::string& name, const OGL_TEXTURE2D_DESC& desc, GLuint unit);
		
		std::string GetUID() const noexcept override;
		std::type_index GetTypeInfo() const noexcept override;

		inline void ChangeToSingleSlice(GLint slice)
		{
			m_is_layered = GL_FALSE;
			m_layer = slice;
		}

		inline void ChangeToAllSlices()
		{
			m_is_layered = GL_TRUE;
			m_layer = 0;
		}

	private:
		GLboolean m_is_layered = GL_TRUE;
		GLint m_layer = 0;
	};
}
#pragma once

#include <Bindables/includer.h>
#include <resourcefactory.h>

namespace Bind {
	class AbstractTexture :public Bindable {
		friend class RenderTarget;
	public:
		AbstractTexture(const OGL_TEXTURE_PARAMETER& param, GLuint unit, const std::string& tag = "");
		AbstractTexture(std::shared_ptr<RawTexture2D> resource, const OGL_TEXTURE_PARAMETER& param, GLuint unit, const std::string& tag = "");
		~AbstractTexture() = default;

		inline OGL_TEXTURE_PARAMETER get_parameter() const noexcept { return m_params; }
		inline OGL_TEXTURE2D_DESC get_description() const noexcept { return m_resource->GetDescription(); }
		void set_params(const OGL_TEXTURE_PARAMETER& param) noxnd;

		bool is_depth_only() const noexcept;

		std::string resource_name() const noexcept;
		inline std::string Tag() const noexcept { return m_tag; }

		void ChangeBindingPoint(GLuint binding) noexcept override;
		GLint GetBindingPoint() const noexcept override;
		bool NeedBindingPoint() noexcept override;

		std::shared_ptr<AbstractResource> GetResource();
		GLuint GetResourceRaw();

		// 复制整张image
		void CopyImage(std::shared_ptr<AbstractTexture> src, GLint src_level = 0, GLint dst_level = 0);
		// 仅仅重新创建底层资源，名称，绑定目标等均不改变
		void DestroyAndCreateNew(const OGL_TEXTURE2D_DESC& new_desc, void* data = nullptr);
		// 只有在确保m_resource已经被销毁之后才调用该函数
		void UpdateNewResource(std::shared_ptr<AbstractResource> resource, const OGL_TEXTURE_PARAMETER& params);

	protected:
		static unsigned char* gen_image_from_file(const std::string& path, int& width, int& height, int& channels, bool is_model = false);

	protected:
		OGL_TEXTURE_PARAMETER m_params;
		GLuint m_unit;
		std::string m_tag;

		std::shared_ptr<RawTexture2D> m_resource;
	};
}
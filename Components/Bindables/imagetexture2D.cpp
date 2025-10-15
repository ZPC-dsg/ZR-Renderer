#include <Bindables/imagetexture2D.h>
#include <logging.h>

#include <stb_image.h>

namespace Bind {
	ImageTexture2D::ImageTexture2D(const std::string& tag, const std::string& path, const OGL_TEXTURE_PARAMETER& param, GLuint unit, bool is_model,  bool generate_mip)
		:AbstractTexture(param, unit) {
		LOGI("Loading texture: {}...;Path: {}", tag.c_str(), path.c_str());

		int width, height, channels;
		unsigned char* data = gen_image_from_file(path, width, height, channels, is_model);

		OGL_TEXTURE2D_DESC desc;
		desc.target = GL_TEXTURE_2D;
		desc.width = width;
		desc.height = height;
		if (channels == 4) {
			desc.cpu_format = GL_RGBA;
			desc.internal_format = GL_RGBA8;
		}
		else if (channels == 1) {
			desc.cpu_format = GL_RED;
			desc.internal_format = GL_R8;
		}

		m_resource = std::dynamic_pointer_cast<RawTexture2D>(ResourceFactory::CreateTexture2D(tag, desc));

		m_resource->Update({ 0,0,(float)desc.width,(float)desc.height }, desc.cpu_format, desc.data_type, (void*)data, true);
		m_resource->SetParameters(param);

		stbi_image_free(data);

		m_path = path;

		LOGI("Loading finished!");
	}

	ImageTexture2D::ImageTexture2D(const std::string& tag, std::shared_ptr<RawTexture2D> texture2D, const OGL_TEXTURE_PARAMETER& param, GLuint unit, bool generate_mip)
		:AbstractTexture(texture2D, param, unit, tag) {
		m_resource->SetParameters(param);
		if (generate_mip) {
			m_resource->GenerateMips();
		}
	}

	ImageTexture2D::ImageTexture2D(const std::string& tag, const OGL_TEXTURE2D_DESC& desc, const OGL_TEXTURE_PARAMETER& param, GLuint unit, bool generate_mip)
		:AbstractTexture(nullptr, param, unit, tag)
	{
		std::string resource_name = tag + "_resource";
		m_resource = std::dynamic_pointer_cast<RawTexture2D>(ResourceFactory::CreateTexture2D(resource_name, desc));
		m_resource->SetParameters(param);
	}

	void ImageTexture2D::Bind() noxnd {
		m_resource->Bind(m_unit);
	}

	void ImageTexture2D::UnBind() noxnd {
		m_resource->UnBind(0);
	}

	std::shared_ptr<ImageTexture2D> ImageTexture2D::Resolve(const std::string& tag, const std::string& path, const OGL_TEXTURE_PARAMETER& param, GLuint unit, bool is_model, bool generate_mip) {
		return BindableResolver::Resolve<ImageTexture2D>(tag, path, param, unit, is_model, generate_mip);
	}

    std::shared_ptr<ImageTexture2D> ImageTexture2D::Resolve(const std::string& tag, std::shared_ptr<RawTexture2D> texture2D, const OGL_TEXTURE_PARAMETER& param, GLuint unit, bool generate_mip ) {
		return BindableResolver::Resolve<ImageTexture2D>(tag, texture2D, param, unit, generate_mip);
	}

	std::shared_ptr<ImageTexture2D> ImageTexture2D::Resolve(const std::string& tag, const OGL_TEXTURE2D_DESC& desc, const OGL_TEXTURE_PARAMETER& param, GLuint unit, bool generate_mip)
	{
		return BindableResolver::Resolve<ImageTexture2D>(tag, desc, param, unit, generate_mip);
	}

	std::string ImageTexture2D::GenerateUID(const std::string& tag, const std::string& path, const OGL_TEXTURE_PARAMETER& param, GLuint unit, bool is_model, bool generate_mip) {
		using namespace std::string_literals;
		return typeid(ImageTexture2D).name() + "#"s + tag + "#_PATH: "s + path;
	}

	std::string ImageTexture2D::GenerateUID(const std::string& tag, std::shared_ptr<RawTexture2D> texture2D, const OGL_TEXTURE_PARAMETER& param, GLuint unit, bool generate_mip) {
		using namespace std::string_literals;

		return typeid(ImageTexture2D).name() + "#"s + tag + "#"s + texture2D->ResourceName();
	}

	std::string ImageTexture2D::GenerateUID(const std::string& tag, const OGL_TEXTURE2D_DESC& desc, const OGL_TEXTURE_PARAMETER& param, GLuint unit, bool generate_mip)
	{
		using namespace std::string_literals;

		return typeid(ImageTexture2D).name() + "#"s + tag + "#"s + tag + "_resource";
	}

	std::string ImageTexture2D::GetUID() const noexcept {
		return m_path.length() ? GenerateUID(resource_name(), m_path, m_params, m_unit, false) :
			GenerateUID(m_tag, m_resource, m_params, m_unit, false);//最后三个参数不影响名称
	}

	std::type_index ImageTexture2D::GetTypeInfo() const noexcept {
		return typeid(ImageTexture2D);
	}
}
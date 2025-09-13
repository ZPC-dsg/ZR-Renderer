#define ENABLE_GPU_TYPE_MAPPER

#include <Bindables/storagetexture2D.h>
#include <Macros/shader_typedef.h>
#include <Bindables/pixelunpackbuffer.h>
#include <logging.h>

#include <cstdlib>

namespace Bind
{
	StorageTexture2D::StorageTexture2D(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, std::shared_ptr<PixelUnpackBuffer> pub)
		:AbstractTexture({}, unit), m_image_name(image_name), m_pub(pub)
	{
		std::string texture_name = image_name + "_texture#";
		m_resource = std::static_pointer_cast<RawTexture2D>(ResourceFactory::CreateTexture2D(texture_name, desc));
	}

	StorageTexture2D::StorageTexture2D(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, void* data)
		:AbstractTexture({}, unit), m_image_name(image_name)
	{
		bool type_fail = false;
		GLenum real_cpu_format = desc.cpu_format;
		GLenum real_data_type = desc.data_type;
		switch (desc.internal_format)
		{
#define X(GPU_TYPE, CPU_TYPE, DATA_TYPE, SIZE) case GPU_TYPE : \
			{ \
				if constexpr (GPUTypeMapper<GPU_TYPE>::is_valid) \
				{ \
					if (GPUTypeMapper<GPU_TYPE>::cpu_type != desc.cpu_format) \
					{ \
						LOGW("CPU data type specified does not match the gpu type specified!CPU type specified : {}.CPU type should be : {}.", desc.cpu_format, GPUTypeMapper<GPU_TYPE>::cpu_type_name); \
						type_fail = true; \
						real_cpu_format = GPUTypeMapper<GPU_TYPE>::cpu_type; \
					} \
					if (GPUTypeMapper<GPU_TYPE>::data_type != desc.data_type) \
					{ \
						LOGW("CPU element type specified does not match the gpu type specified!Element type specified : {}.Element type should be : {}.", desc.data_type, GPUTypeMapper<GPU_TYPE>::data_type_name); \
						type_fail = true; \
						real_data_type = GPUTypeMapper<GPU_TYPE>::data_type; \
					} \
				} \
				break; \
			}

			GPU_TYPE_MAPPER
#undef X
		}

		std::string texture_name = image_name + "_texture#";
		if (type_fail)
		{
			OGL_TEXTURE2D_DESC new_desc = desc;
			new_desc.cpu_format = real_cpu_format;
			new_desc.data_type = real_data_type;
			m_resource = std::static_pointer_cast<RawTexture2D>(ResourceFactory::CreateTexture2D(texture_name, new_desc));
		}
		else
		{
			m_resource = std::static_pointer_cast<RawTexture2D>(ResourceFactory::CreateTexture2D(texture_name, desc));
		}

		if (data)
		{
			std::string pub_name = image_name + "__pub#";
			std::string buffer_name = image_name + "__pub_buffer#";
			size_t pub_size;
			switch (desc.internal_format)
			{
#define X(GPU_TYPE, CPU_TYPE, DATA_TYPE, SIZE) case GPU_TYPE : \
			{ \
				if constexpr (GPUTypeMapper<GPU_TYPE>::is_valid) \
				{ \
					pub_size = desc.width * desc.height * GPUTypeMapper<GPU_TYPE>::pixel_size; \
					break; \
				} \
				else \
				{ \
					LOGE("Invalid internal type : {} occurred", GPUTypeMapper<GPU_TYPE>::gpu_type_name); \
					std::exit(EXIT_FAILURE); \
				} \
			}

				GPU_TYPE_MAPPER
#undef X
			}
			m_pub = BindableResolver::Resolve<PixelUnpackBuffer>(pub_name, buffer_name, pub_size, desc.data_type, data);
		}
	}

	void StorageTexture2D::Bind() noxnd
	{
		if (m_used_as_storage)
		{
			if (m_pub && m_should_initialize)
			{
				InitializeStorage();
			}
			else
			{
				m_resource->BindAsStorage(m_unit);
			}
		}
		else
		{
			m_resource->Bind(m_unit);
		}
	}

	void StorageTexture2D::UnBind() noxnd
	{
		m_resource->UnBindAsStorage(0);
	}

	std::shared_ptr<StorageTexture2D> StorageTexture2D::Resolve(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, std::shared_ptr<PixelUnpackBuffer> pub)
	{
		return BindableResolver::Resolve<StorageTexture2D>(image_name, desc, unit, pub);
	}

	std::shared_ptr<StorageTexture2D> StorageTexture2D::Resolve(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, void* data)
	{
		return BindableResolver::Resolve<StorageTexture2D>(image_name, desc, unit, data);
	}

	std::string StorageTexture2D::GenerateUID(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, std::shared_ptr<PixelUnpackBuffer> pub)
	{
		using namespace std::string_literals;
		return typeid(StorageTexture2D).name() + "#"s + image_name + "#"s + std::to_string(unit);
	}

	std::string StorageTexture2D::GenerateUID(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit, void* data)
	{
		using namespace std::string_literals;
		return typeid(StorageTexture2D).name() + "#"s + image_name + "#"s + std::to_string(unit);
	}

	std::string StorageTexture2D::GetUID() const noexcept
	{
		return GenerateUID(m_image_name, m_resource->GetDescription(), m_unit, m_pub);
	}

	std::type_index StorageTexture2D::GetTypeInfo() const noexcept {
		return typeid(StorageTexture2D);
	}
	
	void StorageTexture2D::UpdatePUBData(void* data, size_t offset, size_t size) noxnd
	{
		m_pub->Bind();
		m_pub->Update(data, offset, size);
		m_pub->UnBind();
	}

	void StorageTexture2D::InitializeStorage()
	{
		m_pub->Bind();
		m_resource->BindAsStorage(m_unit);
		const auto& desc = m_resource->GetDescription();
		m_resource->Update({ 0, 0, float(desc.width), float(desc.height) }, desc.cpu_format, desc.data_type, (void*)0);
		m_pub->UnBind();
	}
}
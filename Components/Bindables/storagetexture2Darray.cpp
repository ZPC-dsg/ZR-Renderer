#include <Bindables/storagetexture2Darray.h>

namespace Bind
{
	StorageTexture2DArray::StorageTexture2DArray(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit)
		:AbstractTexture({}, unit), m_image_name(image_name)
	{
		std::string texture_name = image_name + "_texture#";
		m_resource = std::static_pointer_cast<RawTexture2D>(ResourceFactory::CreateTexture2D(texture_name, desc));
	}

	void StorageTexture2DArray::Bind() noxnd
	{
		m_resource->BindAsStorage(m_unit, m_is_layered, m_layer);
	}

	void StorageTexture2DArray::UnBind() noxnd
	{
		m_resource->UnBindAsStorage(m_unit);
	}

	std::shared_ptr<StorageTexture2DArray> StorageTexture2DArray::Resolve(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit)
	{
		return BindableResolver::Resolve<StorageTexture2DArray>(image_name, desc, unit);
	}

	std::string StorageTexture2DArray::GenerateUID(const std::string& image_name, const OGL_TEXTURE2D_DESC& desc, GLuint unit)
	{
		using namespace std::string_literals;
		return typeid(StorageTexture2DArray).name() + "#"s + image_name + "#"s + std::to_string(unit);
	}

	std::string StorageTexture2DArray::GetUID() const noexcept
	{
		return GenerateUID(m_image_name,  m_resource->GetDescription(), m_unit);
	}

	std::type_index StorageTexture2DArray::GetTypeInfo() const noexcept {
		return typeid(StorageTexture2DArray);
	}
}
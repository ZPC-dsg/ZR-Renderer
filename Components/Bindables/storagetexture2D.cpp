#include <Bindables/storagetexture2D.h>

namespace Bind
{
	StorageTexture2D::StorageTexture2D(const std::string& name, const OGL_TEXTURE2D_DESC& desc, GLuint unit)
		:AbstractTexture({}, unit)
	{
		m_resource = std::static_pointer_cast<RawTexture2D>(ResourceFactory::CreateTexture2D(name, desc));
	}

	void StorageTexture2D::Bind() noxnd
	{
		m_resource->BindAsStorage(m_unit);
	}

	void StorageTexture2D::UnBind() noxnd
	{
		m_resource->UnBindAsStorage(0);
	}

	std::shared_ptr<StorageTexture2D> StorageTexture2D::Resolve(const std::string& name, const OGL_TEXTURE2D_DESC& desc, GLuint unit)
	{
		return BindableResolver::Resolve<StorageTexture2D>(name, desc, unit);
	}

	std::string StorageTexture2D::GenerateUID(const std::string& name, const OGL_TEXTURE2D_DESC& desc, GLuint unit)
	{
		using namespace std::string_literals;
		return typeid(StorageTexture2D).name() + "#"s + name;
	}

	std::string StorageTexture2D::GetUID() const noexcept
	{
		return GenerateUID(resource_name(), m_resource->GetDescription(), m_unit);
	}

	std::type_index StorageTexture2D::GetTypeInfo() const noexcept {
		return typeid(StorageTexture2D);
	}
}
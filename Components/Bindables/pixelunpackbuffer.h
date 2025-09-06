#pragma once

#include <Bindables/bindable.h>
#include <resourcefactory.h>

namespace Bind
{
	class PixelUnpackBuffer :public Bindable
	{
	public:
		PixelUnpackBuffer(const std::string& name, const std::string& buffer_name, size_t size, GLenum data_type, void* data = nullptr);
		~PixelUnpackBuffer() = default;

		void Bind() noxnd override;
		void UnBind() noxnd override;

		static std::shared_ptr<PixelUnpackBuffer> Resolve(const std::string& name, const std::string& buffer_name, size_t size, GLenum data_type, void* data = nullptr);
		static std::string GenerateUID(const std::string& name, const std::string& buffer_name, size_t size, GLenum data_type, void* data = nullptr);

		std::string GetUID() const noexcept override;
		std::type_index GetTypeInfo() const noexcept override;

		void Update(void* data, size_t offset = 0, size_t size = 0) noxnd; // 不允许改变数据类型，只是更新数据本身

	private:
		std::string m_name;
		GLenum m_data_type;
		size_t m_size;
		
		std::shared_ptr<RawBuffer> m_buffer;
	};
}
#pragma once

#include <Bindables/bindable.h>
#include <resourcefactory.h>

namespace Bind
{
	class StorageBuffer :public Bindable
	{
	public:
		StorageBuffer(const std::string& name, const std::string& buffer_name, size_t size, GLuint binding, GLbitfield flags = 0);
		~StorageBuffer() = default;

		void Bind() noxnd override;
		void UnBind() noxnd override;

		static std::shared_ptr<StorageBuffer> Resolve(const std::string& name, const std::string& buffer_name, size_t size, GLuint binding, GLbitfield flags = 0);
		static std::string GenerateUID(const std::string& name, const std::string& buffer_name, size_t size, GLuint binding, GLbitfield flags = 0);

		std::string GetUID() const noexcept override;
		std::type_index GetTypeInfo() const noexcept override;

	private:
		std::string m_name;
		GLuint m_binding;
		size_t m_size;

		std::shared_ptr<RawBuffer> m_buffer;
	};
}
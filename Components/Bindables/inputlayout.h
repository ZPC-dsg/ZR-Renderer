#pragma once

#include <Bindables/vertexbuffer.h>
#include <Bindables/indexbuffer.h>

namespace Bind {
	class InputLayout :public Bindable {
	public:
		InputLayout(const std::string& tag, std::vector<std::shared_ptr<VertexBuffer>> vertex, std::vector<std::shared_ptr<VertexBuffer>> instance = {}, std::shared_ptr<IndexBuffer> index = nullptr);
		~InputLayout() = default;

		void Bind() noxnd override;
		void UnBind() noxnd override;

		static std::shared_ptr<InputLayout> Resolve(const std::string& tag, std::vector<std::shared_ptr<VertexBuffer>> vertex, std::vector<std::shared_ptr<VertexBuffer>> instance = {}, std::shared_ptr<IndexBuffer> index = nullptr);
		static std::string GenerateUID(const std::string& tag, std::vector<std::shared_ptr<VertexBuffer>> vertex, std::vector<std::shared_ptr<VertexBuffer>> instance = {}, std::shared_ptr<IndexBuffer> index = nullptr);
		std::string GetUID() const noexcept override;

		std::type_index GetTypeInfo() const noexcept override;

		inline bool HasIndexBuffer() const noexcept { return m_index != nullptr; };
		inline bool IsInstanceDraw() const noexcept { return m_instance.size(); };
		unsigned int GetVertexCount() const noexcept;//���ٴ���һ��pervertex��vertexbuffer
		unsigned int GetInstanceCount() const noxnd;

	private:
		void bind_impl() noxnd;

	private:
		GLuint m_VAO;
		std::string m_tag;
		std::vector<std::shared_ptr<VertexBuffer>> m_vertex;//���ܻ�󶨶��buffer������һ���𶥵㻺�壬һ����ʵ������
		std::vector<std::shared_ptr<VertexBuffer>> m_instance;
		std::shared_ptr<IndexBuffer> m_index;
	};
}
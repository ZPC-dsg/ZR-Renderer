#pragma once

#include <Bindables/inputlayout.h>

#include <memory>
#include <vector>
#include <functional>
#include <glad/glad.h>

#define VERTEX_GENERATOR \
	X(Position, m_positions, glm::vec3) \
	X(Normal, m_normals, glm::vec3) \
	X(Tangent, m_tangents, glm::vec3) \
	X(Bitangent, m_bitangents, glm::vec3)

namespace Bind {
	class InputLayout;
}

namespace DrawItems {
	enum class VertexType {
#define X(Enum, Memeber, Type) Enum,
		VERTEX_GENERATOR
#undef X
		Texcoord,
		VertexColor,
		InstanceData
	};

	std::string vertex_type_to_string(VertexType type);

	class Drawable {
	public:
		Drawable() = default;
		Drawable(const std::string& name);
		virtual ~Drawable() = default;

		inline std::shared_ptr<Bind::InputLayout> GetVertex() const noexcept { return m_VAO; };
		inline void SetName(const std::string& name) noexcept { m_name = name; };
		inline std::string GetName() const noexcept { return m_name; };

		void Draw();

		void GenerateVAO(const std::vector<Dynamic::Dsr::VertexAttrib>& attribs, std::vector<VertexType> instruction);

	protected:
		std::string m_name;

		std::shared_ptr<Bind::InputLayout> m_VAO;
		size_t m_render_index = 0;//使用哪个渲染函数

		static std::vector<std::function<void(std::shared_ptr<Bind::InputLayout>)>> m_renderfunctions;

	protected:
		std::vector<glm::vec3> m_positions;
		std::vector<glm::vec3> m_normals;
		std::vector<glm::vec3> m_tangents;
		std::vector<glm::vec3> m_bitangents;
		std::vector<std::vector<glm::vec2>> m_texcoords;
		std::vector<std::vector<glm::vec4>> m_vertexcolors;

		std::vector<uint32_t> m_indices;

		std::vector<std::pair<LeafType, std::vector<AvailableType>>> m_instance_data = {};

	private:
		bool CheckAttribExists(const std::vector<VertexType>& attribs);
	};

	inline void DrawDefault(std::shared_ptr<Bind::InputLayout> VAO) {
		glDrawArrays(GL_TRIANGLES, 0, VAO->GetVertexCount());
	}

	inline void DrawIndexed(std::shared_ptr<Bind::InputLayout> VAO) {
		glDrawElements(GL_TRIANGLES, VAO->GetVertexCount(), GL_UNSIGNED_INT, 0);
	}

	inline void DrawInstanced(std::shared_ptr<Bind::InputLayout> VAO) {
		glDrawArraysInstanced(GL_TRIANGLES, 0, VAO->GetVertexCount(), VAO->GetInstanceCount());
	}

	inline void DrawIndexedInstanced(std::shared_ptr<Bind::InputLayout> VAO) {
		glDrawElementsInstanced(GL_TRIANGLES, VAO->GetVertexCount(), GL_UNSIGNED_INT, 0, VAO->GetInstanceCount());
	}
}

#ifndef DRAWABLE_SOURCE_FILE
#undef VERTEX_GENERATOR
#endif

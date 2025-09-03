#include <Drawables/plane.h>

namespace DrawItems {
	Plane::Plane(const std::string& name, std::vector<std::pair<LeafType, std::vector<AvailableType>>>&& instance_data) :Drawable(name) {
		m_positions.resize(4);
		m_normals.resize(4);
		m_tangents.resize(4);
		m_bitangents.resize(4);
		m_texcoords.resize(1);
		m_texcoords[0].resize(4);
		m_indices.resize(6);

		m_positions[0] = glm::vec3(-1, -1, 0);
		m_normals[0] = glm::vec3(0, 0, 1);
		m_tangents[0] = glm::vec3(1, 0, 0);
		m_bitangents[0] = glm::vec3(0, 1, 0);
		m_texcoords[0][0] = glm::vec2(0, 0);

		m_positions[1] = glm::vec3(1, -1, 0);
		m_normals[1] = glm::vec3(0, 0, 1);
		m_tangents[1] = glm::vec3(1, 0, 0);
		m_bitangents[1] = glm::vec3(0, 1, 0);
		m_texcoords[0][1] = glm::vec2(1, 0);

		m_positions[2] = glm::vec3(1, 1, 0);
		m_normals[2] = glm::vec3(0, 0, 1);
		m_tangents[2] = glm::vec3(1, 0, 0);
		m_bitangents[2] = glm::vec3(0, 1, 0);
		m_texcoords[0][2] = glm::vec2(1, 1);

		m_positions[3] = glm::vec3(-1, 1, 0);
		m_normals[3] = glm::vec3(0, 0, 1);
		m_tangents[3] = glm::vec3(1, 0, 0);
		m_bitangents[3] = glm::vec3(0, 1, 0);
		m_texcoords[0][3] = glm::vec2(0, 1);

		m_indices = {
			1,2,0,0,2,3
		};

		m_instance_data = std::move(instance_data);
	}
}
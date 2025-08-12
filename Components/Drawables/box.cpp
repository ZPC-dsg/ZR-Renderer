#include <Drawables/box.h>

namespace DrawItems {
	Box::Box(const std::string& name) :Drawable(name) {
		m_positions.resize(24);
		m_normals.resize(24);
		m_tangents.resize(24);
		m_bitangents.resize(24);
		m_texcoords.resize(1);
		m_texcoords[0].resize(24);
		
		//backface
		m_positions[0] = glm::vec3(1, -1, -1);
		m_normals[0] = glm::vec3(0, 0, -1);
		m_tangents[0] = glm::vec3(-1, 0, 0);
		m_bitangents[0] = glm::vec3(0, 1, 0);
		m_texcoords[0][0] = glm::vec2(0, 0);

		m_positions[1] = glm::vec3(-1, -1, -1);
		m_normals[1] = glm::vec3(0, 0, -1);
		m_tangents[1] = glm::vec3(-1, 0, 0);
		m_bitangents[1] = glm::vec3(0, 1, 0);
		m_texcoords[0][1] = glm::vec2(1, 0);

		m_positions[2] = glm::vec3(-1, 1, -1);
		m_normals[2] = glm::vec3(0, 0, -1);
		m_tangents[2] = glm::vec3(-1, 0, 0);
		m_bitangents[2] = glm::vec3(0, 1, 0);
		m_texcoords[0][2] = glm::vec2(1, 1);

		m_positions[3] = glm::vec3(1, 1, -1);
		m_normals[3] = glm::vec3(0, 0, -1);
		m_tangents[3] = glm::vec3(-1, 0, 0);
		m_bitangents[3] = glm::vec3(0, 1, 0);
		m_texcoords[0][3] = glm::vec2(0, 1);

		//frontface
		m_positions[4] = glm::vec3(-1, -1, 1);
		m_normals[4] = glm::vec3(0, 0, 1);
		m_tangents[4] = glm::vec3(1, 0, 0);
		m_bitangents[4] = glm::vec3(0, 1, 0);
		m_texcoords[0][4] = glm::vec2(0, 0);

		m_positions[5] = glm::vec3(1, -1, 1);
		m_normals[5] = glm::vec3(0, 0, 1);
		m_tangents[5] = glm::vec3(1, 0, 0);
		m_bitangents[5] = glm::vec3(0, 1, 0);
		m_texcoords[0][5] = glm::vec2(1, 0);

		m_positions[6] = glm::vec3(1, 1, 1);
		m_normals[6] = glm::vec3(0, 0, 1);
		m_tangents[6] = glm::vec3(1, 0, 0);
		m_bitangents[6] = glm::vec3(0, 1, 0);
		m_texcoords[0][6] = glm::vec2(1, 1);

		m_positions[7] = glm::vec3(-1, 1, 1);
		m_normals[7] = glm::vec3(0, 0, 1);
		m_tangents[7] = glm::vec3(1, 0, 0);
		m_bitangents[7] = glm::vec3(0, 1, 0);
		m_texcoords[0][7] = glm::vec2(0, 1);

		//leftface
		m_positions[8] = glm::vec3(-1, -1, -1);
		m_normals[8] = glm::vec3(-1, 0, 0);
		m_tangents[8] = glm::vec3(0, 0, 1);
		m_bitangents[8] = glm::vec3(0, 1, 0);
		m_texcoords[0][8] = glm::vec2(0, 0);

		m_positions[9] = glm::vec3(-1, -1, 1);
		m_normals[9] = glm::vec3(-1, 0, 0);
		m_tangents[9] = glm::vec3(0, 0, 1);
		m_bitangents[9] = glm::vec3(0, 1, 0);
		m_texcoords[0][9] = glm::vec2(1, 0);

		m_positions[10] = glm::vec3(-1, 1, 1);
		m_normals[10] = glm::vec3(-1, 0, 0);
		m_tangents[10] = glm::vec3(0, 0, 1);
		m_bitangents[10] = glm::vec3(0, 1, 0);
		m_texcoords[0][10] = glm::vec2(1, 1);

		m_positions[11] = glm::vec3(-1, 1, -1);
		m_normals[11] = glm::vec3(-1, 0, 0);
		m_tangents[11] = glm::vec3(0, 0, 1);
		m_bitangents[11] = glm::vec3(0, 1, 0);
		m_texcoords[0][11] = glm::vec2(0, 1);

		//rightface
		m_positions[12] = glm::vec3(1, -1, 1);
		m_normals[12] = glm::vec3(1, 0, 0);
		m_tangents[12] = glm::vec3(0, 0, -1);
		m_bitangents[12] = glm::vec3(0, 1, 0);
		m_texcoords[0][12] = glm::vec2(0, 0);
		
		m_positions[13] = glm::vec3(1, -1, -1);
		m_normals[13] = glm::vec3(1, 0, 0);
		m_tangents[13] = glm::vec3(0, 0, -1);
		m_bitangents[13] = glm::vec3(0, 1, 0);
		m_texcoords[0][13] = glm::vec2(1, 0);

		m_positions[14] = glm::vec3(1, 1, -1);
		m_normals[14] = glm::vec3(1, 0, 0);
		m_tangents[14] = glm::vec3(0, 0, -1);
		m_bitangents[14] = glm::vec3(0, 1, 0);
		m_texcoords[0][14] = glm::vec2(1, 1);

		m_positions[15] = glm::vec3(1, 1, 1);
		m_normals[15] = glm::vec3(1, 0, 0);
		m_tangents[15] = glm::vec3(0, 0, -1);
		m_bitangents[15] = glm::vec3(0, 1, 0);
		m_texcoords[0][15] = glm::vec2(0, 1);

		//bottom face
		m_positions[16] = glm::vec3(-1, -1, -1);
		m_normals[16] = glm::vec3(0, -1, 0);
		m_tangents[16] = glm::vec3(1, 0, 0);
		m_bitangents[16] = glm::vec3(0, 0, 1);
		m_texcoords[0][16] = glm::vec2(0, 0);

		m_positions[17] = glm::vec3(1, -1, -1);
		m_normals[17] = glm::vec3(0, -1, 0);
		m_tangents[17] = glm::vec3(1, 0, 0);
		m_bitangents[17] = glm::vec3(0, 0, 1);
		m_texcoords[0][17] = glm::vec2(1, 0);

		m_positions[18] = glm::vec3(1, -1, 1);
		m_normals[18] = glm::vec3(0, -1, 0);
		m_tangents[18] = glm::vec3(1, 0, 0);
		m_bitangents[18] = glm::vec3(0, 0, 1);
		m_texcoords[0][18] = glm::vec2(1, 1);

		m_positions[19] = glm::vec3(-1, -1, 1);
		m_normals[19] = glm::vec3(0, -1, 0);
		m_tangents[19] = glm::vec3(1, 0, 0);
		m_bitangents[19] = glm::vec3(0, 0, 1);
		m_texcoords[0][19] = glm::vec2(0, 1);

		//top face
		m_positions[20] = glm::vec3(-1, 1, 1);
		m_normals[20] = glm::vec3(0, 1, 0);
		m_tangents[20] = glm::vec3(1, 0, 0);
		m_bitangents[20] = glm::vec3(0, 0, -1);
		m_texcoords[0][20] = glm::vec2(0, 0);

		m_positions[21] = glm::vec3(1, 1, 1);
		m_normals[21] = glm::vec3(0, 1, 0);
		m_tangents[21] = glm::vec3(1, 0, 0);
		m_bitangents[21] = glm::vec3(0, 0, -1);
		m_texcoords[0][21] = glm::vec2(1, 0);

		m_positions[22] = glm::vec3(1, 1, -1);
		m_normals[22] = glm::vec3(0, 1, 0);
		m_tangents[22] = glm::vec3(1, 0, 0);
		m_bitangents[22] = glm::vec3(0, 0, -1);
		m_texcoords[0][22] = glm::vec2(1, 1);

		m_positions[23] = glm::vec3(-1, 1, -1);
		m_normals[23] = glm::vec3(0, 1, 0);
		m_tangents[23] = glm::vec3(1, 0, 0);
		m_bitangents[23] = glm::vec3(0, 0, -1);
		m_texcoords[0][23] = glm::vec2(0, 1);

		m_indices = {
			1,0,2,2,0,3,
			5,6,4,4,6,7,
			9,10,8,8,10,11,
			13,14,12,12,14,15,
			17,18,16,16,18,19,
			21,22,20,20,22,23
		};
	}
}
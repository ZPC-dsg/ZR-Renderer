#include <Drawables/sphere.h>

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

namespace DrawItems {
	Sphere::Sphere(const std::string& name, const std::vector<std::pair<LeafType, std::vector<AvailableType>>>& instance_data, unsigned int levels, unsigned int slices)
		:Drawable(name), m_levels(levels), m_slices(slices)
	{
        float radius = 1.0f;

        uint32_t vertexCount = 2 + (levels - 1) * (slices + 1);
        uint32_t indexCount = 6 * (levels - 1) * slices;
        m_positions.resize(vertexCount);
        m_normals.resize(vertexCount);
        m_texcoords.resize(1);
        m_texcoords[0].resize(vertexCount);
        m_tangents.resize(vertexCount);
        m_bitangents.resize(vertexCount);
        m_indices.resize(indexCount);

        uint32_t vIndex = 0, iIndex = 0;

        float phi = 0.0f, theta = 0.0f;
        float per_phi = glm::pi<float>() / levels;
        float per_theta = 2 * glm::pi<float>() / slices;
        float x, y, z;

        // ���붥�˵�
        m_positions[vIndex] = glm::vec3(0.0f, 1.0f, 0.0f);
        m_normals[vIndex] = glm::vec3(0.0f, 1.0f, 0.0f);
        m_tangents[vIndex] = glm::vec3(1.0f, 0.0f, 0.0f);
        m_bitangents[vIndex] = glm::vec3(0.0f, 0.0f, -1.0f);
        m_texcoords[0][vIndex++] = glm::vec2(0.0f, 0.0f);


        for (uint32_t i = 1; i < levels; ++i)
        {
            phi = per_phi * i;
            // ��Ҫslices + 1����������Ϊ �����յ���Ϊͬһ�㣬����������ֵ��һ��
            for (uint32_t j = 0; j <= slices; ++j)
            {
                theta = per_theta * j;
                x = radius * sinf(phi) * cosf(theta);
                y = radius * cosf(phi);
                z = radius * sinf(phi) * sinf(theta);
                // ������ֲ����ꡢ��������Tangent��������������
                glm::vec3 pos(x, y, z);

                m_positions[vIndex] = pos;
                m_normals[vIndex] = glm::normalize(pos);
                m_tangents[vIndex] = glm::vec3(-sinf(theta), 0.0f, cosf(theta));
                m_bitangents[vIndex] = glm::normalize(glm::cross(m_normals[vIndex], m_tangents[vIndex]));
                m_texcoords[0][vIndex++] = glm::vec2(theta / 2 / glm::pi<float>(), phi / glm::pi<float>());
            }
        }

        // ����׶˵�
        m_positions[vIndex] = glm::vec3(0.0f, -radius, 0.0f);
        m_normals[vIndex] = glm::vec3(0.0f, -1.0f, 0.0f);
        m_tangents[vIndex] = glm::vec3(-1.0f, 0.0f, 0.0f);
        m_bitangents[vIndex] = glm::vec3(0.0f, 0.0f, -1.0f);
        m_texcoords[0][vIndex++] = glm::vec2(0.0f, 1.0f);


        // ��������
        if (levels > 1)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                m_indices[iIndex++] = 0;
                m_indices[iIndex++] = j % (slices + 1) + 1;
                m_indices[iIndex++] = j;
            }
        }


        for (uint32_t i = 1; i < levels - 1; ++i)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                m_indices[iIndex++] = (i - 1) * (slices + 1) + j;
                m_indices[iIndex++] = (i - 1) * (slices + 1) + j % (slices + 1) + 1;
                m_indices[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;

                m_indices[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;
                m_indices[iIndex++] = i * (slices + 1) + j;
                m_indices[iIndex++] = (i - 1) * (slices + 1) + j;
            }
        }

        // �𽥷�������
        if (levels > 1)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                m_indices[iIndex++] = (levels - 2) * (slices + 1) + j;
                m_indices[iIndex++] = (levels - 2) * (slices + 1) + j % (slices + 1) + 1;
                m_indices[iIndex++] = (levels - 1) * (slices + 1) + 1;
            }
        }

        m_instance_data = instance_data;
	}
}
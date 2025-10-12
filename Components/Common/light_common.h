#pragma once

#include <glm/glm.hpp>

#define LIGHT_TYPE_DIRECTIONAL  0
#define LIGHT_TYPE_POINT        1
#define LIGHT_TYPE_SPOT         2
#define LIGHT_TYPE_RECT         3

#define LIGHT_STATUS_INVALID 0xFFFFFFFF

namespace Common
{
	struct LightCommon
	{
		// ������direction�洢������
		glm::vec3 m_position;
		unsigned int m_flags = LIGHT_STATUS_INVALID;
		glm::vec3 m_color = glm::vec3(1.0);
		// ���ھ۹�ƣ�intensity�ķ��Ŵ���direction�ĵ��������������Ļ��Ǹ���
		float m_intensity = 1.0f;
		// �۹��xy�����洢directionǰ����������zw�洢cos outer angle�Լ� 1.0 / (cos inner - cos outer)
		glm::vec4 m_customs = glm::vec4{}; 

		static LightCommon ConstructDefaultDirection();
		static LightCommon ConstructDefaultPoint();
		static LightCommon ConstructDefaultSpot();
		static LightCommon ConstructDefaultRect();

		void Invalidate();
		void Validate();
	};
}

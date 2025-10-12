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
		// 方向光的direction存储在这里
		glm::vec3 m_position;
		unsigned int m_flags = LIGHT_STATUS_INVALID;
		glm::vec3 m_color = glm::vec3(1.0);
		// 对于聚光灯，intensity的符号代表direction的第三个分量是正的还是负的
		float m_intensity = 1.0f;
		// 聚光灯xy分量存储direction前两个分量，zw存储cos outer angle以及 1.0 / (cos inner - cos outer)
		glm::vec4 m_customs = glm::vec4{}; 

		static LightCommon ConstructDefaultDirection();
		static LightCommon ConstructDefaultPoint();
		static LightCommon ConstructDefaultSpot();
		static LightCommon ConstructDefaultRect();

		void Invalidate();
		void Validate();
	};
}

#include <Common/light_common.h>

namespace Common
{
	LightCommon LightCommon::ConstructDefaultDirection()
	{
		LightCommon light;
		light.m_position = glm::vec3(0.0, -1.0, 0.0);
		light.m_flags = LIGHT_TYPE_DIRECTIONAL;

		return light;
	}

	LightCommon LightCommon::ConstructDefaultPoint()
	{
		LightCommon light;
		light.m_position = glm::vec3(0.0);
		light.m_flags = LIGHT_TYPE_POINT;

		return light;
	}

	LightCommon LightCommon::ConstructDefaultSpot()
	{
		LightCommon light;
		light.m_position = glm::vec3(0.0);
		light.m_flags = LIGHT_TYPE_SPOT;
		light.m_intensity = -1.0;
		
		glm::vec3 default_direction = glm::vec3(0.0, 0.0, -1.0);
		float default_cos_outer = 0.5;
		float default_inverse_inner_minus_outer = 1.0 / (std::sqrt(2) / 2 - 0.5);
		light.m_customs = glm::vec4(default_direction.x, default_direction.y, default_cos_outer, default_inverse_inner_minus_outer);

		return light;
	}

	LightCommon LightCommon::ConstructDefaultRect()
	{
		// TODO
		return LightCommon{};
	}

	void LightCommon::Invalidate()
	{
		m_flags = LIGHT_STATUS_INVALID;
	}

	void LightCommon::Validate()
	{
		if (m_flags == LIGHT_STATUS_INVALID)
		{
			m_flags = 0;
		}
	}
}
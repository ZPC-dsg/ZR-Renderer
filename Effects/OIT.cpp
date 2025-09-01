#include "OIT.h"

namespace OITEffects
{
	OIT::OIT(const std::string& name)
		:Utils(name)
	{
		globalSettings::mainCamera.set_perspective(60.0f, globalSettings::screen_width, globalSettings::screen_height, 0.1f, 512.0f);
		globalSettings::mainCamera.set_position(glm::vec3(0.0f, 0.0f, 4.0f));
	}

	void OIT::render()
	{

	}

	void OIT::prepare()
	{

	}

	void OIT::prepare_ui(const std::string& name)
	{

	}
}
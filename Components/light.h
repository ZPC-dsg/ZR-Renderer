#pragma once

#include <glm/glm.hpp>

struct PointLight {
	glm::vec3 position;
	glm::vec3 color;
};

struct DirectionLight {
	glm::vec3 direction;
	glm::vec3 color;
};

struct SpotLight {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 direction;
	float inner_angle;
	float outer_angle;
};
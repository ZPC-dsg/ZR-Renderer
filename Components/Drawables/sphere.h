#pragma once

#include <Drawables/drawable.h>

namespace DrawItems {
	class Sphere :public Drawable {
	public:
		Sphere(const std::string& name = "", unsigned int levels = 20, unsigned int slices = 20);
		~Sphere() = default;

	private:
		unsigned int m_levels = 20;
		unsigned int m_slices = 20;
	};
}
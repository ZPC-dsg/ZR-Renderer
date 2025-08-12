#pragma once

#include <Drawables/drawable.h>

namespace DrawItems {
	class Plane :public Drawable {
	public:
		Plane(const std::string& name = "");
		~Plane() = default;

	private:
	};
}
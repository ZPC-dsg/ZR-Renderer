#pragma once

#include <Drawables/drawable.h>

namespace DrawItems {
	class Plane :public Drawable {
	public:
		Plane(const std::string& name = "", const std::vector<std::pair<LeafType, std::vector<AvailableType>>>& instance_data = {});
		~Plane() = default;

	private:
	};
}
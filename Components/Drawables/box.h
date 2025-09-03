#pragma once

#include <Drawables/drawable.h>
#include <Macros/shader_typedef.h>

namespace DrawItems {
	class Box :public Drawable {
	public:
		Box(const std::string& name = "", std::vector<std::pair<LeafType, std::vector<AvailableType>>>&& instance_data = {});
		~Box() = default;

	private:
	};
}
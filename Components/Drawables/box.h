#pragma once

#include <Drawables/drawable.h>

namespace DrawItems {
	class Box :public Drawable {
	public:
		Box(const std::string& name = "");
		~Box() = default;

	private:
	};
}
#pragma once

#include <utils.h>

namespace OITEffects
{
	class OIT :public Utils
	{
	public:
		OIT(const std::string& name);
		~OIT() = default;

		void render() override;
		void prepare() override;
		void prepare_ui(const std::string& name) override;

	private:

	private:

	};
}
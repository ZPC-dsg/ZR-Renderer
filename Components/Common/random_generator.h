#pragma once

#include <random>
#include <cassert>
#include <glm/glm.hpp>

namespace Common
{
	class UniformGenerator
	{
	public:
		static float Generate(float min_val, float max_val)
		{
			assert(max_val >= min_val);
			return Get().m_dist(Get().m_engine) * (max_val - min_val) + min_val;
		}

		static glm::vec2 Generate(glm::vec2 min_val, glm::vec2 max_val)
		{
			return { Generate(min_val.x, max_val.x), Generate(min_val.y, max_val.y) };
		}

		static glm::vec3 Generate(glm::vec3 min_val, glm::vec3 max_val)
		{
			return { Generate(min_val.x, max_val.x), Generate(min_val.y, max_val.y), Generate(min_val.z, max_val.z)};
		}

		static glm::vec4 Generate(glm::vec4 min_val, glm::vec4 max_val)
		{
			return { Generate(min_val.x, max_val.x), Generate(min_val.y, max_val.y), Generate(min_val.z, max_val.z), Generate(min_val.w, max_val.w) };
		}

		static std::vector<float> Generate(float min_val, float max_val, size_t data_size)
		{
			assert(max_val >= min_val);

			std::vector<float> res(data_size);
			for (size_t i = 0; i < data_size; i++)
			{
				res[i] = Get().m_dist(Get().m_engine) * (max_val - min_val) + min_val;
			}

			return res;
		}

		static std::vector<glm::vec2> Generate(glm::vec2 min_val, glm::vec2 max_val, size_t data_size)
		{
			std::vector<float> item1 = Generate(min_val.x, max_val.x, data_size);
			std::vector<float> item2 = Generate(min_val.y, max_val.y, data_size);

			std::vector<glm::vec2> res(data_size);
			for (size_t i = 0; i < data_size; i++)
			{
				res[i] = glm::vec2{ item1[i],item2[i] };
			}

			return res;
		}

		static std::vector<glm::vec3> Generate(glm::vec3 min_val, glm::vec3 max_val, size_t data_size)
		{
			std::vector<float> item1 = Generate(min_val.x, max_val.x, data_size);
			std::vector<float> item2 = Generate(min_val.y, max_val.y, data_size);
			std::vector<float> item3 = Generate(min_val.z, max_val.z, data_size);

			std::vector<glm::vec3> res(data_size);
			for (size_t i = 0; i < data_size; i++)
			{
				res[i] = glm::vec3{ item1[i],item2[i],item3[i]};
			}

			return res;
		}

		static std::vector<glm::vec4> Generate(glm::vec4 min_val, glm::vec4 max_val, size_t data_size)
		{
			std::vector<float> item1 = Generate(min_val.x, max_val.x, data_size);
			std::vector<float> item2 = Generate(min_val.y, max_val.y, data_size);
			std::vector<float> item3 = Generate(min_val.z, max_val.z, data_size);
			std::vector<float> item4 = Generate(min_val.w, max_val.w, data_size);

			std::vector<glm::vec4> res(data_size);
			for (size_t i = 0; i < data_size; i++)
			{
				res[i] = glm::vec4{ item1[i],item2[i],item3[i],item4[i]};
			}

			return res;
		}

		static void Reset()
		{
			Get().m_is_initialized = false;
		}

	private:
		static UniformGenerator& Get()
		{
			static UniformGenerator generator;
			if (!generator.m_is_initialized)
			{
				std::random_device rd;
				generator.m_engine = std::mt19937{ rd() };
				generator.m_is_initialized = true;
			}
			return generator;
		}

	private:
		bool m_is_initialized = false;
		std::mt19937 m_engine;
		std::uniform_real_distribution<> m_dist = std::uniform_real_distribution<>{ 0.0f,1.0f };
	};
}
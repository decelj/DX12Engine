#pragma once

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_SIZE_T_LENGTH
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <utility>

namespace glm
{
	template<typename T>
	struct HashVec
	{
		HashVec() = default;

		size_t operator()(const T& v) const
		{
			size_t hash = 0u;
			for (uint32_t i = 0; i < T::length(); ++i)
			{
				hash ^= std::hash<typename T::value_type>{}(v[i]);
			}
			return hash;
		}
	};

	template<typename T>
	std::string ToString(const T& v)
	{
		std::string output;
		for (uint32_t i = 0; i < T::length() - 1u; ++i)
		{
			output += std::to_string(v[i]);
			output += ", ";
		}
		output += std::to_string(v[T::length() - 1u]);
		return output;
	}
}

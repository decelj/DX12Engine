// OBJLoader.cpp : Defines the functions for the static library.
//

#include "pch.h"

namespace OBJLoader
{
	struct Vertex
	{
		glm::vec3 m_Position;
		glm::vec3 m_Normal;
		glm::vec2 m_UV;

		bool operator==(const Vertex& other) const
		{
			return !(*this != other);
		}

		bool operator!=(const Vertex& other) const
		{
			return std::memcmp(this, &other, sizeof(Vertex)) != 0;
		}
	};

	bool LoadFile(const std::wstring& file, std::vector<Vertex>* outVerts, std::vector<uint16_t>* outIndicies);
	bool LoadFile(const std::wstring& file, std::vector<Vertex>* outVerts, std::vector<uint32_t>* outIndicies);
}

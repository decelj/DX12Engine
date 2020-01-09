#include "pch.h"
#include "OBJLoader.h"

#include <filesystem>
#include <fstream>
#include <locale>
#include <codecvt>
#include <cstring>
#include <cerrno>
#include <sstream>
#include <limits>
#include <array>
#include <unordered_map>
#include <string.h>

namespace fs = std::filesystem;

namespace OBJLoader
{

void ParseTexCoord(std::istringstream& sstream, const size_t lineNumber, std::vector<glm::vec2>* outUVs)
{
	assert(outUVs != nullptr);

	glm::vec2 uv;
	sstream >> uv.x >> uv.y;
	outUVs->emplace_back(uv);

	if (!sstream)
	{
		std::string error = "Error parsing texture coordinate, line ";
		error += std::to_string(lineNumber);
		throw std::runtime_error(error);
	}
}

void ParseVertexPosition(std::istringstream& sstream, const size_t lineNumber, std::vector<glm::vec3>* outPositions)
{
	assert(outPositions != nullptr);

	glm::vec3 pos;
	sstream >> pos.x >> pos.y >> pos.z;
	outPositions->emplace_back(pos);

	if (!sstream)
	{
		std::string error = "Error parsing vertex position, line ";
		error += std::to_string(lineNumber);
		throw std::runtime_error(error);
	}
}

void ParseNormal(std::istringstream& sstream, const size_t lineNumber, std::vector<glm::vec3>* outNormals)
{
	assert(outNormals != nullptr);

	glm::vec3 normal;
	sstream >> normal.x >> normal.y >> normal.z;
	outNormals->emplace_back(glm::normalize(normal));

	if (!sstream)
	{
		std::string error = "Error parsing normal, line ";
		error += std::to_string(lineNumber);
		throw std::runtime_error(error);
	}
}

void ParseFace(std::istringstream& sstream, const size_t lineNumber,
	const std::vector<glm::vec3>& positions,
	const std::vector<glm::vec3>& normals,
	const std::vector<glm::vec2>& texCoords,
	std::vector<Vertex>* outVerts)
{
	constexpr uint32_t kInvalidValue = std::numeric_limits<uint32_t>::max();

	std::string errorStr = "Error parsing face, line ";
	errorStr += std::to_string(lineNumber);

	auto ParseVertexTokens = [&](const std::string& vertexToken)
	{
		std::array<uint32_t, 3u> values = { kInvalidValue, kInvalidValue, kInvalidValue };
		uint32_t outPos = 0u;
		size_t startPos = 0u;
		size_t endPos = 0u;
		do
		{
			endPos = vertexToken.find_first_of('/', startPos);
			if (outPos >= 3u)
			{
				errorStr += ": too many subtokens for vertex";
				throw std::runtime_error(errorStr);
			}

			values[outPos++] = (uint32_t)std::stoi(vertexToken.substr(startPos, endPos - startPos));
			startPos = endPos + 1;
		} while (endPos != std::string::npos);

		return values;
	};

	auto ParseVertex = [&](const std::string& vertexToken, Vertex* vert)
	{
		assert(vert != nullptr);
		std::array<uint32_t, 3u> vertexIdxs = ParseVertexTokens(vertexToken);

		if (vertexIdxs[0] >= positions.size())
		{
			errorStr += ": invalid vertex number ";
			errorStr += std::to_string(vertexIdxs[0]);
			throw std::runtime_error(errorStr);
		}
		vert->m_Position = positions[vertexIdxs[0]];

		if (vertexIdxs[1] >= texCoords.size())
		{
			if (vertexIdxs[1] != kInvalidValue)
			{
				errorStr += ": invalid texture coordinate number ";
				errorStr += std::to_string(vertexIdxs[1]);
				throw std::runtime_error(errorStr);
			}
		}
		else
		{
			vert->m_UV = texCoords[vertexIdxs[1]];
		}

		bool useFaceNormal = false;
		if (vertexIdxs[2] >= normals.size())
		{
			if (vertexIdxs[2] != kInvalidValue)
			{
				errorStr += ": invalid normal number ";
				errorStr += std::to_string(vertexIdxs[2]);
				throw std::runtime_error(errorStr);
			}

			useFaceNormal = true;
		}
		else
		{
			vert->m_Normal = normals[vertexIdxs[2]];
		}

		return useFaceNormal;
	};

	std::array<bool, 3u> useFaceNormal = { false, false, false };
	for (uint32_t i = 0; i < 3u; ++i)
	{
		std::string vertexToken;
		sstream >> vertexToken;
		if (!sstream)
		{
			throw std::runtime_error(errorStr);
		}

		Vertex vert = {};
		useFaceNormal[i] = ParseVertex(vertexToken, &vert);
		outVerts->emplace_back(std::move(vert));
	}

	if (	outVerts->at(outVerts->size() - 2u) == outVerts->back()
		||	outVerts->at(outVerts->size() - 3u) == outVerts->back()
		||	outVerts->at(outVerts->size() - 2u) == outVerts->at(outVerts->size() - 3u))
	{
		// Degenerate face, kill it
		for (uint32_t i = 0u; i < 3u; ++i)
		{
			outVerts->pop_back();
		}
		return;
	}

	for (uint32_t i = 0u; i < 3u; ++i)
	{
		if (!useFaceNormal[i])
		{
			continue;
		}

		size_t vertsSize = outVerts->size();
		glm::vec3 ab = glm::normalize(outVerts->at(vertsSize - 2u).m_Position - outVerts->at(vertsSize - 3u).m_Position);
		glm::vec3 ac = glm::normalize(outVerts->at(vertsSize - 1u).m_Position - outVerts->at(vertsSize - 3u).m_Position);
		glm::vec3 faceNormal = glm::normalize(glm::cross(ab, ac));

		outVerts->at(vertsSize - (3u - i)).m_Normal = faceNormal;
	}
}

template<typename IndexT>
void CreateIndicies(std::vector<Vertex>& inVerts, std::vector<Vertex>* outVerts, std::vector<IndexT>* outIndicies)
{
	struct VertexHash
	{
		size_t operator()(const Vertex& vert) const
		{
			size_t hash = 1217u;
			hash ^= glm::HashVec<glm::vec3>{}(vert.m_Normal);
			hash ^= glm::HashVec<glm::vec3>{}(vert.m_Position);
			hash ^= glm::HashVec<glm::vec2>{}(vert.m_UV);
			return hash;
		}
	};
	using VertexCacheT = std::unordered_map<Vertex, IndexT, VertexHash>;

	VertexCacheT vertexCache;
	IndexT nextIdx = 0u;
	auto GetVertexIdx = [&](const Vertex& vert)
	{
		typename VertexCacheT::const_iterator it = vertexCache.find(vert);
		if (it != vertexCache.end())
		{
			return it->second;
		}

		if (vertexCache.size() >= 64u)
		{
			vertexCache.clear();
		}

		assert(nextIdx < std::numeric_limits<IndexT>::max());
		vertexCache[vert] = nextIdx;
		outVerts->push_back(vert);
		return nextIdx++;
	};

	std::reverse(inVerts.begin(), inVerts.end());
	while (!inVerts.empty())
	{
		outIndicies->push_back(GetVertexIdx(inVerts.back()));
		inVerts.pop_back();
	}
}

template<typename IndexT>
bool _LoadFile(const std::wstring& file, std::vector<Vertex>* outVerts, std::vector<IndexT>* outIndicies)
{
	assert(outVerts != nullptr);
	assert(outIndicies != nullptr);

	fs::path filePath = file;
	filePath = fs::absolute(filePath);

	std::ifstream fstream(filePath);
	if (!fstream)
	{
		char errnoStrBuffer[1024];
		memset(errnoStrBuffer, 0, 1024);
		strerror_s(errnoStrBuffer, errno);

		std::string error = "Could not open file \"";
		error += std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(file);
		error += "\": ";
		error += errnoStrBuffer;
		throw std::runtime_error(error);
	}

	// Note: OBJ files are 1 indexed. Insert dummy value to make indexing easier.
	std::vector<glm::vec2> texCoords(1);
	std::vector<glm::vec3> normals(1);
	std::vector<glm::vec3> positions(1);
	std::vector<Vertex> loadedVerts;
	size_t lineNumber = 0u;
	std::string line;
	while (std::getline(fstream, line))
	{
		std::istringstream sstream(line);
		std::string type;

		sstream >> type;

		if (type.empty() || type[0] == '#')
		{
		}
		else if (type == "v")
		{
			ParseVertexPosition(sstream, lineNumber, &positions);
		}
		else if (type == "vt")
		{
			ParseTexCoord(sstream, lineNumber, &texCoords);
		}
		else if (type == "vn")
		{
			ParseNormal(sstream, lineNumber, &normals);
		}
		else if (type == "f")
		{
			ParseFace(sstream, lineNumber, positions, normals, texCoords, &loadedVerts);
		}
		else if (type == "g")
		{
			// TODO: Meshes
		}
		else if (type == "s")
		{
			// Smooth shading, ignore
		}
		else
		{
			std::string error = "Error parsing OBJ file: Unexpected token \"";
			error += type;
			error += "\" on line ";
			error += std::to_string(lineNumber);
			throw std::runtime_error(error);
		}

		++lineNumber;
	}

	// Free memory
	normals.clear();
	texCoords.clear();
	positions.clear();

	CreateIndicies(loadedVerts, outVerts, outIndicies);

	return true;
}

bool LoadFile(const std::wstring& file, std::vector<Vertex>* outVerts, std::vector<uint16_t>* outIndicies)
{
	return _LoadFile(file, outVerts, outIndicies);
}

bool LoadFile(const std::wstring& file, std::vector<Vertex>* outVerts, std::vector<uint32_t>* outIndicies)
{
	return _LoadFile(file, outVerts, outIndicies);
}

}

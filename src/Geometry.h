#pragma once

#include "stdafx.h"
#include "Buffer.h"
#include "VertexLayoutMngr.h"

class UploadManager;


struct GeometryContants
{
	glm::mat4	m_XForm;
};


class Geometry
{
public:
	Geometry(const void* verticies, uint32_t vertexCount, uint32_t vertexSize, const void* indicies, uint32_t indexCount, UploadManager& uploadMngr);
	~Geometry() = default;
	Geometry(const Geometry& other) = delete;

	virtual void NextFrame();

	void Draw(DXCommandList& cmdList) const;
	glm::vec3 Position() const { return m_ConstantData.m_XForm[3]; }

	void SetPosition(const glm::vec3& position)
	{
		m_ConstantData.m_XForm[3].x = position.x;
		m_ConstantData.m_XForm[3].y = position.y;
		m_ConstantData.m_XForm[3].z = position.z;
		m_ConstantsDirty = true;
	}

protected:
	virtual void SetContantBuffer(DXCommandList& cmdList) const = 0;
	virtual void UpdateConstants() = 0;

	GeometryContants	m_ConstantData;
	bool				m_ConstantsDirty;

private:
	VertexBuffer		m_VertexBuffer;
	IndexBuffer			m_IndexBuffer;
};


class StaticGeometry : public Geometry
{
public:
	StaticGeometry(const void* verticies, uint32_t vertexCount, uint32_t vertexSize, const void* indicies, uint32_t indexCount, UploadManager& uploadMngr);

	template<typename VContainer, typename IContainer>
	StaticGeometry(const VContainer& verticies, const IContainer& indicies, UploadManager& uploadMngr)
		: StaticGeometry((const void*)verticies.data(), (uint32_t)verticies.size(), (uint32_t)sizeof(typename VContainer::value_type),
						 (const void*)indicies.data(), (uint32_t)indicies.size(), uploadMngr)
	{
	}

	template<typename VContainer, typename IContainer>
	StaticGeometry(VContainer&& verticies, IContainer&& indicies, UploadManager& uploadMngr)
		: StaticGeometry((const void*)verticies.data(), (uint32_t)verticies.size(), (uint32_t)sizeof(typename VContainer::value_type),
						 (const void*)indicies.data(), (uint32_t)indicies.size(), uploadMngr)
	{
		verticies.clear();
		indicies.clear();
	}

	~StaticGeometry() = default;

protected:
	void SetContantBuffer(DXCommandList& cmdList) const override;
	void UpdateConstants() override;

private:
	ConstantBuffer	m_ConstantBuffer;
};


class RigidGeometry : public Geometry
{
public:
	RigidGeometry(const void* verticies, uint32_t vertexCount, uint32_t vertexSize, const void* indicies, uint32_t indexCount, UploadManager& uploadMngr);

	template<typename VContainer, typename IContainer>
	RigidGeometry(const VContainer& verticies, const IContainer& indicies, UploadManager& uploadMngr)
		: RigidGeometry((const void*)verticies.data(), (uint32_t)verticies.size(), (uint32_t)sizeof(typename VContainer::value_type),
						(const void*)indicies.data(), (uint32_t)indicies.size(), uploadMngr)
	{
	}

	template<typename VContainer, typename IContainer>
	RigidGeometry(VContainer&& verticies, IContainer&& indicies, UploadManager& uploadMngr)
		: RigidGeometry((const void*)verticies.data(), (uint32_t)verticies.size(), (uint32_t)sizeof(typename VContainer::value_type),
						(const void*)indicies.data(), (uint32_t)indicies.size(), uploadMngr)
	{
		verticies.clear();
		indicies.clear();
	}

	~RigidGeometry() = default;

	void NextFrame() override;

protected:
	void SetContantBuffer(DXCommandList& cmdList) const override;
	void UpdateConstants() override;

private:
	std::array<std::unique_ptr<ConstantBuffer>, kNumFramesInFlight>	m_ConstantBuffers;
	uint8_t															m_BufferIdx;
	uint8_t															m_PendingConstantUpdates;
};

#include "stdafx.h"
#include "Geometry.h"
#include "DXCommandList.h"
#include "UploadManager.h"

Geometry::Geometry(const void* verticies, uint32_t vertexCount, uint32_t vertexSize, const void* indicies, uint32_t indexCount, UploadManager& uploadMngr)
	: m_ConstantsDirty(false)
	, m_VertexBuffer(vertexCount, vertexSize)
	, m_IndexBuffer(indexCount)
{
	m_ConstantData.m_XForm = glm::identity<glm::mat4>();

	uploadMngr.UploadDataTo(verticies, (size_t)vertexSize * (size_t)vertexCount, m_VertexBuffer);
	uploadMngr.UploadDataTo(indicies, (size_t)indexCount * sizeof(uint32_t), m_IndexBuffer);
}

void Geometry::NextFrame()
{
	if (m_ConstantsDirty)
	{
		UpdateConstants();
		m_ConstantsDirty = false;
	}
}

void Geometry::Draw(DXCommandList& cmdList) const
{
	SetContantBuffer(cmdList);
	cmdList.Native()->IASetIndexBuffer(&m_IndexBuffer.View());
	cmdList.Native()->IASetVertexBuffers(0, 1, &m_VertexBuffer.View());
	cmdList.Native()->DrawIndexedInstanced(m_IndexBuffer.Count(), 1, 0, 0, 0);
}

StaticGeometry::StaticGeometry(const void* verticies, uint32_t vertexCount, uint32_t vertexSize, const void* indicies, uint32_t indexCount, UploadManager& uploadMngr)
	: Geometry(verticies, vertexCount, vertexSize, indicies, indexCount, uploadMngr)
	, m_ConstantBuffer(sizeof(GeometryContants))
{
	m_ConstantBuffer.SetData(0u, m_ConstantData);
}

void StaticGeometry::SetContantBuffer(DXCommandList& cmdList) const
{
	cmdList.Native()->SetGraphicsRootConstantBufferView(1u, m_ConstantBuffer.GetGPUAddress());
}

void StaticGeometry::UpdateConstants()
{
	m_ConstantBuffer.SetData(0u, m_ConstantData);
}

RigidGeometry::RigidGeometry(const void* verticies, uint32_t vertexCount, uint32_t vertexSize, const void* indicies, uint32_t indexCount, UploadManager& uploadMngr)
	: Geometry(verticies, vertexCount, vertexSize, indicies, indexCount, uploadMngr)
	, m_BufferIdx(0u)
	, m_PendingConstantUpdates(0u)
{
	for (std::unique_ptr<ConstantBuffer>& buf : m_ConstantBuffers)
	{
		buf.reset(new ConstantBuffer(sizeof(GeometryContants)));
		buf->SetData(0u, m_ConstantData);
	}
}

void RigidGeometry::NextFrame()
{
	if (m_ConstantsDirty)
	{
		m_PendingConstantUpdates = kNumFramesInFlight;
		m_ConstantsDirty = false;
	}

	m_BufferIdx = (m_BufferIdx + 1u) % kNumFramesInFlight;

	if (m_PendingConstantUpdates)
	{
		UpdateConstants();
	}
}

void RigidGeometry::SetContantBuffer(DXCommandList& cmdList) const
{
	cmdList.Native()->SetGraphicsRootConstantBufferView(1u, m_ConstantBuffers[m_BufferIdx]->GetGPUAddress());
}

void RigidGeometry::UpdateConstants()
{
	assert(m_PendingConstantUpdates);
	--m_PendingConstantUpdates;
	m_ConstantBuffers[m_BufferIdx]->SetData(0u, m_ConstantData);
}

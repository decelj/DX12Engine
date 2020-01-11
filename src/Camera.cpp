#include "stdafx.h"
#include "Camera.h"
#include "DXCommandList.h"

#include <d3d12.h>


Camera::Camera(uint32_t width, uint32_t height, float fov, float nearClip, float farClip)
	: m_ViewQuat()
	, m_Translation(0.f)
	, m_View()
	, m_Proj()
	, m_FOV(glm::radians(fov))
	, m_ViewWidth(width)
	, m_ViewHeight(height)
	, m_NearClip(nearClip)
	, m_FarClip(farClip)
{
	m_Proj = glm::perspectiveFov(m_FOV, (float)m_ViewHeight, (float)m_ViewHeight, m_NearClip, m_FarClip);
}

void Camera::LookAt(const glm::vec3& pos, const glm::vec3& point)
{
	m_ViewQuat = glm::quatLookAt(glm::normalize(point - pos), { 0.f, 1.f, 0.f });
	m_Translation = pos;
	CalculateViewMatrix();
}

void Camera::SetViewport(DXCommandList& cmdList) const
{
	D3D12_VIEWPORT vp = { 0.f, 0.f, (float)m_ViewWidth, (float)m_ViewHeight, 0.f, 1.f };
	cmdList.Native()->RSSetViewports(1, &vp);

	D3D12_RECT scissorRect = { 0u, 0u, (LONG)m_ViewWidth, (LONG)m_ViewHeight };
	cmdList.Native()->RSSetScissorRects(1, &scissorRect);
}

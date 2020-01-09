#include "stdafx.h"
#include "Camera.h"
#include "DXCommandList.h"

#include <d3d12.h>


Camera::Camera(uint32_t width, uint32_t height, float fov, float farClip)
	: m_View()
	, m_Proj()
	, m_FOV(glm::radians(fov))
	, m_ViewWidth(width)
	, m_ViewHeight(height)
	, m_FarClip(farClip)
{
	m_Proj = glm::perspectiveFov(m_FOV, (float)m_ViewHeight, (float)m_ViewHeight, 0.1f, m_FarClip);
}

void Camera::SetPosition(const glm::vec3& pos)
{
	m_View[3] = glm::vec4(pos, 1.f);
}

void Camera::LookAt(const glm::vec3& pos, const glm::vec3& point)
{
	if (std::fabsf(pos[0]) + std::fabsf(pos[2]) < 0.02f)
	{
		m_View = glm::lookAt(pos, point, glm::vec3(0.f, 0.f, 1.f));
	}
	else
	{
		m_View = glm::lookAt(pos, point, glm::vec3(0.f, 1.f, 0.f));
	}
}

void Camera::SetViewport(DXCommandList& cmdList) const
{
	D3D12_VIEWPORT vp = { 0.f, 0.f, (float)m_ViewWidth, (float)m_ViewHeight, 0.f, 1.f };
	cmdList.Native()->RSSetViewports(1, &vp);

	D3D12_RECT scissorRect = { 0u, 0u, (LONG)m_ViewWidth, (LONG)m_ViewHeight };
	cmdList.Native()->RSSetScissorRects(1, &scissorRect);
}

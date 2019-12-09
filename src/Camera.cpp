#include "stdafx.h"
#include "Camera.h"

Camera::Camera(uint32_t width, uint32_t height, float fov, float farClip)
	: m_View()
	, m_Proj()
	, m_FOV(glm::radians(fov))
	, m_ViewWidth(width)
	, m_ViewHeight(height)
	, m_FarClip(farClip)
{
	m_Proj = glm::perspectiveFov<float>(m_FOV, m_ViewHeight, m_ViewHeight, 0.1f, m_FarClip);
}

void Camera::SetPosition(const glm::vec3& pos)
{
	m_View[3] = glm::vec4(pos, 1.f);
}

void Camera::LookAt(const glm::vec3& pos, const glm::vec3& point)
{
	m_View = glm::lookAt(pos, point, glm::vec3(0.f, 1.f, 0.f));
}

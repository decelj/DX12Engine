#pragma once

#include "stdafx.h"

class Camera
{
public:
	Camera(uint32_t width, uint32_t height, float fov, float farClip);

	void Translate(const glm::vec3& delta)
	{
		m_View = glm::translate(m_View, delta);
	}

	// In degrees
	void Rotate(const glm::vec3& delta)
	{
		m_View = glm::rotate(m_View, delta.x, glm::vec3(1.f, 0.f, 0.f));
		m_View = glm::rotate(m_View, delta.y, glm::vec3(0.f, 1.f, 0.f));
		m_View = glm::rotate(m_View, delta.z, glm::vec3(0.f, 0.f, 1.f));
	}

	void SetPosition(const glm::vec3& pos);
	void LookAt(const glm::vec3& pos, const glm::vec3& point);

	const glm::vec4& Position() const { return m_View[3]; }
	const glm::mat4& View() const { return m_View; }
	const glm::mat4& Proj() const { return m_Proj; }

private:
	glm::mat4	m_View;
	glm::mat4	m_Proj;
	float		m_FOV;
	uint32_t	m_ViewWidth;
	uint32_t	m_ViewHeight;
	float		m_FarClip;
};

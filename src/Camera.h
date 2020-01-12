#pragma once

#include "stdafx.h"

class DXCommandList;

class Camera
{
public:
	Camera(uint32_t width, uint32_t height, float fov, float nearClip, float farClip);

	void Translate(const glm::vec3& delta)
	{
		m_Translation += delta * m_ViewQuat;
		CalculateViewMatrix();
	}

	// In degrees
	void Rotate(const glm::vec3& delta)
	{
		m_ViewQuat = glm::quat(glm::radians(delta)) * m_ViewQuat;
		m_ViewQuat = glm::normalize(m_ViewQuat);
		CalculateViewMatrix();
	}

	void SetRotation(const glm::quat& dir)
	{
		m_ViewQuat = dir;
		CalculateViewMatrix();
	}

	void SetTranslation(const glm::vec3& pos)
	{
		m_Translation = pos;
		CalculateViewMatrix();
	}

	// TODO: Remove me?
	void LookAt(const glm::vec3& pos, const glm::vec3& point);

	const glm::vec3  LookVector() const { return glm::vec3(m_ViewQuat.x, m_ViewQuat.y, m_ViewQuat.z); }
	const glm::vec3  Position() const { return m_Translation * m_ViewQuat; }
	const glm::mat4& View() const { return m_View; }
	const glm::mat4& Proj() const { return m_Proj; }

	void SetViewport(DXCommandList& cmdList) const;

private:
	void CalculateViewMatrix()
	{
		glm::mat4 rotate = glm::mat4_cast(m_ViewQuat);
		glm::mat4 translate = glm::translate(glm::identity<glm::mat4>(), -m_Translation);
		m_View = rotate * translate;
	}

	glm::quat	m_ViewQuat;
	glm::vec3	m_Translation;

	glm::mat4	m_View;
	glm::mat4	m_Proj;

	float		m_FOV;
	uint32_t	m_ViewWidth;
	uint32_t	m_ViewHeight;
	float		m_FarClip;
	float		m_NearClip;
};

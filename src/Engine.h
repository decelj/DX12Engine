#pragma once

#include "stdafx.h"
#include "Camera.h"
#include "UploadManager.h"
#include "Buffer.h"
#include "RenderTarget.h"
#include "Geometry.h"
#include "Light.h"
#include "Texture.h"

#include <bitset>

class Window;
class DXCommandList;

class Engine
{
public:
	Engine(const Window& window);
	Engine(const Engine& other) = delete;
	~Engine() = default;

	void RenderFrame();

	void OnKeyDown(unsigned char key)
	{
		m_KeyState[static_cast<uint32_t>(key)] = true;
	}

	void OnKeyUp(unsigned char key)
	{
		m_KeyState[static_cast<uint32_t>(key)] = false;
	}

	void OnMousePress(const glm::ivec2& pos);
	void OnMouseRelease(const glm::ivec2& pos);
	void OnMouseMove(const glm::ivec2& pos);

private:
	void LoadGeometry();
	void LoadShaders();
	void SetupLights();

	void UpdateCamera();

	struct GlobalFrameConstants
	{
		glm::mat4	m_View;
		glm::mat4	m_Proj;
		glm::mat4	m_ViewProj;
		glm::vec3	m_CameraPos;
		float		m_Padding;
	};

	std::unique_ptr<DepthTarget>				m_DepthBuffer;
	std::vector<std::unique_ptr<RenderTarget>>	m_BackBuffers;
	std::unique_ptr<DXCommandList>				m_CommandList;

	std::vector<std::unique_ptr<Geometry>>		m_SceneGeometry;
	std::vector<std::unique_ptr<Light>>			m_Lights;

	ShadowMapRenderer							m_ShadowMapRenderer;

	ReleasedUniquePtr<ID3DBlob>					m_VertexShader;
	ReleasedUniquePtr<ID3DBlob>					m_PixelShader;

	ReleasedUniquePtr<ID3D12PipelineState>		m_PrimaryPSO;
	ReleasedUniquePtr<ID3D12RootSignature>		m_PrimaryRootSignature;

	Camera										m_Camera;
	IFFArray<std::unique_ptr<ConstantBuffer>>	m_FrameConstBuffers;

	std::unique_ptr<Texture2D>					m_CheckboardTexture;

	UploadManager								m_UploadManager;

	size_t										m_Frame;

	std::bitset<256u>							m_KeyState;

	float										m_CameraPitch;
	float										m_CameraYaw;
	float										m_CameraSpeed;

	glm::ivec2									m_ClickPos;
};

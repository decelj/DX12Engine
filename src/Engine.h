#pragma once

#include "stdafx.h"
#include "Camera.h"
#include "UploadManager.h"
#include "Buffer.h"
#include "RenderTarget.h"
#include "Geometry.h"

class Window;
class DXCommandList;

class Engine
{
public:
	Engine(const Window& window);
	Engine(const Engine& other) = delete;
	~Engine() = default;

	void RenderFrame();

	void OnKeyDown(char key);
	void OnKeyUp(char key);
	void OnMousePress(const glm::ivec2& pos);
	void OnMouseRelease(const glm::ivec2& pos);
	void OnMouseMove(const glm::ivec2& pos);

private:
	void LoadGeometry();
	void LoadShaders();

	struct GlobalFrameConstatns
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 viewProj;
	};

	std::unique_ptr<DepthTarget>				m_DepthBuffer;
	std::vector<std::unique_ptr<RenderTarget>>	m_BackBuffers;
	std::unique_ptr<DXCommandList>				m_CommandList;

	std::vector<std::unique_ptr<Geometry>>		m_SceneGeometry;

	ReleasedUniquePtr<ID3DBlob>					m_VertexShader;
	ReleasedUniquePtr<ID3DBlob>					m_PixelShader;

	ReleasedUniquePtr<ID3D12PipelineState>		m_PrimaryPSO;
	ReleasedUniquePtr<ID3D12RootSignature>		m_PrimaryRootSignature;

	Camera										m_Camera;
	GlobalFrameConstatns						m_FrameConstantData;
	IFFArray<std::unique_ptr<ConstantBuffer>>	m_FrameConstBuffers;

	UploadManager								m_UploadManager;

	size_t										m_Frame;

	float										m_CameraSpeed;
	glm::vec3									m_CameraVelocity;
	glm::ivec2									m_ClickPos;
};

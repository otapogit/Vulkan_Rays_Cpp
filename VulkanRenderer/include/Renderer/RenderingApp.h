#pragma once
#include "core_fpcamera.h"

class VulkanRenderApp {
public:
	VulkanRenderApp(int WindowWidth, int WindowHeight);
	~VulkanRenderApp();
	void initRt();

private:
	void CreateCamera(glm::vec3 pos);
	void CreateCamera(glm::vec3 pos, float FOV, float znear, float zfar);
	CameraFirstPerson* m_pCamera = NULL;
	float m_windowWidth, m_windowHeight;
};
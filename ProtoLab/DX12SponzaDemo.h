#pragma once

#include "DXSample.h"
#include "DX12/DX12.h"

class Scene;
class Camera;
class Renderer;

class DX12SponzaDemo : public DXSample
{
	using super = DXSample;

public:
	DX12SponzaDemo(uint32_t width, uint32_t height, std::wstring name);

	virtual void OnInit(GFX_HWND hwnd) override final;
	virtual void OnUpdate(DX::StepTimer const& timer) override final;
	virtual void OnRender() override final;
	virtual void OnFlip() override final;
	virtual void OnDestroy() override final;

#ifdef _XBOX_ONE
	virtual void OnSuspending() override final;
	virtual void OnResuming() override final;
#endif

	void DrawScene();

private:
	void CreateDevice();
	void CreateResources();
	void LoadAssets();
	void LoadCornellBox();

	DX12GraphicsManager*								m_GraphicsManager;

	std::shared_ptr<Renderer>							m_Renderer;
	std::shared_ptr<Scene>								m_Scene;
	std::shared_ptr<Camera>								m_Camera;
};

#pragma once

#include "../DX12/DX12.h"

class Scene;

class Renderer
{
public:
	Renderer(GFX_HWND hwnd, int32_t width, int32_t height);
	~Renderer();

	void Render(Scene* pScene);

	void ResolveToSwapChain();

	void Flip();

private:
	int32_t m_Width;
	int32_t m_Height;

	std::shared_ptr<DX12SwapChain> m_SwapChain;
};


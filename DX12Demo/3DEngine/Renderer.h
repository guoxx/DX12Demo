#pragma once

class Scene;

class Renderer
{
public:
	Renderer(GFX_HWND hwnd, int32_t width, int32_t height);
	~Renderer();

	void Render(Scene* pScene);

	void Flip();

private:
	int32_t m_Width;
	int32_t m_Height;
};


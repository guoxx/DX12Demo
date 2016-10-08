#pragma once

class Scene;

class Renderer
{
public:
	Renderer(int32_t width, int32_t height);
	~Renderer();

	void Render(Scene* pScene);

	void Flip();

private:
	int32_t m_Width;
	int32_t m_Height;
};


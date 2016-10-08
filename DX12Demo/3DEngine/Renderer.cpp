#include "pch.h"
#include "Renderer.h"


Renderer::Renderer(int32_t width, int32_t height)
	: m_Width{ width}
	, m_Height{ height }
{
}

Renderer::~Renderer()
{
}

void Renderer::Render(Scene* pScene)
{
}

void Renderer::Flip()
{
}

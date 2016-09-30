#pragma once

#include "Actor.h"

class Camera;
class DX11GraphicContext;
class DX11DepthStencilRenderTarget;

class ILight : public Actor
{
public:
	ILight();
	virtual ~ILight();
};


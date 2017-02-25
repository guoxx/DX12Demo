#pragma once

#include "../Actor.h"

class Camera;

class ILight : public Actor
{
public:
	ILight();
	virtual ~ILight();
};


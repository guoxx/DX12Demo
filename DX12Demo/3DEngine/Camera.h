#pragma once

#include "Actor.h"

class Camera : public Actor
{
public:
	Camera();
	~Camera();

	//void lookAt(DirectX::XMVECTOR eye, DirectX::XMVECTOR target, DirectX::XMVECTOR up);

	void SetViewParams(float fovy, float aspectRatio, float zNear, float zFar);

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetInvViewMatrix() const;

	DirectX::XMMATRIX GetProjectionMatrix() const;
	DirectX::XMMATRIX GetInvProjectionMatrix() const;

	DirectX::XMMATRIX GetViewProjectionMatrix() const;
	DirectX::XMMATRIX GetInvViewProjectionMatrix() const;

private:

	// column-major matrix data
	DirectX::XMFLOAT4X4 m_ProjectionMatrix;
};


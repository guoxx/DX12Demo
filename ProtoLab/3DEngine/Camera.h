#pragma once

#include "Actor.h"

class Camera : public Actor
{
public:
	Camera();
	~Camera();

	void LookAt(DirectX::XMVECTOR eye, DirectX::XMVECTOR target, DirectX::XMVECTOR up);

	void SetViewParams(float fovy, float aspectRatio, float zNear, float zFar);

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetInvViewMatrix() const;

	DirectX::XMMATRIX GetProjectionMatrix() const;
	DirectX::XMMATRIX GetInvProjectionMatrix() const;

	DirectX::XMMATRIX GetViewProjectionMatrix() const;
	DirectX::XMMATRIX GetInvViewProjectionMatrix() const;

	float GetNear() const { return m_Near; }
	float GetFar() const { return m_Far; }

	float GetFovY() const { return m_FovY; }

	float GetAspectRatio() const { return m_AspectRatio; }

private:
	// column-major matrix data
	DirectX::XMFLOAT4X4 m_ProjectionMatrix;
	float m_Near;
	float m_Far;
	float m_FovY;
	float m_AspectRatio;
};


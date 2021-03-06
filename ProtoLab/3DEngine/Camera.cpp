#include "pch.h"
#include "Camera.h"


Camera::Camera()
{
	DirectX::XMStoreFloat4x4(&m_ProjectionMatrix, DirectX::XMMatrixIdentity());
}

Camera::~Camera()
{
}

void Camera::LookAt(DirectX::XMVECTOR eye, DirectX::XMVECTOR target, DirectX::XMVECTOR up)
{
	DirectX::XMMATRIX mView = DirectX::XMMatrixLookAtRH(eye, target, up);
	DirectX::XMMATRIX mModel = DirectX::XMMatrixInverse(nullptr, mView);
	DirectX::XMStoreFloat4x4(&m_LocalMatrix, mModel);

	// TODO: better code
	// this will cause _localMatrix been updated again
	// and lose precision due to float arithmetic
	DirectX::XMVECTOR scaleVec;
	DirectX::XMVECTOR rotationQuat;
	DirectX::XMVECTOR translationVec;
	DirectX::XMMatrixDecompose(&scaleVec, &rotationQuat, &translationVec, mModel);

	double pitch, yaw, roll;
	EulerianToEulerAngle(rotationQuat, pitch, yaw, roll);
	m_RotationPitchYawRoll = DirectX::XMFLOAT4{ static_cast<float>(roll), static_cast<float>(pitch), static_cast<float>(yaw), 0.0f };
	DirectX::XMStoreFloat4(&m_Scale, scaleVec);
	DirectX::XMStoreFloat4(&m_Translation, translationVec);

	UpdateWorldMatrixDeferred();
}

void Camera::SetViewParams(float fovy, float aspectRatio, float zNear, float zFar)
{
	m_Near = zNear;
	m_Far = zFar;
	m_FovY = fovy;
	m_AspectRatio = aspectRatio;

	DirectX::XMMATRIX mProj = DirectX::XMMatrixPerspectiveFovRH(DirectX::XMConvertToRadians(fovy), aspectRatio, zNear, zFar);
	DirectX::XMStoreFloat4x4(&m_ProjectionMatrix, mProj);
}

DirectX::XMMATRIX Camera::GetViewMatrix() const
{
	DirectX::XMMATRIX mModel = GetWorldMatrix();
	DirectX::XMMATRIX mView = DirectX::XMMatrixInverse(nullptr, mModel);
	return mView;
}

DirectX::XMMATRIX Camera::GetInvViewMatrix() const
{
	return DirectX::XMMatrixInverse(nullptr, GetViewMatrix());
}

DirectX::XMMATRIX Camera::GetProjectionMatrix() const
{
	return DirectX::XMLoadFloat4x4(&m_ProjectionMatrix);
}

DirectX::XMMATRIX Camera::GetInvProjectionMatrix() const
{
	return DirectX::XMMatrixInverse(nullptr, GetProjectionMatrix());
}

DirectX::XMMATRIX Camera::GetViewProjectionMatrix() const
{
	DirectX::XMMATRIX viewMat = GetViewMatrix();
	DirectX::XMMATRIX projMat = GetProjectionMatrix();
	DirectX::XMMATRIX viewPorjMat = DirectX::XMMatrixMultiply(viewMat, projMat);
	return viewPorjMat;
}

DirectX::XMMATRIX Camera::GetInvViewProjectionMatrix() const
{
	return DirectX::XMMatrixInverse(nullptr, GetViewProjectionMatrix());
}
